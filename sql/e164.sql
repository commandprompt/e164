-- E164 type regression test SQL script

SET search_path = public;
\set ECHO none
\i e164.sql
\set ECHO all
\set VERBOSITY terse

SET SEARCH_PATH to public, e164;

CREATE TABLE telephone_numbers
(
    telephone_number e164 PRIMARY KEY
);

COPY telephone_numbers (telephone_number) FROM STDIN;
+18887355977
+18007246269
+17137292424
+16094926522
+12078652196
+13032899913
+18162796113
+16158551760
+18125224008
+18162212045
+19412583400
+442070342900
+442073779923
+442085187347
+442087782777
+442089816811
+448456032458
+441223208301
+441223207072
+35312121220
+35318572979
+27116223170
+27117825226
+27114314068
+27119491263
+81188364224
+81188361268
+81188327353
+81185242051
+81762226757
+81762412341
+9123692496
+9126410553
+9128102290
+9126552802
+2034810166
+2023366848
+20222777000
+20222686035
+20225912502
+559132294022
+557932173229
+552124527106
+556233576049
+551150557460
+78122328260
+78122328250
+78122326983
+74959808440
+74959808441
+74956260391
+78123254044
\.

SELECT *
FROM telephone_numbers
ORDER BY telephone_number;

SELECT e164_raw(telephone_number)
FROM telephone_numbers
ORDER BY telephone_number;

SELECT e164_raw(telephone_number)
FROM telephone_numbers
ORDER BY CAST(telephone_number AS text);

SELECT e164_raw(telephone_number) as raw_phone_number
    , country_code(telephone_number)
FROM telephone_numbers
ORDER BY telephone_number;

SELECT e164_raw(telephone_number) as raw_phone_number
    , e164_raw(a_telephone_number) as a_raw_phone_number
    , telephone_number < a_telephone_number as lt
    , telephone_number <= a_telephone_number as le
    , telephone_number = a_telephone_number as eq
    , telephone_number >= a_telephone_number as ge
    , telephone_number > a_telephone_number as gt
    , telephone_number <> a_telephone_number as ne
FROM telephone_numbers
CROSS JOIN (SELECT CAST('+442073779923' AS e164)) AS a(a_telephone_number)
ORDER BY telephone_number;

CREATE INDEX telephone_number_hash_idx
ON telephone_numbers USING hash
(telephone_number) ;

DROP TABLE telephone_numbers;

-- Invalid input strings
-- non-numeric characters in string
SELECT CAST('+1234567890a' AS e164);
SELECT CAST('+a' AS e164);
-- leading or trailing whitespace
SELECT CAST(' +1234567890' AS e164);
SELECT CAST('+1234567890 ' AS e164);
SELECT CAST(' +1234567890 ' AS e164);

-- no prefix
SELECT CAST('1234567890' AS e164);

-- too short for e164
SELECT CAST('+1' AS e164);
-- too long for e164
SELECT CAST('+12345678901234567890' AS e164);

-- unassigned country codes
-- 0 is reserved
SELECT CAST('+01234567890' AS e164);
-- 280 is Spare with Note
SELECT CAST('+280123456' AS e164);
-- 295 is Spare without Note
SELECT CAST('+295123456' AS e164);

-- inconsistent length and country code (subscriber number minimum lengths)
-- Geographic Area and Global Service numbers require a subscriber number
-- of at least one digit. As *all* E164 numbers must have a subscriber
-- number of at least one digit, Geographic Area and Global Service numbers
-- without subscriber numbers will be invalid in general and not raise
-- this specific error.
-- Geographic Area: minimum 1 digit
-- '+1' should actually be caught as a minimum length error before this
SELECT CAST('+1' AS e164);
SELECT CAST('+11' AS e164);
-- These should raise no subscriber number digit errors
SELECT CAST('+81' AS e164);
SELECT CAST('+811' AS e164); -- okay
SELECT CAST('+220' AS e164);
SELECT CAST('+2201' AS e164); -- okay
-- Global Service: minimum 1 digit
SELECT CAST('+800' AS e164);
SELECT CAST('+808' AS e164);
SELECT CAST('+8001' AS e164); -- okay
SELECT CAST('+8081' AS e164); -- okay

-- Inconsistent Group of Countries and Network numbers should raise
-- inconsistent E164 number
-- Group of Countries: minimum 2 digits
SELECT CAST('+3881' AS e164);
SELECT CAST('+38812' AS e164); -- okay
SELECT CAST('+3882' AS e164);
SELECT CAST('+38822' AS e164); -- okay
-- Network: minimum 2 digits
SELECT CAST('+8701' AS e164);
SELECT CAST('+87019' AS e164); -- okay
SELECT CAST('+8711' AS e164);
SELECT CAST('+87119' AS e164); -- okay

-- Full 15 digits
SELECT CAST('+998999999999999' AS e164); -- okay

-- Formatted numbers parsing
-- Normal
SELECT CAST('+1 (23) 456' AS e164); -- okay
SELECT CAST('+1(23)456' AS e164); -- okay
-- Garbage in text
SELECT CAST('+1a23 456' AS e164);
-- 'Bogus' country code
SELECT CAST('+123 456' AS e164); -- okay
-- Country code 'exceeding' allowed range
SELECT CAST('+1234 56' AS e164); -- okay
-- Trailing paren
SELECT CAST('+1 (23)' AS e164);
-- 'Misplaced' area code parens
SELECT CAST('+1 23 (456) 789' AS e164); -- okay
-- 'Misplaced' area code parens, again
SELECT CAST('+1 23(456) 789' AS e164); -- okay
-- Leading paren
SELECT CAST('+(1) 23' AS e164);
-- Double parens
SELECT CAST('+1 (23) (456)' AS e164);
-- Mismatched parens
SELECT CAST('+1 ((' AS e164);
-- Empty parens
SELECT CAST('+1 ()' AS e164);
-- Extra space before paren
SELECT CAST('+1 ( 23) 456' AS e164); -- okay
-- Space inside parens
SELECT CAST('+1 (2 3) 456' AS e164); -- okay
-- Right paren first
SELECT CAST('+1 )23) 456' AS e164);
-- Leading space
SELECT CAST('+ 1 23 456' AS e164); -- okay
-- Trailing space
SELECT CAST('+1 23 456 ' AS e164);
-- Extra inner space
SELECT CAST('+1  23 456' AS e164); -- okay

-- The Christmas tree formatting test
SELECT CAST('+12' AS e164);
SELECT CAST('+123' AS e164);
SELECT CAST('+1234' AS e164);
SELECT CAST('+12345' AS e164);
SELECT CAST('+123456' AS e164);
SELECT CAST('+1234567' AS e164);
SELECT CAST('+12345678' AS e164);
SELECT CAST('+123456789' AS e164);
SELECT CAST('+1234567890' AS e164);
SELECT CAST('+12345678901' AS e164);
SELECT CAST('+123456789012' AS e164);
SELECT CAST('+1234567890123' AS e164);
SELECT CAST('+12345678901234' AS e164);
SELECT CAST('+123456789012345' AS e164);
