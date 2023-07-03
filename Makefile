HEADERS=$(wildcard *.hpp)
SRC = $(wildcard *.cpp)
OBJ = $(SRC:.cpp=.o)

LDFLAGS = -lstdc++ -lstdc++fs -lm -lpng -ltiff -lSDL2 -pthread
CXXFLAGS = -std=c++17 -Wall -O0
# -Wconversion -Wsign-conversion

all: imagegrid-viewer

.PHONY: etags
etags:
	find . -type f \( -iname "*.[ch]pp" -o -iname "*.py" -o -iname "Makefile" \) -print | etags -

.PHONY: debug
debug: CXXFLAGS += -DDEBUG_MESSAGES -DDEBUG_IO -g
debug: imagegrid-viewer

imagegrid-viewer: $(OBJ) imagegrid-viewer.o
	$(CC) -o $@ $^ $(LDFLAGS)

imagegrid-viewer.o: $(SRC) $(HEADERS)

.PHONY: clean
clean:
	rm -f $(OBJ)
