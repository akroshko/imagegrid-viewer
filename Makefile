HEADERS=$(wildcard *.hpp)
SRC = $(wildcard *.cpp)
OBJ = $(SRC:.cpp=.o)

LDFLAGS = -lstdc++ -lstdc++fs -lm -lpng -ltiff -lSDL2 -pthread
CXXFLAGS = -std=c++17 -Wall
# -Wconversion -Wsign-conversion

all: imagegrid-viewer

.PHONY: debug
debug: CXXFLAGS += -DDEBUG_MESSAGES -DDEBUG_IO -g
debug: imagegrid-viewer

imagegrid-viewer: $(OBJ) imagegrid-viewer.o
	$(CC) -o $@ $^ $(LDFLAGS)

imagegrid-viewer.o: $(SRC) $(HEADERS)

.PHONY: clean
clean:
	rm -f $(OBJ)
