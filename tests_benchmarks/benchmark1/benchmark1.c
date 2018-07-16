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
Explanations for this benchmark:
tp_benchmark.txt is a transliteration profile file that reproduces the transliterations from https://sourceware.org/bugzilla/show_bug.cgi?id=23076.
This transliteration profile is obtained from tp_UTF-8__ISO-8859-1IGNORE.txt with the additional transliterations.
This benchmark compares glibc iconv() with a crafted locale, to tranliteration_profile_iconv() with a text profile or a binary profile.
We prepare a binary profile from the text profile.
Let n be the main number of iterations.
There are 3 sub-benchmarks:
 - The profile is loaded, the locale is set,
   and we mesure the time to convert/transliterate n times the benchmark input string,
   then we free the profile/switch back the locale.
   These are the ideal conditions for performances when the cost of loading/freeing is unsignificant.
 - We mesure the time to load the profile/set the locale, convert/transliterate, and free the profile/switch back the locale n times.
   These are the worst conditions when you have to set up everything for each conversion/transliteration.
 - We mesure the time of of n/30 iterations where in each iteration we load the profile/set the locale, convert/transliterate 30 times, and free the profile/switch back.
   These are average conditions to represent a small EDIFACT file with a low value of 30 fields to convert.
   EDIFACTs can be made of many thousands of fields.
Each sub-benchmark is done 3 times: one time with text profile, one time with binary profile, one time with locale.
Because of the tight coupling of locales, in real world examples we may have to compare worst conditions with iconv() and crafted locale,
to average conditions with tranliteration_profile_iconv().
Because loading text profile is very slow, n is fixed to only 300 in this benchmark.
You can add -O flag to CFLAGS in the makefile to improve the performances.
*/
/*
/!\Help wanted/!\, I tried to benchmark glibc iconv() along with transliteration_profile_iconv(),
but I did not succeed.
As you can see in the code below, there is a b_activate_iconv = 0.
And there is also commented out commands in the Makefile.
What I tried was to customize glibc iconv() along the lines suggested in https://sourceware.org/bugzilla/show_bug.cgi?id=23076.
I edited a locale definition file fr_FR@test in this directory (the Makefile puts it in the correct directory on Debian).
I edited a charmap definition file ISO-8859-1-test in this directory (the Makefile puts it in the correct directory on Debian).
The makefile then edits the file /etc/locale.gen and executes locale-gen.
Everything so far goes well.
But iconv_open() below fails because there is no gconv module for ISO-8859-1-test.
If I rename ISO-8859-1-test everywhere with ISO-8859-1 which I don't recommend (make copies of the correct files for ISO-8859-1) iconv_open() works but iconv() does not transliterate as specified by the fr_FR@test file.
(You can check that the setlocale is correct since it returns "fr_FR@test".
Euro symbol is transliterated to EUR instead of the currency symbol...
What I could test with PHP is not working with C.)
I tried also adding the locale in /usr/share/i18n/SUPPORTED,
then running locale-gen and iconvconfig.
But it didn't generate a new gconv module for ISO-8859-1-test.
So far I don't know if it's possible to generate a new gconv module for a new charmap without compiling glibc.
I didn't found any command to do so.
*/

#include "../test_functions.c"
#include <time.h>
#include <iconv.h>
#include <stddef.h>
#include <locale.h>
#include <error.h>


int main(int argc, char *argv[]){

  int i_result = 0;
  t_transliteration_profile* p_transliteration_profile = NULL;
  size_t i_current_line = 0;
  size_t i_current_column = 0;
  size_t i_current_offset = 0;
  unsigned char* s_input_string = NULL;
  size_t i_size_input_string;
  unsigned char* s_input_string_copy_of_pointer = NULL;
  size_t i_copy_size_input_string;
  unsigned char* s_output_string = NULL;
  size_t i_size_output_string;
  size_t i_current_read_offset;
  long int i_number_of_iterations = 300;
  long int i_number_of_sub_iterations = 30;//average conditions
  struct timespec start, finish;
  double f_elapsed_time = 0.0;
  iconv_t iconv_descriptor = (iconv_t)-1;
  char* s_old_locale = NULL;
  char* s_saved_locale = NULL;
  char* output_buffer = NULL;
  char* output_buffer_copy_of_pointer = NULL;
  size_t i_outbytes_left = 1024;
  int b_activate_iconv = 0;

  do{
    printf("Creating binary profile\n");
    i_result = transliteration_profile_load_from_text(
        "tp_benchmark.txt",
        &p_transliteration_profile,
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
      return i_result;
    }

    i_result = transliteration_profile_dump_to_bin(
        "tp_benchmark_bin.test_result",
        p_transliteration_profile
    );

    if(i_result != 0){
      printf(
          "An error occurred when dumping the profile to bin (error code %d).\n",
          i_result
      );
      break;
    }
    printf("Binary profile created\n");

    printf("Testing:\n");
    i_result = from_file_to_string(
        "input_string.txt",
        &s_input_string,
        &i_size_input_string
    );
    if(i_result != 0){
      printf(
          "An error occurred when reading input string (error code %d).\n",
          i_result
      );
      break;
    }
    s_input_string_copy_of_pointer = s_input_string;
    i_copy_size_input_string = i_size_input_string;


    printf("-with text profile: ");
    i_result = transliteration_profile_iconv(
        p_transliteration_profile,
        s_input_string,
        i_size_input_string,
        &s_output_string,
        &i_size_output_string,
        &i_current_read_offset
    );
    if(i_result != 0){
      printf(
          "An error occurred when transliterating (text)"
          " (error code %d, current_read_offset %zu).\n",
          i_result,
          i_current_read_offset
      );
      break;
    }

    i_result = from_string_to_file(
        "result_text.test_result",
        s_output_string,
        i_size_output_string
    );
    if(i_result != 0){
      printf(
          "An error occurred when writing result (text) (error code %d).\n",
          i_result
      );
      break;
    }

    //test if both files are equal
    i_result = mydiff("expected_output_string.txt", "result_text.test_result");

    if(i_result != 0){
      printf(
          "The transliteration (text) is wrong (error code %d).\n",
          i_result
      );
      break;
    }
    printf("ok.\n");

    free(s_output_string);
    s_output_string = NULL;
    transliteration_profile_free(p_transliteration_profile);
    p_transliteration_profile = NULL;


    printf("-with binary profile: ");
    i_result = transliteration_profile_load_from_bin(
        "tp_benchmark_bin.test_result",
        &p_transliteration_profile,
        &i_current_offset
    );

    if(i_result != 0){
      printf(
          "An error occurred when loading profile from bin at offset %zu (error code %d).\n",
          i_current_offset,
          i_result
      );
      return i_result;
    }

    i_result = transliteration_profile_iconv(
        p_transliteration_profile,
        s_input_string,
        i_size_input_string,
        &s_output_string,
        &i_size_output_string,
        &i_current_read_offset
    );
    if(i_result != 0){
      printf(
          "An error occurred when transliterating (bin)"
          " (error code %d, current_read_offset %zu).\n",
          i_result,
          i_current_read_offset
      );
      break;
    }

    i_result = from_string_to_file(
        "result_bin.test_result",
        s_output_string,
        i_size_output_string
    );
    if(i_result != 0){
      printf(
          "An error occurred when writing result (bin) (error code %d).\n",
          i_result
      );
      break;
    }

    //test if both files are equal
    i_result = mydiff("expected_output_string.txt", "result_bin.test_result");

    if(i_result != 0){
      printf(
          "The transliteration (bin) is wrong (error code %d).\n",
          i_result
      );
      break;
    }
    printf("ok.\n");

    free(s_output_string);
    s_output_string = NULL;
    transliteration_profile_free(p_transliteration_profile);
    p_transliteration_profile = NULL;


    if(b_activate_iconv){
      printf("-with iconv(): ");
      output_buffer = (char*) calloc(i_outbytes_left, sizeof(char));
      if(output_buffer == NULL){
        printf("An error occurred when allocating output buffer (iconv)\n");
        i_result = I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
        break;
      }
      output_buffer_copy_of_pointer = output_buffer;

      s_old_locale = setlocale(LC_ALL, NULL);
      s_saved_locale = strdup(s_old_locale);
      if(s_saved_locale == NULL){
        printf("An error occurred when saving old locale.\n");
        i_result= -1;
        break;
      }
      setlocale(LC_ALL, "fr_FR@test");

      iconv_descriptor = iconv_open("ISO-8859-1-test//TRANSLIT//IGNORE", "UTF-8");
      if(iconv_descriptor == (iconv_t) -1){
        printf("An error occurred when opening iconv descriptor.\n");
        i_result= -1;
        if(errno == EINVAL){
          error(0, 0, "conversion from UTF-8 to ISO-8859-1-test//TRANSLIT//IGNORE not available");
        }
        else{
          perror("iconv_open");
        }
        break;
      }

      i_result = (int) iconv(
          iconv_descriptor,
          (char**) &s_input_string_copy_of_pointer,
          &i_copy_size_input_string,
          &output_buffer_copy_of_pointer,
          &i_outbytes_left
      );
      if(i_result == -1){
        printf("An error occurred when transliterating (iconv), errno = %d \n", errno);
        switch(errno){
          case EILSEQ:
            printf(
              "The conversion stopped because of an invalid byte sequence in the input.\n"
              "After the call, *inbuf points at the first byte of the invalid byte sequence.\n"
              "Input string before: pointer %p and content %s\n"
              "Input string after: pointer %p and content %s\n",
              (void*)s_input_string,
              s_input_string,
              (void*)s_input_string_copy_of_pointer,
              s_input_string_copy_of_pointer
            );
          break;

          case E2BIG:
            printf("The conversion stopped because it ran out of space in the output buffer.\n");
          break;

          case EINVAL:
            printf("The conversion stopped because of an incomplete byte sequence at the end of the input buffer.\n");
          break;

          case EBADF:
            printf("The cd argument is invalid.\n");
          break;
        }
        s_input_string_copy_of_pointer = s_input_string;
        i_copy_size_input_string = i_size_input_string;
        break;
      }
      s_input_string_copy_of_pointer = s_input_string;
      i_copy_size_input_string = i_size_input_string;

      s_output_string = (unsigned char *) calloc(i_result, sizeof(unsigned char));
      if(s_output_string == NULL){
        printf("An error occurred when allocating string (iconv)\n");
        i_result = I_ERROR__COULD_NOT_ALLOCATE_MEMORY;
        break;
      }

      memcpy(
          s_output_string,
          output_buffer,
          i_result
      );
      output_buffer_copy_of_pointer = output_buffer;
      i_outbytes_left = 1024;

      i_result = from_string_to_file(
          "result_iconv.test_result",
          s_output_string,
          i_size_output_string
      );
      if(i_result != 0){
        printf(
            "An error occurred when writing result (iconv) (error code %d).\n",
            i_result
        );
        break;
      }

      //test if both files are equal
      i_result = mydiff("expected_output_string.txt", "result_iconv.test_result");

      if(i_result != 0){
        printf(
            "The transliteration (iconv) is wrong (error code %d).\n",
            i_result
        );
        break;
      }
      printf("ok.\n");

      free(s_output_string);
      s_output_string = NULL;

      setlocale(LC_ALL, s_saved_locale);
      free(s_saved_locale);
      s_saved_locale = NULL;
      iconv_close(iconv_descriptor);
      iconv_descriptor = (iconv_t)-1;
    }


    //---------------------------------------------------
    //First benchmark: ideal conditions
    //---------------------------------------------------
    printf("First benchmark: ideal conditions\n");
    printf("-with text profile: ");
    i_result = transliteration_profile_load_from_text(
        "tp_benchmark.txt",
        &p_transliteration_profile,
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
      return i_result;
    }

    clock_gettime(CLOCK_MONOTONIC, &start);
    for(long int i = 0; i < i_number_of_iterations; ++i){
      i_result = transliteration_profile_iconv(
          p_transliteration_profile,
          s_input_string,
          i_size_input_string,
          &s_output_string,
          &i_size_output_string,
          &i_current_read_offset
      );
      if(i_result != 0){
        printf(
            "An error occurred when transliterating (text)"
            " (error code %d, current_read_offset %zu).\n",
            i_result,
            i_current_read_offset
        );
        break;
      }
      free(s_output_string);
      s_output_string = NULL;
    }
    clock_gettime(CLOCK_MONOTONIC, &finish);
    f_elapsed_time = finish.tv_sec - start.tv_sec;
    f_elapsed_time += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("%f \n", f_elapsed_time);

    transliteration_profile_free(p_transliteration_profile);
    p_transliteration_profile = NULL;


    printf("-with binary profile: ");
    i_result = transliteration_profile_load_from_bin(
        "tp_benchmark_bin.test_result",
        &p_transliteration_profile,
        &i_current_offset
    );

    if(i_result != 0){
      printf(
          "An error occurred when loading profile from bin at offset %zu (error code %d).\n",
          i_current_offset,
          i_result
      );
      return i_result;
    }

    clock_gettime(CLOCK_MONOTONIC, &start);
    for(long int i = 0; i < i_number_of_iterations; ++i){
      i_result = transliteration_profile_iconv(
          p_transliteration_profile,
          s_input_string,
          i_size_input_string,
          &s_output_string,
          &i_size_output_string,
          &i_current_read_offset
      );
      if(i_result != 0){
        printf(
            "An error occurred when transliterating (bin)"
            " (error code %d, current_read_offset %zu).\n",
            i_result,
            i_current_read_offset
        );
        break;
      }

      free(s_output_string);
      s_output_string = NULL;
    }
    clock_gettime(CLOCK_MONOTONIC, &finish);
    f_elapsed_time = finish.tv_sec - start.tv_sec;
    f_elapsed_time += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("%f \n", f_elapsed_time);

    transliteration_profile_free(p_transliteration_profile);
    p_transliteration_profile = NULL;


    //---------------------------------------------------
    //Second benchmark: worst conditions
    //---------------------------------------------------
    printf("Second benchmark: worst conditions\n");
    printf("-with text profile: ");
    clock_gettime(CLOCK_MONOTONIC, &start);
    for(long int i = 0; i < i_number_of_iterations; ++i){
      i_result = transliteration_profile_load_from_text(
          "tp_benchmark.txt",
          &p_transliteration_profile,
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
        return i_result;
      }

      i_result = transliteration_profile_iconv(
          p_transliteration_profile,
          s_input_string,
          i_size_input_string,
          &s_output_string,
          &i_size_output_string,
          &i_current_read_offset
      );
      if(i_result != 0){
        printf(
            "An error occurred when transliterating (text)"
            " (error code %d, current_read_offset %zu).\n",
            i_result,
            i_current_read_offset
        );
        break;
      }
      free(s_output_string);
      s_output_string = NULL;
      transliteration_profile_free(p_transliteration_profile);
      p_transliteration_profile = NULL;
    }
    clock_gettime(CLOCK_MONOTONIC, &finish);
    f_elapsed_time = finish.tv_sec - start.tv_sec;
    f_elapsed_time += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("%f \n", f_elapsed_time);


    printf("-with binary profile: ");
    clock_gettime(CLOCK_MONOTONIC, &start);
    for(long int i = 0; i < i_number_of_iterations; ++i){
      i_result = transliteration_profile_load_from_bin(
          "tp_benchmark_bin.test_result",
          &p_transliteration_profile,
          &i_current_offset
      );

      if(i_result != 0){
        printf(
            "An error occurred when loading profile from bin at offset %zu (error code %d).\n",
            i_current_offset,
            i_result
        );
        return i_result;
      }

      i_result = transliteration_profile_iconv(
          p_transliteration_profile,
          s_input_string,
          i_size_input_string,
          &s_output_string,
          &i_size_output_string,
          &i_current_read_offset
      );
      if(i_result != 0){
        printf(
            "An error occurred when transliterating (bin)"
            " (error code %d, current_read_offset %zu).\n",
            i_result,
            i_current_read_offset
        );
        break;
      }

      free(s_output_string);
      s_output_string = NULL;
      transliteration_profile_free(p_transliteration_profile);
      p_transliteration_profile = NULL;
    }
    clock_gettime(CLOCK_MONOTONIC, &finish);
    f_elapsed_time = finish.tv_sec - start.tv_sec;
    f_elapsed_time += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("%f \n", f_elapsed_time);



    //---------------------------------------------------
    //Third benchmark: average conditions
    //---------------------------------------------------
    printf("Third benchmark: average conditions\n");
    printf("-with text profile: ");
    clock_gettime(CLOCK_MONOTONIC, &start);
    for(long int i = 0; i < i_number_of_iterations / i_number_of_sub_iterations; ++i){
      i_result = transliteration_profile_load_from_text(
          "tp_benchmark.txt",
          &p_transliteration_profile,
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
        return i_result;
      }

      for(long int i = 0; i < i_number_of_sub_iterations; ++i){
        i_result = transliteration_profile_iconv(
            p_transliteration_profile,
            s_input_string,
            i_size_input_string,
            &s_output_string,
            &i_size_output_string,
            &i_current_read_offset
        );
        if(i_result != 0){
          printf(
              "An error occurred when transliterating (text)"
              " (error code %d, current_read_offset %zu).\n",
              i_result,
              i_current_read_offset
          );
          break;
        }
        free(s_output_string);
        s_output_string = NULL;
      }
      transliteration_profile_free(p_transliteration_profile);
      p_transliteration_profile = NULL;
    }
    clock_gettime(CLOCK_MONOTONIC, &finish);
    f_elapsed_time = finish.tv_sec - start.tv_sec;
    f_elapsed_time += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("%f \n", f_elapsed_time);


    printf("-with binary profile: ");
    clock_gettime(CLOCK_MONOTONIC, &start);
    for(long int i = 0; i < i_number_of_iterations / i_number_of_sub_iterations; ++i){
      i_result = transliteration_profile_load_from_bin(
          "tp_benchmark_bin.test_result",
          &p_transliteration_profile,
          &i_current_offset
      );

      if(i_result != 0){
        printf(
            "An error occurred when loading profile from bin at offset %zu (error code %d).\n",
            i_current_offset,
            i_result
        );
        return i_result;
      }

      for(long int i = 0; i < i_number_of_sub_iterations; ++i){
        i_result = transliteration_profile_iconv(
            p_transliteration_profile,
            s_input_string,
            i_size_input_string,
            &s_output_string,
            &i_size_output_string,
            &i_current_read_offset
        );
        if(i_result != 0){
          printf(
              "An error occurred when transliterating (bin)"
              " (error code %d, current_read_offset %zu).\n",
              i_result,
              i_current_read_offset
          );
          break;
        }

        free(s_output_string);
        s_output_string = NULL;
      }
      transliteration_profile_free(p_transliteration_profile);
      p_transliteration_profile = NULL;
    }
    clock_gettime(CLOCK_MONOTONIC, &finish);
    f_elapsed_time = finish.tv_sec - start.tv_sec;
    f_elapsed_time += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("%f \n", f_elapsed_time);

  }
  while(0);

  if(p_transliteration_profile != NULL){
    printf("Freeing test profile\n");
    transliteration_profile_free(p_transliteration_profile);
  }
  if(s_input_string != NULL){
    printf("Freeing input string\n");
    free(s_input_string);
    s_input_string = NULL;
  }
  if(s_output_string != NULL){
    printf("Freeing output string\n");
    free(s_output_string);
    s_output_string = NULL;
  }
  if(iconv_descriptor != (iconv_t)-1){
    printf("Freeing iconv descriptor\n");
    iconv_close(iconv_descriptor);
    iconv_descriptor = (iconv_t)-1;
  }
  /*
  if(s_old_locale != NULL){
    free(s_old_locale);
    s_old_locale = NULL;
  }
  */
  if(s_saved_locale != NULL){
    setlocale(LC_ALL, s_saved_locale);
    free(s_saved_locale);
    s_saved_locale = NULL;
  }
  if(output_buffer != NULL){
    free(output_buffer);
    output_buffer = NULL;
    output_buffer_copy_of_pointer = NULL;
  }

  return i_result;
}//end function main()



