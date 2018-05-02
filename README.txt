This file is part of transliteration_profile_iconv library.

transliteration_profile_iconv is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

transliteration_profile_iconv is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with transliteration_profile_iconv.  If not, see <http://www.gnu.org/licenses/>.

©Copyright 2018 Laurent Lyaudet

----------------------------------------------------------------------------
Preamble
----------------------------------------------------------------------------
This folder contains the source of the library transliteration_profile_iconv.

This library provides functions to load, compose, dump transliteration profiles, 
and transliterate (and/or convert encoding of) arbitrary strings (array of octets) using a transliteration profile.

What can be done by iconv() can be done using transliteration_profile_iconv.
Transliterating/converting using non standard encodings or non standard translation between standard encodings 
can be done with transliteration_profile_iconv when it is impossible to do with iconv
(see https://sourceware.org/bugzilla/show_bug.cgi?id=23076).

If you want to do something standard, just use iconv, 
since the more general framework of transliteration_profile_iconv comes at the cost of slower performances.


----------------------------------------------------------------------------
Idea behind transliteration_profile_iconv
----------------------------------------------------------------------------
When you convert character strings from one encoding to another encoding,
you converts code points of the input encoding to code points of the output encoding;
and optionnaly with iconv you can also transliterate code points from the input encoding 
that don't have a matching code point in the output encoding.
In this context, transliterate means tranlating the code point of the input encoding with 
a sequence of code points in the output encoding.
Example:
 UTF-8   |  ISO-8859-1
 é       |  é          <- convert
 €       |  EUR        <- transliterate
The transliteration rules may be from locales (iconv in glibc), or hardcoded (libiconv).
However, if we look at how the 'é' was converted and the '€' was transliterated 
from UTF-8 to ISO-8859-1 in binary/hexadecimal,
 UTF-8        |  ISO-8859-1
 é = 0xc3a9   |  é = 0xe9        <- convert
 € = 0xe282ac |  EUR = 0x455552  <- transliterate
the difference between "convert" and "transliterate" is less evident.
In fact, both cases are "transliteration" (in the meaning used in this library)
from a sequence of octets to another sequence of octets.
Hence, transliteration_profile_iconv defines a "transliteration profile" 
as a mathematical function that maps an element of a set of tuples of octets
to an element of another set of tuples of octets.
The lengths of the input tuples are assumed to be small
(but the limit is the memory of the computer),
the lengths of the output tuples must be less than 127 octets with the current implementation.
We have the following additional assumption that no input tuple is a prefix of another input tuple.
From this assumption, for any input string $s and any starting position $i inside this string,
let us denote cdr($s, $i) the substring of $s from position $i to the end of $s :
- either there is a prefix of cdr($s, $i) that is equal to one of the input tuples of the transliteration profile,
  in which case this prefix/tuple is unique,
- or there is no such prefix (and the library will stop and return error code -1).
From this rule, the algorithm is trivial :
  - $i = 0
  - loop) find prefix in cdr($s, $i)
  - "transliterate"
  - increase $i
  - repeat until error or end of string to transliterate 
This assumption is correct with character encodings.
Nevertheless, it would not be hard to extend the library to greedily match the longest input tuple.


----------------------------------------------------------------------------
Technical details
----------------------------------------------------------------------------
The transliteration profiles can be saved as/loaded from a text file where:
- each line is '\n' terminated
- on each line there is :
  - two hexadecimal strings of pair lengths that are separated by a single space character
  - or the second hexadecimal string may be replaced by a negative integer between -1 and -128:
    -- -1 will tell to transliteration_profile_iconv to ignore the "character/tuple/prefix" (or equivalently maps it to the empty tuple) and continue
    -- -2, -3, ..., or -128 will define user controlled error codes in case the user wants to distinguish between untransliterated characters.
- there is no comment, no other character allowed apart ASCII [a-f0-9 \-]

Loaded transliteration profiles are stored in a tree structure that is memory expensive,
and that's why transliteration_profile_iconv is less efficient than iconv for most uses.




