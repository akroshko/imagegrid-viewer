$(info "This Makefile is just an interface to cmake for convenient command-line shortcuts and completion from the project root.")

all: develop

.PHONY: develop
develop:
	if [ -d build/develop ]; then rm -f build/develop; fi; mkdir -p build/develop && \
	cd build/develop && cmake -DCMAKE_BUILD_TYPE=Develop ../.. && VERBOSE=1 make -j4;

.PHONY: debug
debug:
	if [ -d build/debug ]; then rm -f build/debug; fi; mkdir -p build/debug && \
	cd build/debug && cmake -DCMAKE_BUILD_TYPE=Debug ../.. && VERBOSE=1 make -j4;

.PHONY: profile
profile:
	if [ -d build/profile ]; then rm -f build/profile; fi; mkdir -p build/profile && \
	cd build/profile && cmake -DCMAKE_BUILD_TYPE=Profile ../.. && VERBOSE=1 make -j4;

.PHONY: fast
fast:
	if [ -d build/fast ]; then rm -f build/fast; fi; mkdir -p build/fast && \
	cd build/fast && cmake -DCMAKE_BUILD_TYPE=Fast ../.. && VERBOSE=1 make -j4;

.PHONY: sanitize
sanitize:
	if [ -d build/sanitize ]; then rm -f build/sanitize; fi; mkdir -p build/sanitize && \
	cd build/sanitize && cmake -DCMAKE_BUILD_TYPE=Sanitize ../.. && VERBOSE=1 make -j4;

.PHONY: thread-sanitize
thread-sanitize:
	if [ -d build/thread-sanitize ]; then rm -f build/thread-sanitize; fi; mkdir -p build/thread-sanitize && \
	cd build/thread-sanitize && cmake -DCMAKE_BUILD_TYPE=Thread_Sanitize ../.. && VERBOSE=1 make -j4;

.PHONY: clean-all
clean-all:
	if [ -d build/ ]; then rm -rf build/; fi;
