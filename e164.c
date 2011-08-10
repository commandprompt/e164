/*
 * E.164 Telephone Number Type for PostgreSQL: PG type interface
 *
 * Copyright (c) 2007-2011, Michael Glaesemann
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
#include "postgres.h"
#include "libpq/pqformat.h"
#include "access/hash.h"
#include "e164_base.h"

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

/*
 * makeText function taken from AndrewSN's ip4r
 */
static text *
makeText(int stringLength)
{
    text * textString = (text *) palloc0(stringLength + VARHDRSZ);
    SET_VARSIZE(textString, stringLength + VARHDRSZ);
    return textString;
}

static void
handleE164ParseError(E164ParseResult error, const char * string,
                     E164CountryCode countryCode)
{
    switch (error) {
        case E164ParseErrorBadFormat:
            ereport(ERROR,
                    (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                     errmsg("invalid E164 number format: \"%s\"", string),
                     errhint("E164 numbers begin with a \"+\" followed by digits.")));
            break;
        case E164ParseErrorInvalidPrefix:
            ereport(ERROR,
                    (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                     errmsg("invalid E164 prefix: \"%s\"", string),
                     errhint("E164 numbers must begin with \"%c\".", E164_PREFIX)));
            break;
        case E164ParseErrorStringTooLong:
            ereport(ERROR,
                    (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                     errmsg("string too long: \"%s\"", string),
                     errhint("E164 values must have at most %d digits.", E164MaximumNumberOfDigits)));
            break;
        case E164ParseErrorStringTooShort:
            ereport(ERROR,
                    (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                     errmsg("string too short \"%s\"", string),
                     errhint("E164 numbers must have at least %d digits.", E164MinimumNumberOfDigits)));
            break;
        case E164ParseErrorInvalidType:
            ereport(ERROR,
                    (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                     errmsg("invalid E164 country code for E164 number \"%s\": %d", string, countryCode)));
            break;
        case E164ParseErrorNoSubscriberNumberDigits:
            ereport(ERROR,
                    (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                     errmsg("no subscriber number digits in E164 number \"%s\"", string)));
            break;
        case E164ParseErrorUnassignedType:
            ereport(ERROR,
                    (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                     errmsg("unassigned country code for E164 number \"%s\": %d", string, countryCode)));
            break;
        case E164ParseErrorTypeLengthMismatch:
            ereport(ERROR,
                    (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                     errmsg("inconsistent length and country code for E164 number \"%s\" (country code: %d)", string, countryCode)));
            break;
        default:
            AssertArg(false);
    }
}

/*
 * PostgreSQL Interface functions
 */
#define DatumGetE164P(X) DatumGetInt64(X)
#define E164PGetDatum(X) Int64GetDatum(X)

#define PG_GETARG_E164(X) PG_GETARG_INT64((int64) X)
#define PG_RETURN_E164(X) PG_RETURN_INT64((int64) X)

Datum e164_in(PG_FUNCTION_ARGS);
Datum e164_out(PG_FUNCTION_ARGS);

Datum e164_hash(PG_FUNCTION_ARGS);
Datum e164_send(PG_FUNCTION_ARGS);
Datum e164_recv(PG_FUNCTION_ARGS);

Datum e164_lt(PG_FUNCTION_ARGS);
Datum e164_le(PG_FUNCTION_ARGS);
Datum e164_eq(PG_FUNCTION_ARGS);
Datum e164_ge(PG_FUNCTION_ARGS);
Datum e164_gt(PG_FUNCTION_ARGS);
Datum e164_ne(PG_FUNCTION_ARGS);

Datum e164_cmp(PG_FUNCTION_ARGS);

Datum e164_cast_to_text(PG_FUNCTION_ARGS);

Datum country_code(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(e164_in);
Datum
e164_in(PG_FUNCTION_ARGS)
{
    const char * theString = PG_GETARG_CSTRING(0);
    E164 theNumber;
    E164CountryCode theCode;
    E164ParseResult parseResult = e164FromString(&theNumber, theString, &theCode);
    if (E164NoParseError == parseResult)
    {
        PG_RETURN_E164(theNumber);
    }
    else
        handleE164ParseError(parseResult, theString, theCode);

    return 0;	/* keep compiler quiet */
}

PG_FUNCTION_INFO_V1(e164_out);
Datum
e164_out(PG_FUNCTION_ARGS)
{
    E164	theNumber = PG_GETARG_E164(0);
    char * theString = palloc(E164MaximumStringLength + 1);
    (void) stringFromE164(theString, E164MaximumStringLength + 1, theNumber);
    PG_RETURN_CSTRING(theString);
}

PG_FUNCTION_INFO_V1(e164_recv);
Datum
e164_recv(PG_FUNCTION_ARGS)
{
    StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
    PG_RETURN_E164((E164) pq_getmsgint(buf, sizeof(E164)));
}

PG_FUNCTION_INFO_V1(e164_send);
Datum
e164_send(PG_FUNCTION_ARGS)
{
    E164 arg1 = PG_GETARG_E164(0);
    StringInfoData buf;

    pq_begintypsend(&buf);
    pq_sendint(&buf, arg1, sizeof(E164));
    PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

PG_FUNCTION_INFO_V1(e164_cast_to_text);
Datum
e164_cast_to_text(PG_FUNCTION_ARGS)
{
    E164	theNumber = PG_GETARG_E164(0);
    char	buffer[E164MaximumStringLength + 1];
    int		stringLength;
    text *	textString;

    stringLength = stringFromE164(buffer, E164MaximumStringLength + 1,
                                  theNumber);

    textString = makeText(stringLength);
    memcpy(VARDATA(textString), buffer, stringLength);

    PG_RETURN_TEXT_P(textString);
}

PG_FUNCTION_INFO_V1(country_code);
Datum
country_code(PG_FUNCTION_ARGS)
{
    E164	aNumber = PG_GETARG_E164(0);
    char	buffer[E164MaximumCountryCodeLength + 1];
    int		stringLength;
    text *	textString;

    stringLength = countryCodeStringFromE164(buffer,
                                             E164MaximumCountryCodeLength + 1,
                                             aNumber);

    textString = makeText(stringLength);
    memcpy(VARDATA(textString), buffer, stringLength);

    PG_RETURN_TEXT_P(textString);
}

PG_FUNCTION_INFO_V1(e164_lt);
Datum
e164_lt(PG_FUNCTION_ARGS)
{
    PG_RETURN_BOOL(0 > e164Comparison(PG_GETARG_E164(0),
                                      PG_GETARG_E164(1)));
}

PG_FUNCTION_INFO_V1(e164_le);
Datum
e164_le(PG_FUNCTION_ARGS)
{
    PG_RETURN_BOOL(0 >= e164Comparison(PG_GETARG_E164(0),
                                       PG_GETARG_E164(1)));
}

PG_FUNCTION_INFO_V1(e164_eq);
Datum
e164_eq(PG_FUNCTION_ARGS)
{
    PG_RETURN_BOOL(0 == e164Comparison(PG_GETARG_E164(0),
                                       PG_GETARG_E164(1)));
}

PG_FUNCTION_INFO_V1(e164_ge);
Datum
e164_ge(PG_FUNCTION_ARGS)
{
    PG_RETURN_BOOL(0 <= e164Comparison(PG_GETARG_E164(0),
                                       PG_GETARG_E164(1)));
}

PG_FUNCTION_INFO_V1(e164_gt);
Datum
e164_gt(PG_FUNCTION_ARGS)
{
    PG_RETURN_BOOL(0 < e164Comparison(PG_GETARG_E164(0),
                                      PG_GETARG_E164(1)));
}

PG_FUNCTION_INFO_V1(e164_ne);
Datum
e164_ne(PG_FUNCTION_ARGS)
{
    PG_RETURN_BOOL(0 != e164Comparison(PG_GETARG_E164(0),
                                       PG_GETARG_E164(1)));
}

PG_FUNCTION_INFO_V1(e164_cmp);
Datum
e164_cmp(PG_FUNCTION_ARGS)
{
    int64 comparison = e164Comparison(PG_GETARG_E164(0),
                                      PG_GETARG_E164(1));
    int result = ((comparison < 0) ?
                  -1 :
                  ((comparison > 0) ?
                   1 :
                   0));
    PG_RETURN_INT32(result);
}

PG_FUNCTION_INFO_V1(e164_hash);
Datum
e164_hash(PG_FUNCTION_ARGS)
{
    E164 arg1 = PG_GETARG_E164(0);
    return hash_any((unsigned char *)&arg1, sizeof(E164));
}
