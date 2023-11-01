HEADERS=$(wildcard *.hpp)  $(wildcard imagegrid/*.hpp) $(wildcard c_misc/*.hpp) $(wildcard c_sdl/*.hpp) $(wildcard c_io_net/*.hpp)
# for clean only
SRC = $(wildcard *.cpp) $(wildcard imagegrid/*.cpp) $(wildcard c_misc/*.cpp) $(wildcard c_sdl/*.cpp) $(wildcard c_io_net/*.cpp)
OBJ = $(SRC:.cpp=.o)
# for compile
SRC_PROG = $(wildcard *.cpp) $(wildcard imagegrid/*.cpp)
SRC_CIONET = $(wildcard c_io_net/*.cpp)
SRC_CSDL = $(wildcard c_sdl/*.cpp)
SRC_CMISC = $(wildcard c_misc/*.cpp)
OBJ_CIONET = $(SRC_CIONET:.cpp=.o)
OBJ_CSDL = $(SRC_CSDL:.cpp=.o)
OBJ_CMISC = $(SRC_CMISC:.cpp=.o)
# create object variables for programs that are actually run
SRC_MAIN=$(filter-out $(wildcard test_*.cpp),$(SRC_PROG))
SRC_TEST = $(filter-out $(wildcard imagegrid-viewer.cpp),$(SRC_PROG))
OBJ_MAIN = $(SRC_MAIN:.cpp=.o)
OBJ_TEST = $(SRC_TEST:.cpp=.o)

LDFLAGS=-flto=auto -lstdc++ -lstdc++fs -lm -lpng -lSDL2 -lSDL2_ttf -ltiff -lzip -pthread
CXXFLAGS_ALWAYS=-std=c++17 -fno-exceptions -Wall -Wextra -Wshadow -Wundef
CXXFLAGS_SAFE=$(CXXFLAGS_ALWAYS) -Wpedantic
CXXFLAGS_UNSAFE=$(CXXFLAGS_ALWAYS) -fno-rtti
CXXFLAGS_SDL=$(CXXFLAGS_ALWAYS)
# -Wconversion -Wsign-conversion
# specific CFLAGS
CXXFLAGS_ALL=-flto -g3 -Og
CXXFLAGS_DEBUG=-g3 -Og

# TODO: reenable test code
# all: imagegrid-viewer test_file
all: CXXFLAGS_SAFE += $(CXXFLAGS_ALL)
all: CXXFLAGS_UNSAFE += $(CXXFLAGS_ALL)
all: CXXFLAGS_SDL += $(CXXFLAGS_ALL)
all: imagegrid-viewer

.PHONY: sanitize-address-undefined
sanitize-address-undefined: CXXFLAGS_SAFE += $(CXXFLAGS_ALL) -fsanitize=address,undefined
sanitize-address-undefined: CXXFLAGS_UNSAFE += $(CXXFLAGS_ALL) -fsanitize=address,undefined
sanitize-address-undefined: CXXFLAGS_SDL += $(CXXFLAGS_ALL) -fsanitize=address,undefined
sanitize-address-undefined: LDFLAGS += -fsanitize=address -lubsan
sanitize-address-undefined: imagegrid-viewer

.PHONY: sanitize-thread
sanitize-thread: CXXFLAGS_SAFE += $(CXXFLAGS_ALL) -fsanitize=thread
sanitize-thread: CXXFLAGS_UNSAFE += $(CXXFLAGS_ALL) -fsanitize=thread
sanitize-thread: CXXFLAGS_SDL += $(CXXFLAGS_ALL) -fsanitize=thread
sanitize-thread: LDFLAGS += -ltsan
sanitize-thread: imagegrid-viewer

.PHONY: debug
debug: CXXFLAGS_SAFE += $(CXXFLAGS_DEBUG)
debug: CXXFLAGS_UNSAFE += $(CXXFLAGS_DEBUG)
debug: CXXFLAGS_SDL += $(CXXFLAGS_DEBUG)
debug: imagegrid-viewer

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
	clang-tidy -checks=bugprone-*,cert-* -header-filter=.* $(SRC_MAIN) $(SRC_CIONET) $(SRC_CMISC) $(SRC_CSDL)

.PHONY: iwyu
iwyu:
	echo "$(SRC_MAIN) $(SRC_CIONET) $(SRC_CMISC) $(SRC_CSDL)"'\0' | tr ' ' '\0' | xargs -0 -n1 iwyu -Xiwyu --cxx17ns -Xiwyu --no_fwd_decls -std=c++17

imagegrid-viewer: $(OBJ_MAIN) $(OBJ_CIONET) $(OBJ_CMISC) $(OBJ_CSDL)
	$(CC) -o $@ $^ $(LDFLAGS)

# TODO: fix this up later
# test_file: CXXFLAGS_SAFE += -g3 -O0
# test_file: CXXFLAGS_UNSAFE += -g3 -O0
# test_file: $(OBJ_TEST) $(OBJ_CIONET) $(OBJ_CMISC)
# 	$(CC) $(CXXFLAGS_SAFE) -o $@ $^ $(LDFLAGS)

$(OBJ_MAIN): $(SRC_MAIN) $(HEADERS)
	$(CC) $(CXXFLAGS_SAFE) -c -o $@ $*.cpp

$(OBJ_CIONET) $(OBJ_CMISC): $(SRC_CIONET) $(SRC_CMISC) $(HEADERS)
	$(CC) $(CXXFLAGS_UNSAFE) -c -o $@ $*.cpp

$(OBJ_CSDL): $(SRC_CSDL) $(HEADERS)
	$(CC) $(CXXFLAGS_SDL) -c -o $@ $*.cpp

.PHONY: clean
clean:
	rm -f $(OBJ)
