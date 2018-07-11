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

This file comes with the version 1.0.0 of transliteration_profile_iconv.

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

/!\/!\/!\
If you want to do something standard, just use iconv,
since the more general framework of transliteration_profile_iconv comes at the cost of slower performances.
/!\/!\/!\


----------------------------------------------------------------------------
Idea behind transliteration_profile_iconv
----------------------------------------------------------------------------
When you convert character strings from one encoding to another encoding,
you converts code points of the input encoding to code points of the output encoding;
and optionally with iconv you can also transliterate code points from the input encoding
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
from a tuple of octets to another tuple of octets.
Hence, transliteration_profile_iconv defines a "transliteration profile"
as a mathematical function that maps an element of a set of tuples of octets
to an element of another set of tuples of octets.
(It maps a tuple of octets to another tuple of octets,
but don't forget that not all tuples, even of some small fixed/bounded length,
are required to be in the domain of the mapping).
The lengths of the tuples are assumed to be small
(but the limit is the memory of the computer and/or limits given by the OS).
We have the following additional assumption that no input tuple is a prefix of another input tuple.
From this assumption, for any input string $s and any starting position $i inside this string,
let us denote cdr($s, $i) the substring of $s from position $i to the end of $s:
- either there is a prefix of cdr($s, $i) that is equal to one of the input tuples of the transliteration profile,
  in which case this prefix/tuple is unique,
- or there is no such prefix (and the main function of this library will stop and return error code 1).
From this rule, the algorithm is trivial :
  - $i = 0
  - loop) find prefix in cdr($s, $i)
  - "transliterate"
  - increase $i
  - repeat until error or end of string to transliterate
This assumption is correct with character encodings.


----------------------------------------------------------------------------
Technical details
----------------------------------------------------------------------------
The transliteration profiles can be saved as/loaded from a text file where:
- each line is '\n' terminated
- on each line there is :
  - two hexadecimal strings of pair lengths that are separated by a single space character
  - or the second hexadecimal string may be replaced by:
    -- 'i' this single letter will tell to transliteration_profile_iconv to ignore the input "character/tuple/prefix"
       (or equivalently maps it to the empty tuple) and continue,
       'i' is explicit because a missing hexadecimal string may also denote an error when the user created his profile
    -- a negative integer between -1 and -32768: will define user controlled error codes in case the user wants to distinguish between untransliterated characters.
- there is no comment, no other character allowed apart ASCII [a-f0-9 i\-]

Loaded transliteration profiles are stored in a tree structure that is memory expensive,
and that's why transliteration_profile_iconv is less efficient than iconv for most uses.


----------------------------------------------------------------------------
Going further
----------------------------------------------------------------------------
Remember "We have the following additional assumption that no input tuple is a prefix of another input tuple.", I lied :P
It is not hard to extend the library to greedily match the longest input tuple.
Let's generalize a little bit.
Assume that we have all the tuples of octets that we want without any constraint/assumption.
If we have two such tuples $t1 and $t2 such that $t1 is a prefix of $t2,
and we have a string starting with $t2,
we need a way to tell if we're going to keep the match between a shorter beginning of the string and $t1,
or the match between a longer beginning of the string and $t2, since both are possible choices.
Assume that that choice is deterministic and doesn't change with the "past" of the algorithm
(the part of the string that was already processed), then there was no reason to say in the profile
that $t2 was a possible match if we choose $t1 instead of $t2.
What happens if we have 3 possibles prefixes or more?
If the choices between the possible prefixes are transitive, we have an order and the greedy match is still the only option.
(That would be the case if you assign to each match a "priority", like an integer.)
But that's not the more general framework (even deterministic, past insensitive).
Indeed, we could have the following function $f from the set $IS(A) of all non-empty initial sections of the set $A of 3 nested prefixes:
($A is ordered by prefix order and an initial section of an order is the subset of all elements lower than a fixed element)
$A = {$t1, $t2, $t3}
$IS(A) = {
  {$t1},
  {$t1, $t2},
  {$t1, $t2, $t3}
}

$f = {
  ({$t1}, $t1),//choose $t1
  ({$t1, $t2}, $t1),//choose $t1
  ({$t1, $t2, $t3}, $t2)//choose $t2
}
As you see, the match with $t3 was just a trigger for choosing $t2 we can neither say that we prefer $t1 over $t2, nor $t2 over $t1 ; it's contextual.
That generalization is out of scope of this library for now.
Hence we assume that we have an order of preference over the nested prefixes from the profile ;
and thus the only choice that gives a reason for actually writing the tuples in the profile is to take the longest possible one.

