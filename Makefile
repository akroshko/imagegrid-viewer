CC=clang
HEADERS=$(wildcard *.hpp) $(wildcard datatypes/*.hpp) $(wildcard imagegrid/*.hpp) $(wildcard c_misc/*.hpp) $(wildcard c_sdl2/*.hpp) $(wildcard c_io_net/*.hpp)
# for clean only
SRC = $(wildcard *.cpp) $(wildcard datatypes/*.cpp) $(wildcard imagegrid/*.cpp) $(wildcard c_misc/*.cpp) $(wildcard c_sdl2/*.cpp) $(wildcard c_io_net/*.cpp)
OBJ = $(SRC:.cpp=.o)
# grops of source files
SRC_PROG = $(wildcard *.cpp) $(wildcard datatypes/*.cpp) $(wildcard imagegrid/*.cpp)
SRC_CIONET = $(wildcard c_io_net/*.cpp)
SRC_CSDL = $(wildcard c_sdl2/*.cpp)
SRC_CMISC = $(wildcard c_misc/*.cpp)
SRC_NOMAIN = $(filter-out $(wildcard imagegrid-viewer.cpp), $(SRC_PROG) $(SRC_CIONET) $(SRC_CSDL) $(SRC_CMISC))
# groups of object files
OBJ_PROG = $(SRC_PROG:.cpp=.o)
OBJ_CIONET = $(SRC_CIONET:.cpp=.o)
OBJ_CSDL = $(SRC_CSDL:.cpp=.o)
OBJ_CMISC = $(SRC_CMISC:.cpp=.o)
OBJ_NOMAIN = $(SRC_NOMAIN:.cpp=.o)
# the primary program
SRC_PRIMARY = $(SRC_NOMAIN) imagegrid-viewer.cpp
OBJ_PRIMARY = $(SRC_PRIMARY:.cpp=.o)
# unit tests
SRC_TEST_BASIC = $(SRC_NOMAIN) tests/test_basic.cpp
OBJ_TEST_BASIC = $(SRC_TEST_BASIC:.cpp=.o)
# manual/integration tests

# create object variables for programs that are actually run
# SRC_MAIN=$(filter-out $(wildcard test_*.cpp),$(SRC_PROG))

OBJ_MAIN = $(SRC_MAIN:.cpp=.o)
OBJ_ALL=$(OBJ) $(OBJ_TEST)

LDFLAGS=-flto=auto -lstdc++ -lstdc++fs -lm -lpng -lSDL2 -lSDL2_ttf -ltiff -lzip -pthread
CXXFLAGS_ALWAYS=-std=c++17 -fno-exceptions -Wall -Wextra -Wshadow -Wundef
CXXFLAGS_SAFE=$(CXXFLAGS_ALWAYS) -Wpedantic
CXXFLAGS_UNSAFE=$(CXXFLAGS_ALWAYS) -fno-rtti
CXXFLAGS_SDL=$(CXXFLAGS_ALWAYS)
# -Wconversion -Wsign-conversion
# specific CFLAGS
CXXFLAGS_ALL=-g3 -O2
CXXFLAGS_FAST=-O3 -march=native
CXXFLAGS_SANITIZE=-g3 -O2
CXXFLAGS_PROFILE=-g3 -O2 -pg
CXXFLAGS_DEBUG=-g3 -O0

# TODO: reenable test code
# all: imagegrid-viewer test_file
all: CXXFLAGS_SAFE += $(CXXFLAGS_ALL)
all: CXXFLAGS_UNSAFE += $(CXXFLAGS_ALL)
all: CXXFLAGS_SDL += $(CXXFLAGS_ALL)
all: imagegrid-viewer test

.PHONY: release
fast: CXXFLAGS_SAFE += $(CXXFLAGS_FAST)
fast: CXXFLAGS_UNSAFE += $(CXXFLAGS_FAST)
fast: CXXFLAGS_SDL += $(CXXFLAGS_FAST)
fast: imagegrid-viewer

.PHONY: sanitize-address-undefined
sanitize-address-undefined: CXXFLAGS_SAFE += $(CXXFLAGS_SANITIZE) -fsanitize=address,undefined
sanitize-address-undefined: CXXFLAGS_UNSAFE += $(CXXFLAGS_SANITIZE) -fsanitize=address,undefined
sanitize-address-undefined: CXXFLAGS_SDL += $(CXXFLAGS_SANITIZE) -fsanitize=address,undefined
sanitize-address-undefined: LDFLAGS += -fsanitize=address -lubsan
sanitize-address-undefined: imagegrid-viewer

.PHONY: sanitize-thread
sanitize-thread: CXXFLAGS_SAFE += $(CXXFLAGS_SANITIZE) -fsanitize=thread
sanitize-thread: CXXFLAGS_UNSAFE += $(CXXFLAGS_SANITIZE) -fsanitize=thread
sanitize-thread: CXXFLAGS_SDL += $(CXXFLAGS_SANITIZE) -fsanitize=thread
sanitize-thread: LDFLAGS += -ltsan
sanitize-thread: imagegrid-viewer

.PHONY: profile
profile: CXXFLAGS_SAFE += $(CXXFLAGS_PROFILE)
profile: CXXFLAGS_UNSAFE += $(CXXFLAGS_PROFILE)
profile: CXXFLAGS_SDL += $(CXXFLAGS_PROFILE)
profile: LDFLAGS += -pg
profile: imagegrid-viewer


.PHONY: debug
debug: CXXFLAGS_SAFE += $(CXXFLAGS_DEBUG)
debug: CXXFLAGS_UNSAFE += $(CXXFLAGS_DEBUG)
debug: CXXFLAGS_SDL += $(CXXFLAGS_DEBUG)
debug: imagegrid-viewer

########################################
## tools

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
	clang-tidy -checks=bugprone-*,cert-* -header-filter=.* $(SRC_PRIMARY)

.PHONY: cppcheck
cppcheck:
	cppcheck --enable=all $(SRC_PRIMARY)

.PHONY: iwyu
iwyu:
	echo "$(SRC_PRIMARY)"'\0' | tr ' ' '\0' | xargs -0 -n1 iwyu -Xiwyu --cxx17ns -Xiwyu --no_fwd_decls -std=c++17

########################################
## actual programs

imagegrid-viewer: $(OBJ_PRIMARY)
	$(CC) -o $@ $^ $(LDFLAGS)

.PHONY: test
test: tests/test_basic
	tests/test_basic

tests/test_basic: $(OBJ_TEST_BASIC)
	$(CC) -o $@ $^ $(LDFLAGS)

# TODO: fix this up later
# test_file: CXXFLAGS_SAFE += -g3 -O0
# test_file: CXXFLAGS_UNSAFE += -g3 -O0
# test_file: $(OBJ_TEST) $(OBJ_CIONET) $(OBJ_CMISC)
# 	$(CC) $(CXXFLAGS_SAFE) -o $@ $^ $(LDFLAGS)

$(OBJ_PROG): $(SRC_PROG) $(HEADERS)
	$(CC) $(CXXFLAGS_SAFE) -c -o $@ $*.cpp

$(OBJ_CIONET) $(OBJ_CMISC): $(SRC_CIONET) $(SRC_CMISC) $(HEADERS)
	$(CC) $(CXXFLAGS_UNSAFE) -c -o $@ $*.cpp

$(OBJ_CSDL): $(SRC_CSDL) $(HEADERS)
	$(CC) $(CXXFLAGS_SDL) -c -o $@ $*.cpp

.PHONY: clean
clean:
	rm -f $(OBJ_ALL)
	rm -f $(OBJ_TEST_BASIC)
