ifeq ($(OS),Windows_NT)
    CXX=g++    
else
    CXX=x86_64-w64-mingw32-g++
endif


CXXFLAGS= -c -O3 -Wall
LDFLAGS= -static-libgcc -static-libstdc++ -static -Wall
EFLAG= -lws2_32

SOURCES= main.cpp crypto.cpp sharedResources.cpp blockApps.cpp richkware.cpp network.cpp storage.cpp protocol.cpp
OBJECTS= $(SOURCES:.cpp=.o)
EXECUTABLE= Richkware.exe

.PHONY : all
all: $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) $^ $(EFLAG) -o  $@

.cpp.o:
	$(CXX) $(CXXFLAGS) $< -o $@

.PHONY : clean
clean:
	rm -f *.o *~
