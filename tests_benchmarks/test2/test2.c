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
/*
Explanations for this test:
tp_test_iconv.txt is a transliteration profile file where:
 - A is doubled (41 4141),
 - B is ignored (42 i),
 - C returns an error (43 -12)
 - line returns are kept (0a 0a)
to_convert.txt contains a string that must be transliterated with success.
AABAA -> AAAAAAAA that you can found in expected_result.txt
to_error.txt contains a string AACAA that must return -12 according to the profile.
Both tests are performed with "raw" and "shrink1" tree structures representing the profile.
*/

#include "../test_functions.c"



int main(int argc, char *argv[]){

  int i_result = 0;
  t_transliteration_profile* p_transliteration_profile_raw = NULL;
  t_transliteration_profile* p_transliteration_profile_shrink1 = NULL;
  size_t i_current_line;
  size_t i_current_column;
  unsigned char* s_input_string = NULL;
  size_t i_size_input_string;
  unsigned char* s_output_string = NULL;
  size_t i_size_output_string;
  size_t i_current_read_offset;

  do{
    printf("Loading test profile\n");
    i_result = transliteration_profile_load_from_text(
        "tp_test_iconv.txt",
        &p_transliteration_profile_raw,
        &i_current_line,
        &i_current_column
    );

    if(i_result != 0){
      printf(
          "An error occurred when loading profile at line %zu and column %zu (error code %d).\n",
          i_current_line,
          i_current_column,
          i_result
      );
      break;
    }

    printf("Shrinking test profile\n");
    i_result = transliteration_profile_from_raw_to_shrink1(
        p_transliteration_profile_raw,
        &p_transliteration_profile_shrink1
    );
    if(i_result != 0){
      printf(
          "An error occurred when shrinking profile (error code %d).\n",
          i_result
      );
      break;
    }

    i_result = from_file_to_string(
        "to_convert.txt",
        &s_input_string,
        &i_size_input_string
    );
    if(i_result != 0){
      printf(
          "An error occurred when converting first file to string (error code %d).\n",
          i_result
      );
      break;
    }

    i_result = from_string_to_file(
        "copy_of_to_convert.test_result",
        s_input_string,
        i_size_input_string
    );
    if(i_result != 0){
      printf(
          "An error occurred when dumping copy of to_convert.txt (error code %d).\n",
          i_result
      );
      break;
    }

    //test if both files are equal
    i_result = mydiff("to_convert.txt", "copy_of_to_convert.test_result");

    if(i_result != 0){
      printf(
          "The file/string functions don't work (error code %d).\n",
          i_result
      );
      break;
    }
    printf("The file/string functions work.\n");

    i_result = transliteration_profile_iconv(
        p_transliteration_profile_raw,
        s_input_string,
        i_size_input_string,
        &s_output_string,
        &i_size_output_string,
        &i_current_read_offset
    );
    if(i_result != 0){
      printf(
          "An error occurred when transliterating (raw) first file"
          " (error code %d, current_read_offset %zu).\n",
          i_result,
          i_current_read_offset
      );
      break;
    }

    i_result = from_string_to_file(
        "result_raw.test_result",
        s_output_string,
        i_size_output_string
    );
    if(i_result != 0){
      printf(
          "An error occurred when dumping first result (raw) (error code %d).\n",
          i_result
      );
      break;
    }

    //test if both files are equal
    i_result = mydiff("expected_result.txt", "result_raw.test_result");

    if(i_result != 0){
      printf(
          "The first transliteration (raw) is wrong (error code %d).\n",
          i_result
      );
      break;
    }
    printf("The first transliteration (raw) is correct.\n");

    free(s_output_string);
    s_output_string = NULL;

    i_result = transliteration_profile_iconv(
        p_transliteration_profile_shrink1,
        s_input_string,
        i_size_input_string,
        &s_output_string,
        &i_size_output_string,
        &i_current_read_offset
    );
    if(i_result != 0){
      printf(
          "An error occurred when transliterating (shrink1) first file"
          " (error code %d, current_read_offset %zu).\n",
          i_result,
          i_current_read_offset
      );
      break;
    }

    i_result = from_string_to_file(
        "result_shrink1.test_result",
        s_output_string,
        i_size_output_string
    );
    if(i_result != 0){
      printf(
          "An error occurred when dumping first result (shrink1) (error code %d).\n",
          i_result
      );
      break;
    }

    //test if both files are equal
    i_result = mydiff("expected_result.txt", "result_shrink1.test_result");

    if(i_result != 0){
      printf(
          "The first transliteration (shrink1) is wrong (error code %d).\n",
          i_result
      );
      break;
    }
    printf("The first transliteration (shrink1) is correct.\n");

    free(s_output_string);
    s_output_string = NULL;
    free(s_input_string);
    s_input_string = NULL;

    i_result = from_file_to_string(
        "to_error.txt",
        &s_input_string,
        &i_size_input_string
    );
    if(i_result != 0){
      printf(
          "An error occurred when converting second file to string (error code %d).\n",
          i_result
      );
      break;
    }

    i_result = transliteration_profile_iconv(
        p_transliteration_profile_raw,
        s_input_string,
        i_size_input_string,
        &s_output_string,
        &i_size_output_string,
        &i_current_read_offset
    );
    if(i_result != -12){
      printf(
          "Error -12 was not thrown when transliterating (raw) second file"
          " (error code %d, current_read_offset %zu).\n",
          i_result,
          i_current_read_offset
      );
      i_result = -12;
      break;
    }
    printf("Error -12 was correctly thrown when transliterating (raw) second file\n");

    i_result = transliteration_profile_iconv(
        p_transliteration_profile_shrink1,
        s_input_string,
        i_size_input_string,
        &s_output_string,
        &i_size_output_string,
        &i_current_read_offset
    );
    if(i_result != -12){
      printf(
          "Error -12 was not thrown when transliterating (shrink1) second file"
          " (error code %d, current_read_offset %zu).\n",
          i_result,
          i_current_read_offset
      );
      i_result = -12;
      break;
    }
    i_result = 0;
    printf("Error -12 was correctly thrown when transliterating (shrink1) second file\n");
  }
  while(0);

  printf("Freeing everything\n");
  if(p_transliteration_profile_raw != NULL){
    transliteration_profile_free(p_transliteration_profile_raw);
    p_transliteration_profile_raw = NULL;
  }
  if(p_transliteration_profile_shrink1 != NULL){
    transliteration_profile_free(p_transliteration_profile_shrink1);
    p_transliteration_profile_shrink1 = NULL;
  }
  if(s_input_string != NULL){
    free(s_input_string);
    s_input_string = NULL;
  }
  if(s_output_string != NULL){
    free(s_output_string);
    s_output_string = NULL;
  }
  return i_result;
}//end function main()



