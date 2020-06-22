all : bin/readFt4222Status bin/readMt25Status bin/mt25Writer bin/mt25Reader bin/mt25Tester

bin/readFt4222Status : build/readFt4222Status.o
	g++ build/readFt4222Status.o -lft4222 -L lib -o bin/readFt4222Status

build/readFt4222Status.o : src/readFt4222Status.cpp include/mt25Flash.h include/ft4222CustomLibrary.h
	g++ src/readFt4222Status.cpp -I include -std=c++11 -c -o build/readFt4222Status.o

bin/readMt25Status : build/readMt25Status.o
	g++ build/readMt25Status.o -lft4222 -L lib -o bin/readMt25Status

build/readMt25Status.o : src/readMt25Status.cpp include/mt25Flash.h include/ft4222CustomLibrary.h
	g++ src/readMt25Status.cpp -I include -std=c++11 -c -o build/readMt25Status.o

bin/mt25Writer : build/mt25Writer.o
	g++ build/mt25Writer.o -lft4222 -L lib -o bin/mt25Writer

build/mt25Writer.o : src/mt25Writer.cpp include/mt25Flash.h include/ft4222CustomLibrary.h
	g++ src/mt25Writer.cpp -I include -std=c++11 -c -o build/mt25Writer.o

bin/mt25Reader : build/mt25Reader.o
	g++ build/mt25Reader.o -lft4222 -L lib -o bin/mt25Reader

build/mt25Reader.o : src/mt25Reader.cpp include/mt25Flash.h include/ft4222CustomLibrary.h
	g++ src/mt25Reader.cpp -I include -std=c++11 -c -o build/mt25Reader.o

bin/mt25Tester : build/mt25Tester.o
	g++ build/mt25Tester.o -lft4222 -L lib -o bin/mt25Tester

build/mt25Tester.o : src/mt25Tester.cpp include/mt25Flash.h include/ft4222CustomLibrary.h
	g++ src/mt25Tester.cpp -I include -std=c++11 -c -o build/mt25Tester.o

clean:
	rm -f build/*
	rm -f bin/*
