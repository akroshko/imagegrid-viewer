# https://spin.atomicobject.com/2016/08/26/makefile-c-projects/
# http://nuclear.mutantstargoat.com/articles/make/
#   see section a makefile for 99% of your programs

HEADERS=$(wildcard *.hpp)
SRC = $(wildcard *.cpp)
OBJ = $(SRC:.cpp=.o)

LDFLAGS = -lstdc++ -lstdc++fs -lm -lpng -ltiff -lSDL2
CXXFLAGS = -std=c++17 -g -Wall
# doesn't make a huge difference for now
# CXXFLAGS = -std=c++17 -O3 -funroll-loops -Wall

imagegrid-viewer: $(OBJ) imagegrid-viewer.o
	$(CC) -o $@ $^ $(LDFLAGS)

imagegrid-viewer.o: $(SRC) $(HEADERS)

.PHONY: clean
clean:
	rm -f $(OBJ)
