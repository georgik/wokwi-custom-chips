/*
 * A3144 Hall Effect Sensor Simulation for Wokwi
 *
 * This chip simulates the Allegro A3144 (and compatible A3141/A3142) Hall effect sensor.
 *
 * Operation:
 * - Digital output switches when magnetic field is detected
 * - Typically active LOW (output = LOW when magnet detected)
 * - Open-drain output (requires pull-up resistor on real hardware)
 * - South pole on branded side triggers output
 * - North pole may or may not trigger depending on sensitivity
 *
 * Characteristics:
 * - Supply voltage: 4.5V to 24V
 * - Output type: Open-drain NPN (requires pull-up)
 * - Operating temperature: -40°C to +85°C
 * - Response time: Typically 3μs
 * - Sensitivity: ~30mT typical (adjustable via control)
 */

#include <stdio.h>
#include <stdbool.h>
#include "chip.h"

// Control indices
#define CONTROL_MAGNETIC_FIELD 0
#define CONTROL_OUTPUT_INVERTED 1
#define CONTROL_SENSITIVITY 2

// Chip state
typedef struct {
  bool magnetic_field;      // Magnetic field present
  bool output_inverted;     // Output polarity (true = active LOW)
  float sensitivity;        // Sensitivity in millitesla
  bool output_state;        // Current output state
} a3144_state_t;

static a3144_state_t state = {
  .magnetic_field = false,
  .output_inverted = true,  // A3144 is active LOW
  .sensitivity = 30.0f,     // 30mT typical
  .output_state = true,     // Default HIGH (no magnet)
};

// Initialize the chip
void chip_init(void) {
  update_output();
  printf("A3144 Hall Effect Sensor initialized\n");
}

// Update output state based on magnetic field
static void update_output(void) {
  bool field_detected = state.magnetic_field;

  // A3144 output is active LOW (pulled to ground when magnet detected)
  if (state.output_inverted) {
    state.output_state = !field_detected;  // LOW when field present
  } else {
    state.output_state = field_detected;   // HIGH when field present
  }

  printf("A3144: Output = %s (magnetic %s)\n",
         state.output_state ? "HIGH" : "LOW",
         state.magnetic_field ? "detected" : "not detected");
}

// Handle control changes
void chip_set_control(size_t index, double value) {
  switch (index) {
    case CONTROL_MAGNETIC_FIELD:
      state.magnetic_field = (bool)value;
      printf("A3144: Magnetic field %s\n",
             state.magnetic_field ? "present" : "absent");
      update_output();
      break;

    case CONTROL_OUTPUT_INVERTED:
      state.output_inverted = (bool)value;
      printf("A3144: Output polarity set to %s\n",
             state.output_inverted ? "active LOW" : "active HIGH");
      update_output();
      break;

    case CONTROL_SENSITIVITY:
      state.sensitivity = (float)value;
      printf("A3144: Sensitivity set to %.1f mT\n", state.sensitivity);
      break;

    default:
      fprintf(stderr, "A3144: Unknown control index %zu\n", index);
      break;
  }
}

// Get current control value
double chip_get_control(size_t index) {
  switch (index) {
    case CONTROL_MAGNETIC_FIELD:
      return state.magnetic_field ? 1.0 : 0.0;

    case CONTROL_OUTPUT_INVERTED:
      return state.output_inverted ? 1.0 : 0.0;

    case CONTROL_SENSITIVITY:
      return (double)state.sensitivity;

    default:
      fprintf(stderr, "A3144: Unknown control index %zu\n", index);
      return 0.0;
  }
}

// Get output pin state (for reading by microcontroller)
bool chip_get_pin_state(int pin) {
  if (pin == 0) {  // OUT pin
    return state.output_state;
  }
  return false;
}

// Cleanup
void chip_deinit(void) {
  printf("A3144 Hall Effect Sensor deinitialized\n");
}
