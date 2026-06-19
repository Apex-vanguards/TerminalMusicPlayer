EXE = mrcat_player
SRCS = src/* main.cpp
CXX = g++
CXXFLAGS = -Wall -std=c++17
LIBS = -lncurses -lpthread -ldl

all:
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(EXE) $(LIBS)
clean:
	rm -rf $(EXE)

