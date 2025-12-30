BUILD_DIR = build
PROGRAM_NAME = Calgine

CMAKE = /usr/bin/cmake
GENERATOR = Ninja

C_COMPILER = /usr/bin/gcc
CXX_COMPILER = /usr/bin/g++

build:
	$(CMAKE) -S . -B $(BUILD_DIR) \
		-G $(GENERATOR) \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=TRUE \
		-DCMAKE_C_COMPILER=$(C_COMPILER) \
		-DCMAKE_CXX_COMPILER=$(CXX_COMPILER)
	$(CMAKE) --build $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

run: build
	./$(BUILD_DIR)/$(PROGRAM_NAME)
