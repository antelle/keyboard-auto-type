.PHONY: build

build:
	cmake -B build .
	cmake --build build -j4

rebuild: clean build

clean:
	rm -rf build xcode

format:
	find keyboard-auto-type example -name '*.cpp' -o -name '*.h' | \
		xargs clang-format -i --verbose

check:
	cppcheck --enable=all --inline-suppr keyboard-auto-type
	cmake -B build/check -D RUN_CLANG_TIDY=1 .
	cmake --build build/check

xcode-project:
	cmake \
		-G Xcode \
		-B xcode \
		-D CMAKE_C_COMPILER="$$(xcrun -find c++)" \
		-D CMAKE_CXX_COMPILER="$$(xcrun -find cc)" \
		.

run-example: build
	build/output/example
