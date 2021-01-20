# This makefile can be used with GNU Make and Windows NMAKE

# Windows NMAKE \
!ifndef 0 # \ 
RUN_EXAMPLE = build\output\Debug\example.exe # \ 
RUN_TESTS = build\output\Debug\test.exe # \
!else
# GNU Make
RUN_EXAMPLE = build/output/example
RUN_TESTS = build/output/test
# \
!endif

RUN_CMAKE = cmake --build build -j4

all: configure cmake
	$(RUN_CMAKE)

configure:
	cmake -B build .

cmake:
	$(RUN_CMAKE)

rebuild: clean all

clean:
	git clean -fxd build xcode

format:
	cmake -B build -D KEYBOARD_AUTO_TYPE_WITH_CLANG_FORMAT=1 .
	cmake --build build --target clang-format

check: clang-tidy cppcheck

compile-commands-for-checks:
	cmake -B build -D CMAKE_EXPORT_COMPILE_COMMANDS=1 -D KEYBOARD_AUTO_TYPE_WITH_EXAMPLE=1 .

clang-tidy: compile-commands-for-checks
	cmake --build build --target clang-tidy

cppcheck: compile-commands-for-checks
	cppcheck --enable=all --inline-suppr --error-exitcode=2 --project=build/compile_commands.json --suppress=unusedFunction --suppress=missingIncludeSystem keyboard-auto-type

build-tests:
	cmake -B build -D KEYBOARD_AUTO_TYPE_WITH_TESTS=1 -D KEYBOARD_AUTO_TYPE_USE_SANITIZERS=1 .
	$(RUN_CMAKE)

build-tests-noexcept:
	cmake -B build -D KEYBOARD_AUTO_TYPE_WITH_TESTS=1 -D KEYBOARD_AUTO_TYPE_USE_SANITIZERS=1 -D KEYBOARD_AUTO_TYPE_DISABLE_CPP_EXCEPTIONS=1 .
	$(RUN_CMAKE)

tests: build-tests
	$(RUN_TESTS)

tests-noexcept: build-tests-noexcept
	$(RUN_TESTS) --gtest_filter="AutoTypeErrorsTest.*"

tests-all: clean tests clean tests-noexcept

xcode-project:
	cmake -G Xcode -B xcode -D CMAKE_C_COMPILER="$$(xcrun -find c++)" -D CMAKE_CXX_COMPILER="$$(xcrun -find cc)" -D KEYBOARD_AUTO_TYPE_WITH_TESTS=1 -D KEYBOARD_AUTO_TYPE_WITH_EXAMPLE=1 .

vs-project:
	cmake -B build -D KEYBOARD_AUTO_TYPE_WITH_TESTS=1 -D KEYBOARD_AUTO_TYPE_WITH_EXAMPLE=1 .

build-example:
	cmake -B build -D KEYBOARD_AUTO_TYPE_WITH_EXAMPLE=1 .
	$(RUN_CMAKE)

run-example: build-example
	$(RUN_EXAMPLE)
