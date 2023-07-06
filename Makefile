HEADERS=$(wildcard *.hpp)
SRC = $(wildcard *.cpp)
SRC_MAIN = $(filter-out $(wildcard test_*.cpp),$(SRC))
OBJ_MAIN = $(SRC_MAIN:.cpp=.o)
SRC_TEST = $(filter-out $(wildcard imagegrid-viewer.cpp),$(SRC))
OBJ_TEST = $(SRC_TEST:.cpp=.o)
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

imagegrid-viewer: $(OBJ_MAIN) imagegrid-viewer.o
	$(CC) -o $@ $^ $(LDFLAGS)

imagegrid-viewer.o: $(SRC_MAIN) $(HEADERS)

# might want a debug flag thing
test_file: CXXFLAGS += -DDEBUG_MESSAGES -DDEBUG_IO -g
test_file: $(OBJ_TEST) test_file.o
	$(CC) -o $@ $^ $(LDFLAGS)

test_file.o: $(SRC_TEST) $(HEADERS)

.PHONY: clean
clean:
	rm -f $(OBJ)
