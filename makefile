OBJS = MetaDataInfo.o ConfigFileInput.o main.o
FILESET = ConfigFileInput.hh ConfigFileInput.cc MetaDataInfo.hh MetaDataInfo.cc main.cpp
CC = g++ -std=c++11
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

Sim05: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o Sim05 -lpthread
ConfigFileInput.o: ConfigFileInput.hh ConfigFileInput.cc
	$(CC) $(CFLAGS) ConfigFileInput.cc
MetaDataInfo.o: MetaDataInfo.hh MetaDataInfo.cc ConfigFileInput.hh PCB.hh MemoryFunction.hh 
	$(CC) $(CFLAGS) MetaDataInfo.cc
main.o:	main.cpp ConfigFileInput.hh  MetaDataInfo.hh PCB.hh
	$(CC) $(CFLAGS) main.cpp
Sim05_NelsonEugene.tar.gz: $(FILESET) MemoryFunction.hh PCB.hh makefile refman.pdf
	tar -cvzf $@ $^
clean:
	rm *.o Sim05
