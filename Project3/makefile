OBJS1 = travelMonitorClient.o travelVirus.o queries.o bloom.o hash.o

SOURCE1 = travelMonitor.cpp travelVirus.cpp queries.cpp bloom.cpp hash.cpp

HEADER1 = bloom.h hash.h travelVirus.h queries.h

OBJS2 = monitorServer.o MonitorGet.o MonitorData.o bloom.o hash.o virusList.o citizen.o skiplist.o thread.o

SOURCE2 = monitorServer.cpp MonitorGet.cpp MonitorData.cpp virusList.cpp citizen.cpp bloom.cpp hash.cpp skiplist.cpp thread.cpp

HEADER2 = bloom.h hash.h MonitorGet.h MonitorData.h virusList.h citizen.h skiplist.h thread.h

OUT1 = travelMonitorClient

OUT2 = monitorServer

CC = g++
FLAGS = -std=c++11 -c

all : $(OUT1) $(OUT2)

$(OUT1): $(OBJS1)
	$(CC) -o $@ $(OBJS1)

$(OUT2): $(OBJS2)
	$(CC) -pthread -o $@ $(OBJS2)

travelMonitorClient.o: travelMonitorClient.cpp
	$(CC) -c  travelMonitorClient.cpp

travelVirus.o: travelVirus.cpp
	$(CC) -c travelVirus.cpp

queries.o: queries.cpp
	$(CC) -c queries.cpp

monitorServer.o: monitorServer.cpp
	$(CC) -c  monitorServer.cpp
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

thread.o: thread.cpp
	$(CC) -c thread.cpp


clean:
	rm -f $(OBJS1) $(OUT1) $(OBJS2) $(OUT2)
