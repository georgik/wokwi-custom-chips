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
 *
 * Characteristics:
 * - Supply voltage: 4.5V to 24V
 * - Output type: Open-drain NPN (requires pull-up)
 * - Operating temperature: -40°C to +85°C
 * - Response time: Typically 3μs
 */

#include <stdio.h>
#include <stdlib.h>
#include "wokwi-api.h"

// Attribute handles
static uint32_t magnetic_field_attr;
static uint32_t output_inverted_attr;

// Pin handles
static pin_t out_pin;

// Timer handle
static timer_t poll_timer;

// Previous values for change detection
static uint32_t prev_magnetic_field = 0;
static uint32_t prev_inverted = 1;

// Update output pin based on attributes
static void update_output(void) {
  uint32_t magnetic_field = attr_read(magnetic_field_attr);
  uint32_t inverted = attr_read(output_inverted_attr);

  // A3144 is active LOW: output goes LOW when magnetic field is detected
  bool field_detected = (magnetic_field > 50); // Threshold for "detected"

  bool output_state;
  if (inverted) {
    // Active LOW mode (default A3144 behavior)
    output_state = !field_detected;
  } else {
    // Active HIGH mode
    output_state = field_detected;
  }

  // Write to output pin
  pin_write(out_pin, output_state ? HIGH : LOW);

  // Log only when values change
  if (magnetic_field != prev_magnetic_field || inverted != prev_inverted) {
    printf("A3144: Magnetic field=%lu, Inverted=%lu, Output=%s\n",
           magnetic_field, inverted,
           output_state ? "HIGH" : "LOW");
    prev_magnetic_field = magnetic_field;
    prev_inverted = inverted;
  }
}

// Timer callback - called periodically to check attribute changes
static void poll_callback(void *user_data) {
  (void)user_data; // Unused
  update_output();
}

// Initialize the chip
void chip_init(void) {
  // Initialize attributes (magnetic field strength: 0-100, default 0)
  magnetic_field_attr = attr_init("magneticField", 0);

  // Initialize output inverted attribute (0=normal, 1=inverted), default 1 (inverted/active LOW)
  output_inverted_attr = attr_init("outputInverted", 1);

  // Initialize OUT pin as output
  out_pin = pin_init("OUT", OUTPUT_HIGH);

  // Set initial output state
  update_output();

  // Set up a timer to poll attributes every 100ms (100,000 microseconds)
  const timer_config_t timer_config = {
    .callback = poll_callback,
    .user_data = NULL,
  };
  poll_timer = timer_init(&timer_config);
  timer_start(poll_timer, 100000, true); // 100ms, repeating

  printf("A3144 Hall Effect Sensor initialized\n");
}
