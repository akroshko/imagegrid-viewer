HEADERS=$(wildcard *.hpp) $(wildcard cinterface/*.hpp) $(wildcard cdata/*.hpp)
SRC = $(wildcard *.cpp) $(wildcard cinterface/*.cpp) $(wildcard cdata/*.cpp)
SRC_MAIN = $(filter-out $(wildcard test_*.cpp),$(SRC))
OBJ_MAIN = $(SRC_MAIN:.cpp=.o)
SRC_TEST = $(filter-out $(wildcard imagegrid-viewer.cpp),$(SRC))
OBJ_TEST = $(SRC_TEST:.cpp=.o)
OBJ = $(SRC:.cpp=.o)

LDFLAGS = -lstdc++ -lstdc++fs -lm -lpng -ltiff -lSDL2 -pthread
CXXFLAGS = -std=c++17 -Wall -Wshadow -Wundef
# -Wconversion -Wsign-conversion

all: imagegrid-viewer test_file

.PHONY: etags
etags:
	find . -type f \( -iname "*.[ch]pp" -o -iname "*.py" -o -iname "Makefile" \) -print | etags -

.PHONY: compilation_commands
compilation_commands:
	make clean; bear -- make all
	rc -J

.PHONY: rtags
rtags:
	rc -J

.PHONY: tidy
tidy:
	clang-tidy -checks=bugprone-*,cert-* -header-filter=.* $(SRC_MAIN)

.PHONY: debug
debug: imagegrid-viewer-debug

imagegrid-viewer-debug: CXXFLAGS += -DDEBUG_MESSAGES -DDEBUG_IO -g -O0
imagegrid-viewer-debug: $(OBJ_MAIN) imagegrid-viewer.o
	$(CC) -o $@ $^ $(LDFLAGS)

imagegrid-viewer: CXXFLAGS += -O2 -g
imagegrid-viewer: $(OBJ_MAIN) imagegrid-viewer.o
	$(CC) -o $@ $^ $(LDFLAGS)

imagegrid-viewer.o: $(SRC_MAIN) $(HEADERS)

# might want a debug flag thing
test_file: CXXFLAGS += -DDEBUG_MESSAGES -DDEBUG_IO -g -O0
test_file: $(OBJ_TEST) test_file.o
	$(CC) -o $@ $^ $(LDFLAGS)

test_file.o: $(SRC_TEST) $(HEADERS)

.PHONY: clean
clean:
	rm -f $(OBJ)
