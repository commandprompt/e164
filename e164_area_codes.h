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
#ifndef E164_AREA_CODES_H
#define E164_AREA_CODES_H

#include "e164_base.h"

typedef int E164AreaCode;

typedef struct E164AreaCodesFormat
{
    int countryCode;
    int defaultAreaCodeLength;
    char * exceptionsList;
} E164AreaCodesFormat;

typedef struct E164AreaCodesInfo
{
    int numberOfFormats;
    E164AreaCodesFormat formats[];
} E164AreaCodesInfo;


extern int e164AreaCodeLengthOf(E164 aNumber, E164CountryCode aCountryCode,
                                int countryCodeLength);

extern void e164SetAreaCodesInfo(E164AreaCodesInfo * codesInfo);

extern bool parseE164AreaCodesFormat(char * aFormat,
                                     E164AreaCodesInfo ** theCodesInfo);

#endif /* !E164_AREA_CODES_H */
