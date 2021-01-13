.PHONY: configure build

configure:
	cmake -B build .

build:
	cmake --build build -j4

rebuild: clean build

clean:
	rm -rf build xcode

format:
	find keyboard-auto-type tests example -name '*.cpp' -o -name '*.h' | \
		xargs clang-format -i --verbose

check:
	cmake -B build/check -D RUN_CLANG_TIDY=1 .
	cmake --build build/check
	cppcheck --enable=all --inline-suppr keyboard-auto-type

xcode-project:
	cmake \
		-G Xcode \
		-B xcode \
		-D CMAKE_C_COMPILER="$$(xcrun -find c++)" \
		-D CMAKE_CXX_COMPILER="$$(xcrun -find cc)" \
		.

run-example:
	cmake -B build -D KEYBOARD_AUTO_TYPE_WITH_EXAMPLE=1 .
	$(MAKE) build
	build/output/example

test:
	cmake -B build -D KEYBOARD_AUTO_TYPE_WITH_TESTS=1 .
	$(MAKE) build
	build/output/tests
