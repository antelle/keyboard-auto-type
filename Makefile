# This makefile can be used with GNU Make and Windows NMAKE

# No part of this Makefile is required to build the project
# it's a convenience measure to launch CMake commands

BUILD_DIR = build

# Windows NMAKE \
!ifndef 0 # \ 
CLEAN = if exist $(BUILD_DIR) rmdir $(BUILD_DIR) /s /q # \ 
RUN_EXAMPLE = $(BUILD_DIR)\example\Debug\example.exe # \ 
RUN_TESTS_EXCEPT = $(BUILD_DIR)\sub\tests-except\test\Debug\test.exe # \ 
RUN_TESTS_NOEXCEPT = $(BUILD_DIR)\sub\tests-noexcept\test\Debug\test.exe # \
!else
# GNU Make
CLEAN = rm -rf $(BUILD_DIR) xcode
RUN_EXAMPLE = $(BUILD_DIR)/example/example
RUN_TESTS_EXCEPT = $(BUILD_DIR)/sub/tests-except/test/test
RUN_TESTS_NOEXCEPT = $(BUILD_DIR)/sub/tests-noexcept/test/test
# \
!endif

all:
	cmake -B $(BUILD_DIR) -DKEYBOARD_AUTO_TYPE_WITH_EXAMPLE=1 .
	cmake --build $(BUILD_DIR) -j4

rebuild: clean all

clean:
	$(CLEAN)

run-example: all
	$(RUN_EXAMPLE)

xcode-project:
	cmake -G Xcode -B xcode -DCMAKE_C_COMPILER="$$(xcrun -find c++)" -DCMAKE_CXX_COMPILER="$$(xcrun -find cc)" -DKEYBOARD_AUTO_TYPE_WITH_TESTS=1 -DKEYBOARD_AUTO_TYPE_WITH_EXAMPLE=1 .

vs-project:
	cmake -B $(BUILD_DIR) -DKEYBOARD_AUTO_TYPE_WITH_TESTS=1 -DKEYBOARD_AUTO_TYPE_WITH_EXAMPLE=1 .

format:
	cmake -B $(BUILD_DIR)/sub/format -DKEYBOARD_AUTO_TYPE_WITH_CLANG_FORMAT=1 .
	cmake --build $(BUILD_DIR)/sub/format --target clang-format

check: clang-tidy cppcheck

compile-commands-for-checks:
	cmake -B $(BUILD_DIR)/sub/checks -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DKEYBOARD_AUTO_TYPE_WITH_STATIC_ANALYSIS=1 -DKEYBOARD_AUTO_TYPE_WITH_EXAMPLE=1 .

clang-tidy: compile-commands-for-checks
	cmake --build $(BUILD_DIR)/sub/checks --target clang-tidy

cppcheck: compile-commands-for-checks
	cmake --build $(BUILD_DIR)/sub/checks --target cppcheck

build-tests-except:
	cmake -B $(BUILD_DIR)/sub/tests-except -DKEYBOARD_AUTO_TYPE_WITH_TESTS=1 -DKEYBOARD_AUTO_TYPE_USE_SANITIZERS=1 .
	cmake --build $(BUILD_DIR)/sub/tests-except -j4

build-tests-noexcept:
	cmake -B $(BUILD_DIR)/sub/tests-noexcept -DKEYBOARD_AUTO_TYPE_WITH_TESTS=1 -DKEYBOARD_AUTO_TYPE_USE_SANITIZERS=1 -DKEYBOARD_AUTO_TYPE_DISABLE_CPP_EXCEPTIONS=1 .
	cmake --build $(BUILD_DIR)/sub/tests-noexcept -j4

tests-except: build-tests-except
	$(RUN_TESTS_EXCEPT)

tests-noexcept: build-tests-noexcept
	$(RUN_TESTS_NOEXCEPT) --gtest_filter="AutoTypeErrorsTest.*"

tests: tests-except tests-noexcept

tests-ci-macos:
	# tests-except
	cmake -B $(BUILD_DIR)/sub/tests-except -DKEYBOARD_AUTO_TYPE_WITH_TESTS=1 -DKEYBOARD_AUTO_TYPE_USE_SANITIZERS=1 .
	cmake --build $(BUILD_DIR)/sub/tests-except -j4
	$(RUN_TESTS_EXCEPT) --gtest_filter=-AutoTypeKeysTest.text_unicode_basic
	# tests-noexcept
	cmake -B $(BUILD_DIR)/sub/tests-noexcept -DKEYBOARD_AUTO_TYPE_WITH_TESTS=1 -DKEYBOARD_AUTO_TYPE_USE_SANITIZERS=1 -DKEYBOARD_AUTO_TYPE_DISABLE_CPP_EXCEPTIONS=1 .
	cmake --build $(BUILD_DIR)/sub/tests-noexcept -j4
	$(RUN_TESTS_NOEXCEPT) --gtest_filter="AutoTypeErrorsTest.*"

tests-ci-windows:
	# tests-except
	cmake -B $(BUILD_DIR)/sub/tests-except -DKEYBOARD_AUTO_TYPE_WITH_TESTS=1 -DKEYBOARD_AUTO_TYPE_USE_SANITIZERS=1 .
	cmake --build $(BUILD_DIR)/sub/tests-except -j4
	$(RUN_TESTS_EXCEPT) --gtest_filter=-AutoTypeKeysTest.text_wstring:-AutoTypeKeysTest.text_unicode_basic:-AutoTypeKeysTest.text_unicode_emoji:-AutoTypeKeysTest.text_unicode_supplementary_ideographic
	# tests-noexcept
	cmake -B $(BUILD_DIR)/sub/tests-noexcept -DKEYBOARD_AUTO_TYPE_WITH_TESTS=1 -DKEYBOARD_AUTO_TYPE_USE_SANITIZERS=1 -DKEYBOARD_AUTO_TYPE_DISABLE_CPP_EXCEPTIONS=1 .
	cmake --build $(BUILD_DIR)/sub/tests-noexcept -j4
	$(RUN_TESTS_NOEXCEPT) --gtest_filter="AutoTypeErrorsTest.*"
