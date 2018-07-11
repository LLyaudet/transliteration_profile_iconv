#This file is part of transliteration_profile_iconv library.
#
#transliteration_profile_iconv is free software: you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation, either version 3 of the License, or
#(at your option) any later version.
#
#transliteration_profile_iconv is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.
#
#You should have received a copy of the GNU General Public License
#along with transliteration_profile_iconv.  If not, see <http://www.gnu.org/licenses/>.
#
#©Copyright 2018 Laurent Lyaudet


build-tests: build-test1 build-test2 build-test3 build-test4

build-test1:
	gcc -W ./tests_benchmarks/test1/test1.c -o ./tests_benchmarks/test1/test1.exe

build-test2:
	gcc -W ./tests_benchmarks/test2/test2.c -o ./tests_benchmarks/test2/test2.exe

build-test3:
	gcc -W ./tests_benchmarks/test3/test3.c -o ./tests_benchmarks/test3/test3.exe

build-test4:
	gcc -W ./tests_benchmarks/test4/test4.c -o ./tests_benchmarks/test4/test4.exe

clean:
	rm -f ./tests_benchmarks/test1/*.exe
	rm -f ./tests_benchmarks/test1/*.test_result
	rm -f ./tests_benchmarks/test2/*.exe
	rm -f ./tests_benchmarks/test2/*.test_result
	rm -f ./tests_benchmarks/test3/*.exe
	rm -f ./tests_benchmarks/test3/*.test_result
	rm -f ./tests_benchmarks/test4/*.exe
	rm -f ./tests_benchmarks/test4/*.test_result
