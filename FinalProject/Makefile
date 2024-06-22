# Hsin Li
# 2024-06-20
# EE450 Final Project
# Makefile for the server and client programs

CXX = g++
CXXFLAGS = -Wall -std=c++11
LDFLAGS =

# List of source files
SOURCES = serverM.cpp serverC.cpp serverRTH.cpp serverEEB.cpp client.cpp
# Generate list of object files
OBJECTS = $(SOURCES:.cpp=.o)
# Generate list of executable names
EXECUTABLES = $(SOURCES:.cpp=)

all: $(EXECUTABLES)

serverM: serverM.o
	$(CXX) $(LDFLAGS) -o $@ $^

serverC: serverC.o
	$(CXX) $(LDFLAGS) -o $@ $^

serverRTH: serverRTH.o
	$(CXX) $(LDFLAGS) -o $@ $^

serverEEB: serverEEB.o
	$(CXX) $(LDFLAGS) -o $@ $^

client: client.o
	$(CXX) $(LDFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLES)