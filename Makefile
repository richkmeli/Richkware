# Copyright 2016 Riccardo Melioli.

# executable name
TARGET   = Richkware

ifeq ($(OS),Windows_NT)
    CXX=g++
    RM=cmd /C rmdir /Q /S
    MKDIR=mkdir
else
    CXX=x86_64-w64-mingw32-g++
    RM=rm -rf
    MKDIR=mkdir -p
endif

# directories
SRCDIR   = src
OBJDIR   = obj
BINDIR   = bin
TESTDIR	 = test
INCLUDES = -I($(HOME)/include
LIBINCLUDES = -L $(HOME)/lib

# files
SOURCES  := $(wildcard $(SRCDIR)/*.cpp)
INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
EXECUTABLE := $(BINDIR)/$(TARGET).exe
TESTS	 := $(wildcard $(TESTDIR)/*.cpp)
EXECUTABLE_TEST := $(TESTDIR)/$(TARGET)_TEST.exe
INC = $(INCLUDES) $(LIBINCLUDES)

# flags
CXXFLAGS= -c -std=c++11 -O3 -Wall  -I include/
LDFLAGS= -static-libgcc -static-libstdc++ -static -Wall
EFLAG= -lws2_32


.PHONY : all
all: clean make_directories build

.PHONY : build
build: $(EXECUTABLE)

.PHONY : make_directories
make_directories:
	$(MKDIR) $(OBJDIR) $(BINDIR)

.PHONY : clean
clean:
	-$(RM) $(OBJDIR) $(BINDIR)

.PHONY : test
test: clean make_directories $(EXECUTABLE_TEST)

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

$(EXECUTABLE): $(OBJECTS) main.o
	$(CXX) $(LDFLAGS) $(OBJECTS) $(OBJDIR)/main.o $(EFLAG) -o $@

main.o:
	$(CXX) $(CXXFLAGS) main.cpp -o $(OBJDIR)/main.o

$(EXECUTABLE_TEST): $(OBJECTS) test.o
	$(CXX) $(LDFLAGS) $(OBJECTS) $(TESTDIR)/test.o $(EFLAG) -o $@

test.o:
	$(CXX) $(CXXFLAGS) $(TESTDIR)/test.cpp -o $(TESTDIR)/test.o

