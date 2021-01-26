# This makefile can be used with GNU Make and Windows NMAKE

# No part of this Makefile is required to build the project
# it's a convenience measure to launch CMake commands

# Windows NMAKE \
!ifndef 0 # \ 
CLEAN = if exist build rmdir /s /q # \ 
RUN_EXAMPLE = build\example\Debug\example.exe # \ 
RUN_TESTS_EXCEPT = build\sub\tests-except\test\Debug\test.exe # \ 
RUN_TESTS_NOEXCEPT = build\sub\tests-noexcept\test\Debug\test.exe # \
!else
# GNU Make
CLEAN = rm -rf build xcode
RUN_EXAMPLE = build/example/example
RUN_TESTS_EXCEPT = build/sub/tests-except/test/test
RUN_TESTS_NOEXCEPT = build/sub/tests-noexcept/test/test
# \
!endif

gtest_filter = *

all:
	cmake -B build -DKEYBOARD_AUTO_TYPE_WITH_EXAMPLE=1 .
	cmake --build build -j4

rebuild: clean all

clean:
	$(CLEAN)

run-example: all
	$(RUN_EXAMPLE)

xcode-project:
	cmake -G Xcode -B xcode -DCMAKE_C_COMPILER="$$(xcrun -find c++)" -DCMAKE_CXX_COMPILER="$$(xcrun -find cc)" -DKEYBOARD_AUTO_TYPE_WITH_TESTS=1 -DKEYBOARD_AUTO_TYPE_WITH_EXAMPLE=1 .

vs-project:
	cmake -B build -DKEYBOARD_AUTO_TYPE_WITH_TESTS=1 -DKEYBOARD_AUTO_TYPE_WITH_EXAMPLE=1 .

format:
	cmake -B build/sub/format -DKEYBOARD_AUTO_TYPE_WITH_CLANG_FORMAT=1 .
	cmake --build build/sub/format --target clang-format

check: clang-tidy cppcheck

compile-commands-for-checks:
	cmake -B build/sub/checks -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DKEYBOARD_AUTO_TYPE_WITH_STATIC_ANALYSIS=1 -DKEYBOARD_AUTO_TYPE_WITH_EXAMPLE=1 .

clang-tidy: compile-commands-for-checks
	cmake --build build/sub/checks --target clang-tidy

cppcheck: compile-commands-for-checks
	cmake --build build/sub/checks --target cppcheck

build-tests-except:
	cmake -B build/sub/tests-except -DKEYBOARD_AUTO_TYPE_WITH_TESTS=1 -DKEYBOARD_AUTO_TYPE_USE_SANITIZERS=1 .
	cmake --build build/sub/tests-except -j4

build-tests-noexcept:
	cmake -B build/sub/tests-noexcept -DKEYBOARD_AUTO_TYPE_WITH_TESTS=1 -DKEYBOARD_AUTO_TYPE_USE_SANITIZERS=1 -DKEYBOARD_AUTO_TYPE_DISABLE_CPP_EXCEPTIONS=1 .
	cmake --build build/sub/tests-noexcept -j4

tests-except: build-tests-except
	$(RUN_TESTS_EXCEPT) --gtest_filter="$(gtest_filter)"

tests-noexcept: build-tests-noexcept
	$(RUN_TESTS_NOEXCEPT) --gtest_filter="AutoTypeErrorsTest.*"

tests: tests-except tests-noexcept

x11-keysyms:
	node scripts/x11-keysyms

run-x11-print-layout:
	cmake -B build -DKEYBOARD_AUTO_TYPE_WITH_X11_PRINT_LAYOUT=1 .
	cmake --build build -j4
	build/x11-print-layout/x11-print-layout > tmp/layout.txt

build-test-keys-app:
	cd test-keys-app && npm ci

run-test-keys-app:
	node test-keys-app/start.js

ci: build-test-keys-app tests