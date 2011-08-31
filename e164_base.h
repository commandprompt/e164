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
#ifndef E164_BASE_H
#define E164_BASE_H

#include "postgres.h"

#define E164_PREFIX_STRING "+"

#define E164_MAX_COUNTRY_CODE_VALUE 999

typedef enum E164StructureLimit
{
    E164MaximumNumberOfDigits = 15,
    E164PrefixStringLength    = sizeof(E164_PREFIX_STRING) - 1,

    E164MaximumRawStringLength = E164MaximumNumberOfDigits + E164PrefixStringLength,
/* Note this does *not* include the string terminator */
/*
 * There may be two parens symbols for the area code, plus up to 4
 * space symbols in a formatted E164 number, thus +6 to the raw number
 * string length.
 */
    E164MaximumStringLength   = E164MaximumRawStringLength + 6,
/*
 * E164MinimumStringLength is pretty conservative:
 * prefix (1) + country code (1) + subscriber number (1)
 */
    E164MinimumStringLength   = 3,
    E164MinimumNumberOfDigits = 2,

    E164MaximumCountryCodeLength               = 3,
    E164GeographicAreaMinimumCountryCodeLength = 1,
    E164GeographicAreaMaximumCountryCodeLength = 3,     /* = E164MaximumCountryCodeLength */
    E164GlobalServiceCountryCodeLength         = 3,     /* = E164MaximumCountryCodeLength */
    E164NetworkCountryCodeLength               = 3,     /* = E164MaximumCountryCodeLength */
    E164GroupOfCountriesCountryCodeLength      = 3,     /* = E164MaximumCountryCodeLength */

/*
 * Minimum Subscriber Number lengths for the various E164Types
 * These are absolute (and unrealistic) minimums. However, true
 * minimums are country specific, and until this implementation
 * is country-code specific, this should do.
 */
    E164GeographicAreaMinimumSubscriberNumberLength   = 1,
    E164GlobalServiceMinimumSubscriberNumberLength    = 1,
    E164NetworkMinimumSubscriberNumberLength          = 2,
    E164GroupOfCountriesMinimumSubscriberNumberLength = 2
} E164StructureLimit;

typedef int4 E164CountryCode;
typedef uint64 E164;

/*
 * There are four types of assigned E164:
 *    * Geographic Area numbers
 *    * Global Service numbers
 *    * Network numbers
 *    * Group of Countries numbers
 * Each of these E164 number types have well-defined formats.
 *
 * There are three types of unassigned E164 as well:
 *     * Reserved numbers
 *     * Spare codes with notes
 *     * Spare codes without notes
 *
 * For the purposes of the implementation, all unassigned E164 numbers
 * will be considered invalid.
 *
 * An E164's type can be determined by inspecting its country code --
 * (at most) the first three digits. (Some Geographic Area country codes are one
 * or 2 digits in length.)
 *
 * Some country codes are reserved, and others are spare (as of yet unassigned).
 * These reserved and spare codes are rejected as invalid by this implementation,
 * in part because one cannot determine such a number's type and therefore whether
 * the number matches the format of its type.
 */
typedef enum E164Type
{
    E164GeographicArea,
    E164GlobalService,
    E164Network,
    E164GroupOfCountries,
    /* Unassigned codes */
    E164Reserved,
    E164SpareWithNote,
    E164SpareWithoutNote,
    E164Invalid
} E164Type;

/*
static const char * e164TypeName[] = {
    "E.164 Geographic Area",
    "E.164 Global Service",
    "E.164 Network",
    "E.164 Group of Countries",
    "E.164 Reserved",
    "E.164 Spare with Note",
    "E.164 Spare without Note",
    "E.164 Invalid"
};
*/


const E164Type e164TypeFor[E164_MAX_COUNTRY_CODE_VALUE + 1];

extern E164 e164FromString (const char * aString);
extern int stringFromE164 (char * aString, int stringLength, E164 aNumber);
extern int rawStringFromE164 (char * aString, int stringLength, E164 aNumber);
extern int countryCodeStringFromE164 (char * aString, int stringLength,
                                      E164 aNumber);

extern int64 e164Comparison (E164 firstNumber, E164 secondNumber);

extern bool stringHasValidE164Prefix (const char * aString);
extern bool e164CountryCodeIsInRange (E164CountryCode theCountryCode);

extern bool isUnassignedE164Type (E164Type aType);
extern bool isValidE164Type (E164Type aType);
extern bool isInvalidE164Type (E164Type aType);

extern E164Type e164TypeForCountryCode (E164CountryCode theCountryCode);
extern bool isValidE164CountryCodeType (E164CountryCode theCountryCode);
extern bool isInvalidE164CountryCodeType (E164CountryCode theCountryCode);

#endif /* !E164_BASE_H */
