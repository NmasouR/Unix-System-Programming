OBJS = vaccineMonitor.o bloom.o hash.o citizen.o file.o skiplist.o virusList.o
SOURCE = vaccineMonitor.cpp bloom.cpp hash.cpp citizen.cpp file.cpp skiplist.cpp virusList.cpp
HEADER = bloom.h hash.h citizen.h file.h skiplist.h virusList.h
OUT = vaccineMonitor
CC = g++
FLAGS = -std=c++11 -c

$(OUT): $(OBJS)
	$(CC) -o $@ $(OBJS)

vaccineMonitor.o: vaccineMonitor.cpp
	g++ -c  vaccineMonitor.cpp

bloom.o: bloom.cpp
	$(CC) -c bloom.cpp

hash.o: hash.cpp
	$(CC) -c hash.cpp

citizen.o: citizen.cpp
	$(CC) -c citizen.cpp

file.o: file.cpp
	$(CC) $(FLAGS) file.cpp

skiplist.o: skiplist.cpp
	$(CC) -c skiplist.cpp

virusList.o: virusList.cpp
	$(CC) -c virusList.cpp


clean:
	rm -f $(OBJS) $(OUT)