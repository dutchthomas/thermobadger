#ifndef AVR_ONEWIRE_H_
#define AVR_ONEWIRE_H_
#define ONEWIRE_DEVICE_DISCONNECTED 255
#include <stdint.h>
#include <stdbool.h>

/* The struct OnewireDevice provides a base struct for all
 * onewire-protocol using devices. Since the transmission
 * protocol is the same for all onewire devices, these functions
 * may be used as a base for onewire device drivers. Although the
 * datasheet specifies that any amount of onewire devices may
 * be present on the bus, the current implementation only allows
 * for a single device per bus. In order to use multiple devices,
 * the SEARCH_ROM and MATCH_ROM commands must be used to activate
 * devices based on the serial_no. */

struct OnewireDevice
{
  uint64_t crc : 8;
  uint64_t serial_no : 48;
  uint64_t family_code : 8;
  volatile uint8_t* bus_port;
  uint8_t bus_pin : 4;
  uint8_t resolution : 4;
};

// One-wire general
enum OnewireCommands{SEARCH_ROM = 0xF0, READ_ROM = 0x33, MATCH_ROM = 0x55, SKIP_ROM = 0xCC, 
    ALARM_SEARCH = 0xEC};

/* Initializes the onewire device to struct OnewireDevice. Device
 * must be connected to bus_register on bus_pin. */
bool onewire_setup_device(struct OnewireDevice *device, 
                            volatile uint8_t *bus_register, uint8_t bus_pin);
void onewire_send_byte(const struct OnewireDevice *device, uint8_t byte);
uint8_t onewire_read_byte(const struct OnewireDevice *device);
bool onewire_send_reset_pulse(const struct OnewireDevice *device);
bool onewire_read_device_info(struct OnewireDevice *device);
#endif
