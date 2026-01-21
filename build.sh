#!/bin/bash
# Build script for Wokwi custom chips
# This script compiles all chip source files to WebAssembly binaries

set -euo pipefail  # Exit on error, undefined vars, and pipe failures
IFS=$'\n\t'         # Set safe internal field separator

# Colors for output (disable if not a terminal)
if [[ -t 1 ]]; then
    RED='\033[0;31m'
    GREEN='\033[0;32m'
    YELLOW='\033[1;33m'
    NC='\033[0m' # No Color
else
    RED=''
    GREEN=''
    YELLOW=''
    NC=''
fi

# Logging functions
log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1" >&2
}

# Check if wokwi-cli is installed
check_wokwi_cli() {
    if ! command -v wokwi-cli &> /dev/null; then
        log_error "wokwi-cli is not installed"
        echo "Installation options:"
        echo "  1. Run: npm install -g wokwi-cli"
        echo "  2. Visit: https://github.com/wokwi/wokwi-cli"
        exit 1
    fi

    # Check version (requires v0.20.0 or later for chip compilation)
    local version
    version=$(wokwi-cli --version 2>/dev/null | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' || echo "0.0.0")
    log_info "wokwi-cli version: $version"
}

# Validate chip directory structure
validate_chip_dir() {
    local chip_dir=$1
    local chip_name=$2

    if [[ ! -d "$chip_dir" ]]; then
        log_error "Chip directory not found: $chip_dir"
        return 1
    fi

    if [[ ! -f "$chip_dir/chip.c" ]]; then
        log_error "Source file not found: $chip_dir/chip.c"
        return 1
    fi

    if [[ ! -f "$chip_dir/chip.json" ]]; then
        log_error "Configuration file not found: $chip_dir/chip.json"
        return 1
    fi

    # Validate JSON syntax
    if ! python3 -m json.tool "$chip_dir/chip.json" > /dev/null 2>&1; then
        log_error "Invalid JSON in $chip_dir/chip.json"
        return 1
    fi

    log_info "Validated $chip_name"
    return 0
}

# Build a single chip
build_chip() {
    local chip_dir=$1
    local chip_name=$2
    local output_file="dist/${chip_name}.chip.wasm"

    log_info "Building $chip_name..."

    # Validate directory structure
    if ! validate_chip_dir "$chip_dir" "$chip_name"; then
        return 1
    fi

    # Compile the chip
    if ! wokwi-cli chip compile "$chip_dir/chip.c" -o "$output_file"; then
        log_error "Failed to compile $chip_name"
        return 1
    fi

    # Copy chip.json to dist with correct naming
    if ! cp "$chip_dir/chip.json" "dist/${chip_name}.chip.json"; then
        log_error "Failed to copy chip.json for $chip_name"
        return 1
    fi

    # Verify WASM file was created
    if [[ ! -f "$output_file" ]]; then
        log_error "Compilation succeeded but output file not found: $output_file"
        return 1
    fi

    # Get file size
    local size
    size=$(du -h "$output_file" | cut -f1)
    log_info "Built $chip_name: $output_file ($size)"

    return 0
}

# Main build process
main() {
    log_info "Starting Wokwi custom chips build..."

    # Create dist directory
    mkdir -p dist

    # Check dependencies
    check_wokwi_cli

    # Track build status
    local failed_builds=0
    local successful_builds=0

    # Build all chips (add new chips here)
    if build_chip "a3144" "a3144"; then
        ((successful_builds++))
    else
        ((failed_builds++))
    fi

    # Summary
    echo ""
    log_info "Build Summary"
    echo "  Successful: $successful_builds"
    echo "  Failed: $failed_builds"

    if [[ $failed_builds -gt 0 ]]; then
        log_error "Some chips failed to build"
        echo ""
        echo "Output files:"
        ls -lh dist/ 2>/dev/null || true
        exit 1
    fi

    echo ""
    log_info "All chips built successfully!"
    echo "Output files:"
    ls -lh dist/
}

# Run main function
main "$@"
