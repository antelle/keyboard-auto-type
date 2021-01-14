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
	find keyboard-auto-type test example -name '*.cpp' -o -name '*.h' | \
		xargs clang-format -i --verbose

check:
	cmake -B build \
		-D CMAKE_EXPORT_COMPILE_COMMANDS=1 \
		-D KEYBOARD_AUTO_TYPE_WITH_TESTS=1 \
		-D KEYBOARD_AUTO_TYPE_WITH_EXAMPLE=1 \
		.
	find keyboard-auto-type -name '*.cpp' -o -name '*.h' | \
		xargs clang-tidy -p build \
			-checks=*,-modernize-use-trailing-return-type,-fuchsia-*,-readability-implicit-bool-conversion,-cppcoreguidelines-pro-type-reinterpret-cast,-llvm-header-guard \
			-warnings-as-errors=* \
			--quiet
	cppcheck --enable=all --inline-suppr keyboard-auto-type test example

xcode-project:
	cmake \
		-G Xcode \
		-B xcode \
		-D CMAKE_C_COMPILER="$$(xcrun -find c++)" \
		-D CMAKE_CXX_COMPILER="$$(xcrun -find cc)" \
		.

example:
	cmake -B build -D KEYBOARD_AUTO_TYPE_WITH_EXAMPLE=1 .
	$(MAKE) build

run-example: example
	build/output/example

test:
	cmake -B build -D KEYBOARD_AUTO_TYPE_WITH_TESTS=1 .
	$(MAKE) build
	build/output/test --gtest_filter=AutoTypeTest.*
