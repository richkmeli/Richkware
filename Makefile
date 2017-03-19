ifeq ($(OS),Windows_NT)
    CC=g++    
else
    CC=x86_64-w64-mingw32-g++
endif

CFLAGS= -c -O3 -Wall
LDFLAGS= -static-libgcc -static-libstdc++ -Wall
EFLAG= -lws2_32
SOURCES=main.cpp crypto.cpp sharedList.cpp blockApps.cpp richkware.cpp network.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=Richkware.exe

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(EFLAG) -o  $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.o *~
