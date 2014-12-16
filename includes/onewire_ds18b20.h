#ifndef ONEWIRE_DS18B20_H_
#define ONEWIRE_DS18B20_H_
#include <stdint.h>
#include <stdbool.h>
#include "onewire.h"
#define DS18B20_CONVERSION_IN_PROGRESS (636.2f)

/* DS18B20 specific */
enum {CONVERT_T = 0x44, WRITE_SCRATCHPAD = 0x4E, READ_SCRATCHPAD = 0xBE, 
    COPY_SCRATCHPAD = 0x48, RECALL_E2 = 0xB8, READ_POWER_SUPPLY = 0xB4};
enum ThermResolution {LOW = 1, MED, HIGH};

float ds18b20_get_temperature(const struct OnewireDevice *therm);
bool ds18b20_start_conversion(const struct OnewireDevice *therm);
float ds18b20_read_temperature(const struct OnewireDevice *therm);
bool ds18b20_setup_device(struct OnewireDevice *therm, 
                            volatile uint8_t *bus_register, uint8_t bus_pin);
bool ds18b20_set_resolution(struct OnewireDevice *therm, enum ThermResolution decimal_point_resolution);
#endif
