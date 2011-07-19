/*
 * E164 Type base functions
 * $Id: e164_base.c 53 2007-09-10 01:13:48Z glaesema $
 */

#include "postgres.h"
#include "e164_base.h"

/*
 * The largest possible E164 number is 999_999_999_999_999, which is
 * equal to 0x3_8d7e_a4c6_7fff.  Thus the number mask value.
 *
 * The largest valid E164 number is currently 998_999_999_999_999,
 * according to this document:
 *
 * http://www.itu.int/dms_pub/itu-t/opb/sp/T-SP-E.164D-2009-PDF-E.pdf
 */
#define E164_NUMBER_MASK    UINT64CONST(0x0003FFFFFFFFFFFF)
#define E164_CC_LENGTH_MASK UINT64CONST(0x000C000000000000)
#define E164_NUMBER_AND_CC_LENGTH_MASK (E164_NUMBER_MASK | E164_CC_LENGTH_MASK)
#define E164_ONE_DIGIT_CC_MASK   UINT64CONST(0x0004000000000000)
#define E164_TWO_DIGIT_CC_MASK   UINT64CONST(0x0008000000000000)
#define E164_THREE_DIGIT_CC_MASK UINT64CONST(0x000C000000000000)
#define E164_CC_LENGTH_OFFSET 50

#define E164_GEOGRAPHIC_AREA_MASK    UINT64CONST(0x0010000000000000)
#define E164_GLOBAL_SERVICE_MASK     UINT64CONST(0x0020000000000000)
#define E164_NETWORK_MASK            UINT64CONST(0x0040000000000000)
#define E164_GROUP_OF_COUNTRIES_MASK UINT64CONST(0x0080000000000000)

/*
 * Function prototypes
 */
static inline E164Type e164Type (E164 theNumber);
static inline int e164CountryCodeLength (E164 aNumber);

static int parseE164String (const char * aNumberString,
                            const char ** theDigits,
                            E164CountryCode * aCode);

static inline bool hasValidLengthForE164Type (int numberLength,
                                              int countryCodeLength,
                                              E164Type aType);
static inline void initializeE164WithCountryCode (E164 * aNumber,
                                                  E164CountryCode aCountryCode);

static inline bool isValidE164PrefixChar (char aChar);
static inline bool stringHasValidE164Prefix (const char * aString);

static inline bool isEndOfString (const char * theChar);
static inline bool eachCharIsDigit (const char * aString);

static inline bool e164CountryCodeIsInRange (E164CountryCode theCountryCode);
static inline void checkE164CountryCodeForRangeError (E164CountryCode theCountryCode);

static inline bool isOneDigitE164CountryCode (E164CountryCode theCountryCode);
static inline bool isTwoDigitE164CountryCode (E164CountryCode theCountryCode);
static inline bool isThreeDigitE164CountryCode (E164CountryCode theCountryCode);
static inline bool isUnassignedE164Type (E164Type aType);
static inline bool isInvalidE164Type (E164Type aType);
static inline E164Type e164TypeForCountryCode (E164CountryCode theCountryCode);

/*
 * Function definitions
 */

int e164Comparison (E164 firstNumber, E164 secondNumber)
{
    if ((firstNumber & E164_NUMBER_AND_CC_LENGTH_MASK) <
        (secondNumber & E164_NUMBER_AND_CC_LENGTH_MASK))
        return -1;
    else if ((firstNumber & E164_NUMBER_AND_CC_LENGTH_MASK) ==
             (secondNumber & E164_NUMBER_AND_CC_LENGTH_MASK))
        return 0;
    else
        return 1;

};

bool e164IsEqualTo (E164 firstNumber, E164 secondNumber)
{
    return (0 == e164Comparison(firstNumber, secondNumber));
};

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


/*
 * e164Type returns the E164Type of the E164 argument
 */
static inline
E164Type e164Type (E164 theNumber)
{
    if (theNumber & E164_GEOGRAPHIC_AREA_MASK)
        return E164GeographicArea;
    else if (theNumber & E164_GLOBAL_SERVICE_MASK)
        return E164GlobalService;
    else if (theNumber & E164_NETWORK_MASK)
        return E164Network;
    else if (theNumber & E164_GROUP_OF_COUNTRIES_MASK)
        return E164GroupOfCountries;

    elog(ERROR, "E164 value is invalid: " UINT64_FORMAT, theNumber);
}

/*
 * countryCodeStringFromE164 assigns the country code for aNumber to aString, returning
 * the number of characters written (in this case, the number of digits in the
 * country code).
 */
int countryCodeStringFromE164 (char * aString, E164 aNumber)
{
    return snprintf(aString, e164CountryCodeLength(aNumber) + 1,
                    UINT64_FORMAT, (aNumber & E164_NUMBER_MASK));
}

/*
 * e164CountryCodeLength returns the length of the country code for an E164 number
 */
static inline
int e164CountryCodeLength (E164 aNumber)
{
    return (int) ((aNumber & E164_CC_LENGTH_MASK) >> E164_CC_LENGTH_OFFSET);
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
    /*
     * Only Geographic Areas have country codes with lengths 1 or 2.
     */
    if (isOneDigitE164CountryCode(aCountryCode))
        *aNumber = E164_ONE_DIGIT_CC_MASK | E164_GEOGRAPHIC_AREA_MASK;
    else if (isTwoDigitE164CountryCode(aCountryCode))
        *aNumber = E164_TWO_DIGIT_CC_MASK | E164_GEOGRAPHIC_AREA_MASK;
    else if (isThreeDigitE164CountryCode(aCountryCode))
    {
        uint64 typeMask = 0;
        E164Type aType = e164TypeForCountryCode(aCountryCode);
        switch (aType)
        {
            case E164GeographicArea:
                typeMask = E164_GEOGRAPHIC_AREA_MASK;
                break;
            case E164GlobalService:
                typeMask = E164_GLOBAL_SERVICE_MASK;
                break;
            case E164Network:
                typeMask = E164_NETWORK_MASK;
                break;
            case E164GroupOfCountries:
                typeMask = E164_GROUP_OF_COUNTRIES_MASK;
                break;
            default:
                elog(ERROR, "E164Type value is invalid: %d", aType);
        }
        *aNumber = E164_THREE_DIGIT_CC_MASK | typeMask;
    }
    else
    {
        elog(ERROR, "E164CountryCode value is invalid: %d", aCountryCode);
    }
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
 * isOneDigitE164CountryCode returns true if theCountryCode is a single digit
 * and false otherwise.
 */
static inline
bool isOneDigitE164CountryCode (E164CountryCode theCountryCode)
{
    return ((0 <= theCountryCode) && (9 >= theCountryCode));
}

/*
 * isTwoDigitE164CountryCode returns true if theCountryCode is two digits
 * and false otherwise.
 */
static inline
bool isTwoDigitE164CountryCode (E164CountryCode theCountryCode)
{
    return ((10 <= theCountryCode) && (99 >= theCountryCode));
}

/*
 * isThreeDigitE164CountryCode returns true if theCountryCode is three digits
 * and false otherwise.
 */
static inline
bool isThreeDigitE164CountryCode (E164CountryCode theCountryCode)
{
    return ((100 <= theCountryCode) && (999 >= theCountryCode));
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
