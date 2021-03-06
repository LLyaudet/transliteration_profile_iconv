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
  size_t i_number_of_elements_for_realloc = 0;

  #ifdef DEBUG_TRANSLITERATION_PROFILE
  printf(
      "Call: transliteration_profile_load_from_text() %s %p %p %p\n",
      s_filename,
      (void*)p_p_transliteration_profile,
      (void*)p_i_current_line,
      (void*)p_i_current_column
  );
  #endif

  *p_i_current_line = 0;
  *p_i_current_column = 0;

  //first we allocate the transliteration profile
  *p_p_transliteration_profile = (t_transliteration_profile*) calloc(
      1,
      sizeof(t_transliteration_profile)
  );
  if(*p_p_transliteration_profile == NULL){
    return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
  }
  (*p_p_transliteration_profile)->i_profile_type = I_PROFILE_TYPE__RAW;
  (*p_p_transliteration_profile)->i_number_of_nodes = 0;
  (*p_p_transliteration_profile)->i_max_depth = 0;
  (*p_p_transliteration_profile)->p_root_node = NULL;

  //and we allocate the root node
  (*p_p_transliteration_profile)->p_root_node = (t_transliteration_node*) calloc(
      1,
      sizeof(t_transliteration_node)
  );
  if((*p_p_transliteration_profile)->p_root_node == NULL){
    free(*p_p_transliteration_profile);
    *p_p_transliteration_profile = NULL;
    return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
  }
  p_root_node = (*p_p_transliteration_profile)->p_root_node;
  p_current_node = p_root_node;
  ++((*p_p_transliteration_profile)->i_number_of_nodes);
  p_current_node->i_node_index = (*p_p_transliteration_profile)->i_number_of_nodes;
  p_current_node->i_minimum_son = I_MAXIMUM_SON;
  p_current_node->i_maximum_son = 0;
  p_current_node->arr_p_sons = NULL;
  p_current_node->i_status = I_STATUS__SKIP;
  p_current_node->i_transliteration_size = 0;
  p_current_node->i_allocated_size = 0;
  p_current_node->s_transliteration = NULL;
  //with its array of sons
  p_current_node->arr_p_sons = (t_transliteration_node**) calloc(
      I_MAXIMUM_SON + 1,
      sizeof(t_transliteration_node*)
  );
  if(p_current_node->arr_p_sons == NULL){
    transliteration_profile_free(*p_p_transliteration_profile);
    *p_p_transliteration_profile = NULL;
    return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
  }
  for(int i = 0; i <= I_MAXIMUM_SON; ++i){
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
    printf("line %zu, column %zu, read state %d\n", *p_i_current_line, *p_i_current_column, i_current_read_state);
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
            //we allocate the new node
            p_new_node = (t_transliteration_node*) calloc(
                1,
                sizeof(t_transliteration_node)
            );
            if(p_new_node == NULL){
              i_error_code = I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
              break;
            }
            ++((*p_p_transliteration_profile)->i_number_of_nodes);
            p_new_node->i_node_index = (*p_p_transliteration_profile)->i_number_of_nodes;
            p_new_node->i_minimum_son = I_MAXIMUM_SON;
            p_new_node->i_maximum_son = 0;
            p_new_node->arr_p_sons = NULL;
            p_new_node->i_status = I_STATUS__SKIP;
            p_new_node->i_transliteration_size = 0;
            p_new_node->i_allocated_size = 0;
            p_new_node->s_transliteration = NULL;
            //with its array of sons
            p_new_node->arr_p_sons = (t_transliteration_node**) calloc(
                I_MAXIMUM_SON + 1,
                sizeof(t_transliteration_node*)
            );
            if(p_new_node->arr_p_sons == NULL){
              i_error_code = I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
              break;
            }
            for(int i = 0; i <= I_MAXIMUM_SON; ++i){
              p_new_node->arr_p_sons[i] = NULL;
            }
            p_current_node->arr_p_sons[i_current_octet] = p_new_node;
            if(p_current_node->i_minimum_son > i_current_octet){
              p_current_node->i_minimum_son = i_current_octet;
            }
            if(p_current_node->i_maximum_son < i_current_octet){
              p_current_node->i_maximum_son = i_current_octet;
            }
            if((*p_i_current_column / 2) > (*p_p_transliteration_profile)->i_max_depth){
              (*p_p_transliteration_profile)->i_max_depth = *p_i_current_column / 2;
            }
          }//end if(p_current_node->arr_p_sons[i_current_octet] == NULL)
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
          i_error_code = I_ERROR__LINE_MUST_CONTINUE_WITH_HEXADECIMAL_DIGIT_OR_I_OR_MINUS;
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
            if(p_current_node->i_allocated_size == I_MAXIMUM_LENGTH_OF_TRANSLITERATION_PER_CHARACTER){
              i_error_code = I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
              break;
            }
            if(p_current_node->i_allocated_size > 1 << 30){
              i_number_of_elements_for_realloc = I_MAXIMUM_LENGTH_OF_TRANSLITERATION_PER_CHARACTER;
            }
            else{
              i_number_of_elements_for_realloc = p_current_node->i_allocated_size << 1;
            }
            if(i_number_of_elements_for_realloc <= p_current_node->i_allocated_size
              || i_number_of_elements_for_realloc * sizeof(unsigned char) < i_number_of_elements_for_realloc
            ){
              i_error_code = I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
              break;
            }
            p_for_realloc = realloc(
                p_current_node->s_transliteration,
                i_number_of_elements_for_realloc * sizeof(unsigned char)
            );
            if(p_for_realloc == NULL){
              i_error_code = I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
              break;
            }
            p_current_node->i_allocated_size = i_number_of_elements_for_realloc;
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
          i_error_code = I_ERROR__LINE_MUST_CONTINUE_WITH_DECIMAL_DIGIT_OR_LINE_RETURN;
        }
      break;

      default:
        i_error_code = I_ERROR__UNKNOWN_READ_STATE;
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
  FILE * file = NULL;
  int i_error_code = 0;
  t_transliteration_node* p_root_node;
  t_transliteration_node* p_current_node;
  size_t* depths = NULL;
  int i_number_of_sons = 0;
  size_t i_node_index = 0;

  #ifdef DEBUG_TRANSLITERATION_PROFILE
  printf(
      "Call: transliteration_profile_load_from_bin() %s %p %p\n",
      s_filename,
      (void*)p_p_transliteration_profile,
      (void*)p_i_current_offset
  );
  #endif

  *p_i_current_offset = 0;
  *p_p_transliteration_profile = NULL;

  file = fopen(s_filename, "r");
  if(file == NULL){
    return I_ERROR__COULD_NOT_OPEN_FILE;
  }

  //first we allocate the transliteration profile
  *p_p_transliteration_profile = (t_transliteration_profile*) calloc(1, sizeof(t_transliteration_profile));
  if(*p_p_transliteration_profile == NULL){
    fclose(file);
    return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
  }
  (*p_p_transliteration_profile)->i_profile_type = I_PROFILE_TYPE__SHRINK1;
  (*p_p_transliteration_profile)->i_number_of_nodes = 0;//will be read
  (*p_p_transliteration_profile)->i_max_depth = 0;//will be computed anew
  (*p_p_transliteration_profile)->p_root_node = NULL;

  //and we read the corresponding data in the binary profile
  if(fread(&((*p_p_transliteration_profile)->i_number_of_nodes), sizeof(size_t), 1, file) != 1){
    *p_i_current_offset = ftell(file);
    fclose(file);
    free(*p_p_transliteration_profile);
    *p_p_transliteration_profile = NULL;
    return I_ERROR__COULD_NOT_READ_NUMBER_OF_NODES;
  }

  //We allocate all the nodes
  (*p_p_transliteration_profile)->p_root_node = (t_transliteration_node*) calloc(
      (*p_p_transliteration_profile)->i_number_of_nodes,
      sizeof(t_transliteration_node)
  );
  if((*p_p_transliteration_profile)->p_root_node == NULL){
    *p_i_current_offset = ftell(file);
    fclose(file);
    free(*p_p_transliteration_profile);
    *p_p_transliteration_profile = NULL;
    return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
  }

  p_root_node = (*p_p_transliteration_profile)->p_root_node;
  //we set the pointers so that everything is cleanly freed if something goes wrong later
  for(size_t i = 0; i < (*p_p_transliteration_profile)->i_number_of_nodes; ++i){
    p_current_node = &(p_root_node[i]);
    p_current_node->arr_p_sons = NULL;
    p_current_node->s_transliteration = NULL;
  }

  //We allocate the depths
  depths = (size_t*) calloc(
      (*p_p_transliteration_profile)->i_number_of_nodes,
      sizeof(size_t)
  );
  if(depths == NULL){
    *p_i_current_offset = ftell(file);
    fclose(file);
    transliteration_profile_free(*p_p_transliteration_profile);
    *p_p_transliteration_profile = NULL;
    return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
  }

  //The main loop on the nodes
  for(size_t i = 0; i < (*p_p_transliteration_profile)->i_number_of_nodes; ++i){
    p_current_node = &(p_root_node[i]);
    p_current_node->i_node_index = i + 1;
    if(fread(&(p_current_node->i_minimum_son), sizeof(unsigned char), 1, file) != 1){
      i_error_code =  I_ERROR__COULD_NOT_READ_MINIMUM_SON;
      break;
    }
    if(fread(&(p_current_node->i_maximum_son), sizeof(unsigned char), 1, file) != 1){
      i_error_code =  I_ERROR__COULD_NOT_READ_MAXIMUM_SON;
      break;
    }
    i_number_of_sons = p_current_node->i_maximum_son - p_current_node->i_minimum_son + 1;
    if(fread(&(p_current_node->i_status), sizeof(int16_t), 1, file) != 1){
      i_error_code =  I_ERROR__COULD_NOT_READ_STATUS;
      break;
    }
    if(fread(&(p_current_node->i_transliteration_size), sizeof(size_t), 1, file) != 1){
      i_error_code =  I_ERROR__COULD_NOT_READ_TRANSLITERATION_SIZE;
      break;
    }
    p_current_node->i_allocated_size = p_current_node->i_transliteration_size;
    //we check that the root node is "skip"
    if(i == 0
      && (p_current_node->i_transliteration_size > 0
         || p_current_node->i_status != I_STATUS__SKIP)
    ){
      i_error_code =  I_ERROR__ROOT_NODE_SHOULD_HAVE_SKIP_STATUS_AND_NO_TRANSLITERATION;
      break;
    }

    if(i_number_of_sons >= 1){
      //We allocate its array of sons
      p_current_node->arr_p_sons = (t_transliteration_node**) calloc(
          i_number_of_sons,
          sizeof(t_transliteration_node*)
      );
      if(p_current_node->arr_p_sons == NULL){
        i_error_code = I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
        break;
      }
      for(int j = 0; j < i_number_of_sons; ++j){
        p_current_node->arr_p_sons[j] = NULL;
      }
      //We read the file to know the index of sons
      for(int j = 0; j < i_number_of_sons; ++j){
        if(fread(&i_node_index, sizeof(size_t), 1, file) != 1){
          i_error_code =  I_ERROR__COULD_NOT_READ_SON_INDEX;
          break;
        }
        if(i_node_index == 0){
          //when there is no son for some octet i_node_index will be 0
          continue;
        }
        if(i_node_index <= i + 1){
          i_error_code =  I_ERROR__SON_INDEX_MUST_BE_AFTER;
          break;
        }
        if(i_node_index > (*p_p_transliteration_profile)->i_number_of_nodes){
          i_error_code =  I_ERROR__SON_INDEX_MUST_BE_AT_MOST_THE_NUMBER_OF_NODES;
          break;
        }
        p_current_node->arr_p_sons[j] = &(p_root_node[i_node_index - 1]);
        if(depths[i_node_index - 1] < depths[i] + 1){
          depths[i_node_index - 1] = depths[i] + 1;
        }
      }//end for(int j = 0; j < i_number_of_sons; ++j)
      if(i_error_code != 0){
        break;
      }
    }//end if(i_number_of_sons >= 1)

    if(p_current_node->i_transliteration_size > 0){
      //We allocate the string
      p_current_node->s_transliteration = (unsigned char*) calloc(
          p_current_node->i_transliteration_size,
          sizeof(unsigned char)
      );
      if(p_current_node->s_transliteration == NULL){
        i_error_code = I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
        break;
      }
      //and we read it
      if(fread(
             p_current_node->s_transliteration,
             sizeof(unsigned char),
             p_current_node->i_transliteration_size,
             file
         ) != p_current_node->i_transliteration_size
      ){
        i_error_code =  I_ERROR__COULD_NOT_READ_TRANSLITERATION;
        break;
      }
    }//end if(p_current_node->i_transliteration_size > 0)
  }//end for(size_t i = 0; i < (*p_p_transliteration_profile)->i_number_of_nodes; ++i)

  //We get the maximum depth
  (*p_p_transliteration_profile)->i_max_depth = 0;
  for(size_t i = 0; i < (*p_p_transliteration_profile)->i_number_of_nodes; ++i){
    if((*p_p_transliteration_profile)->i_max_depth < depths[i]){
      (*p_p_transliteration_profile)->i_max_depth = depths[i];
    }
  }

  *p_i_current_offset = ftell(file);
  fclose(file);
  free(depths);
  if(i_error_code != 0){
    transliteration_profile_free(*p_p_transliteration_profile);
    *p_p_transliteration_profile = NULL;
  }

  return i_error_code;
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
      "Call: transliteration_profile_compose() %p %p %p\n",
      (void*)p_transliteration_profile_1,
      (void*)p_transliteration_profile_2,
      (void*)p_p_transliteration_profile_result
  );
  #endif
  switch(p_transliteration_profile_1->i_profile_type){
    case I_PROFILE_TYPE__RAW:
    return transliteration_profile_compose__raw(
        p_transliteration_profile_1,
        p_transliteration_profile_2,
        p_p_transliteration_profile_result
    );

    case I_PROFILE_TYPE__SHRINK1:
    return transliteration_profile_compose__shrink1(
        p_transliteration_profile_1,
        p_transliteration_profile_2,
        p_p_transliteration_profile_result
    );

    default:
    return I_ERROR__UNKNOWN_PROFILE_TYPE;
  }
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
      "Call: transliteration_profile_dump_to_text() %s %p\n",
      s_filename,
      (void*)p_transliteration_profile
  );
  #endif
  switch(p_transliteration_profile->i_profile_type){
    case I_PROFILE_TYPE__RAW:
    return transliteration_profile_dump_to_text__raw(s_filename, p_transliteration_profile);

    case I_PROFILE_TYPE__SHRINK1:
    return transliteration_profile_dump_to_text__shrink1(s_filename, p_transliteration_profile);

    default:
    return I_ERROR__UNKNOWN_PROFILE_TYPE;
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

  t_transliteration_profile* p_transliteration_profile_for_dump = NULL;
  int i_error_code = 0;
  t_transliteration_node* p_root_node = NULL;
  t_transliteration_node* p_current_node = NULL;
  FILE* file = NULL;
  int i_number_of_sons = 0;
  size_t i_node_index = 0;

  #ifdef DEBUG_TRANSLITERATION_PROFILE
  printf(
      "Call: transliteration_profile_dump_to_bin() %s %p\n",
      s_filename,
      (void*)p_transliteration_profile
  );
  #endif

  switch(p_transliteration_profile->i_profile_type){
    case I_PROFILE_TYPE__RAW:
      i_error_code = transliteration_profile_from_raw_to_shrink1(
          p_transliteration_profile,
          &p_transliteration_profile_for_dump
      );
      if(i_error_code != 0){
        return i_error_code;
      }
    break;

    case I_PROFILE_TYPE__SHRINK1:
      p_transliteration_profile_for_dump = p_transliteration_profile;
    break;

    default:
    return I_ERROR__UNKNOWN_PROFILE_TYPE;
  }

  file = fopen(s_filename, "w");
  if(file == NULL){
    if(p_transliteration_profile->i_profile_type == I_PROFILE_TYPE__RAW){
      transliteration_profile_free(p_transliteration_profile_for_dump);
    }
    return I_ERROR__COULD_NOT_OPEN_FILE;
  }

  p_root_node = p_transliteration_profile_for_dump->p_root_node;

  do{
    if(fwrite(&(p_transliteration_profile_for_dump->i_number_of_nodes), sizeof(size_t), 1, file) != 1){
      i_error_code = I_ERROR__COULD_NOT_WRITE_DATA;
      break;
    }

    //The main loop on the nodes
    for(size_t i = 0; i < p_transliteration_profile_for_dump->i_number_of_nodes; ++i){
      p_current_node = &(p_root_node[i]);

      if(fwrite(&(p_current_node->i_minimum_son), sizeof(unsigned char), 1, file) != 1){
        i_error_code =  I_ERROR__COULD_NOT_WRITE_DATA;
        break;
      }
      if(fwrite(&(p_current_node->i_maximum_son), sizeof(unsigned char), 1, file) != 1){
        i_error_code =  I_ERROR__COULD_NOT_WRITE_DATA;
        break;
      }
      if(fwrite(&(p_current_node->i_status), sizeof(int16_t), 1, file) != 1){
        i_error_code =  I_ERROR__COULD_NOT_WRITE_DATA;
        break;
      }
      if(fwrite(&(p_current_node->i_transliteration_size), sizeof(size_t), 1, file) != 1){
        i_error_code =  I_ERROR__COULD_NOT_WRITE_DATA;
        break;
      }

      i_number_of_sons = p_current_node->i_maximum_son - p_current_node->i_minimum_son + 1;
      //if(i_number_of_sons >= 1){
        for(int j = 0; j < i_number_of_sons; ++j){
          i_node_index = 0;
          if(p_current_node->arr_p_sons[j] != NULL){
            i_node_index = p_current_node->arr_p_sons[j]->i_node_index;
          }
          if(fwrite(&i_node_index, sizeof(size_t), 1, file) != 1){
            i_error_code =  I_ERROR__COULD_NOT_WRITE_DATA;
            break;
          }
        }//end for(int j = 0; j < i_number_of_sons; ++j)
        if(i_error_code != 0){
          break;
        }
      //}//end if(i_number_of_sons >= 1)

      if(p_current_node->i_transliteration_size > 0){
        if(fwrite(
               p_current_node->s_transliteration,
               sizeof(unsigned char),
               p_current_node->i_transliteration_size,
               file
           ) != p_current_node->i_transliteration_size
        ){
          i_error_code =  I_ERROR__COULD_NOT_WRITE_DATA;
          break;
        }
      }//end if(p_current_node->i_transliteration_size > 0)
    }//end for(size_t i = 0; i < p_transliteration_profile_for_dump->i_number_of_nodes; ++i)
  }
  while(0);

  fclose(file);
  if(p_transliteration_profile->i_profile_type == I_PROFILE_TYPE__RAW){
    transliteration_profile_free(p_transliteration_profile_for_dump);
  }
  return i_error_code;
}//end function transliteration_profile_dump_to_bin()



/**
 * Transliteration profile management
 * Free the memory of a transliteration profile
 */
void transliteration_profile_free(t_transliteration_profile* p_transliteration_profile){

  t_transliteration_node* p_current_node;

  #ifdef DEBUG_TRANSLITERATION_PROFILE
  printf(
      "Call: transliteration_profile_free() %p\n",
      (void*)p_transliteration_profile
  );
  #endif

  //nested function for prefix traversal
  int free_node_prefix(t_transliteration_node* p_transliteration_node){
    #ifdef DEBUG_TRANSLITERATION_PROFILE
    printf(
        "Call: free_node_prefix() %p\n",
        (void*)p_transliteration_node
    );
    #endif
    if(p_transliteration_node->s_transliteration != NULL){
      free(p_transliteration_node->s_transliteration);
      p_transliteration_node->s_transliteration = NULL;
    }
    return 0;
  }//end function free_node_prefix()

  //nested function for postfix traversal
  int free_node_postfix(t_transliteration_node* p_transliteration_node){
    #ifdef DEBUG_TRANSLITERATION_PROFILE
    printf(
        "Call: free_node_postfix() %p\n",
        (void*)p_transliteration_node
    );
    #endif
    if(p_transliteration_node->arr_p_sons != NULL){
      free(p_transliteration_node->arr_p_sons);
      p_transliteration_node->arr_p_sons = NULL;
    }
    free(p_transliteration_node);
    return 0;
  }//end function free_node_postfix()

  switch(p_transliteration_profile->i_profile_type){
    case I_PROFILE_TYPE__RAW:
      transliteration_profile_traversal(
          p_transliteration_profile,
          (&free_node_prefix),
          (&free_node_postfix)
      );
    break;

    case I_PROFILE_TYPE__SHRINK1:
      for(size_t i = 0; i < p_transliteration_profile->i_number_of_nodes; ++i){
        p_current_node = &(p_transliteration_profile->p_root_node[i]);
        if(p_current_node->arr_p_sons != NULL){
          free(p_current_node->arr_p_sons);
          p_current_node->arr_p_sons = NULL;
        }
        if(p_current_node->s_transliteration != NULL){
          free(p_current_node->s_transliteration);
          p_current_node->s_transliteration = NULL;
        }
      }
      free(p_transliteration_profile->p_root_node);
    break;

    default:
    printf("Unknown profile type to free\n");
    //return I_ERROR__UNKNOWN_PROFILE_TYPE;
  }

  free(p_transliteration_profile);
}//end function transliteration_profile_free()



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
){
  #ifdef DEBUG_TRANSLITERATION_PROFILE
  printf(
      "Call: transliteration_profile_iconv() %p %p %zu %p %p %p\n",
      (void*)p_transliteration_profile,
      (void*)s_input_string,
      i_size_input_string,
      (void*)p_s_output_string,
      (void*)p_i_size_output_string,
      (void*)p_i_current_read_offset
  );
  #endif

  switch(p_transliteration_profile->i_profile_type){
    case I_PROFILE_TYPE__RAW:
    return transliteration_profile_iconv__raw(
        p_transliteration_profile,
        s_input_string,
        i_size_input_string,
        p_s_output_string,
        p_i_size_output_string,
        p_i_current_read_offset
    );

    case I_PROFILE_TYPE__SHRINK1:
    return transliteration_profile_iconv__shrink1(
        p_transliteration_profile,
        s_input_string,
        i_size_input_string,
        p_s_output_string,
        p_i_size_output_string,
        p_i_current_read_offset
    );

    default:
    return I_ERROR__UNKNOWN_PROFILE_TYPE;
  }

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
      "Call: transliteration_profile_traversal() %p %p %p\n",
      (void*)p_transliteration_profile,
      (void*)p_function_prefix,
      (void*)p_function_postfix
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
      "Call: transliteration_profile_traversal__raw_node() %p %p %p\n",
      (void*)p_transliteration_node,
      (void*)p_function_prefix,
      (void*)p_function_postfix
  );
  #endif

  i_result = (*p_function_prefix)(p_transliteration_node);
  if(i_result != 0){
    return i_result;
  }

  if(p_transliteration_node->arr_p_sons != NULL){
    for(int i = 0; i <= I_MAXIMUM_SON; ++i){
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
  }

  i_result = (*p_function_postfix)(p_transliteration_node);
  return i_result;
}//end function transliteration_profile_traversal__raw_node()



/**
 * Transliteration profile management
 * Traverse a shrinked transliteration profile from the current node
 */
int transliteration_profile_traversal__shrink1_node(
  t_transliteration_node* p_transliteration_node,
  int (*p_function_prefix) (t_transliteration_node*),
  int (*p_function_postfix) (t_transliteration_node*)
){
  int i_result;

  #ifdef DEBUG_TRANSLITERATION_PROFILE
  printf(
      "Call: transliteration_profile_traversal__shrink1_node() %p %p %p\n",
      (void*)p_transliteration_node,
      (void*)p_function_prefix,
      (void*)p_function_postfix
  );
  #endif

  i_result = (*p_function_prefix)(p_transliteration_node);
  if(i_result != 0){
    return i_result;
  }

  if(p_transliteration_node->arr_p_sons != NULL){
    for(
        int i = 0, i_max = p_transliteration_node->i_maximum_son - p_transliteration_node->i_minimum_son + 1;
        i < i_max;
        ++i
    ){
      if(p_transliteration_node->arr_p_sons[i] != NULL){
        i_result = transliteration_profile_traversal__shrink1_node(
            p_transliteration_node->arr_p_sons[i],
            p_function_prefix,
            p_function_postfix
        );
        if(i_result != 0){
          return i_result;
        }
      }
    }
  }

  i_result = (*p_function_postfix)(p_transliteration_node);
  return i_result;
}//end function transliteration_profile_traversal__shrink1_node()



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
  size_t i_current_depth = 0;
  t_transliteration_node* p_current_node;
  int i_error_code = 0;

  #ifdef DEBUG_TRANSLITERATION_PROFILE
  printf(
      "Call: transliteration_profile_dump_to_text__raw() %s %p\n",
      s_filename,
      (void*)p_transliteration_profile
  );
  #endif

  if(p_transliteration_profile->i_profile_type != I_PROFILE_TYPE__RAW){
    return I_ERROR__WRONG_PROFILE_TYPE;
  }

  file = fopen(s_filename, "w");
  if(file == NULL){
    return I_ERROR__COULD_NOT_OPEN_FILE;
  }

  arr_prefix = (int*) calloc(
      p_transliteration_profile->i_max_depth + 1,//root node has depth 0
      sizeof(int)
  );
  if(arr_prefix == NULL){
    fclose(file);
    return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
  }

  arr_p_ascendants = (t_transliteration_node**) calloc(
      p_transliteration_profile->i_max_depth + 1,//root node has depth 0
      sizeof(t_transliteration_node*)
  );
  if(arr_p_ascendants == NULL){
    fclose(file);
    free(arr_prefix);
    return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
  }

  p_current_node = p_transliteration_profile->p_root_node;
  arr_p_ascendants[0] = p_current_node;
  arr_prefix[0] = 0;

  while(p_current_node != NULL){
    if(arr_prefix[i_current_depth] > p_current_node->i_maximum_son){
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
          if(fprintf(file, "%02x", arr_prefix[i]) < 0){
            i_error_code = I_ERROR__COULD_NOT_WRITE_CHARACTER;
            break;
          }
        }
        if(i_error_code != 0){
          break;
        }

        //we print a space
        if(putc(' ', file) == EOF){
          i_error_code = I_ERROR__COULD_NOT_WRITE_CHARACTER;
          break;
        }

        //we print the error code
        if(p_current_node->i_status < 0){
          if(fprintf(file, "%d", p_current_node->i_status) < 0){
            i_error_code = I_ERROR__COULD_NOT_WRITE_CHARACTER;
            break;
          }
        }
        //or the transliteration
        else{//if p_current_node->i_status == I_STATUS__VALID
          if(p_current_node->i_transliteration_size == 0){
            if(putc('i', file) == EOF){
              i_error_code = I_ERROR__COULD_NOT_WRITE_CHARACTER;
              break;
            }
          }
          else{
            for(unsigned long i = 0; i < p_current_node->i_transliteration_size; ++i){
              if(fprintf(file, "%02x", p_current_node->s_transliteration[i]) < 0){
                i_error_code = I_ERROR__COULD_NOT_WRITE_CHARACTER;
                break;
              }
            }
            if(i_error_code != 0){
              break;
            }
          }
        }
        if(putc('\n', file) == EOF){
          i_error_code = I_ERROR__COULD_NOT_WRITE_CHARACTER;
          break;
        }
      }
      //go back one level
      --i_current_depth;
      ++arr_prefix[i_current_depth];
      p_current_node = arr_p_ascendants[i_current_depth];
      continue;
    }//end if(arr_prefix[i_current_depth] > p_current_node->i_maximum_son)

    if(p_current_node->arr_p_sons[arr_prefix[i_current_depth]] != NULL){
      //go forward one level
      p_current_node = p_current_node->arr_p_sons[arr_prefix[i_current_depth]];
      ++i_current_depth;
      arr_prefix[i_current_depth] = 0;
      arr_p_ascendants[i_current_depth] = p_current_node;
      continue;
    }//end if(p_current_node->arr_p_sons[arr_prefix[i_current_depth]] != NULL)

    ++arr_prefix[i_current_depth];
  }//end while(p_current_node != NULL)

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

  FILE* file = NULL;
  int* arr_prefix = NULL;
  t_transliteration_node** arr_p_ascendants = NULL;
  size_t i_current_depth = 0;
  t_transliteration_node* p_current_node;
  int i_error_code = 0;

  #ifdef DEBUG_TRANSLITERATION_PROFILE
  printf(
      "Call: transliteration_profile_dump_to_text__shrink1() %s %p\n",
      s_filename,
      (void*)p_transliteration_profile
  );
  #endif

  if(p_transliteration_profile->i_profile_type != I_PROFILE_TYPE__SHRINK1){
    return I_ERROR__WRONG_PROFILE_TYPE;
  }

  file = fopen(s_filename, "w");
  if(file == NULL){
    return I_ERROR__COULD_NOT_OPEN_FILE;
  }

  arr_prefix = (int*) calloc(
      p_transliteration_profile->i_max_depth + 1,//root node has depth 0
      sizeof(int)
  );
  if(arr_prefix == NULL){
    fclose(file);
    return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
  }

  arr_p_ascendants = (t_transliteration_node**) calloc(
      p_transliteration_profile->i_max_depth + 1,//root node has depth 0
      sizeof(t_transliteration_node*)
  );
  if(arr_p_ascendants == NULL){
    fclose(file);
    free(arr_prefix);
    return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
  }

  p_current_node = p_transliteration_profile->p_root_node;
  arr_p_ascendants[0] = p_current_node;
  arr_prefix[0] = 0;

  while(p_current_node != NULL){
    if(arr_prefix[i_current_depth] > p_current_node->i_maximum_son){
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
          if(fprintf(file, "%02x", arr_prefix[i]) < 0){
            i_error_code = I_ERROR__COULD_NOT_WRITE_CHARACTER;
            break;
          }
        }
        if(i_error_code != 0){
          break;
        }

        //we print a space
        if(putc(' ', file) == EOF){
          i_error_code = I_ERROR__COULD_NOT_WRITE_CHARACTER;
          break;
        }

        //we print the error code
        if(p_current_node->i_status < 0){
          if(fprintf(file, "%d", p_current_node->i_status) < 0){
            i_error_code = I_ERROR__COULD_NOT_WRITE_CHARACTER;
            break;
          }
        }
        //or the transliteration
        else{//if p_current_node->i_status == I_STATUS__VALID
          if(p_current_node->i_transliteration_size == 0){
            if(putc('i', file) == EOF){
              i_error_code = I_ERROR__COULD_NOT_WRITE_CHARACTER;
              break;
            }
          }
          else{
            for(unsigned long i = 0; i < p_current_node->i_transliteration_size; ++i){
              if(fprintf(file, "%02x", p_current_node->s_transliteration[i]) < 0){
                i_error_code = I_ERROR__COULD_NOT_WRITE_CHARACTER;
                break;
              }
            }
            if(i_error_code != 0){
              break;
            }
          }
        }
        if(putc('\n', file) == EOF){
          i_error_code = I_ERROR__COULD_NOT_WRITE_CHARACTER;
          break;
        }
      }
      //go back one level
      --i_current_depth;
      ++arr_prefix[i_current_depth];
      p_current_node = arr_p_ascendants[i_current_depth];
      continue;
    }//end if(arr_prefix[i_current_depth] > p_current_node->i_maximum_son)

    if(arr_prefix[i_current_depth] >= p_current_node->i_minimum_son
      && p_current_node->arr_p_sons[arr_prefix[i_current_depth] - p_current_node->i_minimum_son] != NULL
    ){
      //go forward one level
      p_current_node = p_current_node->arr_p_sons[arr_prefix[i_current_depth] - p_current_node->i_minimum_son];
      ++i_current_depth;
      arr_prefix[i_current_depth] = 0;
      arr_p_ascendants[i_current_depth] = p_current_node;
      continue;
    }//end if(p_current_node->arr_p_sons[arr_prefix[i_current_depth]] != NULL)

    ++arr_prefix[i_current_depth];
  }//end while(p_current_node != NULL)

  fclose(file);
  free(arr_prefix);
  free(arr_p_ascendants);
  return i_error_code;
}//end function transliteration_profile_dump_to_text__shrink1()



/**
 * Transliteration profile management
 * Shrink a raw transliteration profile (shrink1)
 */
int transliteration_profile_from_raw_to_shrink1(
  t_transliteration_profile* p_transliteration_profile_from,
  t_transliteration_profile** p_p_transliteration_profile_to
){
  int* arr_prefix = NULL;
  t_transliteration_node** arr_p_ascendants = NULL;
  size_t i_current_depth = 0;
  t_transliteration_node* p_root_node_shrink;
  t_transliteration_node* p_current_node_raw;
  t_transliteration_node* p_current_node_shrink;
  int i_number_of_sons = 0;

  #ifdef DEBUG_TRANSLITERATION_PROFILE
  printf(
      "Call: transliteration_profile_from_raw_to_shrink1() %p %p\n",
      (void*)p_transliteration_profile_from,
      (void*)p_p_transliteration_profile_to
  );
  #endif

  *p_p_transliteration_profile_to = NULL;

  if(p_transliteration_profile_from->i_profile_type != I_PROFILE_TYPE__RAW){
    return I_ERROR__WRONG_PROFILE_TYPE;
  }

  arr_prefix = (int*) calloc(
      p_transliteration_profile_from->i_max_depth + 1,//root node has depth 0
      sizeof(int)
  );
  if(arr_prefix == NULL){
    return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
  }

  arr_p_ascendants = (t_transliteration_node**) calloc(
      p_transliteration_profile_from->i_max_depth + 1,//root node has depth 0
      sizeof(t_transliteration_node*)
  );
  if(arr_p_ascendants == NULL){
    free(arr_prefix);
    return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
  }

  //first we allocate the transliteration profile
  *p_p_transliteration_profile_to = (t_transliteration_profile*) calloc(1, sizeof(t_transliteration_profile));
  if(*p_p_transliteration_profile_to == NULL){
    free(arr_prefix);
    free(arr_p_ascendants);
    return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
  }
  (*p_p_transliteration_profile_to)->i_profile_type = I_PROFILE_TYPE__SHRINK1;
  (*p_p_transliteration_profile_to)->i_number_of_nodes = p_transliteration_profile_from->i_number_of_nodes;
  (*p_p_transliteration_profile_to)->i_max_depth = p_transliteration_profile_from->i_max_depth;
  (*p_p_transliteration_profile_to)->p_root_node = NULL;

  //and we allocate all the nodes
  (*p_p_transliteration_profile_to)->p_root_node = (t_transliteration_node*) calloc(
      p_transliteration_profile_from->i_number_of_nodes,
      sizeof(t_transliteration_node)
  );
  if((*p_p_transliteration_profile_to)->p_root_node == NULL){
    free(arr_prefix);
    free(arr_p_ascendants);
    free(*p_p_transliteration_profile_to);
    *p_p_transliteration_profile_to = NULL;
    return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
  }
  for(size_t i = 0; i < (*p_p_transliteration_profile_to)->i_number_of_nodes; ++i){
    p_current_node_shrink = &((*p_p_transliteration_profile_to)->p_root_node[i]);
    p_current_node_shrink->arr_p_sons = NULL;
    p_current_node_shrink->s_transliteration = NULL;
  }

  p_current_node_raw = p_transliteration_profile_from->p_root_node;
  p_root_node_shrink = (*p_p_transliteration_profile_to)->p_root_node;
  p_current_node_shrink = p_root_node_shrink;

  //We initialize the root node
  p_current_node_shrink->i_node_index = p_current_node_raw->i_node_index;
  p_current_node_shrink->i_minimum_son = p_current_node_raw->i_minimum_son;
  p_current_node_shrink->i_maximum_son = p_current_node_raw->i_maximum_son;
  p_current_node_shrink->i_status = p_current_node_raw->i_status;
  p_current_node_shrink->i_transliteration_size = p_current_node_raw->i_transliteration_size;
  //we don't need extra space
  p_current_node_shrink->i_allocated_size = p_current_node_raw->i_transliteration_size;
  //with its array of sons
  if(p_current_node_shrink->i_maximum_son >= p_current_node_shrink->i_minimum_son){
    i_number_of_sons = p_current_node_shrink->i_maximum_son - p_current_node_shrink->i_minimum_son + 1;
    p_current_node_shrink->arr_p_sons = (t_transliteration_node**) calloc(
        i_number_of_sons,
        sizeof(t_transliteration_node*)
    );
    if(p_current_node_shrink->arr_p_sons == NULL){
      free(arr_prefix);
      free(arr_p_ascendants);
      transliteration_profile_free(*p_p_transliteration_profile_to);
      *p_p_transliteration_profile_to = NULL;
      return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
    }
    for(int i = 0; i < i_number_of_sons; ++i){
      p_current_node_shrink->arr_p_sons[i] = NULL;
    }
  }
  //and we check that the root node is "skip"
  if(p_current_node_shrink->i_transliteration_size > 0
    || p_current_node_shrink->i_status != I_STATUS__SKIP
  ){
    free(arr_prefix);
    free(arr_p_ascendants);
    transliteration_profile_free(*p_p_transliteration_profile_to);
    *p_p_transliteration_profile_to = NULL;
    return I_ERROR__ROOT_NODE_SHOULD_HAVE_SKIP_STATUS_AND_NO_TRANSLITERATION;
  }

  arr_p_ascendants[0] = p_current_node_raw;
  arr_prefix[0] = 0;

  while(p_current_node_raw != NULL){
    if(arr_prefix[i_current_depth] > p_current_node_raw->i_maximum_son){
      //we're done with the sons

      if(i_current_depth == 0){
        //we're done
        break;
      }

      //go back one level
      --i_current_depth;
      ++arr_prefix[i_current_depth];
      p_current_node_raw = arr_p_ascendants[i_current_depth];
      p_current_node_shrink = &(p_root_node_shrink[p_current_node_raw->i_node_index - 1]);
      continue;
    }//end if(arr_prefix[i_current_depth] > p_current_node_raw->i_maximum_son)

    if(p_current_node_raw->arr_p_sons[arr_prefix[i_current_depth]] != NULL){
      //go forward one level
      p_current_node_raw = p_current_node_raw->arr_p_sons[arr_prefix[i_current_depth]];
      p_current_node_shrink->arr_p_sons[
          arr_prefix[i_current_depth] - p_current_node_shrink->i_minimum_son
      ] = &(p_root_node_shrink[p_current_node_raw->i_node_index - 1]);
      p_current_node_shrink = &(p_root_node_shrink[p_current_node_raw->i_node_index - 1]);
      ++i_current_depth;
      arr_prefix[i_current_depth] = 0;
      arr_p_ascendants[i_current_depth] = p_current_node_raw;

      //we copy the node content
      p_current_node_shrink->i_node_index = p_current_node_raw->i_node_index;
      p_current_node_shrink->i_minimum_son = p_current_node_raw->i_minimum_son;
      p_current_node_shrink->i_maximum_son = p_current_node_raw->i_maximum_son;
      p_current_node_shrink->i_status = p_current_node_raw->i_status;
      p_current_node_shrink->i_transliteration_size = p_current_node_raw->i_transliteration_size;
      //we don't need extra space
      p_current_node_shrink->i_allocated_size = p_current_node_raw->i_transliteration_size;
      //with its array of sons
      if(p_current_node_shrink->i_maximum_son >= p_current_node_shrink->i_minimum_son){
        i_number_of_sons = p_current_node_shrink->i_maximum_son - p_current_node_shrink->i_minimum_son + 1;
        p_current_node_shrink->arr_p_sons = (t_transliteration_node**) calloc(
            i_number_of_sons,
            sizeof(t_transliteration_node*)
        );
        if(p_current_node_shrink->arr_p_sons == NULL){
          free(arr_prefix);
          free(arr_p_ascendants);
          transliteration_profile_free(*p_p_transliteration_profile_to);
          *p_p_transliteration_profile_to = NULL;
          return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
        }
        for(int i = 0; i < i_number_of_sons; ++i){
          p_current_node_shrink->arr_p_sons[i] = NULL;
        }
      }
      //and its transliteration string
      if(p_current_node_shrink->i_transliteration_size > 0){
        p_current_node_shrink->s_transliteration = (unsigned char*) calloc(
            p_current_node_shrink->i_transliteration_size,
            sizeof(unsigned char)
        );
        if(p_current_node_shrink->s_transliteration == NULL){
          free(arr_prefix);
          free(arr_p_ascendants);
          transliteration_profile_free(*p_p_transliteration_profile_to);
          *p_p_transliteration_profile_to = NULL;
          return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
        }
        memcpy(
            p_current_node_shrink->s_transliteration,
            p_current_node_raw->s_transliteration,
            p_current_node_shrink->i_transliteration_size
        );
      }//end if(p_current_node_shrink->i_transliteration_size > 0)

      continue;
    }//end if(p_current_node_raw->arr_p_sons[arr_prefix[i_current_depth]] != NULL)

    ++arr_prefix[i_current_depth];
  }//while(p_current_node_raw != NULL)

  return 0;
}//end function transliteration_profile_from_raw_to_shrink1()



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
  size_t* p_i_current_read_offset
){
  size_t i_allocated_length_output_string = 0;
  size_t i_offset_since_last_prefix_match = 0;
  t_transliteration_node* p_transliteration_node_root = NULL;
  t_transliteration_node* p_transliteration_node_last_match = NULL;
  t_transliteration_node* p_transliteration_node_current = NULL;
  void* p_for_realloc = NULL;
  size_t i_number_of_elements_for_realloc = 0;

  #ifdef DEBUG_TRANSLITERATION_PROFILE
  printf(
      "Call: transliteration_profile_iconv__raw() %p %p %zu %p %p %p\n",
      (void*)p_transliteration_profile,
      (void*)s_input_string,
      i_size_input_string,
      (void*)p_s_output_string,
      (void*)p_i_size_output_string,
      (void*)p_i_current_read_offset
  );
  #endif

  p_transliteration_node_root = p_transliteration_profile->p_root_node;
  *p_s_output_string = NULL;
  *p_i_size_output_string = 0;
  *p_i_current_read_offset = 0;

  if(p_transliteration_profile->i_profile_type != I_PROFILE_TYPE__RAW){
    return I_ERROR__WRONG_PROFILE_TYPE;
  }

  //Outer loop : read and transliterate until end of input string
  while(*p_i_current_read_offset < i_size_input_string){
    p_transliteration_node_last_match = NULL;
    p_transliteration_node_current = p_transliteration_node_root;
    //Inner loop : read until no lengthier prefix is found
    while(*p_i_current_read_offset < i_size_input_string
      && p_transliteration_node_current->arr_p_sons[s_input_string[*p_i_current_read_offset]] != NULL
    ){
      p_transliteration_node_current =
          p_transliteration_node_current->arr_p_sons[s_input_string[*p_i_current_read_offset]];
      ++(*p_i_current_read_offset);
      if(p_transliteration_node_current->i_status < 0
        || p_transliteration_node_current->i_status == I_STATUS__VALID
      ){
        p_transliteration_node_last_match = p_transliteration_node_current;
        i_offset_since_last_prefix_match = 0;
      }
      else{
        ++i_offset_since_last_prefix_match;
      }
    }

    //Apply the match found if possible
    //-Error case 1 : No prefix found
    if(p_transliteration_node_last_match == NULL){
      if(*p_s_output_string != NULL){
        free(*p_s_output_string);
        *p_s_output_string = NULL;
        *p_i_size_output_string = 0;
      }
      return I_ERROR__NO_PREFIX_FOUND;
    }
    //-Error case 2 : User error
    if(p_transliteration_node_last_match->i_status < 0){
      if(*p_s_output_string != NULL){
        free(*p_s_output_string);
        *p_s_output_string = NULL;
        *p_i_size_output_string = 0;
      }
      return p_transliteration_node_last_match->i_status;
    }
    //-Valid
    *p_i_current_read_offset -= i_offset_since_last_prefix_match;
    //--ignore match
    if(p_transliteration_node_last_match->i_transliteration_size == 0){
      continue; //nothing to do
    }
    //--concatenate transliteration
    if(i_allocated_length_output_string == 0){
      *p_s_output_string = (unsigned char *) calloc(8, sizeof(unsigned char));
      if(*p_s_output_string == NULL){
        return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
      }
      i_allocated_length_output_string = 8;
    }
    i_number_of_elements_for_realloc = i_allocated_length_output_string;
    *p_i_size_output_string += p_transliteration_node_last_match->i_transliteration_size;
    while(*p_i_size_output_string > i_number_of_elements_for_realloc){
      i_number_of_elements_for_realloc = i_number_of_elements_for_realloc << 1;//multiply by two
      if(i_number_of_elements_for_realloc <= i_allocated_length_output_string){
        free(*p_s_output_string);
        *p_s_output_string = NULL;
        *p_i_size_output_string = 0;
        return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
      }
    }
    if(i_allocated_length_output_string < i_number_of_elements_for_realloc){
      if(i_number_of_elements_for_realloc * sizeof(unsigned char) < i_number_of_elements_for_realloc){
        free(*p_s_output_string);
        *p_s_output_string = NULL;
        *p_i_size_output_string = 0;
        return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
      }
      p_for_realloc = realloc(
          *p_s_output_string,
          i_number_of_elements_for_realloc * sizeof(unsigned char)
      );
      if(p_for_realloc == NULL){
        free(*p_s_output_string);
        *p_s_output_string = NULL;
        *p_i_size_output_string = 0;
        return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
      }
      i_allocated_length_output_string = i_number_of_elements_for_realloc;
      *p_s_output_string = (unsigned char*) p_for_realloc;
    }
    memcpy(
      &((*p_s_output_string)[*p_i_size_output_string - p_transliteration_node_last_match->i_transliteration_size]),
      p_transliteration_node_last_match->s_transliteration,
      p_transliteration_node_last_match->i_transliteration_size
    );

  }//end Outer Loop while(*p_i_current_read_offset < i_size_input_string)

  return 0;
}//end function transliteration_profile_iconv__raw()



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
  size_t* p_i_current_read_offset
){
  size_t i_allocated_length_output_string = 0;
  size_t i_offset_since_last_prefix_match = 0;
  t_transliteration_node* p_transliteration_node_root = NULL;
  t_transliteration_node* p_transliteration_node_last_match = NULL;
  t_transliteration_node* p_transliteration_node_current = NULL;
  void* p_for_realloc = NULL;
  size_t i_number_of_elements_for_realloc = 0;

  #ifdef DEBUG_TRANSLITERATION_PROFILE
  printf(
      "Call: transliteration_profile_iconv__shrink1() %p %p %zu %p %p %p\n",
      (void*)p_transliteration_profile,
      (void*)s_input_string,
      i_size_input_string,
      (void*)p_s_output_string,
      (void*)p_i_size_output_string,
      (void*)p_i_current_read_offset
  );
  #endif

  p_transliteration_node_root = p_transliteration_profile->p_root_node;
  *p_s_output_string = NULL;
  *p_i_size_output_string = 0;
  *p_i_current_read_offset = 0;

  if(p_transliteration_profile->i_profile_type != I_PROFILE_TYPE__SHRINK1){
    return I_ERROR__WRONG_PROFILE_TYPE;
  }

  //Outer loop : read and transliterate until end of input string
  while(*p_i_current_read_offset < i_size_input_string){
    p_transliteration_node_last_match = NULL;
    p_transliteration_node_current = p_transliteration_node_root;
    //Inner loop : read until no lengthier prefix is found
    while(*p_i_current_read_offset < i_size_input_string
      && p_transliteration_node_current->arr_p_sons != NULL
      && p_transliteration_node_current->i_minimum_son <= s_input_string[*p_i_current_read_offset]
      && p_transliteration_node_current->i_maximum_son >= s_input_string[*p_i_current_read_offset]
      && p_transliteration_node_current->arr_p_sons[
             s_input_string[*p_i_current_read_offset] - p_transliteration_node_current->i_minimum_son
         ] != NULL
    ){
      p_transliteration_node_current = p_transliteration_node_current->arr_p_sons[
          s_input_string[*p_i_current_read_offset] - p_transliteration_node_current->i_minimum_son
      ];
      ++(*p_i_current_read_offset);
      if(p_transliteration_node_current->i_status < 0
        || p_transliteration_node_current->i_status == I_STATUS__VALID
      ){
        p_transliteration_node_last_match = p_transliteration_node_current;
        i_offset_since_last_prefix_match = 0;
      }
      else{
        ++i_offset_since_last_prefix_match;
      }
    }

    //Apply the match found if possible
    //-Error case 1 : No prefix found
    if(p_transliteration_node_last_match == NULL){
      if(*p_s_output_string != NULL){
        free(*p_s_output_string);
        *p_s_output_string = NULL;
        *p_i_size_output_string = 0;
      }
      return I_ERROR__NO_PREFIX_FOUND;
    }
    //-Error case 2 : User error
    if(p_transliteration_node_last_match->i_status < 0){
      if(*p_s_output_string != NULL){
        free(*p_s_output_string);
        *p_s_output_string = NULL;
        *p_i_size_output_string = 0;
      }
      return p_transliteration_node_last_match->i_status;
    }
    //-Valid
    *p_i_current_read_offset -= i_offset_since_last_prefix_match;
    //--ignore match
    if(p_transliteration_node_last_match->i_transliteration_size == 0){
      continue; //nothing to do
    }
    //--concatenate transliteration
    if(i_allocated_length_output_string == 0){
      *p_s_output_string = (unsigned char *) calloc(8, sizeof(unsigned char));
      if(*p_s_output_string == NULL){
        return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
      }
      i_allocated_length_output_string = 8;
    }
    i_number_of_elements_for_realloc = i_allocated_length_output_string;
    *p_i_size_output_string += p_transliteration_node_last_match->i_transliteration_size;
    while(*p_i_size_output_string > i_number_of_elements_for_realloc){
      i_number_of_elements_for_realloc = i_number_of_elements_for_realloc << 1;//multiply by two
      if(i_number_of_elements_for_realloc <= i_allocated_length_output_string){
        free(*p_s_output_string);
        *p_s_output_string = NULL;
        *p_i_size_output_string = 0;
        return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
      }
    }
    if(i_allocated_length_output_string < i_number_of_elements_for_realloc){
      if(i_number_of_elements_for_realloc * sizeof(unsigned char) < i_number_of_elements_for_realloc){
        free(*p_s_output_string);
        *p_s_output_string = NULL;
        *p_i_size_output_string = 0;
        return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
      }
      p_for_realloc = realloc(
          *p_s_output_string,
          i_number_of_elements_for_realloc * sizeof(unsigned char)
      );
      if(p_for_realloc == NULL){
        free(*p_s_output_string);
        *p_s_output_string = NULL;
        *p_i_size_output_string = 0;
        return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
      }
      i_allocated_length_output_string = i_number_of_elements_for_realloc;
      *p_s_output_string = (unsigned char*) p_for_realloc;
    }
    memcpy(
      &((*p_s_output_string)[*p_i_size_output_string - p_transliteration_node_last_match->i_transliteration_size]),
      p_transliteration_node_last_match->s_transliteration,
      p_transliteration_node_last_match->i_transliteration_size
    );

  }//end Outer Loop while(*p_i_current_read_offset < i_size_input_string)

  return 0;
}//end function transliteration_profile_iconv_shrink1()



/**
 * Transliteration profile management
 * Performs a mathematical composition of two transliteration profiles (maps, see README)
 */
int transliteration_profile_compose__raw(
  t_transliteration_profile* p_transliteration_profile_1,
  t_transliteration_profile* p_transliteration_profile_2,
  t_transliteration_profile** p_p_transliteration_profile_result
){

  int* arr_prefix = NULL;
  t_transliteration_node** arr_p_ascendants = NULL;
  size_t i_current_depth = 0;
  t_transliteration_node* p_root_node_result;
  t_transliteration_node* p_current_node_1;
  t_transliteration_node* p_current_node_result;
  int i_number_of_sons = 0;
  int i_result = 0;
  size_t i_current_read_offset = 0;

  #ifdef DEBUG_TRANSLITERATION_PROFILE
  printf(
      "Call: transliteration_profile_compose__raw() %p %p %p\n",
      (void*)p_transliteration_profile_1,
      (void*)p_transliteration_profile_2,
      (void*)p_p_transliteration_profile_result
  );
  #endif

  *p_p_transliteration_profile_result = NULL;

  if(p_transliteration_profile_1->i_profile_type != I_PROFILE_TYPE__RAW){
    return I_ERROR__WRONG_PROFILE_TYPE;
  }

  arr_prefix = (int*) calloc(
      p_transliteration_profile_1->i_max_depth + 1,//root node has depth 0
      sizeof(int)
  );
  if(arr_prefix == NULL){
    return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
  }

  arr_p_ascendants = (t_transliteration_node**) calloc(
      p_transliteration_profile_1->i_max_depth + 1,//root node has depth 0
      sizeof(t_transliteration_node*)
  );
  if(arr_p_ascendants == NULL){
    free(arr_prefix);
    return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
  }

  //first we allocate the transliteration profile
  *p_p_transliteration_profile_result = (t_transliteration_profile*) calloc(
      1,
      sizeof(t_transliteration_profile)
  );
  if(*p_p_transliteration_profile_result == NULL){
    free(arr_prefix);
    free(arr_p_ascendants);
    return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
  }
  (*p_p_transliteration_profile_result)->i_profile_type = I_PROFILE_TYPE__SHRINK1;
  (*p_p_transliteration_profile_result)->i_number_of_nodes = p_transliteration_profile_1->i_number_of_nodes;
  (*p_p_transliteration_profile_result)->i_max_depth = p_transliteration_profile_1->i_max_depth;
  (*p_p_transliteration_profile_result)->p_root_node = NULL;

  //and we allocate all the nodes
  (*p_p_transliteration_profile_result)->p_root_node = (t_transliteration_node*) calloc(
      p_transliteration_profile_1->i_number_of_nodes,
      sizeof(t_transliteration_node)
  );
  if((*p_p_transliteration_profile_result)->p_root_node == NULL){
    free(arr_prefix);
    free(arr_p_ascendants);
    free(*p_p_transliteration_profile_result);
    *p_p_transliteration_profile_result = NULL;
    return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
  }
  for(size_t i = 0; i < (*p_p_transliteration_profile_result)->i_number_of_nodes; ++i){
    p_current_node_result = &((*p_p_transliteration_profile_result)->p_root_node[i]);
    p_current_node_result->arr_p_sons = NULL;
    p_current_node_result->s_transliteration = NULL;
  }

  p_current_node_1 = p_transliteration_profile_1->p_root_node;
  p_root_node_result = (*p_p_transliteration_profile_result)->p_root_node;
  p_current_node_result = p_root_node_result;

  //We initialize the root node
  p_current_node_result->i_node_index = p_current_node_1->i_node_index;
  p_current_node_result->i_minimum_son = p_current_node_1->i_minimum_son;
  p_current_node_result->i_maximum_son = p_current_node_1->i_maximum_son;
  p_current_node_result->i_status = p_current_node_1->i_status;
  p_current_node_result->i_transliteration_size = p_current_node_1->i_transliteration_size;
  //we don't need extra space
  p_current_node_result->i_allocated_size = p_current_node_1->i_transliteration_size;
  //with its array of sons
  if(p_current_node_result->i_maximum_son >= p_current_node_result->i_minimum_son){
    i_number_of_sons = p_current_node_result->i_maximum_son - p_current_node_result->i_minimum_son + 1;
    p_current_node_result->arr_p_sons = (t_transliteration_node**) calloc(
        i_number_of_sons,
        sizeof(t_transliteration_node*)
    );
    if(p_current_node_result->arr_p_sons == NULL){
      free(arr_prefix);
      free(arr_p_ascendants);
      transliteration_profile_free(*p_p_transliteration_profile_result);
      *p_p_transliteration_profile_result = NULL;
      return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
    }
    for(int i = 0; i < i_number_of_sons; ++i){
      p_current_node_result->arr_p_sons[i] = NULL;
    }
  }
  //and we check that the root node is "skip"
  if(p_current_node_result->i_transliteration_size > 0
    || p_current_node_result->i_status != I_STATUS__SKIP
  ){
    free(arr_prefix);
    free(arr_p_ascendants);
    transliteration_profile_free(*p_p_transliteration_profile_result);
    *p_p_transliteration_profile_result = NULL;
    return I_ERROR__ROOT_NODE_SHOULD_HAVE_SKIP_STATUS_AND_NO_TRANSLITERATION;
  }

  arr_p_ascendants[0] = p_current_node_1;
  arr_prefix[0] = 0;

  while(p_current_node_1 != NULL){
    if(arr_prefix[i_current_depth] > p_current_node_1->i_maximum_son){
      //we're done with the sons

      if(i_current_depth == 0){
        //we're done
        break;
      }

      //go back one level
      --i_current_depth;
      ++arr_prefix[i_current_depth];
      p_current_node_1 = arr_p_ascendants[i_current_depth];
      p_current_node_result = &(p_root_node_result[p_current_node_1->i_node_index - 1]);
      continue;
    }//end if(arr_prefix[i_current_depth] > p_current_node_1->i_maximum_son)

    if(p_current_node_1->arr_p_sons[arr_prefix[i_current_depth]] != NULL){
      //go forward one level
      p_current_node_1 = p_current_node_1->arr_p_sons[arr_prefix[i_current_depth]];
      p_current_node_result->arr_p_sons[
          arr_prefix[i_current_depth] - p_current_node_result->i_minimum_son
      ] = &(p_root_node_result[p_current_node_1->i_node_index - 1]);
      p_current_node_result = &(p_root_node_result[p_current_node_1->i_node_index - 1]);
      ++i_current_depth;
      arr_prefix[i_current_depth] = 0;
      arr_p_ascendants[i_current_depth] = p_current_node_1;

      //we copy the node content
      p_current_node_result->i_node_index = p_current_node_1->i_node_index;
      p_current_node_result->i_minimum_son = p_current_node_1->i_minimum_son;
      p_current_node_result->i_maximum_son = p_current_node_1->i_maximum_son;
      p_current_node_result->i_status = p_current_node_1->i_status;
      p_current_node_result->i_transliteration_size = 0;//iconv below may change this value
      p_current_node_result->i_allocated_size = 0;//it will be updated accordingly
      //with its array of sons
      if(p_current_node_result->i_maximum_son >= p_current_node_result->i_minimum_son){
        i_number_of_sons = p_current_node_result->i_maximum_son - p_current_node_result->i_minimum_son + 1;
        p_current_node_result->arr_p_sons = (t_transliteration_node**) calloc(
            i_number_of_sons,
            sizeof(t_transliteration_node*)
        );
        if(p_current_node_result->arr_p_sons == NULL){
          free(arr_prefix);
          free(arr_p_ascendants);
          transliteration_profile_free(*p_p_transliteration_profile_result);
          *p_p_transliteration_profile_result = NULL;
          return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
        }
        for(int i = 0; i < i_number_of_sons; ++i){
          p_current_node_result->arr_p_sons[i] = NULL;
        }
      }
      //and its transliteration string
      if(p_current_node_1->i_transliteration_size > 0){
        i_result = transliteration_profile_iconv(
            p_transliteration_profile_2,
            p_current_node_1->s_transliteration,
            p_current_node_1->i_transliteration_size,
            &p_current_node_result->s_transliteration,
            &p_current_node_result->i_transliteration_size,
            &i_current_read_offset
        );
        if(i_result > 0){
          free(arr_prefix);
          free(arr_p_ascendants);
          transliteration_profile_free(*p_p_transliteration_profile_result);
          *p_p_transliteration_profile_result = NULL;
          return i_result;
        }
        p_current_node_result->i_allocated_size = p_current_node_result->i_transliteration_size;
        //user defined errors are keeped in the composition profile
        if(i_result < 0){
          p_current_node_result->i_status = i_result;
        }
      }//end if(p_current_node_result->i_transliteration_size > 0)

      continue;
    }//end if(p_current_node_1->arr_p_sons[arr_prefix[i_current_depth]] != NULL)

    ++arr_prefix[i_current_depth];
  }//end while(p_current_node_1 != NULL)

  return 0;
}//end function transliteration_profile_compose__raw()



/**
 * Transliteration profile management
 * Performs a mathematical composition of two transliteration profiles (maps, see README)
 */
int transliteration_profile_compose__shrink1(
  t_transliteration_profile* p_transliteration_profile_1,
  t_transliteration_profile* p_transliteration_profile_2,
  t_transliteration_profile** p_p_transliteration_profile_result
){

  int* arr_prefix = NULL;
  t_transliteration_node** arr_p_ascendants = NULL;
  size_t i_current_depth = 0;
  t_transliteration_node* p_root_node_result;
  t_transliteration_node* p_current_node_1;
  t_transliteration_node* p_current_node_result;
  int i_number_of_sons = 0;
  int i_result = 0;
  size_t i_current_read_offset = 0;

  #ifdef DEBUG_TRANSLITERATION_PROFILE
  printf(
      "Call: transliteration_profile_compose__shrink1() %p %p %p\n",
      (void*)p_transliteration_profile_1,
      (void*)p_transliteration_profile_2,
      (void*)p_p_transliteration_profile_result
  );
  #endif

  *p_p_transliteration_profile_result = NULL;

  if(p_transliteration_profile_1->i_profile_type != I_PROFILE_TYPE__SHRINK1){
    return I_ERROR__WRONG_PROFILE_TYPE;
  }

  arr_prefix = (int*) calloc(
      p_transliteration_profile_1->i_max_depth + 1,//root node has depth 0
      sizeof(int)
  );
  if(arr_prefix == NULL){
    return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
  }

  arr_p_ascendants = (t_transliteration_node**) calloc(
      p_transliteration_profile_1->i_max_depth + 1,//root node has depth 0
      sizeof(t_transliteration_node*)
  );
  if(arr_p_ascendants == NULL){
    free(arr_prefix);
    return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
  }

  //first we allocate the transliteration profile
  *p_p_transliteration_profile_result = (t_transliteration_profile*) calloc(
      1,
      sizeof(t_transliteration_profile)
  );
  if(*p_p_transliteration_profile_result == NULL){
    free(arr_prefix);
    free(arr_p_ascendants);
    return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
  }
  (*p_p_transliteration_profile_result)->i_profile_type = I_PROFILE_TYPE__SHRINK1;
  (*p_p_transliteration_profile_result)->i_number_of_nodes = p_transliteration_profile_1->i_number_of_nodes;
  (*p_p_transliteration_profile_result)->i_max_depth = p_transliteration_profile_1->i_max_depth;
  (*p_p_transliteration_profile_result)->p_root_node = NULL;

  //and we allocate all the nodes
  (*p_p_transliteration_profile_result)->p_root_node = (t_transliteration_node*) calloc(
      p_transliteration_profile_1->i_number_of_nodes,
      sizeof(t_transliteration_node)
  );
  if((*p_p_transliteration_profile_result)->p_root_node == NULL){
    free(arr_prefix);
    free(arr_p_ascendants);
    free(*p_p_transliteration_profile_result);
    *p_p_transliteration_profile_result = NULL;
    return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
  }
  for(size_t i = 0; i < (*p_p_transliteration_profile_result)->i_number_of_nodes; ++i){
    p_current_node_result = &((*p_p_transliteration_profile_result)->p_root_node[i]);
    p_current_node_result->arr_p_sons = NULL;
    p_current_node_result->s_transliteration = NULL;
  }

  p_current_node_1 = p_transliteration_profile_1->p_root_node;
  //will be negative if needed
  i_number_of_sons = p_current_node_1->i_maximum_son - p_current_node_1->i_minimum_son + 1;
  p_root_node_result = (*p_p_transliteration_profile_result)->p_root_node;
  p_current_node_result = p_root_node_result;

  //We initialize the root node
  p_current_node_result->i_node_index = p_current_node_1->i_node_index;
  p_current_node_result->i_minimum_son = p_current_node_1->i_minimum_son;
  p_current_node_result->i_maximum_son = p_current_node_1->i_maximum_son;
  p_current_node_result->i_status = p_current_node_1->i_status;
  p_current_node_result->i_transliteration_size = p_current_node_1->i_transliteration_size;
  //we don't need extra space
  p_current_node_result->i_allocated_size = p_current_node_1->i_transliteration_size;
  //with its array of sons
  if(i_number_of_sons >= 1){
    p_current_node_result->arr_p_sons = (t_transliteration_node**) calloc(
        i_number_of_sons,
        sizeof(t_transliteration_node*)
    );
    if(p_current_node_result->arr_p_sons == NULL){
      free(arr_prefix);
      free(arr_p_ascendants);
      transliteration_profile_free(*p_p_transliteration_profile_result);
      *p_p_transliteration_profile_result = NULL;
      return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
    }
    for(int i = 0; i < i_number_of_sons; ++i){
      p_current_node_result->arr_p_sons[i] = NULL;
    }
  }
  //and we check that the root node is "skip"
  if(p_current_node_result->i_transliteration_size > 0
    || p_current_node_result->i_status != I_STATUS__SKIP
  ){
    free(arr_prefix);
    free(arr_p_ascendants);
    transliteration_profile_free(*p_p_transliteration_profile_result);
    *p_p_transliteration_profile_result = NULL;
    return I_ERROR__ROOT_NODE_SHOULD_HAVE_SKIP_STATUS_AND_NO_TRANSLITERATION;
  }

  arr_p_ascendants[0] = p_current_node_1;
  arr_prefix[0] = 0;

  while(p_current_node_1 != NULL){
    if(arr_prefix[i_current_depth] >= i_number_of_sons){
      //we're done with the sons

      if(i_current_depth == 0){
        //we're done
        break;
      }

      //go back one level
      --i_current_depth;
      ++arr_prefix[i_current_depth];
      p_current_node_1 = arr_p_ascendants[i_current_depth];
      //will be negative if needed
      i_number_of_sons = p_current_node_1->i_maximum_son - p_current_node_1->i_minimum_son + 1;
      p_current_node_result = &(p_root_node_result[p_current_node_1->i_node_index - 1]);
      continue;
    }//end if(arr_prefix[i_current_depth] >= i_number_of_sons)

    if(p_current_node_1->arr_p_sons[arr_prefix[i_current_depth]] != NULL){
      //go forward one level
      p_current_node_1 = p_current_node_1->arr_p_sons[arr_prefix[i_current_depth]];
      //will be negative if needed
      i_number_of_sons = p_current_node_1->i_maximum_son - p_current_node_1->i_minimum_son + 1;
      p_current_node_result->arr_p_sons[
          arr_prefix[i_current_depth] - p_current_node_result->i_minimum_son
      ] = &(p_root_node_result[p_current_node_1->i_node_index - 1]);
      p_current_node_result = &(p_root_node_result[p_current_node_1->i_node_index - 1]);
      ++i_current_depth;
      arr_prefix[i_current_depth] = 0;
      arr_p_ascendants[i_current_depth] = p_current_node_1;

      //we copy the node content
      p_current_node_result->i_node_index = p_current_node_1->i_node_index;
      p_current_node_result->i_minimum_son = p_current_node_1->i_minimum_son;
      p_current_node_result->i_maximum_son = p_current_node_1->i_maximum_son;
      p_current_node_result->i_status = p_current_node_1->i_status;
      p_current_node_result->i_transliteration_size = 0;//iconv below may change this value
      p_current_node_result->i_allocated_size = 0;//it will be updated accordingly
      //with its array of sons
      if(i_number_of_sons >= 1){
        p_current_node_result->arr_p_sons = (t_transliteration_node**) calloc(
            i_number_of_sons,
            sizeof(t_transliteration_node*)
        );
        if(p_current_node_result->arr_p_sons == NULL){
          free(arr_prefix);
          free(arr_p_ascendants);
          transliteration_profile_free(*p_p_transliteration_profile_result);
          *p_p_transliteration_profile_result = NULL;
          return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
        }
        for(int i = 0; i < i_number_of_sons; ++i){
          p_current_node_result->arr_p_sons[i] = NULL;
        }
      }
      //and its transliteration string
      if(p_current_node_1->i_transliteration_size > 0){
        i_result = transliteration_profile_iconv(
            p_transliteration_profile_2,
            p_current_node_1->s_transliteration,
            p_current_node_1->i_transliteration_size,
            &p_current_node_result->s_transliteration,
            &p_current_node_result->i_transliteration_size,
            &i_current_read_offset
        );
        if(i_result > 0){
          free(arr_prefix);
          free(arr_p_ascendants);
          transliteration_profile_free(*p_p_transliteration_profile_result);
          *p_p_transliteration_profile_result = NULL;
          return i_result;
        }
        p_current_node_result->i_allocated_size = p_current_node_result->i_transliteration_size;
        //user defined errors are keeped in the composition profile
        if(i_result < 0){
          p_current_node_result->i_status = i_result;
        }
      }//end if(p_current_node_result->i_transliteration_size > 0)

      continue;
    }//end if(p_current_node_1->arr_p_sons[arr_prefix[i_current_depth]] != NULL)

    ++arr_prefix[i_current_depth];
  }//end while(p_current_node_1 != NULL)

  return 0;
}//end function transliteration_profile_compose__shrink1()



