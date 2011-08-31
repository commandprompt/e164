/*
 * E.164 Telephone Number Type for PostgreSQL: Area code functions
 *
 * Portions Copyright (c) 2007-2011, Michael Glaesemann
 * Portions Copyright (c) 2011, CommandPrompt, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <search.h>
#include "postgres.h"
#include "utils/guc.h"
#include "e164_area_codes.h"

static bool parseAreaCodesInfo(char * aFormat, E164AreaCodesInfo * codesInfo,
                               E164AreaCode * exceptions);

static bool parseAreaCodeExceptions(char * aString, const char * aFormat,
                                    E164AreaCodesFormat * theFormat,
                                    char ** badStopChar);

static bool e164TypeSupportsAreaCode(E164Type aType);
static bool e164CountryCodeSupportsAreaCode(E164CountryCode aCountryCode);

static int compareInts(const void * a, const void * b);

static E164AreaCodesInfo * currentCodesInfo = NULL;

int
e164AreaCodeLengthOf(E164 aNumber, E164CountryCode aCountryCode,
                     int countryCodeLength)
{
    E164AreaCodesFormat * format;
    size_t numberOfFormats;
    char buffer[E164MaximumNumberOfDigits + 1];
    int i;

    if (!e164CountryCodeSupportsAreaCode(aCountryCode) || !currentCodesInfo)
        return 0;

    numberOfFormats = currentCodesInfo->numberOfFormats;
    format = lfind(&aCountryCode, currentCodesInfo->formats, &numberOfFormats,
                   sizeof(E164AreaCodesFormat), compareInts);
    if (!format)
        return 0;

    snprintf(buffer, sizeof(buffer), UINT64_FORMAT, aNumber);

    for (i = 0; i < format->numberOfExceptions; ++i)
    {
        /* TODO: suboptimal */
        char temp[E164MaximumNumberOfDigits + 1];
        int len = snprintf(temp, sizeof(temp), "%d", format->exceptions[i]);
        if (strncmp(buffer + countryCodeLength, temp, len) == 0)
            return len;
    }

    return format->defaultAreaCodeLength;
}

void
e164SetAreaCodesInfo(E164AreaCodesInfo * codesInfo)
{
    currentCodesInfo = codesInfo;
}

/*
 * e164.area_codes_format = '+1:xxx;+61:x,11,12,13;+380:xx'
 *
 * The area_codes_format GUC option specifies the per-CountryCode area
 * code patterns.
 *
 * The format string consits of zero or more country code specifiers
 * '+ccc' followed by a colon symbol and the default area code pattern
 * (one or more 'x' symbols.)  The default area code pattern may be
 * optionally followed by a coma and a coma-separated list of area
 * code number exceptions.
 *
 * E.g., for Australia (country code +61,) the default area code
 * pattern is 'x', which means area codes are one digit long by
 * default, with exceptions of area codes '11', '12, and '13' (which
 * are two digit long obviously.)
 *
 * The country code specifiers should be separated by semicolon
 * symbols, the trailing semicolon is optional.
 */
bool
parseE164AreaCodesFormat(char * aFormat, E164AreaCodesInfo ** theCodesInfo)
{
    E164AreaCodesInfo * codesInfo;
    size_t numberOfFormats = 0;
    size_t mainAllocSize = sizeof(E164AreaCodesInfo);
    size_t addedAllocSize = 0;
    const char * p;

    /* Determine required allocation size from the number of stop chars. */
    for (p = aFormat; *p; ++p)
    {
        if (*p == ':')
        {
            ++numberOfFormats;
            mainAllocSize += sizeof(E164AreaCodesFormat);
        }
        else if (*p == ',')
            addedAllocSize += sizeof(int);
    }
    codesInfo = (E164AreaCodesInfo *) malloc(mainAllocSize + addedAllocSize);
    if (!codesInfo)
    {
        GUC_check_errdetail("out of memory");
        return false;
    }
    codesInfo->numberOfFormats = numberOfFormats;

    if (!parseAreaCodesInfo(aFormat, codesInfo,
                            (int *)(((char *) codesInfo) + mainAllocSize)))
    {
        free(codesInfo);
        return false;
    }

    /* Shortcut for the empty option */
    if (codesInfo->numberOfFormats == 0)
    {
        free(codesInfo);
        codesInfo = NULL;
    }

    *theCodesInfo = codesInfo;
    return true;
}

static bool
parseAreaCodesInfo(char * aFormat, E164AreaCodesInfo * codesInfo,
                   E164AreaCode * exceptions)
{
    size_t numberOfFormats = 0;
    char * token0 = aFormat;
    char * savepoint = NULL;
    char * stopChar = NULL;
    for (;;)
    {
        E164CountryCode countryCode;
        E164Type type;
        E164AreaCodesFormat * currentFormat;
        char * token = strtok_r(token0, ";", &savepoint);
        if (!token)
            break;
        token0 = NULL;

        if (!stringHasValidE164Prefix(token))
        {
            GUC_check_errdetail("unexpected prefix string");
            GUC_check_errhint("\"%s\" expected at character %td",
                              E164_PREFIX_STRING, token - aFormat + 1);
            goto fail;
        }

        token += E164PrefixStringLength;
        countryCode = strtol(token, &stopChar, 10);
        if (*stopChar != ':')
            goto bad_stop_char;

        if (!e164CountryCodeIsInRange(countryCode))
        {
            GUC_check_errdetail("unexpected country code number: %d at character %td", countryCode, token - aFormat + 1);
            goto fail;
        }

        type = e164TypeForCountryCode(countryCode);
        if (isInvalidE164Type(type))
        {
            GUC_check_errdetail("invalid country code: %d at character %td", countryCode, token - aFormat + 1);
            goto fail;
        }

        if (!e164TypeSupportsAreaCode(type))
        {
            GUC_check_errdetail("unsupported country code: %d at character %td", countryCode, token - aFormat + 1);
            goto fail;
        }

        /*
         * The trick here is to peek into the first field of the
         * E164AreaCodesFormat struct, the countryCode.
         */
        if (lfind(&countryCode, codesInfo->formats, &numberOfFormats,
                  sizeof(E164AreaCodesFormat), compareInts))
        {
            GUC_check_errdetail("duplicate country code: %d at character %td",
                                countryCode, token - aFormat + 1);
            goto fail;
        }
        currentFormat = codesInfo->formats + numberOfFormats++;

        currentFormat->countryCode = countryCode;

        /* Determine default area code length from the number of 'x' chars */
        token = ++stopChar;
        if (*stopChar != 'x')
        {
            GUC_check_errhint("one or more 'x' are expected");
            goto bad_stop_char;
        }
        while (*(++stopChar) == 'x')
            ;

        if (*stopChar && *stopChar != ',')
        {
            GUC_check_errhint("either ',' or ';' or end of string is expected");
            goto bad_stop_char;
        }

        currentFormat->defaultAreaCodeLength = stopChar - token;

        token = stopChar;
        stopChar = NULL;

        currentFormat->exceptions = exceptions;
        if (!parseAreaCodeExceptions(token, aFormat, currentFormat, &stopChar))
            goto bad_stop_char;

        exceptions += currentFormat->numberOfExceptions;
    }

    return true;

bad_stop_char:
    if (stopChar)
        GUC_check_errdetail("unexpected symbol: '%c' at character %td",
                            *stopChar, stopChar - aFormat + 1);
fail:
    return false;
}

static bool
parseAreaCodeExceptions(char * aString, const char * aFormat,
                        E164AreaCodesFormat * theFormat,
                        char ** badStopChar)
{
    size_t numberOfExceptions = 0;
    E164AreaCode * exceptions = theFormat->exceptions;
    char * savepoint = NULL;
    for (;;)
    {
        E164AreaCode areaCode;
        char * stopChar;
        char * token = strtok_r(aString, ",", &savepoint);
        if (!token)
            break;
        aString = NULL;

        /* TODO: check for overflow */
        areaCode = strtol(token, &stopChar, 10);
        if (*stopChar)
        {
            *badStopChar = stopChar;
            return false;
        }

        if (lfind(&areaCode, exceptions, &numberOfExceptions,
                  sizeof(int), compareInts))
        {
            GUC_check_errdetail("duplicate area code: %d at character %td",
                                areaCode, token - aFormat + 1);
            return false;
        }
        exceptions[numberOfExceptions++] = areaCode;
    }

    theFormat->numberOfExceptions = numberOfExceptions;
    return true;
}

static bool
e164TypeSupportsAreaCode(E164Type aType)
{
    return aType == E164GeographicArea || aType == E164GroupOfCountries;
}

static bool
e164CountryCodeSupportsAreaCode(E164CountryCode aCountryCode)
{
    return e164TypeSupportsAreaCode(e164TypeForCountryCode(aCountryCode));
}

static int
compareInts(const void * a, const void * b)
{
    return *((const int *) a) - *((const int *) b);
}
