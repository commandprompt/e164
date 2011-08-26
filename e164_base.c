/*
 * E.164 Telephone Number Type for PostgreSQL: base functions
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
#include "e164_base.h"

/*
 * The largest possible E164 number is 999_999_999_999_999, which is
 * equal to 0x3_8D7E_A4C6_7FFF.  Thus the number mask value (50 bits.)
 *
 * The largest valid E164 number is currently 998_999_999_999_999,
 * according to this document:
 *
 * http://www.itu.int/dms_pub/itu-t/opb/sp/T-SP-E.164D-2009-PDF-E.pdf
 *
 * The largest possible Country Code is 999, and the closest mask to
 * covert that is 0x3FF.
 */
#define E164_NUMBER_MASK          UINT64CONST(0x0003FFFFFFFFFFFF)
#define E164_CC_MASK_OFFSET       50
#define E164_CACHED_CC_MASK       (UINT64CONST(0x3FF) << E164_CC_MASK_OFFSET)
#define E164_COMPARISON_MASK      (E164_NUMBER_MASK | E164_CACHED_CC_MASK)

/*
 * The following mask is used in sanity checks.  Update to reflect any
 * changes in the above masks.
 */
#define E164_USED_BITS_MASK       E164_COMPARISON_MASK

#define E164_MAX_NUMBER_VALUE     UINT64CONST(999999999999999)
#define E164_MAX_CC_VALUE         999


/*
 * Function prototypes
 */
static inline void e164SanityCheck(E164 aNumber);

static inline E164ParseResult parseFormattedNumber (const char * aString,
                                                    uint64 * aNumber,
                                                    E164CountryCode * aCountryCode,
                                                    E164Type * aType,
                                                    int * numberOfDigits);
static inline bool hasValidLengthForE164Type (int numberLength,
                                              int countryCodeLength,
                                              E164Type aType);

static inline E164CountryCode e164CountryCodeOf (E164 theNumber);
static inline E164CountryCode e164CountryCodeOf_no_check (E164 theNumber);

static inline void insertChar (char * aString, int stringLength, char sym, int position);

static inline int rawStringFromE164_no_check (char * aString, int stringLength,
                                              E164 aNumber);
static inline int stringFromE164_no_check (char * aString, int stringLength,
                                           E164 aNumber);

static inline bool isValidE164PrefixChar (char aChar);
static inline bool stringHasValidE164Prefix (const char * aString);

static inline bool isEndOfString (const char * theChar);
static inline bool eachCharIsDigit (const char * aString);

static inline bool e164CountryCodeIsInRange (E164CountryCode theCountryCode);
static inline void checkE164CountryCodeForRangeError (E164CountryCode theCountryCode);
static inline int countryCodeLengthOf (E164CountryCode countryCode);

static inline bool isUnassignedE164Type (E164Type aType);
static inline bool isValidE164Type (E164Type aType);
static inline bool isInvalidE164Type (E164Type aType);

static inline E164Type e164TypeForCountryCode (E164CountryCode theCountryCode);
static inline bool isValidE164CountryCodeType (E164CountryCode theCountryCode);
static inline bool isInvalidE164CountryCodeType (E164CountryCode theCountryCode);


/*
 * Function definitions
 */

static inline
void e164SanityCheck(E164 aNumber)
{
    E164CountryCode theCountryCode;
#ifdef USE_ASSERT_CHECKING
    char numberString[E164MaximumStringLength + 1];
    E164 parsedNumber;
    E164CountryCode parsedCountryCode;
#endif

    if (0 != (aNumber & ~E164_USED_BITS_MASK))
        elog(ERROR, "unused high bits tainted in an E164 value: " UINT64_FORMAT,
             aNumber);

    if (E164_MAX_NUMBER_VALUE < (aNumber & E164_NUMBER_MASK))
        elog(ERROR, "the E164 number exceeds maximum possible value: " UINT64_FORMAT,
             aNumber);

    theCountryCode = e164CountryCodeOf_no_check(aNumber);
    if (!e164CountryCodeIsInRange(theCountryCode))
        elog(ERROR, "the country code in an E164 value exceeds allowed range: %d (" UINT64_FORMAT ")",
             theCountryCode, aNumber);

    if (isInvalidE164CountryCodeType(theCountryCode))
        elog(ERROR, "the country code in an E164 value is invalid: %d (" UINT64_FORMAT ")",
             theCountryCode, aNumber);

#ifdef USE_ASSERT_CHECKING
    rawStringFromE164_no_check(numberString, E164MaximumStringLength + 1,
                               aNumber);

    Assert(E164ParseOK == e164FromString(&parsedNumber, numberString,
                                         &parsedCountryCode));
    Assert(parsedNumber == aNumber);
    Assert(parsedCountryCode == theCountryCode);
#endif
}


int64 e164Comparison (E164 firstNumber, E164 secondNumber)
{
    e164SanityCheck(firstNumber);
    e164SanityCheck(secondNumber);

    return ((int64)(firstNumber & E164_COMPARISON_MASK) -
            (int64)(secondNumber & E164_COMPARISON_MASK));
}

static inline
E164CountryCode e164CountryCodeOf (E164 theNumber)
{
    e164SanityCheck(theNumber);
    return e164CountryCodeOf_no_check(theNumber);
}

static inline
E164CountryCode e164CountryCodeOf_no_check (E164 theNumber)
{
    return (theNumber & E164_CACHED_CC_MASK) >> E164_CC_MASK_OFFSET;
}

/*
 * countryCodeStringFromE164 assigns the country code for aNumber to aString, returning
 * the number of characters written (in this case, the number of digits in the
 * country code).
 */
int countryCodeStringFromE164 (char * aString, int stringLength, E164 aNumber)
{
    e164SanityCheck(aNumber);
    return snprintf(aString, stringLength,
                    "%d", e164CountryCodeOf(aNumber));
}

static inline
void insertChar (char * aString, int stringLength, char sym, int position)
{
    Assert(stringLength >= 0);
    Assert(position >= 0);
    Assert(position < stringLength);

    memmove(aString + position + 1, aString + position,
            stringLength - position + 1);
    aString[position] = sym;
}

/*
 * stringFromE164 assigns the string representation of aNumber to aString
 */
int stringFromE164 (char * aString, int stringLength, E164 aNumber)
{
    e164SanityCheck(aNumber);
    return stringFromE164_no_check(aString, stringLength, aNumber);
}

int rawStringFromE164 (char * aString, int stringLength, E164 aNumber)
{
    e164SanityCheck(aNumber);
    return rawStringFromE164_no_check(aString, stringLength, aNumber);
}

static inline
int rawStringFromE164_no_check (char * aString, int stringLength, E164 aNumber)
{
    return snprintf(aString, stringLength,
                    "+" UINT64_FORMAT, (aNumber & E164_NUMBER_MASK));
}

/*
 * Insert spaces into the rest of the phone number digits, to
 * group them in packs of 4 from the tail, wherever possible,
 * otherwise try to group in packs of 3.
 *
 * The resulting tail looks like this in the most general case:
 *
 * +CC (AC) 12 345 6789
 */
static const char * format_patterns[15] = {
    "", /* padding, never used */
    "x",
    "xx",
    "xxx",
    "xxxx",
    "xx xxx",
    "xxx xxx",
    "xxx xxxx",
    "xxxx xxxx",
    "xx xxx xxxx",
    "xxx xxx xxxx",
    "xxx xxxx xxxx",
    "xxxx xxxx xxxx",
    "xx xxx xxxx xxxx",
    "xxx xxx xxxx xxxx"
};

static inline
int stringFromE164_no_check (char * aString, int stringLength, E164 aNumber)
{
    char buffer[E164MaximumStringLength + 1];
    char temp[E164MaximumStringLength + 1];
    int n = rawStringFromE164_no_check(buffer, sizeof(buffer), aNumber);

    int len;
    char * pos;
    char * tpos;
    const char * pattern;

    E164CountryCode countryCode = e164CountryCodeOf_no_check(aNumber);

    /* Country Code length */
    int ccl = countryCodeLengthOf(countryCode);

    /* Area Code length */
    int acl = 0; /* TODO: determine from country code and NSN */

    /* Copy the prefix and country code to temp buffer. */
    len = E164PrefixStringLength + ccl;
    memcpy((tpos = temp), (pos = buffer), len);
    pos += len;
    tpos += len;
    *(tpos++) = ' ';

    /*
     * Check if there's enough digits for the area code and the rest
     * of the number.
     */
    if (len + acl >= n)
        elog(ERROR, "no digits follow the area code in an E164 number: "
             UINT64_FORMAT, (aNumber & E164_NUMBER_MASK));

    if (acl > 0) {
        *(tpos++) = '(';
        memcpy(tpos, pos, acl);
        pos += acl;
        tpos += acl;
        *(tpos++) = ')';
        *(tpos++) = ' ';
    }

    /* Format the rest according to a pre-defined format pattern. */
    len = (buffer + n) - pos;
    if (len >= 15)
        elog(ERROR, "trailing digits found in an E164 number: " UINT64_FORMAT,
             (aNumber & E164_NUMBER_MASK));

    for (pattern = format_patterns[len]; ; ++pattern) {
        if (*pattern == 'x')
            *(tpos++) = *(pos++);
        else
            *(tpos++) = *pattern;

        if (!*pattern) /* if we've just copied the the null byte */
            break;
    }

    strncpy(aString, temp, stringLength);
    /*
     * Return actual length required for formatting the number, so the
     * caller have a chance to adjust the passed buffer size.
     */
    return tpos - temp;
}

/*
 * isValidE164PrefixChar returns true if theChar is a valid E164 prefix character
 * and false otherwise.
 */
static inline
bool isValidE164PrefixChar (char aChar)
{
    return (E164_PREFIX == aChar);
}

/*
 * stringHasValidE164Prefix returns true if aString has a valid E164 prefix
 * and false otherwise.
 */
static inline
bool stringHasValidE164Prefix (const char * aString)
{
    return isValidE164PrefixChar(*aString);
}

/*
 * e164FromString takes a string as its second argument and
 * assigns to its first argument the E164 value represented
 * by the string. If the assignment is successful, e164FromString
 * returns true, and false otherwise.
 */
E164ParseResult e164FromString (E164 * aNumber, const char * aString,
                                E164CountryCode * aCode)
{
    E164ParseResult result;
    const int stringLength = strlen(aString);
    E164CountryCode aCountryCode = 0;
    int numberOfCountryCodeDigits;
    int totalNumberOfDigits;
    E164Type theType = E164Invalid;
    /*
     * Make sure string doesn't exceed maximum length
     */
    if (E164MinimumStringLength > stringLength)
        return E164ParseErrorStringTooShort;
    /*
     * Check for a valid E164 prefix
     */
    if (!stringHasValidE164Prefix(aString))
        return E164ParseErrorInvalidPrefix;

    /* Advance past prefix */
    aString += E164PrefixStringLength;

    result = parseFormattedNumber(aString, aNumber, &aCountryCode,
                                  &theType, &totalNumberOfDigits);
    if (E164ParseOK != result)
        return result;

    /*
     * Assign country code.
     * If it's invalid, it'll be used in the error message.
     */
    *aCode = aCountryCode;
    if (isInvalidE164Type(theType))
        return E164ParseErrorInvalidType;
    else if (isUnassignedE164Type(theType))
        return E164ParseErrorUnassignedType;

    numberOfCountryCodeDigits = countryCodeLengthOf(aCountryCode);
    /*
     * Need some digits for the subscriber number
     */
    if (totalNumberOfDigits <= numberOfCountryCodeDigits)
        return E164ParseErrorNoSubscriberNumberDigits;

    /*
     * Check number against E164Type
     * This tests against absolute (and unrealistic) minimums.
     * See comment regarding minimum Subscriber Number lengths
     */
    if (!hasValidLengthForE164Type(totalNumberOfDigits,
                                   numberOfCountryCodeDigits,
                                   theType))
        return E164ParseErrorTypeLengthMismatch;

    *aNumber |= (((uint64) aCountryCode) << E164_CC_MASK_OFFSET);

    return E164ParseOK;
}

/*
 * parseFormattedNumber tries to parse the phone number string which
 * may contain spaces and/or parens of the following general format
 * (no leading E164 prefix symbol '+'):
 *
 * 1 (234) 567 8901
 *
 * The string is treated as if there were no non-digit symbols, and a
 * few simple rules are enforced on the placement of non-digits:
 *
 * * Paren symbols, if present, must be balanced.
 *
 * * No leading/trailing parens are allowed.
 *
 * The function also detects the country code of a parsed number as
 * well as its type.  Since no country code may be a prefix of another
 * (longer) country code, the first valid value is thought to be the
 * country code of a parsed number.
 */
/* TODO: consider dashes also */
static inline
E164ParseResult parseFormattedNumber (const char * aString,
                                      uint64 * aNumber,
                                      E164CountryCode * aCountryCode,
                                      E164Type * aType,
                                      int * numberOfDigits)
{
    uint64 theNumber = 0;
    E164CountryCode theCountryCode = 0;
    E164Type theType = E164Invalid;
    const char * p = aString;
    char prevChar = 0;
    bool lparen = false;
    bool rparen = false;
    int digitCount = 0;
    for (; *p; prevChar = *(p++))
    {
        if (isdigit(*p))
        {
            if (++digitCount > E164MaximumNumberOfDigits)
                return E164ParseErrorStringTooLong;
            theNumber *= 10;
            theNumber += (*p - '0');
            if (isInvalidE164Type(theType))
            {
                theCountryCode = theNumber;
                theType = e164TypeForCountryCode(theCountryCode);
            }
        }
        else if (*p == '(')
        {
            /* Forbid second left paren or leading paren */
            if (lparen || !prevChar)
                return E164ParseErrorBadFormat;
            lparen = true;
        }
        else if (*p == ')')
        {
            /* Check parens balance, forbid empty parens */
            if (!lparen || rparen || prevChar == '(')
                return E164ParseErrorBadFormat;
            rparen = true;
        }
        else if (!isspace(*p))
            return E164ParseErrorBadFormat;
    }
    /* Forbid trailing space or paren */
    if (!isdigit(prevChar))
        return E164ParseErrorBadFormat;

    *aNumber = theNumber;
    *aCountryCode = theCountryCode;
    *aType = theType;
    *numberOfDigits = digitCount;
    return E164ParseOK;
}

/*
 * hasValidLengthForE164Type returns true if the number of digits in
 * the number is consistent with its E164Type and E164CountryCode
 */
static inline
bool hasValidLengthForE164Type (int numberLength,
                                int countryCodeLength,
                                E164Type aType)
{
    int subscriberNumberLength = (numberLength - countryCodeLength);
    if (0 > subscriberNumberLength)
        elog(ERROR, "numberLength and countryCodeLength values are invalid: %d vs. %d", numberLength, countryCodeLength);

    if (0 == subscriberNumberLength)
        return false;

    switch (aType)
    {
        case E164GeographicArea:
            return (E164GeographicAreaMinimumSubscriberNumberLength <= subscriberNumberLength);

        case E164GlobalService:
            return (E164GlobalServiceMinimumSubscriberNumberLength <= subscriberNumberLength);

        case E164Network:
            return (E164NetworkMinimumSubscriberNumberLength <= subscriberNumberLength);

        case E164GroupOfCountries:
            return (E164GroupOfCountriesMinimumSubscriberNumberLength <= subscriberNumberLength);

        default:
            ; /* fall through to elog() */
    }

    elog(ERROR, "E164Type value is invalid: %d", aType);
    return false; /* keep compiler quiet */
}

/*
 * isEndOfString determines if the argument is the end of string character
 */
static inline
bool isEndOfString (const char * theChar)
{
    return ('\0' == *theChar);
}

/*
 * eachCharIsDigit returns true if every character of aString is a digit,
 * and false otherwise.
 */
static inline
bool eachCharIsDigit(const char * aString)
{
    const char * theChar = aString;
    bool isAllDigits = true;
    while (!isEndOfString(theChar))
    {
        if (!isdigit(*theChar))
        {
            isAllDigits = false;
            break;
        }
        theChar++;
    }
    return isAllDigits;
}

/*
 * e164CountryCodeIsInRange returns true if the E164CountryCode argument is
 * within the proper range for E164CountryCodes and false otherwise.
 */
static inline
bool e164CountryCodeIsInRange (E164CountryCode theCountryCode)
{
    return ((0 <= theCountryCode) && (E164_MAX_CC_VALUE >= theCountryCode));
}

/*
 * checkE164CountryCodeForRangeError raises an error if the E164CountryCode
 * argument is out of range.
 */
static inline
void checkE164CountryCodeForRangeError (E164CountryCode theCountryCode)
{
    if (!e164CountryCodeIsInRange(theCountryCode))
        elog(ERROR, "E164CountryCode value is invalid: %d", theCountryCode);
}

static inline
int countryCodeLengthOf (E164CountryCode countryCode)
{
    checkE164CountryCodeForRangeError(countryCode);
    return (countryCode < 10) ? 1 : ((countryCode < 100) ? 2 : 3);
}

/*
 * isUnassignedE164Type returns true if aType is unassigned or false otherwise.
 */
static inline
bool isUnassignedE164Type (E164Type aType)
{
    return ((E164SpareWithoutNote == aType) ||
            (E164SpareWithNote == aType) ||
            (E164Reserved == aType));
}

static inline
bool isValidE164Type (E164Type aType)
{
    return (E164Invalid != aType);
}

/*
 * isInvalidE164Type returns true if aType is invalid or false otherwise.
 */
static inline
bool isInvalidE164Type (E164Type aType)
{
    return !isValidE164Type(aType);
}

static inline
E164Type e164TypeForCountryCode (E164CountryCode theCountryCode)
{
    checkE164CountryCodeForRangeError(theCountryCode);
    return e164TypeFor[theCountryCode];
}

static inline
bool isValidE164CountryCodeType (E164CountryCode theCountryCode)
{
    return isValidE164Type(e164TypeForCountryCode(theCountryCode));
}

static inline
bool isInvalidE164CountryCodeType (E164CountryCode theCountryCode)
{
    return !isValidE164CountryCodeType(theCountryCode);
}
