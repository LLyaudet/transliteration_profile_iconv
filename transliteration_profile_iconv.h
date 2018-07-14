/*
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
*/
#ifndef DEF_transliteration_profile_iconv
#define DEF_transliteration_profile_iconv

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>



//------------------------------------------------------------------------------------
//Constants
//------------------------------------------------------------------------------------
//Limitations
#define I_MAXIMUM_LENGTH_OF_TRANSLITERATION_PER_CHARACTER (((long long int)1 << 32) - 1)
#define I_MINIMUM_USER_DEFINED_ERROR_CODE (-((long int)1 << 15))
#define I_MAXIMUM_SON 255 //for now we're working with octets

//Status constants
#define I_STATUS__SKIP 0
#define I_STATUS__VALID 1
//Negative status are error codes

//Error codes
//Positive error codes are defined in this library for algorithmical, profile parsing and technical errors
#define I_ERROR__NO_PREFIX_FOUND 1
#define I_ERROR__COULD_NOT_OPEN_FILE 2
#define I_ERROR__COULD_NOT_ALLOCATE_MEMORY 3
#define I_ERROR__EMPTY_PROFILE 4
#define I_ERROR__LINE_MUST_START_WITH_HEXADECIMAL_DIGIT 5
#define I_ERROR__LINE_MUST_CONTINUE_WITH_HEXADECIMAL_DIGIT__EVEN_NUMBER_OF_DIGITS__INPUT_TUPLE 6
#define I_ERROR__LINE_MUST_CONTINUE_WITH_HEXADECIMAL_DIGIT__EVEN_NUMBER_OF_DIGITS__OUTPUT_TUPLE 7
#define I_ERROR__LINE_MUST_CONTINUE_WITH_HEXADECIMAL_DIGIT_OR_SPACE 8
#define I_ERROR__LINE_RETURN_EXPECTED 9
#define I_ERROR__LINE_MUST_CONTINUE_WITH_HEXADECIMAL_DIGIT_OR_A_LINE_RETURN 10
#define I_ERROR__LINE_MUST_CONTINUE_WITH_DECIMAL_DIGIT 11
#define I_ERROR__THE_ERROR_CODE_VALUE_IS_TOO_NEGATIVE 12
#define I_ERROR__NOT_YET_CODED 13
#define I_ERROR__COULD_NOT_WRITE_CHARACTER 14
#define I_ERROR__UNKNOWN_PROFILE_TYPE 15
#define I_ERROR__WRONG_PROFILE_TYPE 16
#define I_ERROR__UNKNOWN_READ_STATE 17
#define I_ERROR__ROOT_NODE_SHOULD_HAVE_SKIP_STATUS_AND_NO_TRANSLITERATION 18
#define I_ERROR__LINE_MUST_CONTINUE_WITH_DECIMAL_DIGIT_OR_LINE_RETURN 19
#define I_ERROR__COULD_NOT_READ_NUMBER_OF_NODES 20
#define I_ERROR__COULD_NOT_READ_MINIMUM_SON 21
#define I_ERROR__COULD_NOT_READ_MAXIMUM_SON 22
#define I_ERROR__COULD_NOT_READ_SON_INDEX 23
#define I_ERROR__SON_INDEX_MUST_BE_AFTER 24
#define I_ERROR__SON_INDEX_MUST_BE_AT_MOST_THE_NUMBER_OF_NODES 25
#define I_ERROR__COULD_NOT_READ_TRANSLITERATION 26
#define I_ERROR__COULD_NOT_WRITE_DATA 27
//Negative error codes are user defined

//Profile types
#define I_PROFILE_TYPE__RAW 1
#define I_PROFILE_TYPE__SHRINK1 2

//#define DEBUG_TRANSLITERATION_PROFILE



//------------------------------------------------------------------------------------
//Structures
//------------------------------------------------------------------------------------

/**
 * The internal nodes for the transliteration profile
 */
typedef struct transliteration_node {
  size_t i_node_index;
  unsigned char i_minimum_son;
  unsigned char i_maximum_son;
  struct transliteration_node** arr_p_sons;
  int16_t i_status;
  size_t i_transliteration_size;
  size_t i_allocated_size;
  unsigned char* s_transliteration;
} t_transliteration_node;



/**
 * The transliteration profile
 */
typedef struct {
  unsigned char i_profile_type;
  size_t i_number_of_nodes;
  size_t i_max_depth;
  t_transliteration_node* p_root_node;
} t_transliteration_profile;




//------------------------------------------------------------------------------------
//External functions
//------------------------------------------------------------------------------------
/**
 * Transliteration profile management
 * Load a transliteration profile from a text file
 */
int transliteration_profile_load_from_text(
  char* s_filename,
  t_transliteration_profile** p_p_transliteration_profile,
  size_t* p_i_current_line,
  size_t* p_i_current_column
);



/**
 * Transliteration profile management
 * Load a transliteration profile from a binary file
 */
int transliteration_profile_load_from_bin(
  char* s_filename,
  t_transliteration_profile** p_p_transliteration_profile,
  size_t* p_i_current_offset
);



/**
 * Transliteration profile management
 * Performs a mathematical composition of two transliteration profiles (maps, see README)
 */
int transliteration_profile_compose(
  t_transliteration_profile* p_transliteration_profile_1,
  t_transliteration_profile* p_transliteration_profile_2,
  t_transliteration_profile** p_p_transliteration_profile_result
);



/**
 * Transliteration profile management
 * Dump a transliteration profile to a text file
 */
int transliteration_profile_dump_to_text(
  char* s_filename,
  t_transliteration_profile* p_transliteration_profile
);



/**
 * Transliteration profile management
 * Dump a transliteration profile to a binary file
 */
int transliteration_profile_dump_to_bin(
  char* s_filename,
  t_transliteration_profile* p_transliteration_profile
);



/**
 * Transliteration profile management
 * Free the memory of a transliteration profile
 */
void transliteration_profile_free(t_transliteration_profile* p_transliteration_profile);



/**
 * Transliteration profile use
 *
 */
int transliteration_profile_iconv(
  t_transliteration_profile* p_transliteration_profile,
  unsigned char* s_input_string,
  size_t i_size_input_string,
  unsigned char** p_s_output_string,
  size_t* p_i_size_output_string,
  size_t* p_i_current_read_offset//for debug if needed
);




//------------------------------------------------------------------------------------
//Internal functions
//------------------------------------------------------------------------------------



/**
 * Transliteration profile management
 * Traverse a transliteration profile
 */
int transliteration_profile_traversal(
  t_transliteration_profile* p_transliteration_profile,
  int (*p_function_prefix) (t_transliteration_node*),
  int (*p_function_postfix) (t_transliteration_node*)
);



/**
 * Transliteration profile management
 * Traverse a raw transliteration profile from the current node
 */
int transliteration_profile_traversal__raw_node(
  t_transliteration_node* p_transliteration_node,
  int (*p_function_prefix) (t_transliteration_node*),
  int (*p_function_postfix) (t_transliteration_node*)
);



/**
 * Transliteration profile management
 * Traverse a shrinked transliteration profile from the current node
 */
int transliteration_profile_traversal__shrink1_node(
  t_transliteration_node* p_transliteration_node,
  int (*p_function_prefix) (t_transliteration_node*),
  int (*p_function_postfix) (t_transliteration_node*)
);



/**
 * Transliteration profile management
 * Dump a raw transliteration profile to a text file
 */
int transliteration_profile_dump_to_text__raw(
  char* s_filename,
  t_transliteration_profile* p_transliteration_profile
);



/**
 * Transliteration profile management
 * Dump a shrinked transliteration profile to a text file (shrink1)
 */
int transliteration_profile_dump_to_text__shrink1(
  char* s_filename,
  t_transliteration_profile* p_transliteration_profile
);



/**
 * Transliteration profile management
 * Shrink a raw transliteration profile (shrink1)
 */
int transliteration_profile_from_raw_to_shrink1(
  t_transliteration_profile* p_transliteration_profile_from,
  t_transliteration_profile** p_p_transliteration_profile_to
);



/**
 * Transliteration profile use
 *
 */
int transliteration_profile_iconv__raw(
  t_transliteration_profile* p_transliteration_profile,
  unsigned char* s_input_string,
  size_t i_size_input_string,
  unsigned char** p_s_output_string,
  size_t* p_i_size_output_string,
  size_t* p_i_current_read_offset//for debug if needed
);



/**
 * Transliteration profile use
 *
 */
int transliteration_profile_iconv__shrink1(
  t_transliteration_profile* p_transliteration_profile,
  unsigned char* s_input_string,
  size_t i_size_input_string,
  unsigned char** p_s_output_string,
  size_t* p_i_size_output_string,
  size_t* p_i_current_read_offset//for debug if needed
);



/**
 * Transliteration profile management
 * Performs a mathematical composition of two transliteration profiles (maps, see README)
 */
int transliteration_profile_compose__raw(
  t_transliteration_profile* p_transliteration_profile_1,
  t_transliteration_profile* p_transliteration_profile_2,
  t_transliteration_profile** p_p_transliteration_profile_result
);



/**
 * Transliteration profile management
 * Performs a mathematical composition of two transliteration profiles (maps, see README)
 */
int transliteration_profile_compose__shrink1(
  t_transliteration_profile* p_transliteration_profile_1,
  t_transliteration_profile* p_transliteration_profile_2,
  t_transliteration_profile** p_p_transliteration_profile_result
);



#endif
