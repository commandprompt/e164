/*
 * E.164 Telephone Number Type for PostgreSQL: E.164 country code types
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
#include "e164_base.h"

const int e164TypeFor[1000] = {
    /* 0..9 */
    E164Reserved, E164GeographicArea, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164GeographicArea, E164Invalid, E164Invalid,
    /* 10..19 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 20..29 */
    E164GeographicArea, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164GeographicArea, E164Invalid, E164Invalid,
    /* 30..39 */
    E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea,
    E164Invalid, E164GeographicArea, E164Invalid, E164Invalid, E164GeographicArea,
    /* 40..49 */
    E164GeographicArea, E164GeographicArea, E164Invalid, E164GeographicArea, E164GeographicArea,
    E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea,
    /* 50..59 */
    E164Invalid, E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea,
    E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea, E164Invalid,
    /* 60..69 */
    E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea,
    E164GeographicArea, E164GeographicArea, E164Invalid, E164Invalid, E164Invalid,
    /* 70..79 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 80..89 */
    E164Invalid, E164GeographicArea, E164GeographicArea, E164Invalid, E164GeographicArea,
    E164Invalid, E164GeographicArea, E164Invalid, E164Invalid, E164Invalid,
    /* 90..99 */
    E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea,
    E164GeographicArea, E164Invalid, E164Invalid, E164GeographicArea, E164Invalid,
    /* 100..109 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 110..119 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 120..129 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 130..139 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 140..149 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 150..159 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 160..169 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 170..179 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 180..189 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 190..199 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,

    /* 200..209 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 210..219 */
    E164SpareWithoutNote, E164SpareWithoutNote, E164GeographicArea, E164GeographicArea, E164SpareWithoutNote,
    E164SpareWithoutNote, E164GeographicArea, E164SpareWithoutNote, E164GeographicArea, E164SpareWithoutNote,
    /* 220..229 */
    E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea,
    E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea,
    /* 230..239 */
    E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea,
    E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea,
    /* 240..249 */
    E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea,
    E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea,
    /* 250..259 */
    E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea,
    E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea, E164SpareWithoutNote,
    /* 260..269 */
    E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea,
    E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea,
    /* 270..279 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 280..289 */
    E164SpareWithNote, E164SpareWithNote, E164SpareWithNote, E164SpareWithNote, E164SpareWithNote,
    E164SpareWithNote, E164SpareWithNote, E164SpareWithNote, E164SpareWithNote, E164SpareWithNote,
    /* 290..299 */
    E164GeographicArea, E164GeographicArea, E164SpareWithoutNote, E164SpareWithoutNote, E164SpareWithoutNote,
    E164SpareWithoutNote, E164SpareWithoutNote, E164GeographicArea, E164GeographicArea, E164GeographicArea,

    /* 300..309 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 310..319 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 320..329 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 330..339 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 340..349 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 350..359 */
    E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea,
    E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea,
    /* 360..369 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 370..379 */
    E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea,
    E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea,
    /* 380..389 */
    E164GeographicArea, E164GeographicArea, E164GeographicArea, E164SpareWithoutNote, E164SpareWithoutNote,
    E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GroupOfCountries, E164GeographicArea,
    /* 390..399 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,

    /* 400..409 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 410..419 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 420..429 */
    E164GeographicArea, E164GeographicArea, E164SpareWithoutNote, E164GeographicArea, E164SpareWithoutNote,
    E164SpareWithoutNote, E164SpareWithoutNote, E164SpareWithoutNote, E164SpareWithoutNote, E164SpareWithoutNote,
    /* 430..439 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 440..449 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 450..459 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 460..469 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 470..479 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 480..489 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 490..499 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,

    /* 500..509 */
    E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea,
    E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea,
    /* 510..519 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 520..529 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 530..539 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 540..549 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 550..559 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 560..569 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 570..579 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 580..589 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 590..599 */
    E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea,
    E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea,

    /* 600..609 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 610..619 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 620..629 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 630..639 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 640..649 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 650..659 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 660..669 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 670..679 */
    E164GeographicArea, E164SpareWithoutNote, E164GeographicArea, E164GeographicArea, E164GeographicArea,
    E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea,
    /* 680..689 */
    E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea, E164SpareWithoutNote,
    E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea,
    /* 690..699 */
    E164GeographicArea, E164GeographicArea, E164GeographicArea, E164SpareWithoutNote, E164SpareWithoutNote,
    E164SpareWithoutNote, E164SpareWithoutNote, E164SpareWithoutNote, E164SpareWithoutNote, E164SpareWithoutNote,

    /* 700..709 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 710..719 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 720..729 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 730..739 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 740..749 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 750..759 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 760..769 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 770..779 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 780..789 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 790..799 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,

    /* 800..809 */
    E164GlobalService, E164SpareWithNote, E164SpareWithNote, E164SpareWithNote, E164SpareWithNote,
    E164SpareWithNote, E164SpareWithNote, E164SpareWithNote, E164GlobalService, E164SpareWithNote,
    /* 810..819 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 820..829 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 830..839 */
    E164SpareWithNote, E164SpareWithNote, E164SpareWithNote, E164SpareWithNote, E164SpareWithNote,
    E164SpareWithNote, E164SpareWithNote, E164SpareWithNote, E164SpareWithNote, E164SpareWithNote,
    /* 840..849 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 850..859 */
    E164GeographicArea, E164SpareWithoutNote, E164GeographicArea, E164GeographicArea, E164SpareWithoutNote,
    E164GeographicArea, E164GeographicArea, E164SpareWithoutNote, E164SpareWithoutNote, E164SpareWithoutNote,
    /* 860..869 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 870..879 */
    E164Network, E164Network, E164Network, E164Network, E164Reserved,
    E164Reserved, E164Reserved, E164Reserved, E164GlobalService, E164Reserved,
    /* 880..889 */
    E164GeographicArea, E164Network, E164Network, E164SpareWithNote, E164SpareWithoutNote,
    E164SpareWithoutNote, E164GeographicArea, E164SpareWithoutNote, E164GlobalService, E164SpareWithoutNote,
    /* 890..899 */
    E164SpareWithNote, E164SpareWithNote, E164SpareWithNote, E164SpareWithNote, E164SpareWithNote,
    E164SpareWithNote, E164SpareWithNote, E164SpareWithNote, E164SpareWithNote, E164SpareWithNote,

    /* 900..909 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 910..919 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 920..929 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 930..939 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 940..949 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 950..959 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 960..969 */
    E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea,
    E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea, E164Reserved,
    /* 970..979 */
    E164Reserved, E164GeographicArea, E164GeographicArea, E164GeographicArea, E164GeographicArea,
    E164GeographicArea, E164GeographicArea, E164GeographicArea, E164SpareWithoutNote, E164GlobalService,
    /* 980..989 */
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    E164Invalid, E164Invalid, E164Invalid, E164Invalid, E164Invalid,
    /* 990..999 */
    E164SpareWithoutNote, E164GlobalService, E164GeographicArea, E164GeographicArea, E164GeographicArea,
    E164GeographicArea, E164GeographicArea, E164SpareWithoutNote, E164GeographicArea, E164Reserved
};
