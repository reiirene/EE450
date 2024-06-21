# Hsin Li
# 2024-06-20
# EE450 Final Project
# Makefile for the server and client programs

# Define the compiler and the flags
CXX = g++
CXXFLAGS = -Wall -g -std=c++11

# Define the target executables
TARGETS = serverM serverC serverRTH serverEEB client

# Define the source and object files for each target
SRCS_serverM = serverM.cpp serverM.h
SRCS_serverC = serverC.cpp serverC.h
SRCS_serverRTH = serverRTH.cpp serverRTH.h
SRCS_serverEEB = serverEEB.cpp serverEEB.h
SRCS_client = client.cpp client.h

OBJS_serverM = $(SRCS_serverM:.cpp=.o)
OBJS_serverC = $(SRCS_serverC:.cpp=.o)
OBJS_serverRTH = $(SRCS_serverRTH:.cpp=.o)
OBJS_serverEEB = $(SRCS_serverEEB:.cpp=.o)
OBJS_client = $(SRCS_client:.cpp=.o)

# Default target
all: $(TARGETS)

# Rule to link the object files into executables
serverM: $(OBJS_serverM)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS_serverM)

serverC: $(OBJS_serverC)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS_serverC)

serverRTH: $(OBJS_serverRTH)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS_serverRTH)

serverEEB: $(OBJS_serverEEB)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS_serverEEB)

client: $(OBJS_client)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS_client)

# Rule to compile the source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

# Clean up the generated files
clean:
	rm -f $(OBJS_serverM) $(OBJS_serverC) $(OBJS_serverRTH) $(OBJS_serverEEB) $(OBJS_client) $(TARGETS)

# Declare targets that do not correspond to actual files
.PHONY: all clean
