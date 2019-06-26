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

# flags
CXXFLAGS= -c -std=c++11 -O3 -Wall
LDFLAGS= -static-libgcc -static-libstdc++ -static -Wall
EFLAG= -lws2_32

# directories
SRCDIR   = src
OBJDIR   = obj
BINDIR   = bin
TESTDIR	 = test

# files
SOURCES  := $(wildcard $(SRCDIR)/*.cpp)
INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
EXECUTABLE := $(BINDIR)/$(TARGET).exe
TESTS	 := $(wildcard $(TESTDIR)/*.cpp)
TEST_OBJECTS  := $(TESTS:$(TESTDIR)/%.cpp=$(TESTDIR)/$(OBJDIR)/%.o)
TEST_EXECUTABLE := $(TESTDIR)/$(BINDIR)/$(TARGET_TEST).exe

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
test: $(EXECUTABLE_TEST)

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

$(EXECUTABLE): $(OBJECTS) main.o
	$(CXX) $(LDFLAGS) $(OBJECTS) $(OBJDIR)/main.o $(EFLAG) -o $@

main.o:
	$(CXX) $(CXXFLAGS) main.cpp -o $(OBJDIR)/main.o

$(EXECUTABLE_TEST): $(OBJECTS_TEST) test.o
	$(CXX) $(LDFLAGS) $(TESTDIR)/$(OBJDIR) $(TESTDIR)/$(OBJDIR)/test.o $(EFLAG) -o $@

$(OBJECTS_TEST): $(TESTDIR)/$(OBJDIR)/%.o : $(TESTDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

test.o:
	$(CXX) $(CXXFLAGS) $(TESTDIR)/test.cpp -o $(TESTDIR)/$(OBJDIR)/test.o

