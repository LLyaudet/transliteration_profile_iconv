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

#define DEBUG_TRANSLITERATION_PROFILE

#include <unistd.h>
//#include <dirent.h>
#include "../../transliteration_profile_iconv.c"



__attribute__((noinline)) void foo(
  char* s_filename,
  t_transliteration_profile** p_p_transliteration_profile,
  size_t* p_i_current_line,
  size_t* p_i_current_column
){
  printf(
      "%s %d %d %d %d %d %d\n",
      s_filename,
      p_p_transliteration_profile,
      p_i_current_line,
      p_i_current_column,
      *p_p_transliteration_profile,
      *p_i_current_line,
      *p_i_current_column
  );
  ++(*p_p_transliteration_profile);
  ++(*p_i_current_line);
  ++(*p_i_current_column);
  printf(
      "%s %d %d %d %d %d %d\n",
      s_filename,
      p_p_transliteration_profile,
      p_i_current_line,
      p_i_current_column,
      *p_p_transliteration_profile,
      *p_i_current_line,
      *p_i_current_column
  );
}



int main(int argc, char *argv[]){

  int i_result = 0;
  t_transliteration_profile* p_transliteration_profile;
  size_t i_current_line;
  size_t i_current_column;

  foo("tp_test_load.txt", &p_transliteration_profile, &i_current_line, &i_current_column);

  printf("Loading test profile\n");
  i_result = transliteration_profile_load_from_text(
      "tp_test_load.txt",
      &p_transliteration_profile,
      &i_current_line,
      &i_current_column
  );

  if(i_result != 0){
    printf(
        "An error occured when loading profile at line %d and column %d (error code %d).\n",
        i_current_line,
        i_current_column,
        i_result
    );
    return i_result;
  }

  printf("Dumping test profile\n");
  i_result = transliteration_profile_dump_to_text(
      "tp_test_dump.test_result",
      p_transliteration_profile
  );

  if(i_result != 0){
    printf(
        "An error occured when dumping the profile (error code %d).\n",
        i_result
    );

    return i_result;
  }


  //test if both files are equal
  i_result = execl("/bin/diff", "diff", "tp_test_load.txt", "tp_test_dump.test_result");
  printf(
      "Result of diff %d.",
      i_result
  );

  printf("Freeing test profile\n");
  transliteration_profile_free(p_transliteration_profile);
  //*/
  return i_result;
}//end function main()
