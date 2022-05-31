OBJS1 = travelMonitor.o travelVirus.o queries.o bloom.o hash.o signalhandler.o

SOURCE1 = travelMonitor.cpp travelVirus.cpp queries.cpp bloom.cpp hash.cpp signalhandler.cpp

HEADER1 = bloom.h hash.h travelVirus.h queries.h signalhandler.h

OBJS2 = Monitor.o MonitorGet.o MonitorData.o bloom.o hash.o virusList.o citizen.o skiplist.o

SOURCE2 = Monitor.cpp MonitorGet.cpp MonitorData.cpp virusList.cpp citizen.cpp bloom.cpp hash.cpp skiplist.cpp

HEADER2 = bloom.h hash.h MonitorGet.h MonitorData.h virusList.h citizen.h skiplist.h

OUT1 = travelMonitor

OUT2 = Monitor

CC = g++
FLAGS = -std=c++11 -c

all : $(OUT1) $(OUT2)

$(OUT1): $(OBJS1)
	$(CC) -o $@ $(OBJS1)

$(OUT2): $(OBJS2)
	$(CC) -o $@ $(OBJS2)

travelMonitor.o: travelMonitor.cpp
	$(CC) -c  travelMonitor.cpp

travelVirus.o: travelVirus.cpp
	$(CC) -c travelVirus.cpp

queries.o: queries.cpp
	$(CC) -c queries.cpp

signalhandler.o: signalhandler.cpp
	$(CC) -c signalhandler.cpp

Monitor.o: Monitor.cpp
	$(CC) -c  Monitor.cpp

bloom.o: bloom.cpp
	$(CC) -c bloom.cpp

hash.o: hash.cpp
	$(CC) -c hash.cpp

citizen.o: citizen.cpp
	$(CC) -c citizen.cpp

MonitorData.o: MonitorData.cpp
	$(CC) -c MonitorData.cpp

MonitorGet.o: MonitorGet.cpp
	$(CC) -c MonitorGet.cpp

skiplist.o: skiplist.cpp
	$(CC) -c skiplist.cpp

virusList.o: virusList.cpp
	$(CC) -c virusList.cpp


clean:
	rm -f $(OBJS1) $(OUT1) $(OBJS2) $(OUT2)

