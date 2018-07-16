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

CC=gcc
CFLAGS=-Wall
VERSION=1_0_0

.PHONY: run-tests run-tests-dynamic clean run-benchmarks run-benchmarks-dynamic


#-----------------------------------------------------------
#Build library
#-----------------------------------------------------------
build: build-static build-dynamic


#Static library
build-static: ./bin/libtransliteration_profile_iconv_$(VERSION).a

./bin/libtransliteration_profile_iconv_$(VERSION).a: ./bin/transliteration_profile_iconv.o
	ar -rcs ./bin/libtransliteration_profile_iconv_$(VERSION).a ./bin/transliteration_profile_iconv.o
#	ar -rc ./bin/libtransliteration_profile_iconv_$(VERSION).a ./bin/transliteration_profile_iconv.o
#	ranlib ./bin/libtransliteration_profile_iconv_$(VERSION).a

./bin/transliteration_profile_iconv.o: ./transliteration_profile_iconv.h ./transliteration_profile_iconv.c
	$(CC) $(CFLAGS) -c ./transliteration_profile_iconv.c -o ./bin/transliteration_profile_iconv.o


#Dynamic library
build-dynamic: ./bin/libtransliteration_profile_iconv_$(VERSION).so

./bin/libtransliteration_profile_iconv_$(VERSION).so: ./bin/transliteration_profile_iconv_dyn.o
	$(CC) -shared -o ./bin/libtransliteration_profile_iconv_$(VERSION).so ./bin/transliteration_profile_iconv_dyn.o

./bin/transliteration_profile_iconv_dyn.o: ./transliteration_profile_iconv.h ./transliteration_profile_iconv.c
	$(CC) $(CFLAGS) -fPIC -c ./transliteration_profile_iconv.c -o ./bin/transliteration_profile_iconv_dyn.o



#-----------------------------------------------------------
#Build tests
#-----------------------------------------------------------
build-tests: build-test1 build-test2 build-test3 build-test4


#Test 1
build-test1: ./tests_benchmarks/test1/test1.exe ./tests_benchmarks/test1/test1_dyn.exe

#static linking requires the library to come after the test object
./tests_benchmarks/test1/test1.exe: ./bin/libtransliteration_profile_iconv_$(VERSION).a ./tests_benchmarks/test1/test1.o
	$(CC) -static -L./bin/ ./tests_benchmarks/test1/test1.o -ltransliteration_profile_iconv_$(VERSION) -o ./tests_benchmarks/test1/test1.exe

./tests_benchmarks/test1/test1_dyn.exe: ./bin/libtransliteration_profile_iconv_$(VERSION).so ./tests_benchmarks/test1/test1.o
	$(CC) -L./bin/ -ltransliteration_profile_iconv_$(VERSION) ./tests_benchmarks/test1/test1.o -o ./tests_benchmarks/test1/test1_dyn.exe

./tests_benchmarks/test1/test1.o: ./transliteration_profile_iconv.h ./tests_benchmarks/test_functions.c ./tests_benchmarks/test1/test1.c
	$(CC) $(CFLAGS) -c ./tests_benchmarks/test1/test1.c -o ./tests_benchmarks/test1/test1.o


#Test 2
build-test2: ./tests_benchmarks/test2/test2.exe ./tests_benchmarks/test2/test2_dyn.exe

#static linking requires the library to come after the test object
./tests_benchmarks/test2/test2.exe: ./bin/libtransliteration_profile_iconv_$(VERSION).a ./tests_benchmarks/test2/test2.o
	$(CC) -static -L./bin/ ./tests_benchmarks/test2/test2.o -ltransliteration_profile_iconv_$(VERSION) -o ./tests_benchmarks/test2/test2.exe

./tests_benchmarks/test2/test2_dyn.exe: ./bin/libtransliteration_profile_iconv_$(VERSION).so ./tests_benchmarks/test2/test2.o
	$(CC) -L./bin/ -ltransliteration_profile_iconv_$(VERSION) ./tests_benchmarks/test2/test2.o -o ./tests_benchmarks/test2/test2_dyn.exe

./tests_benchmarks/test2/test2.o: ./transliteration_profile_iconv.h ./tests_benchmarks/test_functions.c ./tests_benchmarks/test2/test2.c
	$(CC) $(CFLAGS) -c ./tests_benchmarks/test2/test2.c -o ./tests_benchmarks/test2/test2.o


#Test 3
build-test3: ./tests_benchmarks/test3/test3.exe ./tests_benchmarks/test3/test3_dyn.exe

#static linking requires the library to come after the test object
./tests_benchmarks/test3/test3.exe: ./bin/libtransliteration_profile_iconv_$(VERSION).a ./tests_benchmarks/test3/test3.o
	$(CC) -static -L./bin/ ./tests_benchmarks/test3/test3.o -ltransliteration_profile_iconv_$(VERSION) -o ./tests_benchmarks/test3/test3.exe

./tests_benchmarks/test3/test3_dyn.exe: ./bin/libtransliteration_profile_iconv_$(VERSION).so ./tests_benchmarks/test3/test3.o
	$(CC) -L./bin/ -ltransliteration_profile_iconv_$(VERSION) ./tests_benchmarks/test3/test3.o -o ./tests_benchmarks/test3/test3_dyn.exe

./tests_benchmarks/test3/test3.o: ./transliteration_profile_iconv.h ./tests_benchmarks/test_functions.c ./tests_benchmarks/test3/test3.c
	$(CC) $(CFLAGS) -c ./tests_benchmarks/test3/test3.c -o ./tests_benchmarks/test3/test3.o


#Test 4
build-test4: ./tests_benchmarks/test4/test4.exe ./tests_benchmarks/test4/test4_dyn.exe

#static linking requires the library to come after the test object
./tests_benchmarks/test4/test4.exe: ./bin/libtransliteration_profile_iconv_$(VERSION).a ./tests_benchmarks/test4/test4.o
	$(CC) -static -L./bin/ ./tests_benchmarks/test4/test4.o -ltransliteration_profile_iconv_$(VERSION) -o ./tests_benchmarks/test4/test4.exe

./tests_benchmarks/test4/test4_dyn.exe: ./bin/libtransliteration_profile_iconv_$(VERSION).so ./tests_benchmarks/test4/test4.o
	$(CC) -L./bin/ -ltransliteration_profile_iconv_$(VERSION) ./tests_benchmarks/test4/test4.o -o ./tests_benchmarks/test4/test4_dyn.exe

./tests_benchmarks/test4/test4.o: ./transliteration_profile_iconv.h ./tests_benchmarks/test_functions.c ./tests_benchmarks/test4/test4.c
	$(CC) $(CFLAGS) -c ./tests_benchmarks/test4/test4.c -o ./tests_benchmarks/test4/test4.o



#-----------------------------------------------------------
#Install
#-----------------------------------------------------------
install: /usr/lib/libtransliteration_profile_iconv_$(VERSION).so

/usr/lib/libtransliteration_profile_iconv_$(VERSION).so: ./bin/libtransliteration_profile_iconv_$(VERSION).so
	cp -p ./bin/libtransliteration_profile_iconv_$(VERSION).so /usr/lib
	chmod 755 /usr/lib/libtransliteration_profile_iconv_$(VERSION).so



#-----------------------------------------------------------
#Run tests
#-----------------------------------------------------------
test: build-tests run-tests

run-tests:
	cd ./tests_benchmarks/test1/ && echo "\nTest1:" && ./test1.exe && cd ../..
	cd ./tests_benchmarks/test2/ && echo "\nTest2:" && ./test2.exe && cd ../..
	cd ./tests_benchmarks/test3/ && echo "\nTest3:" && ./test3.exe && cd ../..
	cd ./tests_benchmarks/test4/ && echo "\nTest4:" && ./test4.exe && cd ../..

run-tests-dynamic: install
	cd ./tests_benchmarks/test1/ && echo "\nTest1 dyn:" && ./test1_dyn.exe && cd ../..
	cd ./tests_benchmarks/test2/ && echo "\nTest2 dyn:" && ./test2_dyn.exe && cd ../..
	cd ./tests_benchmarks/test3/ && echo "\nTest3 dyn:" && ./test3_dyn.exe && cd ../..
	cd ./tests_benchmarks/test4/ && echo "\nTest4 dyn:" && ./test4_dyn.exe && cd ../..



#-----------------------------------------------------------
#Build benchmarks
#-----------------------------------------------------------
build-benchmarks: build-benchmark1


#Benchmark 1
build-benchmark1: ./tests_benchmarks/benchmark1/benchmark1.exe ./tests_benchmarks/benchmark1/benchmark1_dyn.exe
#/usr/share/i18n/locales/fr_FR@test /usr/share/i18n/charmaps/ISO-8859-1-test.gz
#	echo "\nfr_FR@test ISO-8859-1-test\n" >> /etc/locale.gen
#	locale-gen

#static linking requires the library to come after the test object
./tests_benchmarks/benchmark1/benchmark1.exe: ./bin/libtransliteration_profile_iconv_$(VERSION).a ./tests_benchmarks/benchmark1/benchmark1.o
	$(CC) -static -L./bin/ ./tests_benchmarks/benchmark1/benchmark1.o -ltransliteration_profile_iconv_$(VERSION) -o ./tests_benchmarks/benchmark1/benchmark1.exe

./tests_benchmarks/benchmark1/benchmark1_dyn.exe: ./bin/libtransliteration_profile_iconv_$(VERSION).so ./tests_benchmarks/benchmark1/benchmark1.o
	$(CC) -L./bin/ -ltransliteration_profile_iconv_$(VERSION) ./tests_benchmarks/benchmark1/benchmark1.o -o ./tests_benchmarks/benchmark1/benchmark1_dyn.exe

./tests_benchmarks/benchmark1/benchmark1.o: ./transliteration_profile_iconv.h ./tests_benchmarks/test_functions.c ./tests_benchmarks/benchmark1/benchmark1.c
	$(CC) $(CFLAGS) -c ./tests_benchmarks/benchmark1/benchmark1.c -o ./tests_benchmarks/benchmark1/benchmark1.o

/usr/share/i18n/locales/fr_FR@test: ./tests_benchmarks/benchmark1/fr_FR@test
	cp ./tests_benchmarks/benchmark1/fr_FR@test /usr/share/i18n/locales/

/usr/share/i18n/charmaps/ISO-8859-1-test.gz: ./tests_benchmarks/benchmark1/ISO-8859-1-test.gz
	cp ./tests_benchmarks/benchmark1/ISO-8859-1-test.gz /usr/share/i18n/charmaps/



#-----------------------------------------------------------
#Run benchmarks
#-----------------------------------------------------------
benchmarks: build-benchmarks run-benchmarks

run-benchmarks:
	cd ./tests_benchmarks/benchmark1/ && echo "\nBenchmark1:" && ./benchmark1.exe && cd ../..

run-benchmarks-dynamic: install
	cd ./tests_benchmarks/benchmark1/ && echo "\nBenchmark1 dyn:" && ./benchmark1_dyn.exe && cd ../..



#-----------------------------------------------------------
#Clean
#-----------------------------------------------------------
clean:
	rm -f ./bin/*
	rm -f ./tests_benchmarks/test1/*.o ./tests_benchmarks/test1/*.exe ./tests_benchmarks/test1/*.test_result
	rm -f ./tests_benchmarks/test2/*.o ./tests_benchmarks/test2/*.exe ./tests_benchmarks/test2/*.test_result
	rm -f ./tests_benchmarks/test3/*.o ./tests_benchmarks/test3/*.exe ./tests_benchmarks/test3/*.test_result
	rm -f ./tests_benchmarks/test4/*.o ./tests_benchmarks/test4/*.exe ./tests_benchmarks/test4/*.test_result
	rm -f ./tests_benchmarks/benchmark1/*.o ./tests_benchmarks/benchmark1/*.exe ./tests_benchmarks/benchmark1/*.test_result


