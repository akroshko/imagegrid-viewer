HEADERS=$(wildcard *.hpp)  $(wildcard imagegrid/*.hpp) $(wildcard c_misc/*.hpp) $(wildcard c_io_net/*.hpp)
# for clean only
SRC = $(wildcard *.cpp) $(wildcard imagegrid/*.cpp) $(wildcard c_misc/*.cpp) $(wildcard c_io_net/*.cpp)
OBJ = $(SRC:.cpp=.o)
# for compile
SRC_PROG = $(wildcard *.cpp) $(wildcard imagegrid/*.cpp)
SRC_CIONET = $(wildcard c_io_net/*.cpp)
SRC_CMISC = $(wildcard c_misc/*.cpp)
OBJ_CIONET = $(SRC_CIONET:.cpp=.o)
OBJ_CMISC = $(SRC_CMISC:.cpp=.o)
# create object variables for programs that are actually run
SRC_MAIN=$(filter-out $(wildcard test_*.cpp),$(SRC_PROG))
SRC_TEST = $(filter-out $(wildcard imagegrid-viewer.cpp),$(SRC_PROG))
OBJ_MAIN = $(SRC_MAIN:.cpp=.o)
OBJ_TEST = $(SRC_TEST:.cpp=.o)

LDFLAGS = -lstdc++ -lstdc++fs -lm -lpng -ltiff -lSDL2 -pthread
CXXFLAGS_SAFE = -std=c++17 -Wall -Wextra -Wshadow -Wundef -Wpedantic
CXXFLAGS_UNSAFE = -std=c++17 -Wall -Wextra -Wshadow -Wundef
# -Wconversion -Wsign-conversion

# TODO: reenable test code
# all: imagegrid-viewer test_file
all: imagegrid-viewer

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
	clang-tidy -checks=bugprone-*,cert-* -header-filter=.* $(SRC_MAIN) $(SRC_CIONET) $(SRC_CMISC)

.PHONY: iwyu
iwyu:
	echo "$(SRC_MAIN) $(SRC_CIONET) $(SRC_CMISC)"'\0' | tr ' ' '\0' | xargs -0 -n1 iwyu -Xiwyu --cxx17ns -Xiwyu --no_fwd_decls -std=c++17

imagegrid-viewer: CXXFLAGS_SAFE +=  -g3 -O2
imagegrid-viewer: CXXFLAGS_UNSAFE += -g3 -O2
imagegrid-viewer: $(OBJ_MAIN) $(OBJ_CIONET) $(OBJ_CMISC)
	$(CC) -o $@ $^ $(LDFLAGS)

test_file: CXXFLAGS_SAFE += -g3 -O0
test_file: CXXFLAGS_UNSAFE += -g3 -O0
test_file: $(OBJ_TEST) $(OBJ_CIONET) $(OBJ_CMISC)
	$(CC) $(CXXFLAGS_SAFE) -o $@ $^ $(LDFLAGS)

$(OBJ_MAIN): $(SRC_MAIN) $(HEADERS)
	$(CC) $(CXXFLAGS_SAFE) -c -o $@ $*.cpp

$(OBJ_CIONET) $(OBJ_CMISC): $(SRC_CIONET) $(SRC_CMISC) $(HEADERS)
	$(CC) $(CXXFLAGS_UNSAFE) -c -o $@ $*.cpp

.PHONY: clean
clean:
	rm -f $(OBJ)

# TODO: disabling debug build for now
# .PHONY: debug
# debug: imagegrid-viewer-debug
#
# imagegrid-viewer-debug: CXXFLAGS += -DDEBUG_MESSAGES -DDEBUG_IO -fsanitize=thread,undefined -g3 -O0
# imagegrid-viewer-debug: $(OBJ_MAIN) imagegrid-viewer.o
# 	$(CC) -o $@ $^ $(LDFLAGS)
