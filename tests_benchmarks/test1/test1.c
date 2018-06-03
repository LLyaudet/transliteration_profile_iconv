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

//#define DEBUG_TRANSLITERATION_PROFILE

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



int mydiff(char* s_filename1, char* s_filename2){
  FILE* file1 = NULL;
  FILE* file2 = NULL;
  int i_result = 0;//no difference
  int c1;//that's a char but getc returns an int
  int c2;//that's a char but getc returns an int

  file1 = fopen(s_filename1, "r");
  if(file1 == NULL){
    return I_ERROR__COULD_NOT_OPEN_FILE;
  }

  file2 = fopen(s_filename2, "r");
  if(file2 == NULL){
    fclose(file1);
    return I_ERROR__COULD_NOT_OPEN_FILE;
  }

  do{
    c1 = getc(file1);
    c2 = getc(file2);
    if(c1 != c2){
      i_result = 1;
      break;
    }
  }
  while(c1 != EOF && c2 != EOF);

  fclose(file1);
  fclose(file2);
  return i_result;
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

  do{
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

      break;
    }


    //test if both files are equal
    i_result = mydiff("tp_test_load.txt", "tp_test_dump.test_result");

    if(i_result != 0){
      printf(
          "The loaded profile and the dumped profile did not match (error code %d).\n",
          i_result
      );
      break;
    }
    printf("The loaded profile and the dumped profile match.\n");

    printf("Freeing test profile\n");
    transliteration_profile_free(p_transliteration_profile);
    //*/
  }
  while(0);

  return i_result;
}//end function main()



