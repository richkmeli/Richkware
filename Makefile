ifeq ($(OS),Windows_NT)
	CC=g++    
else
    CC=x86_64-w64-mingw32-g++
endif

CFLAGS=-c -O3
LDFLAGS= -static-libgcc -static-libstdc++
EFLAG= -lws2_32
SOURCES=main.cpp crypto.cpp thread.cpp sharedList.cpp blockApps.cpp richkware.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=Richkware.exe

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(EFLAG) -o  $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@


