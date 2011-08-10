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
 * Function prototypes
 */
static int parseE164String (const char * aNumberString,
                            const char ** theDigits,
                            E164CountryCode * aCode);

static inline bool hasValidLengthForE164Type (int numberLength,
                                              int countryCodeLength,
                                              E164Type aType);
static inline void initializeE164WithCountryCode (E164 * aNumber,
                                                  E164CountryCode aCountryCode);

static inline E164CountryCode e164CountryCodeOf (E164 theNumber);

static inline bool isValidE164PrefixChar (char aChar);
static inline bool stringHasValidE164Prefix (const char * aString);

static inline bool isEndOfString (const char * theChar);
static inline bool eachCharIsDigit (const char * aString);

static inline bool e164CountryCodeIsInRange (E164CountryCode theCountryCode);
static inline void checkE164CountryCodeForRangeError (E164CountryCode theCountryCode);

static inline bool isUnassignedE164Type (E164Type aType);
static inline bool isInvalidE164Type (E164Type aType);
static inline E164Type e164TypeForCountryCode (E164CountryCode theCountryCode);

/*
 * Function definitions
 */
int64 e164Comparison (E164 firstNumber, E164 secondNumber)
{
    return ((int64)(firstNumber & E164_COMPARISON_MASK) -
            (int64)(secondNumber & E164_COMPARISON_MASK));
}

bool e164IsEqualTo (E164 firstNumber, E164 secondNumber)
{
    return (0 == e164Comparison(firstNumber, secondNumber));
}

bool e164IsNotEqualTo (E164 firstNumber, E164 secondNumber)
{
    return (0 != e164Comparison(firstNumber, secondNumber));
}

bool e164IsLessThan (E164 firstNumber, E164 secondNumber)
{
    return (0 > e164Comparison(firstNumber, secondNumber));
}

bool e164IsLessThanOrEqualTo (E164 firstNumber, E164 secondNumber)
{
    return (0 >= e164Comparison(firstNumber, secondNumber));
}

bool e164IsGreaterThanOrEqualTo (E164 firstNumber, E164 secondNumber)
{
    return (0 <= e164Comparison(firstNumber, secondNumber));
}

bool e164IsGreaterThan (E164 firstNumber, E164 secondNumber)
{
    return (0 < e164Comparison(firstNumber, secondNumber));
}

static inline
E164CountryCode e164CountryCodeOf (E164 theNumber)
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
    return snprintf(aString, stringLength,
                    "%d", e164CountryCodeOf(aNumber));
}

/*
 * stringFromE164 assigns the string representation of aNumber to aString
 */
int stringFromE164 (char * aString, E164 aNumber, int stringLength)
{
    return snprintf(aString, stringLength,
                    "+" UINT64_FORMAT, (aNumber & E164_NUMBER_MASK));
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
    const char * theDigits;
    E164ParseResult e164ParseResult;

    e164ParseResult = parseE164String(aString, &theDigits, aCode);
    if (E164NoParseError == e164ParseResult)
    {
        E164 e164Result;
        char *cp;
        initializeE164WithCountryCode(&e164Result, *aCode);
        e164Result = (e164Result | strtoll(theDigits, &cp, 10));
        *aNumber = e164Result;
    }
    return e164ParseResult;
}

/*
 * parseE164String parses aString meant to represent an E164 number,
 * assigning the digits of the E164 number to theDigits and the
 * country code to the aCode parameter.
 *
 * parseE164String returns E164NoParseError on no error, or an error
 * code describing the error encountered.
 */
static
int parseE164String (const char * aNumberString,
                     const char ** theDigits,
                     E164CountryCode * aCode)
{
    const int stringLength = strlen(aNumberString);
    const char * endOfString = aNumberString + stringLength;
    const char * theChar = aNumberString;
    const char * remainder;
    const int remainderLength = stringLength - 1;
    /*
     * Make sure string doesn't exceed maximum length
     */
    if (E164MaximumStringLength < stringLength)
        return E164ParseErrorStringTooLong;
    if (E164MinimumStringLength > stringLength)
        return E164ParseErrorStringTooShort;
    /*
     * Check for a valid E164 prefix
     */
    if (!stringHasValidE164Prefix(aNumberString))
        return E164ParseErrorInvalidPrefix;

    /* Advance past prefix character */
    theChar++;
    remainder = theChar;
    if (eachCharIsDigit(remainder))
    {
        /*
         * If the remainder of the string is all digits,
         * then the remainder can be (potentially) used for the E164 number.
         */

        int numberOfCountryCodeDigits = 0;
        int digitIndex;
        E164CountryCode aCountryCode = 0;
        E164Type theType = E164Invalid;

        *theDigits = remainder;

        for (digitIndex = 0;
             E164MaximumCountryCodeLength > digitIndex;
             digitIndex++)
        {
            E164Type aType;
            if (endOfString <= (remainder + digitIndex))
                return E164ParseErrorNoSubscriberNumberDigits;

            aCountryCode *= 10;
            aCountryCode += remainder[digitIndex] - '0';

            aType = e164TypeForCountryCode(aCountryCode);
            if (E164Invalid != aType)
            {
                numberOfCountryCodeDigits = digitIndex + 1;
                theType = aType;
                break;
            }
        }

        /*
         * Assign country code.
         * If it's invalid, it'll be used in the error message.
         */
        *aCode = aCountryCode;
        if (isInvalidE164Type(theType))
            return E164ParseErrorInvalidType;
        else if (isUnassignedE164Type(theType))
            return E164ParseErrorUnassignedType;
        /*
         * Need some digits for the subscriber number
         */
        if (remainderLength <= numberOfCountryCodeDigits)
            return E164ParseErrorNoSubscriberNumberDigits;
        /*
         * Check number against E164Type
         * This tests against abolute (and unrealistic) minimums.
         * See comment regarding minimum Subscriber Number lengths
         */
        if (hasValidLengthForE164Type(remainderLength,
                                      numberOfCountryCodeDigits,
                                      theType))
            return E164NoParseError;
        else
            return E164ParseErrorTypeLengthMismatch;
    }
    else
        return E164ParseErrorBadFormat;
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
 * intializeE164WithCountryCode intializes aNumber with the length of
 * the countryCode
 */
static inline
void initializeE164WithCountryCode (E164 * aNumber,
                                    E164CountryCode aCountryCode)
{
    checkE164CountryCodeForRangeError(aCountryCode);
    *aNumber = ((uint64) aCountryCode) << E164_CC_MASK_OFFSET;
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
    return ((0 <= theCountryCode) && (999 >= theCountryCode));
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

/*
 * isInvalidE164Type returns true if aType is invalid or false otherwise.
 */
static inline
bool isInvalidE164Type (E164Type aType)
{
    return (E164Invalid == aType);
}

static inline
E164Type e164TypeForCountryCode (E164CountryCode theCountryCode)
{
    checkE164CountryCodeForRangeError(theCountryCode);
    return e164TypeFor[theCountryCode];
}
