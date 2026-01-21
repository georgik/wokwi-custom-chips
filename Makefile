# Makefile for building Wokwi custom chips
# This compiles C source files to WebAssembly (WASM) binaries

# Compiler and flags
CC = clang
TARGET = --target=wasm32-unknown-wasi
CFLAGS = $(TARGET) -nostartfiles -Wl,--import-memory -Wl,--export-table -Wl,--no-entry -Werror -Wall -Wextra -O2

# Directories
SRC_DIR = a3144
DIST_DIR = dist
BUILD_DIR = build

# Chip binary
CHIP_WASM = $(DIST_DIR)/a3144.chip.wasm
CHIP_SRC = $(SRC_DIR)/chip.c
CHIP_JSON = $(SRC_DIR)/chip.json

# Default target
.PHONY: all
all: $(CHIP_WASM)

# Create directories
$(BUILD_DIR) $(DIST_DIR):
	mkdir -p $@

# Compile the chip to WASM
$(CHIP_WASM): $(CHIP_SRC) | $(DIST_DIR)
	$(CC) $(CFLAGS) -o $@ $<

# Copy chip.json to dist directory
.PHONY: json
json: $(CHIP_JSON) | $(DIST_DIR)
	cp $(CHIP_JSON) $(DIST_DIR)/a3144.chip.json

# Build everything (WASM + JSON)
.PHONY: build
build: $(CHIP_WASM) json

# Clean build artifacts
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(DIST_DIR)

# Help target
.PHONY: help
help:
	@echo "Wokwi Custom Chips Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  all       - Build all chips (default)"
	@echo "  build     - Build WASM binaries and copy JSON files"
	@echo "  json      - Copy JSON files to dist directory"
	@echo "  clean     - Remove build artifacts"
	@echo "  help      - Show this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make              # Build all chips"
	@echo "  make clean        # Clean build artifacts"
	@echo "  make build        # Build everything"
