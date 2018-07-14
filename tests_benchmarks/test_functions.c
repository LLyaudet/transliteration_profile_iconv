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

#include <unistd.h>
#include "../transliteration_profile_iconv.h"

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



int from_file_to_string(
  char* s_filename,
  unsigned char** p_s_string,
  size_t* p_i_string_length
){
  FILE* file;
  file = fopen(s_filename, "r");
  if(file == NULL){
    printf("Couldn't open file to convert to string.\n");
    return I_ERROR__COULD_NOT_OPEN_FILE;
  }
  fseek(file, 0, SEEK_END);
  *p_i_string_length = ftell(file);
  fseek(file, 0, SEEK_SET);
  *p_s_string = malloc(*p_i_string_length);
  if(*p_s_string == NULL){
    fclose(file);
    printf("Couldn't allocate string.\n");
    return I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
  }
  fread(*p_s_string, 1, *p_i_string_length, file);
  fclose(file);
  return 0;
}



int from_string_to_file(
  char* s_filename,
  unsigned char* s_string,
  size_t i_string_length
){
  FILE* file;
  file = fopen(s_filename, "w");
  if(file == NULL){
    printf("Couldn't open file to dump string to.\n");
    return I_ERROR__COULD_NOT_OPEN_FILE;
  }
  for(size_t i = 0; i < i_string_length; ++i){
    if(fputc(s_string[i], file) == EOF){
      fclose(file);
      return I_ERROR__COULD_NOT_WRITE_CHARACTER;
    }
  }
  fclose(file);
  return 0;
}



