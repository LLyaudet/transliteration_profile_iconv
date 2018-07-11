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
tp_test_load.txt is a transliteration profile file that contains various cases of transliterations.
This test loads this profile in a tree structure in memory and dumps a binary version of it from the tree structure.
Then the binary version is loaded again into a tree structure and this structure is dumped to a text copy of the original profile.
It then checks that the copy is identical to the original.
*/

//#define DEBUG_TRANSLITERATION_PROFILE

#include "../test_functions.c"



int main(int argc, char *argv[]){

  int i_result = 0;
  t_transliteration_profile* p_transliteration_profile = NULL;
  size_t i_current_line = 0;
  size_t i_current_column = 0;
  size_t i_current_offset = 0;

  printf("Loading test profile from text\n");
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
    printf("Dumping test profile to bin\n");
    i_result = transliteration_profile_dump_to_bin(
        "tp_test_load_bin.test_result",
        p_transliteration_profile
    );

    if(i_result != 0){
      printf(
          "An error occured when dumping the profile to bin (error code %d).\n",
          i_result
      );
      break;
    }

    printf("Freeing first test profile\n");
    transliteration_profile_free(p_transliteration_profile);
  
    printf("Loading test profile from bin\n");
    i_result = transliteration_profile_load_from_bin(
        "tp_test_load_bin.test_result",
        &p_transliteration_profile,
        &i_current_offset
    );

    if(i_result != 0){
      printf(
          "An error occured when loading profile from bin at offset %d (error code %d).\n",
          i_current_offset,
          i_result
      );
      return i_result;
    }

    printf("Dumping test profile to text\n");
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

  }
  while(0);

  if(p_transliteration_profile != NULL){
    printf("Freeing test profile\n");
    transliteration_profile_free(p_transliteration_profile);
  }

  return i_result;
}//end function main()



