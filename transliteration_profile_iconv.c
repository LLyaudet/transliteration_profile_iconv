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

#include "transliteration_profile_iconv.h"


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
){
  FILE * file = NULL;
  int c;//that's a char but getc returns an int
  int i_current_read_state = -1;
  int i_error_code = 0;
  t_transliteration_node* p_root_node;
  t_transliteration_node* p_current_node;
  t_transliteration_node* p_new_node;
  unsigned char i_current_octet = 0;
  long i_status_for_node = 0;
  void* p_for_realloc = NULL;
  size_t i_size_for_realloc = 0;

  #ifdef DEBUG_TRANSLITERATION_PROFILE
  printf(
      "Call: transliteration_profile_load_from_text() %s %d %d %d\n",
      s_filename,
      p_p_transliteration_profile,
      p_i_current_line,
      p_i_current_column
  );
  #endif

  *p_i_current_line = 0;
  *p_i_current_column = 0;

  //first we allocate the transliteration profile
  *p_p_transliteration_profile = (t_transliteration_profile*) calloc(1, sizeof(t_transliteration_profile));
  if(*p_p_transliteration_profile == NULL){
    return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
  }
  (*p_p_transliteration_profile)->i_profile_type = I_PROFILE_TYPE__RAW;
  (*p_p_transliteration_profile)->i_number_of_nodes = 0;
  (*p_p_transliteration_profile)->p_root_node = NULL;

  //and we allocate the root node
  (*p_p_transliteration_profile)->p_root_node = (t_transliteration_node*) calloc(1, sizeof(t_transliteration_node));
  if((*p_p_transliteration_profile)->p_root_node == NULL){
    free(*p_p_transliteration_profile);
    *p_p_transliteration_profile = NULL;
    return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
  }
  p_root_node = (*p_p_transliteration_profile)->p_root_node;
  p_current_node = p_root_node;
  ++((*p_p_transliteration_profile)->i_number_of_nodes);
  p_current_node->i_node_index = (*p_p_transliteration_profile)->i_number_of_nodes;
  p_current_node->i_minimum_son = 255;
  p_current_node->i_maximum_son = 0;
  p_current_node->arr_p_sons = NULL;
  p_current_node->i_status = I_STATUS__SKIP;
  p_current_node->i_transliteration_size = 0;
  p_current_node->i_allocated_size = 0;
  p_current_node->s_transliteration = NULL;
  //with its array of sons
  p_current_node->arr_p_sons = (t_transliteration_node**) calloc(256, sizeof(t_transliteration_node*));
  if(p_current_node->arr_p_sons == NULL){
    transliteration_profile_free(*p_p_transliteration_profile);
    *p_p_transliteration_profile = NULL;
    return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
  }
  for(int i = 0; i < 256; ++i){
    p_current_node->arr_p_sons[i] = NULL;
  }

  file = fopen(s_filename, "r");
  if(file == NULL){
    transliteration_profile_free(*p_p_transliteration_profile);
    *p_p_transliteration_profile = NULL;
    return I_ERROR__COULD_NOT_OPEN_FILE;
  }

  *p_i_current_line = 1;
  *p_i_current_column = 0;

  //i_current_read_state = -1;//nothing read apart new lines
  //i_current_read_state = 0;//just read a line return
  //i_current_read_state = 1;//just read input tuple odd hexadecimal digit
  //i_current_read_state = 2;//just read input tuple even hexadecimal digit
  //i_current_read_state = 3;//just read space after input tuple
  //i_current_read_state = 4;//just read 'i'(gnore)
  //i_current_read_state = 5;//just read output tuple odd hexadecimal digit
  //i_current_read_state = 6;//just read output tuple even hexadecimal digit
  //i_current_read_state = 7;//just read '-'
  //i_current_read_state = 8;//just read a decimal digit for error code

  while((c = getc(file)) != EOF){
    ++(*p_i_current_column);
    #ifdef DEBUG_TRANSLITERATION_PROFILE
    printf("ligne %d colonne %d", *p_i_current_line, *p_i_current_column);
    #endif

    switch(i_current_read_state){
      case -1://nothing read apart new lines
      case 0://just read a line return
        if(c == '\n'){
          //almost nothing to do
          ++(*p_i_current_line);
          *p_i_current_column = 0;
        }
        else if(c >= '0' && c <= '9'){
          i_current_read_state = 1;
          i_current_octet = (((unsigned char)c) - '0') * 16;
        }
        else if(c >= 'a' && c <= 'f'){
          i_current_read_state = 1;
          i_current_octet = (((unsigned char)c) - 'a' + 10) * 16;
        }
        else{
          i_error_code = I_ERROR__LINE_MUST_START_WITH_HEXADECIMAL_DIGIT;
        }
      break;

      case 1://just read input tuple odd hexadecimal digit
        if((c >= '0' && c <= '9')
          || (c >= 'a' && c <= 'f')
        ){
          i_current_read_state = 2;
          if(c >= '0' && c <= '9'){
            i_current_octet += ((unsigned char)c) - '0';
          }
          else{
            i_current_octet += ((unsigned char)c) - 'a' + 10;
          }
          //change current node, create it if needed
          if(p_current_node->arr_p_sons[i_current_octet] == NULL){
            ++((*p_p_transliteration_profile)->i_number_of_nodes);
            p_new_node->i_node_index = (*p_p_transliteration_profile)->i_number_of_nodes;
            p_new_node->i_minimum_son = 255;
            p_new_node->i_maximum_son = 0;
            p_new_node->arr_p_sons = NULL;
            p_new_node->i_status = I_STATUS__SKIP;
            p_new_node->i_transliteration_size = 0;
            p_new_node->i_allocated_size = 0;
            p_new_node->s_transliteration = NULL;
            //with its array of sons
            p_new_node->arr_p_sons = (t_transliteration_node**) calloc(256, sizeof(t_transliteration_node*));
            if(p_new_node->arr_p_sons == NULL){
              i_error_code = I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
              break;
            }
            for(int i = 0; i < 256; ++i){
              p_new_node->arr_p_sons[i] = NULL;
            }
            p_current_node->arr_p_sons[i_current_octet] = p_new_node;
            if(p_current_node->i_minimum_son > i_current_octet){
              p_current_node->i_minimum_son = i_current_octet;
            }
            if(p_current_node->i_maximum_son < i_current_octet){
              p_current_node->i_maximum_son = i_current_octet;
            }
          }
          p_current_node = p_current_node->arr_p_sons[i_current_octet];
        }
        else{
          i_error_code = I_ERROR__LINE_MUST_CONTINUE_WITH_HEXADECIMAL_DIGIT__EVEN_NUMBER_OF_DIGITS__INPUT_TUPLE;
        }
      break;

      case 2://just read input tuple even hexadecimal digit
        if(c == ' '){
          i_current_read_state = 3;
        }
        else if(c >= '0' && c <= '9'){
          i_current_read_state = 1;
          i_current_octet = (((unsigned char)c) - '0') * 16;
        }
        else if(c >= 'a' && c <= 'f'){
          i_current_read_state = 1;
          i_current_octet = (((unsigned char)c) - 'a' + 10) * 16;
        }
        else{
          i_error_code = I_ERROR__LINE_MUST_CONTINUE_WITH_HEXADECIMAL_DIGIT_OR_SPACE;
        }
      break;

      case 3://just read space after input tuple
        if(c == 'i'){
          i_current_read_state = 4;
          p_current_node->i_status = I_STATUS__VALID;
        }
        else if(c >= '0' && c <= '9'){
          i_current_read_state = 5;
          i_current_octet = (((unsigned char)c) - '0') * 16;
        }
        else if(c >= 'a' && c <= 'f'){
          i_current_read_state = 5;
          i_current_octet = (((unsigned char)c) - 'a' + 10) * 16;
        }
        else if(c == '-'){
          i_current_read_state = 7;
          i_status_for_node = 0;
        }
        else{
          i_error_code = I_ERROR__LINE_MUST_CONTINUE_WITH_HEXADECIMAL_DIGIT_OR_SPACE;
        }
      break;

      case 4://just read 'i'(gnore)
        if(c == '\n'){
          i_current_read_state = 0;
          p_current_node = p_root_node;
          ++(*p_i_current_line);
          *p_i_current_column = 0;
        }
        else{
          i_error_code = I_ERROR__LINE_RETURN_EXPECTED;
        }
      break;

      case 5://just read output tuple odd hexadecimal digit
        if((c >= '0' && c <= '9')
          || (c >= 'a' && c <= 'f')
        ){
          i_current_read_state = 6;
          if(c >= '0' && c <= '9'){
            i_current_octet += ((unsigned char)c) - '0';
          }
          else{
            i_current_octet += ((unsigned char)c) - 'a' + 10;
          }
          if(p_current_node->i_transliteration_size == 0){
            p_current_node->s_transliteration = (unsigned char*) calloc(1, sizeof(unsigned char));
            if(p_current_node->s_transliteration == NULL){
              i_error_code = I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
              break;
            }
            p_current_node->i_allocated_size = 1;
          }
          else if(p_current_node->i_allocated_size == p_current_node->i_transliteration_size){
            if(p_current_node->i_allocated_size == 2^32 - 1){
              i_error_code = I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
              break;
            }
            if(p_current_node->i_allocated_size > 2^30){
              i_size_for_realloc = 2^32 - 1;
            }
            else{
              i_size_for_realloc = p_current_node->i_allocated_size * 2;
            }
            p_for_realloc = realloc(p_current_node->s_transliteration, i_size_for_realloc);
            if(p_for_realloc == NULL){
              i_error_code = I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
              break;
            }
            p_current_node->i_allocated_size = (unsigned long) i_size_for_realloc;
            p_current_node->s_transliteration = (unsigned char*) p_for_realloc;
          }
          p_current_node->s_transliteration[p_current_node->i_transliteration_size++] = i_current_octet;
        }
        else{
          i_error_code = I_ERROR__LINE_MUST_CONTINUE_WITH_HEXADECIMAL_DIGIT__EVEN_NUMBER_OF_DIGITS__OUTPUT_TUPLE;
        }
      break;

      case 6://just read output tuple even hexadecimal digit
        if(c == '\n'){
          p_current_node->i_status = I_STATUS__VALID;
          i_current_read_state = 0;
          p_current_node = p_root_node;
          ++(*p_i_current_line);
          *p_i_current_column = 0;
        }
        else if(c >= '0' && c <= '9'){
          i_current_read_state = 5;
          i_current_octet = (((unsigned char)c) - '0') * 16;
        }
        else if(c >= 'a' && c <= 'f'){
          i_current_read_state = 5;
          i_current_octet = (((unsigned char)c) - 'a' + 10) * 16;
        }
        else{
          i_error_code = I_ERROR__LINE_MUST_CONTINUE_WITH_HEXADECIMAL_DIGIT_OR_A_LINE_RETURN;
        }
      break;

      case 7://just read '-'
        if(c >= '1' && c <= '9'){
          i_current_read_state = 8;
          i_status_for_node = c - '0';
        }
        else{
          i_error_code = I_ERROR__LINE_MUST_CONTINUE_WITH_DECIMAL_DIGIT;
        }
      break;

      case 8://just read a decimal digit for error code
        if(c == '\n'){
          p_current_node->i_status = (int) -i_status_for_node;
          i_current_read_state = 0;
          p_current_node = p_root_node;
          ++(*p_i_current_line);
          *p_i_current_column = 0;
        }
        else if(c >= '0' && c <= '9'){
          i_status_for_node *= 10;
          i_status_for_node += c - '0';
          if(-i_status_for_node < I_MINIMUM_USER_DEFINED_ERROR_CODE){
            i_error_code = I_ERROR__THE_ERROR_CODE_VALUE_IS_TOO_NEGATIVE;
            break;
          }
        }
        else{
          i_error_code = I_ERROR__LINE_RETURN_EXPECTED;
        }
      break;
    }//end switch(i_current_read_state)
    if(i_error_code != 0){
      break;
    }
  }//end while((c = getc(file)) != EOF)

  if(i_current_read_state == -1){
    i_error_code = I_ERROR__EMPTY_PROFILE;
  }
  fclose(file);
  if(i_error_code != 0){
    transliteration_profile_free(*p_p_transliteration_profile);
    *p_p_transliteration_profile = NULL;
    return i_error_code;
  }
  return 0;
}//end function transliteration_profile_load_from_text()



/**
 * Transliteration profile management
 * Load a transliteration profile from a binary file
 */
int transliteration_profile_load_from_bin(
  char* s_filename,
  t_transliteration_profile** p_p_transliteration_profile,
  size_t* p_i_current_offset
){
  #ifdef DEBUG_TRANSLITERATION_PROFILE
  printf(
      "Call: transliteration_profile_load_from_bin() %s %d %d\n",
      s_filename,
      p_p_transliteration_profile,
      p_i_current_offset
  );
  #endif
  return I_ERROR__NOT_YET_CODED;
}//end function transliteration_profile_load_from_bin()



/**
 * Transliteration profile management
 * Performs a mathematical composition of two transliteration profiles (maps, see README)
 */
int transliteration_profile_compose(
  t_transliteration_profile* p_transliteration_profile_1,
  t_transliteration_profile* p_transliteration_profile_2,
  t_transliteration_profile** p_p_transliteration_profile_result
){
  #ifdef DEBUG_TRANSLITERATION_PROFILE
  printf(
      "Call: transliteration_profile_compose() %d %d %d\n",
      p_transliteration_profile_1,
      p_transliteration_profile_2,
      p_p_transliteration_profile_result
  );
  #endif
  return I_ERROR__NOT_YET_CODED;
}//end function transliteration_profile_compose()



/**
 * Transliteration profile management
 * Dump a transliteration profile to a text file
 */
int transliteration_profile_dump_to_text(
  char* s_filename,
  t_transliteration_profile* p_transliteration_profile
){
  #ifdef DEBUG_TRANSLITERATION_PROFILE
  printf(
      "Call: transliteration_profile_dump_to_text() %s %d\n",
      s_filename,
      p_transliteration_profile
  );
  #endif
  switch(p_transliteration_profile->i_profile_type){
    case I_PROFILE_TYPE__RAW:
    return transliteration_profile_dump_to_text__raw(s_filename, p_transliteration_profile);

    case I_PROFILE_TYPE__SHRINK1:
    return transliteration_profile_dump_to_text__shrink1(s_filename, p_transliteration_profile);
  }
}//end function transliteration_profile_dump_to_text()



/**
 * Transliteration profile management
 * Dump a transliteration profile to a binary file
 */
int transliteration_profile_dump_to_bin(
  char* s_filename,
  t_transliteration_profile* p_transliteration_profile
){
  #ifdef DEBUG_TRANSLITERATION_PROFILE
  printf(
      "Call: transliteration_profile_dump_to_bin() %s %d\n",
      s_filename,
      p_transliteration_profile
  );
  #endif
  return I_ERROR__NOT_YET_CODED;
}//end function transliteration_profile_dump_to_bin()



/**
 * Transliteration profile management
 * Free the memory of a transliteration profile
 */
void transliteration_profile_free(t_transliteration_profile* p_transliteration_profile){

  #ifdef DEBUG_TRANSLITERATION_PROFILE
  printf(
      "Call: transliteration_profile_free() %d\n",
      p_transliteration_profile
  );
  #endif

  //nested function for prefix traversal
  int free_node_prefix(t_transliteration_node* p_transliteration_node){
    #ifdef DEBUG_TRANSLITERATION_PROFILE
    printf(
        "Call: free_node_prefix() %d\n",
        p_transliteration_node
    );
    #endif
    free(p_transliteration_node->s_transliteration);
    return 0;
  }

  //nested function for postfix traversal
  int free_node_postfix(t_transliteration_node* p_transliteration_node){
    #ifdef DEBUG_TRANSLITERATION_PROFILE
    printf(
        "Call: free_node_prefix() %d\n",
        free_node_postfix
    );
    #endif
    free(p_transliteration_node->arr_p_sons);
    free(p_transliteration_node);
    return 0;
  }

  transliteration_profile_traversal(
      p_transliteration_profile,
      (&free_node_prefix),
      (&free_node_postfix)
  );
  free(p_transliteration_profile);
}//end function transliteration_profile_free()



/**
 * Transliteration profile use
 *
 */
int transliteration_profile_iconv(
  t_transliteration_profile* p_transliteration_profile,
  char* s_input_string,
  size_t i_size_input_string,
  char* s_output_string,
  size_t* p_i_size_output_string
){
  #ifdef DEBUG_TRANSLITERATION_PROFILE
  printf(
      "Call: transliteration_profile_iconv() %d %d %d %d %d\n",
      p_transliteration_profile,
      (long long int)s_input_string,
      i_size_input_string,
      (long long int)s_output_string,
      p_i_size_output_string
  );
  #endif
  return I_ERROR__NOT_YET_CODED;
}//end function transliteration_profile_iconv()




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
){
  #ifdef DEBUG_TRANSLITERATION_PROFILE
  printf(
      "Call: transliteration_profile_traversal() %d %d %d\n",
      p_transliteration_profile,
      p_function_prefix,
      p_function_postfix
  );
  #endif

  switch(p_transliteration_profile->i_profile_type){
    case I_PROFILE_TYPE__RAW:
    return transliteration_profile_traversal__raw_node(
        p_transliteration_profile->p_root_node,
        p_function_prefix,
        p_function_postfix
    );

    case I_PROFILE_TYPE__SHRINK1:
    return transliteration_profile_traversal__shrink1_node(
        p_transliteration_profile->p_root_node,
        p_function_prefix,
        p_function_postfix
    );

    default:
    return I_ERROR__UNKNOWN_PROFILE_TYPE;
  }
}//end function transliteration_profile_traversal()



/**
 * Transliteration profile management
 * Traverse a raw transliteration profile from the current node
 */
int transliteration_profile_traversal__raw_node(
  t_transliteration_node* p_transliteration_node,
  int (*p_function_prefix) (t_transliteration_node*),
  int (*p_function_postfix) (t_transliteration_node*)
){
  int i_result;

  #ifdef DEBUG_TRANSLITERATION_PROFILE
  printf(
      "Call: transliteration_profile_traversal__raw_node() %d %d %d\n",
      p_transliteration_node,
      p_function_prefix,
      p_function_postfix
  );
  #endif

  i_result = (*p_function_prefix)(p_transliteration_node);
  if(i_result != 0){
    return i_result;
  }
  for(int i = 0; i < 256; ++i){
    if(p_transliteration_node->arr_p_sons[i] != NULL){
      i_result = transliteration_profile_traversal__raw_node(
          p_transliteration_node->arr_p_sons[i],
          p_function_prefix,
          p_function_postfix
      );
      if(i_result != 0){
        return i_result;
      }
    }
  }
  i_result = (*p_function_postfix)(p_transliteration_node);
  return i_result;
}



/**
 * Transliteration profile management
 * Traverse a shrinked transliteration profile from the current node
 */
int transliteration_profile_traversal__shrink1_node(
  t_transliteration_node* p_transliteration_node,
  int (*p_function_prefix) (t_transliteration_node*),
  int (*p_function_postfix) (t_transliteration_node*)
){

  #ifdef DEBUG_TRANSLITERATION_PROFILE
  printf(
      "Call: transliteration_profile_traversal__shrink1_node() %d %d %d\n",
      p_transliteration_node,
      p_function_prefix,
      p_function_postfix
  );
  #endif

  return I_ERROR__NOT_YET_CODED;
}




/**
 * Transliteration profile management
 * Dump a raw transliteration profile to a text file
 */
int transliteration_profile_dump_to_text__raw(
  char* s_filename,
  t_transliteration_profile* p_transliteration_profile
){
  FILE* file = NULL;
  int* arr_prefix = NULL;
  t_transliteration_node** arr_p_ascendants = NULL;
  size_t i_allocated_depth = 0;
  size_t i_current_depth = 0;
  void* p_for_realloc = NULL;
  size_t i_size_for_realloc = 0;
  t_transliteration_node* p_current_node;
  int i_error_code = 0;

  #ifdef DEBUG_TRANSLITERATION_PROFILE
  printf(
      "Call: transliteration_profile_dump_to_text__raw() %s %d\n",
      s_filename,
      p_transliteration_profile
  );
  #endif

  file = fopen(s_filename, "w");
  if(file == NULL){
    return I_ERROR__COULD_NOT_OPEN_FILE;
  }

  arr_prefix = (int*) calloc(8, sizeof(int));
  if(arr_prefix == NULL){
    fclose(file);
    return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
  }

  arr_p_ascendants = (t_transliteration_node**) calloc(8, sizeof(t_transliteration_node*));
  if(arr_p_ascendants == NULL){
    fclose(file);
    free(arr_prefix);
    return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
  }
  i_allocated_depth = 8;

  p_current_node = p_transliteration_profile->p_root_node;
  arr_p_ascendants[0] = p_current_node;
  arr_prefix[0] = 0;

  while(p_current_node != NULL){
    if(arr_prefix[i_current_depth] > 255){
      //we're done with the sons

      if(i_current_depth == 0){
        //we're done
        break;
      }

      //we print a line for the current node if needed
      if(p_current_node->i_status < 0
        || p_current_node->i_status == I_STATUS__VALID
      ){
        //we print the prefix
        for(size_t i = 0; i < i_current_depth; ++i){
          if(fprintf(file, "%02x", arr_prefix[i]) != 0){
            i_error_code = I_ERROR__COULD_NOT_WRITE_CHARACTER;
            break;
          }
        }
        if(i_error_code != 0){
          break;
        }

        //we print a space
        if(fputc(' ', file) == EOF){
          i_error_code = I_ERROR__COULD_NOT_WRITE_CHARACTER;
          break;
        }

        //we print the transliteration
        if(p_current_node->i_status < 0){
          if(fprintf(file, "%d", p_current_node->i_status) != 0){
            i_error_code = I_ERROR__COULD_NOT_WRITE_CHARACTER;
            break;
          }
        }
        else{//if p_current_node->i_status == I_STATUS__VALID
          if(p_current_node->i_transliteration_size == 0){
            if(fputc('i', file) == EOF){
              i_error_code = I_ERROR__COULD_NOT_WRITE_CHARACTER;
              break;
            }
          }
          else{
            for(unsigned long i = 0; i < p_current_node->i_transliteration_size; ++i){
              if(fprintf(file, "%02x", p_current_node->s_transliteration[i]) != 0){
                i_error_code = I_ERROR__COULD_NOT_WRITE_CHARACTER;
                break;
              }
            }
            if(i_error_code != 0){
              break;
            }
          }
        }
        if(fputc('\n', file) == EOF){
          i_error_code = I_ERROR__COULD_NOT_WRITE_CHARACTER;
          break;
        }
      }
      //go back one level
      --i_current_depth;
      ++arr_prefix[i_current_depth];
      p_current_node = arr_p_ascendants[i_current_depth];
      continue;
    }//end if(arr_prefix[i_current_depth] > 255)

    if(p_current_node->arr_p_sons[arr_prefix[i_current_depth]] != NULL){
      //go forward one level
      //allocate if needed
      if(i_current_depth == i_allocated_depth - 1){
        if(i_allocated_depth * sizeof(t_transliteration_node*) > SIZE_MAX / 2
          //|| i_allocated_depth * sizeof(int) > SIZE_MAX / 2
        ){
          i_error_code = I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
          break;
        }

        i_size_for_realloc = i_allocated_depth * sizeof(int) * 2;
        p_for_realloc = realloc(arr_prefix, i_size_for_realloc);
        if(p_for_realloc == NULL){
          i_error_code = I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
          break;
        }
        arr_prefix = (int*) p_for_realloc;

        i_size_for_realloc = i_allocated_depth * sizeof(t_transliteration_node*) * 2;
        p_for_realloc = realloc(arr_p_ascendants, i_size_for_realloc);
        if(p_for_realloc == NULL){
          i_error_code = I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
          break;
        }
        arr_p_ascendants = (t_transliteration_node**) p_for_realloc;
      }

      //go forward one level
      p_current_node = p_current_node->arr_p_sons[arr_prefix[i_current_depth]];
      ++i_current_depth;
      arr_prefix[i_current_depth] = 0;
      arr_p_ascendants[i_current_depth] = p_current_node;
      continue;
    }//end if(p_current_node->arr_p_sons[arr_prefix[i_current_depth]] != NULL)

    ++arr_prefix[i_current_depth];
  }
  while(0);

  fclose(file);
  free(arr_prefix);
  free(arr_p_ascendants);
  return i_error_code;
}//end function transliteration_profile_dump_to_text__raw()



/**
 * Transliteration profile management
 * Dump a shrinked transliteration profile to a text file (shrink1)
 */
int transliteration_profile_dump_to_text__shrink1(
  char* s_filename,
  t_transliteration_profile* p_transliteration_profile
){
 
  #ifdef DEBUG_TRANSLITERATION_PROFILE
  printf(
      "Call: transliteration_profile_dump_to_text__shrink1() %s %d\n",
      s_filename,
      p_transliteration_profile
  );
  #endif

  return I_ERROR__NOT_YET_CODED;
}//end function transliteration_profile_dump_to_text__shrink1()



