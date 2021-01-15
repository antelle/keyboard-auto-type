.DEFAULT_GOAL := default

.PHONY: build test example

default: configure build

configure:
	cmake -B build .

build:
	cmake --build build -j4

rebuild: clean build

clean:
	rm -rf build xcode

format:
	find keyboard-auto-type test example -name '*.cpp' -o -name '*.h' -o -name '*.mm' | \
		xargs clang-format -i --verbose

check: clang-tidy cppcheck

clang-tidy:
	cmake -B build \
		-D CMAKE_EXPORT_COMPILE_COMMANDS=1 \
		-D KEYBOARD_AUTO_TYPE_WITH_TESTS=1 \
		-D KEYBOARD_AUTO_TYPE_WITH_EXAMPLE=1 \
		.
	find keyboard-auto-type -name '*.cpp' -o -name '*.h' | \
		xargs clang-tidy -p build

cppcheck:
	cppcheck --enable=all --inline-suppr keyboard-auto-type test example

xcode-project:
	cmake \
		-G Xcode \
		-B xcode \
		-D CMAKE_C_COMPILER="$$(xcrun -find c++)" \
		-D CMAKE_CXX_COMPILER="$$(xcrun -find cc)" \
		-D KEYBOARD_AUTO_TYPE_WITH_TESTS=1 \
		-D KEYBOARD_AUTO_TYPE_WITH_EXAMPLE=1 \
		.

example:
	cmake -B build -D KEYBOARD_AUTO_TYPE_WITH_EXAMPLE=1 .
	$(MAKE) build

run-example: example
	build/output/example

test:
	cmake -B build -D KEYBOARD_AUTO_TYPE_WITH_TESTS=1 .
	$(MAKE) build
	build/output/test #--gtest_filter=AutoTypeTest.*
