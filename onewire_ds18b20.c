#include <stdint.h>
#include "onewire_ds18b20.h"
#include "onewire.h"
#include "pinmanip.h"

static bool ds18b20_get_resolution(struct OnewireDevice *therm);

/* Helper function to convert celsius to fahrenheit */
inline static float celsius_to_fahrenheit(float degrees_celsius)
{
  return (1.8 * degrees_celsius) + 32;
}

/* Blocking temperature read. Gets the temperature from the
 * device therm, blocking until the device finishes.*/
float ds18b20_get_temperature(const struct OnewireDevice *therm)
{
  ds18b20_start_conversion(therm);
  while (onewire_read_byte(therm) == 0);
  return ds18b20_read_temperature(therm);
}

/* Signal the deivce to start a temperature conversion. This
 * function, along with read_temperature, allows for a non-blocking
 * temperature read. Returns false if the device is not present
 * on the bus */
bool ds18b20_start_conversion(const struct OnewireDevice *therm)
{
  if (onewire_send_reset_pulse(therm)) {
    onewire_send_byte(therm, SKIP_ROM);
    onewire_send_byte(therm, CONVERT_T);
    return true;
  } else {
    return false;
  }
}

/* Reads the temperature from a device. Must be called
 * after start_conversion. Returns DS18B20_CONVERSION_IN_PROGRESS
 * if the conversion hasn't finished. Returns ONEWIRE_DEVICE_DISCONNECTED
 * if the device is not responding. Otherwise returns the temperature
 * in fahrenheit. Currently does not support reading negative temperatures.
 * Time for conversion depends on the resolution of the device, see
 * ds18b20_set_resolution for further details. */
float ds18b20_read_temperature(const struct OnewireDevice *therm)
{
  if (onewire_read_byte(therm) == 0) {
    return DS18B20_CONVERSION_IN_PROGRESS;
  } else if (!onewire_send_reset_pulse(therm)) {
    return ONEWIRE_DEVICE_DISCONNECTED;
  }
  onewire_send_byte(therm, SKIP_ROM);
  onewire_send_byte(therm, READ_SCRATCHPAD);
  int8_t decimal_portion_fixed = 0;
  int8_t decimal_portion = onewire_read_byte(therm);
  /* TODO: Implement negative celsius support */
  float reading = ((decimal_portion& 0xF0) >> 4) | (onewire_read_byte(therm) << 4);
  decimal_portion &= 0x0F;
  for (int8_t i = therm->resolution; i >= 0; i--) {
    decimal_portion_fixed |= get_bit(decimal_portion, i) << (4 - i);
  }
  if (decimal_portion_fixed != 0) {
    reading += 1.0f/decimal_portion_fixed;
  }
  return celsius_to_fahrenheit(reading);
}

/* Initializes a DS18B20 devices. Returns false if device is not present */
bool ds18b20_setup_device(struct OnewireDevice *therm, 
                            volatile uint8_t *bus_register, uint8_t bus_pin)
{
  return onewire_setup_device(therm, bus_register, bus_pin) && ds18b20_get_resolution(therm);
}

/* Sets the decimal point resolution of the device. This effects how long a
 * temperature conversion takes. Per the datasheet:
 * Resolution(bits)     Max Conversion Time
 *       9                    93.75ms
 *       10                   187.5ms
 *       11                   375ms
 *       12                   750ms
 */
bool ds18b20_set_resolution(struct OnewireDevice *therm, enum ThermResolution decimal_point_resolution)
{
  // Resolution is specified as byte 2, bits[6:5] of the writable scratchpad
  if (!onewire_send_reset_pulse(therm)) {
    return false;
  }
  onewire_send_byte(therm, SKIP_ROM);
  onewire_send_byte(therm, WRITE_SCRATCHPAD);
  onewire_send_byte(therm, 0x00);
  onewire_send_byte(therm, 0x00);
  onewire_send_byte(therm, decimal_point_resolution << 5);
  therm->resolution = decimal_point_resolution;
  return true;
}

static bool ds18b20_get_resolution(struct OnewireDevice *therm)
{
  /* Read configuration register - byte 4 of scratchpad */
  if (!onewire_send_reset_pulse(therm)) {
    return false;
  }
  onewire_send_byte(therm, SKIP_ROM);
  onewire_send_byte(therm, READ_SCRATCHPAD);
  onewire_read_byte(therm);
  onewire_read_byte(therm);
  onewire_read_byte(therm);
  onewire_read_byte(therm);
  int8_t configuration_register = onewire_read_byte(therm);
  /* Device resolution are bits 5 and 6 of the configuration register */
  therm->resolution = (configuration_register & 0x60) >> 5;
  return true;
}
