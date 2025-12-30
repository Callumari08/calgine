BUILD_DIR = build
DOCS_DIR = docs
PROGRAM_NAME = Calgine

CMAKE = /usr/bin/cmake
GENERATOR = Ninja

C_COMPILER = /usr/bin/gcc
CXX_COMPILER = /usr/bin/g++

generate_build_files:
	$(CMAKE) -S . -B $(BUILD_DIR) \
		-G $(GENERATOR) \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=TRUE \
		-DCMAKE_C_COMPILER=$(C_COMPILER) \
		-DCMAKE_CXX_COMPILER=$(CXX_COMPILER)

build: generate_build_files
	$(CMAKE) --build $(BUILD_DIR)

docs: generate_build_files
	cmake --build build --target $(DOCS_DIR)

clean:
	rm -rf $(BUILD_DIR)

clean_docs:
	rm -rf $(DOCS_DIR)