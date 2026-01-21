# ESP32 A3144 Hall Effect Sensor Example

This example demonstrates how to use the custom A3144 Hall Effect sensor with an ESP32 in Wokwi.

## Overview

This project reads the state of an A3144 Hall Effect sensor connected to GPIO 4 of an ESP32. When a magnetic field is detected, the sensor output goes LOW and the ESP32 logs the event and turns on an LED.

## Hardware Connections

```
A3144 Hall Effect Sensor
├── OUT  → ESP32 GPIO 4
├── VCC  → ESP32 3V3
└── GND  → ESP32 GND

LED (Red)
├── Anode (A)   → ESP32 GPIO 2
└── Cathode (C) → ESP32 GND
```

## How It Works

### A3144 Sensor Behavior

The A3144 is a digital Hall effect sensor with the following characteristics:

- **Active LOW output**: The output pin is pulled LOW when a magnetic field is detected
- **Open-drain output**: Requires a pull-up resistor (enabled internally on ESP32)
- **Default state**: HIGH when no magnetic field is present

### ESP32 Code

The ESP32 code performs the following:

1. **Initializes GPIO 4** as an input with internal pull-up enabled
2. **Initializes GPIO 2** (built-in LED) as an output
3. **Continuously polls** the sensor state every 100ms
4. **Detects state changes** and logs when magnetic field is detected
5. **Controls the LED** - turns ON when magnet detected, OFF when not

### Running the Simulation

1. **Build the project** (if not already built):
   ```bash
   cd esp32-test-project
   idf.py build
   ```

2. **Open in Wokwi**:
   - Use the Wokwi VS Code extension
   - Or open the project in Wokwi CLI

3. **Start the simulation**

4. **Interact with the sensor**:
   - Click on the A3144 sensor in the diagram
   - Toggle the "Magnetic Field" control to simulate magnet presence
   - Watch the serial monitor for detection messages
   - Observe the LED turning on/off

## Expected Output

When you run the simulation, you'll see output similar to this:

```
I (123) A3144-Example: A3144 Hall Effect Sensor Example
I (133) A3144-Example: ================================
I (143) A3144-Example: Sensor connected to GPIO 4
I (153) A3144-Example:
I (163) A3144-Example: The A3144 sensor output is active LOW:
I (173) A3144-Example:   - HIGH (1) = No magnetic field detected
I (183) A3144-Example:   - LOW (0)  = Magnetic field detected
I (193) A3144-Example:
I (203) A3144-Example: Use the sensor controls in Wokwi to simulate magnetic field
I (213) A3144-Example:
I (313) A3144-Example: ✗ No magnetic field
I (513) A3144-Example: ✓ Magnetic field detected! (Count: 1)
I (713) A3144-Example: ✗ No magnetic field
I (913) A3144-Example: ✓ Magnetic field detected! (Count: 2)
```

## Sensor Controls

The A3144 chip in this simulation has three configurable controls:

1. **Magnetic Field** (boolean)
   - Simulates the presence/absence of a magnetic field
   - `false` = No magnet (output HIGH)
   - `true` = Magnet present (output LOW)

2. **Output Inverted** (boolean)
   - Controls output polarity
   - `true` = Active LOW (default A3144 behavior)
   - `false` = Active HIGH

3. **Sensitivity** (number, 1-100 mT)
   - Magnetic field sensitivity in millitesla
   - Default: 30 mT (typical for A3144)

## Customizing the Example

### Changing the GPIO Pin

To use a different GPIO pin, modify this line in `main/esp32-test-project.c`:

```c
#define A3144_SENSOR_PIN    GPIO_NUM_4  // Change to your desired pin
```

Then update the connection in `diagram.json`:

```json
[
  "hall1:OUT",
  "esp:YOUR_NEW_PIN",
  ""
]
```

### Adding Interrupt Detection

For more efficient detection, you can use GPIO interrupts instead of polling:

```c
// Set up interrupt on falling edge (magnet detected)
gpio_set_intr_type(A3144_SENSOR_PIN, GPIO_INTR_NEGEDGE);

// Install ISR service
gpio_install_isr_service(0);

// Add interrupt handler
gpio_isr_handler_add(A3144_SENSOR_PIN, hall_sensor_isr, NULL);
```

### Multiple Sensors

You can connect multiple A3144 sensors by:

1. Adding more sensors to `diagram.json`
2. Configuring additional GPIO pins in the code
3. Reading each sensor independently

## Files

- `main/esp32-test-project.c` - Main ESP32 application code
- `diagram.json` - Wokwi circuit diagram
- `wokwi.toml` - Wokwi project configuration
- `README.md` - This file

## Requirements

- ESP-IDF development environment
- Wokwi VS Code extension or Wokwi CLI
- Compiled A3144 chip binary (in `../dist/a3144.chip.wasm`)

## Building the Chip

If the A3144 chip binary doesn't exist, build it from the root directory:

```bash
cd ..
./build.sh
```

This will compile the A3144 chip to `dist/a3144.chip.wasm`.

## References

- [A3144 Datasheet](https://www.allegromicro.com/en/products/sense/switches-and-latches/hall-effect-switches/a3144)
- [Wokwi Custom Chips Documentation](https://docs.wokwi.com/guides/custom-chips)
- [ESP-IDF GPIO Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html)
