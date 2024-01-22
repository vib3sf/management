CXX = g++
CXXFLAGS = -Wall -g

SRC = $(filter-out main.cpp, $(wildcard *.cpp))
OBJ = $(patsubst %.cpp, %.o, $(SRC))

all: server

%.o: %.cpp $.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

server: main.cpp $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

ifneq (clean, $(MAKECMDGOALS))
-include deps.mk
endif

deps.mk: $(SRC)
	$(CC) -MM $^ > $@

clean:
	rm server
