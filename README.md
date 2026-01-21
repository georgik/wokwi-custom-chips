# Wokwi Custom Chips

This repository contains custom chip implementations for the Wokwi electronic circuit simulator. All chips are implemented in C and compiled to WebAssembly for use in Wokwi simulations.

## Table of Contents

- [Overview](#overview)
- [Project Structure](#project-structure)
- [Available Chips](#available-chips)
- [Building](#building)
- [Using Custom Chips](#using-custom-chips)
- [Development Guide](#development-guide)
- [Common Pitfalls](#common-pitfalls)
- [API Reference](#api-reference)
- [Resources](#resources)

## Overview

Wokwi custom chips extend the simulator with custom electronic components. This repository serves as a template and collection for developing such chips. Each chip consists of:

- **C implementation** (`chip.c`) - Chip logic using the Wokwi Chips API
- **JSON definition** (`chip.json`) - Pinout and configurable controls
- **WASM binary** - Compiled output for the simulator

## Project Structure

```
wokwi-custom-chips/
├── a3144/                        # A3144 Hall Effect Sensor
│   ├── chip.c                   # Chip implementation
│   ├── chip.json                # Pinout and controls definition
│   └── wokwi-api.h              # Wokwi C API header (auto-downloaded)
├── dist/                         # Compiled WASM binaries (generated)
│   ├── a3144.chip.wasm          # Compiled chip binary
│   └── a3144.chip.json          # Chip configuration
├── esp32-test-project/           # Example ESP32 project
│   ├── main/esp32-test-project.c
│   ├── diagram.json             # Circuit diagram
│   └── wokwi.toml               # Project configuration
├── .github/workflows/            # CI/CD
│   └── build.yaml               # Automated builds
├── wokwi.toml                   # Root Wokwi configuration
├── build.sh                     # Build script (recommended)
├── Makefile                     # Alternative build with clang
├── QUICKSTART.md                # Quick start guide
└── README.md                    # This file
```

## Available Chips

### A3144 Hall Effect Sensor

The A3144 is a digital Hall effect sensor that switches its output when a magnetic field is detected. This implementation simulates the Allegro A3144 and compatible sensors (A3141, A3142).

**Characteristics:**
- Supply voltage: 4.5V to 24V
- Output type: Open-drain NPN (requires pull-up)
- Output polarity: Active LOW (default)
- Response time: ~3μs

**Controls:**
- `magneticField` (0-100) - Magnetic field strength in arbitrary units. Values >50 trigger detection
- `outputInverted` (0-1) - Output polarity: 0=active HIGH, 1=active LOW (default)

**Pinout:**
- Pin 1: OUT - Digital output
- Pin 2: VCC - Power input (3.3V or 5V)
- Pin 3: GND - Ground

**Example Usage:**

See `esp32-test-project/` for a complete example showing how to interface the A3144 with an ESP32.

## Building

### Prerequisites

- **wokwi-cli** v0.20.0 or later: `npm install -g wokwi-cli`
- **Python 3** - Required for JSON validation in build script
- **bash** - The build script uses bash-specific features

### Recommended Method: build.sh

The `build.sh` script provides the safest and most complete build process:

```bash
./build.sh
```

**Features:**
- Validates chip directory structure
- Verifies JSON syntax before compilation
- Checks wokwi-cli installation and version
- Reports detailed build status
- Supports multiple chips in one repository
- Continues building other chips if one fails

### Alternative Methods

#### Using wokwi-cli Directly

```bash
# Build a single chip
wokwi-cli chip compile a3144/chip.c -o dist/a3144.chip.wasm

# Copy configuration
cp a3144/chip.json dist/a3144.chip.json
```

#### Using Makefile

Requires clang with WASM target and WASI SDK installed:

```bash
make          # Build all chips
make clean    # Remove build artifacts
make help     # Show available targets
```

#### Using Docker

```bash
docker run --rm -u 1000:1000 -v ${PWD}:/src \
  wokwi/builder-clang-wasm:latest make
```

### GitHub Actions

The repository includes automated builds via GitHub Actions:

- **Push to main/master:** Compiles all chips and creates artifacts
- **Version tags (e.g., v1.0.0):** Creates a GitHub release with binaries

## Using Custom Chips

### In a Wokwi Project

1. **Copy chip files** to your Wokwi project:
   ```
   cp dist/a3144.chip.wasm /path/to/project/chips/
   cp dist/a3144.chip.json /path/to/project/chips/
   ```

2. **Update wokwi.toml** in your project:
   ```toml
   [[chip]]
   name = 'a3144'
   binary = 'chips/a3144.chip.wasm'
   ```

3. **Reference in diagram.json**:
   ```json
   {
     "type": "chip-a3144",
     "id": "hall1",
     "attrs": {}
   }
   ```

4. **Wire connections** in diagram.json:
   ```json
   [
     ["hall1:OUT", "esp:4", ""],
     ["hall1:VCC", "esp:3V3", ""],
     ["hall1:GND", "esp:GND", ""]
   ]
   ```

### With Wokwi CLI

```bash
wokwi-cli --diagram diagram.json --sketch firmware.bin
```

### With VS Code Extension

1. Open project in VS Code
2. Ensure Wokwi extension is installed
3. Open diagram.json or press F5 to start simulation

## Development Guide

### Adding a New Chip

#### Step 1: Create Directory Structure

```bash
mkdir mychip
cd mychip
```

#### Step 2: Write Chip Definition (chip.json)

The chip.json file defines the pinout and user controls. **Important:** Use the correct format as specified in the [Wokwi documentation](https://docs.wokwi.com/chips-api/chip-json):

```json
{
  "name": "My Custom Chip",
  "author": "Your Name",
  "pins": ["OUT", "IN", "VCC", "GND"],
  "controls": [
    {
      "id": "myControl",
      "label": "My Control",
      "type": "range",
      "min": 0,
      "max": 100,
      "step": 1
    }
  ]
}
```

**Critical points:**
- `pins` must be an **array of strings**, not objects
- `controls` use `id` and `label`, not `name` and `displayName`
- Control type must be `"range"` (not `"boolean"` or `"number"`)

#### Step 3: Implement Chip Logic (chip.c)

Use the official Wokwi API (wokwi-api.h is auto-downloaded by wokwi-cli):

```c
#include <stdio.h>
#include "wokwi-api.h"

static uint32_t my_control_attr;
static pin_t out_pin;
static timer_t poll_timer;

static void update_output(void) {
  uint32_t value = attr_read(my_control_attr);
  // Update pin state based on attribute value
  pin_write(out_pin, value > 50 ? HIGH : LOW);
}

static void timer_callback(void *user_data) {
  update_output();
}

void chip_init(void) {
  // Initialize attributes
  my_control_attr = attr_init("myControl", 0);

  // Initialize pins
  out_pin = pin_init("OUT", OUTPUT_HIGH);

  // Set initial state
  update_output();

  // Set up polling timer (see Common Pitfalls below)
  const timer_config_t config = {
    .callback = timer_callback,
    .user_data = NULL
  };
  poll_timer = timer_init(&config);
  timer_start(poll_timer, 100000, true);  // 100ms, repeating
}
```

#### Step 4: Build and Test

```bash
# From root directory
./build.sh

# The build script will automatically:
# - Validate chip.json syntax
# - Compile chip.c to WASM
# - Copy files to dist/ with correct naming
```

#### Step 5: Update Build Files

Add your new chip to `build.sh` in the main() function:

```bash
# Build all chips (add new chips here)
if build_chip "mychip" "mychip"; then
    ((successful_builds++))
else
    ((failed_builds++))
fi
```

Also update the root `wokwi.toml`:

```toml
[[chip]]
name = 'mychip'
binary = 'dist/mychip.chip.wasm'
```

## Common Pitfalls

This section documents common mistakes encountered during development. Review these carefully to avoid wasting time debugging.

### 1. Incorrect chip.json Format

**Problem:** Using array of objects for pins instead of array of strings.

**Wrong:**
```json
"pins": [
  { "name": "OUT", "type": "gpio", "direction": "output" }
]
```

**Correct:**
```json
"pins": ["OUT", "VCC", "GND"]
```

**Impact:** Wokwi will fail to load the chip with a cryptic error message.

### 2. Wrong Control Property Names

**Problem:** Using `name`/`displayName` instead of `id`/`label`.

**Wrong:**
```json
{
  "name": "magneticField",
  "displayName": "Magnetic Field",
  "type": "boolean"
}
```

**Correct:**
```json
{
  "id": "magneticField",
  "label": "Magnetic Field",
  "type": "range",
  "min": 0,
  "max": 100,
  "step": 1
}
```

**Impact:** Controls will not appear in the Wokwi UI.

### 3. Not Responding to Attribute Changes

**Problem:** Reading attributes only in `chip_init()` without updating when controls change.

**Symptoms:**
- Changing slider values in Wokwi UI has no effect
- Output pins remain in initial state
- Microcontroller reads same value regardless of control changes

**Solution:** Use a timer to periodically poll attributes:

```c
static void poll_callback(void *user_data) {
  uint32_t value = attr_read(my_attr);
  // Update pins based on current attribute value
}

void chip_init(void) {
  // ... init code ...

  const timer_config_t config = {
    .callback = poll_callback,
    .user_data = NULL
  };
  timer_t timer = timer_init(&config);
  timer_start(timer, 100000, true);  // Poll every 100ms
}
```

**Why this is necessary:** Wokwi does not provide attribute change callbacks. The chip must actively poll attributes to detect changes.

### 4. Using Custom wokwi-api.h

**Problem:** Creating a custom or hallucinated wokwi-api.h header file.

**Solution:** Let wokwi-cli download the official header:
```bash
rm wokwi-api.h  # Delete custom version
wokwi-cli chip compile chip.c  # Will download correct version
```

**Impact:** Compiling with wrong API leads to linker errors or runtime failures.

### 5. Incorrect Pin Names in Connections

**Problem:** Pin names in diagram.json connections don't match chip.json.

If chip.json defines:
```json
"pins": ["OUT", "VCC", "GND"]
```

Then diagram.json must use:
```json
["chip1:OUT", "esp:4", ""]
```

**Impact:** Connection is silently ignored, leading to confusion why the chip isn't working.

### 6. Position Format in diagram.json

**Problem:** Using object format for `pos` instead of array.

**Wrong:**
```json
"pos": { "x": 100, "y": 100 }
```

**Correct:**
```json
"pos": [100, 100]
```

**Impact:** VS Code Wokwi extension fails to display diagram with "invalid diagram.json" error.

### 7. Forgetting to Rebuild

**Problem:** Modifying chip.c but not rebuilding the WASM binary.

**Solution:** Always rebuild after code changes:
```bash
./build.sh
```

**Note:** Wokwi caches the WASM binary. Changes won't appear until you rebuild and restart the simulation.

### 8. GPIO Pull-up Requirements

**Problem:** Not configuring pull-up resistors for open-drain outputs.

Some sensors (like A3144) have open-drain outputs that require pull-up resistors. In ESP32 code:

```c
gpio_config_t io_conf = {
    .pin_bit_mask = (1ULL << SENSOR_PIN),
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_ENABLE,  // Required for open-drain
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
};
gpio_config(&io_conf);
```

**Impact:** Input pin floats unpredictably, giving random read values.

### 9. Not Validating JSON

**Problem:** Syntax errors in chip.json or diagram.json.

**Solution:** Validate before using:
```bash
python3 -m json.tool chip.json
python3 -m json.tool diagram.json
```

**Or use the build.sh script which validates automatically.**

### 10. Attribute Name Mismatch

**Problem:** Attribute names in chip.c don't match chip.json control IDs.

If chip.json has:
```json
{ "id": "magneticField", ... }
```

Then chip.c must use:
```c
attr_init("magneticField", 0);  // Must match exactly
```

**Case-sensitive:** "magneticField" != "magneticfield"

### 11. Using chip_deinit() Function

**Problem:** Attempting to implement `chip_deinit()` which doesn't exist in the Wokwi API.

**Wrong:**
```c
void chip_deinit(void) {
  timer_stop(poll_timer);
  // Cleanup code
}
```

**Solution:** Remove `chip_deinit()` entirely. The Wokwi API only requires `chip_init()`. Resources are automatically cleaned up when the simulation ends.

**Impact:** The function will never be called, and any cleanup code (like stopping timers) won't execute. This is acceptable since Wokwi handles cleanup on simulation shutdown.

## API Reference

### Required Functions

#### `void chip_init(void)`

Called once when chip is instantiated in the simulation. Use to:
- Initialize attributes with `attr_init()`
- Initialize pins with `pin_init()`
- Set up timers with `timer_init()`
- Set initial pin states

**Note:** There is no `chip_deinit()` function in the Wokwi API. Resources are automatically cleaned up when the simulation ends.

### Core API Functions

#### Attributes

```c
uint32_t attr_init(const char *name, uint32_t default_value)
uint32_t attr_init_float(const char *name, float default_value)
uint32_t attr_read(uint32_t attr_handle)
float attr_read_float(uint32_t attr_handle)
```

#### GPIO

```c
pin_t pin_init(const char *name, uint32_t mode)
void pin_mode(pin_t pin, uint32_t mode)
void pin_write(pin_t pin, uint32_t value)
uint32_t pin_read(pin_t pin)
```

Pin modes: `INPUT`, `OUTPUT`, `INPUT_PULLUP`, `INPUT_PULLDOWN`, `OUTPUT_LOW`, `OUTPUT_HIGH`

#### Timers

```c
timer_t timer_init(const timer_config_t *config)
void timer_start(timer_t timer, uint32_t micros, bool repeat)
void timer_stop(timer_t timer)
```

### Complete API Documentation

See [Wokwi Chips API](https://docs.wokwi.com/chips-api) for complete reference.

## Resources

### Official Documentation
- [Custom Chips Guide](https://docs.wokwi.com/guides/custom-chips)
- [Chips API Reference](https://docs.wokwi.com/chips-api)
- [Chip Definition Format](https://docs.wokwi.com/chips-api/chip-json)
- [wokwi-cli Repository](https://github.com/wokwi/wokwi-cli)

### Examples
- [Inverter Chip](https://wokwi.com/projects/327458636089524820) - Simple digital inverter
- [XOR Gate](https://wokwi.com/projects/329456176677782100) - Logic gate example
- [Timer Chip](https://wokwi.com/projects/341265875285836370) - Timer API usage
- [I2C Counter](https://wokwi.com/projects/344061754973618771) - I2C device with interrupt
- [LM75A Temperature](https://wokwi.com/projects/344037885763125843) - I2C temperature sensor

### Community
- [Wokwi Discord](https://wokwi.com/discord) - #custom-chips channel
- [GitHub Issues](https://github.com/wokwi/wokwi-cli/issues) - Bug reports and feature requests

