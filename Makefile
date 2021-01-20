# This makefile can be used with GNU Make and Windows NMAKE

# Windows NMAKE \
!ifndef 0 # \ 
CLEAN = if exist build rmdir build /s /q # \ 
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

all:
	cmake -B build -D KEYBOARD_AUTO_TYPE_WITH_EXAMPLE=1 .
	cmake --build build -j4

rebuild: clean all

clean:
	$(CLEAN)

format:
	cmake -B build/sub/format -D KEYBOARD_AUTO_TYPE_WITH_CLANG_FORMAT=1 .
	cmake --build build/sub/format --target clang-format

check: clang-tidy cppcheck

compile-commands-for-checks:
	cmake -B build/sub/checks -D CMAKE_EXPORT_COMPILE_COMMANDS=1 -D KEYBOARD_AUTO_TYPE_WITH_STATIC_ANALYSIS=1 -D KEYBOARD_AUTO_TYPE_WITH_EXAMPLE=1 .

clang-tidy: compile-commands-for-checks
	cmake --build build/sub/checks --target clang-tidy

cppcheck: compile-commands-for-checks
	cmake --build build/sub/checks --target cppcheck

build-tests-except:
	cmake -B build/sub/tests-except -D KEYBOARD_AUTO_TYPE_WITH_TESTS=1 -D KEYBOARD_AUTO_TYPE_USE_SANITIZERS=1 .
	cmake --build build/sub/tests-except -j4

build-tests-noexcept:
	cmake -B build/sub/tests-noexcept -D KEYBOARD_AUTO_TYPE_WITH_TESTS=1 -D KEYBOARD_AUTO_TYPE_USE_SANITIZERS=1 -D KEYBOARD_AUTO_TYPE_DISABLE_CPP_EXCEPTIONS=1 .
	cmake --build build/sub/tests-noexcept -j4

tests-except: build-tests-except
	$(RUN_TESTS_EXCEPT)

tests-noexcept: build-tests-noexcept
	$(RUN_TESTS_NOEXCEPT) --gtest_filter="AutoTypeErrorsTest.*"

tests: tests-except tests-noexcept

xcode-project:
	cmake -G Xcode -B xcode -D CMAKE_C_COMPILER="$$(xcrun -find c++)" -D CMAKE_CXX_COMPILER="$$(xcrun -find cc)" -D KEYBOARD_AUTO_TYPE_WITH_TESTS=1 -D KEYBOARD_AUTO_TYPE_WITH_EXAMPLE=1 .

vs-project:
	cmake -B build -D KEYBOARD_AUTO_TYPE_WITH_TESTS=1 -D KEYBOARD_AUTO_TYPE_WITH_EXAMPLE=1 .

run-example: all
	$(RUN_EXAMPLE)
