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

#include "../test_functions.c"



int main(int argc, char *argv[]){

  int i_result = 0;
  t_transliteration_profile* p_transliteration_profile_1 = NULL;
  t_transliteration_profile* p_transliteration_profile_2 = NULL;
  t_transliteration_profile* p_transliteration_profile_composition = NULL;
  size_t i_current_line;
  size_t i_current_column;

  do{
    printf("Loading test profile 1\n");
    i_result = transliteration_profile_load_from_text(
        "../../profiles/text_profiles/tp_ISO-8859-1__UTF-8.txt",
        &p_transliteration_profile_1,
        &i_current_line,
        &i_current_column
    );

    if(i_result != 0){
      printf(
          "An error occured when loading profile 1 at line %d and column %d (error code %d).\n",
          i_current_line,
          i_current_column,
          i_result
      );
      break;
    }

    printf("Loading test profile 2\n");
    i_result = transliteration_profile_load_from_text(
        "../../profiles/text_profiles/tp_UTF-8__ISO-8859-15IGNORE.txt",
        &p_transliteration_profile_2,
        &i_current_line,
        &i_current_column
    );

    if(i_result != 0){
      printf(
          "An error occured when loading profile 2 at line %d and column %d (error code %d).\n",
          i_current_line,
          i_current_column,
          i_result
      );
      break;
    }

    printf("Composition of profiles\n");
    i_result = transliteration_profile_compose(
        p_transliteration_profile_1,
        p_transliteration_profile_2,
        &p_transliteration_profile_composition
    );
    if(i_result != 0){
      printf(
          "An error occured when composing profiles (error code %d).\n",
          i_result
      );
      break;
    }

    printf("Dumping composition profile\n");
    i_result = transliteration_profile_dump_to_text(
        "tp_ISO-8859-1__ISO-8859-15IGNORE.test_result",
        p_transliteration_profile_composition
    );

    if(i_result != 0){
      printf(
          "An error occured when dumping the profile (error code %d).\n",
          i_result
      );
      break;
    }

    //test if both files are equal
    i_result = mydiff("../../profiles/text_profiles/tp_ISO-8859-1__ISO-8859-15IGNORE.txt", "tp_ISO-8859-1__ISO-8859-15IGNORE.test_result");

    if(i_result != 0){
      printf(
          "The composition profile and the reference profile did not match (error code %d).\n",
          i_result
      );
      break;
    }
    printf("The composition profile and the reference profile match.\n");

  }
  while(0);

  printf("Freeing everything\n");
  if(p_transliteration_profile_1 != NULL){
    transliteration_profile_free(p_transliteration_profile_1);
    p_transliteration_profile_1 = NULL;
  }
  if(p_transliteration_profile_2 != NULL){
    transliteration_profile_free(p_transliteration_profile_2);
    p_transliteration_profile_2 = NULL;
  }
  if(p_transliteration_profile_composition != NULL){
    transliteration_profile_free(p_transliteration_profile_composition);
    p_transliteration_profile_composition = NULL;
  }
  return i_result;
}//end function main()



