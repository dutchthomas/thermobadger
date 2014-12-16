#ifndef PINMANIP_H_
#define PINMANIP_H_
#include <stdint.h>

typedef enum {INPUT, OUTPUT} PinMode;
inline volatile uint8_t* port_to_dir_reg(volatile uint8_t *port_reg);
inline volatile uint8_t* port_to_pin_reg(volatile uint8_t *port_reg);
inline void set_pin_mode(volatile uint8_t *port_reg, uint8_t pin, PinMode mode);

inline uint8_t get_bit(uint8_t src, uint8_t bit_i);
inline uint8_t get_bit_pu(uint8_t src, uint8_t bit_i);
inline void clear_pin(volatile uint8_t *src, uint8_t bit_i);
inline void set_pin(volatile uint8_t *src, uint8_t bit_set_mask);
inline void clear_pins(volatile uint8_t *src, uint8_t bit_clear_mask);

inline volatile uint8_t* port_to_dir_reg(volatile uint8_t *port_reg)
{
  return port_reg - 0x01;
}

inline volatile uint8_t* port_to_pin_reg(volatile uint8_t *port_reg)
{
  return port_reg - 0x02;
}
inline void set_pin_mode(volatile uint8_t *port_reg, uint8_t pin, PinMode mode)
{
  if (mode == INPUT) {
      set_pin(port_reg, pin);
      clear_pin(port_to_dir_reg(port_reg), pin);
  } else {
      set_pin(port_to_dir_reg(port_reg), pin);
  }
}

inline uint8_t get_bit_pu(uint8_t src, uint8_t bit_i) 
{ 
    return (( ~src ) & (1 << bit_i)) > 0; 
}

inline uint8_t get_bit(uint8_t src, uint8_t bit_i) 
{ 
    return (src & (1 << bit_i)) > 0; 
}

inline void set_pin(volatile uint8_t *port, uint8_t bit_i)
{
  *port |= 1 << bit_i;
}

inline void clear_pin(volatile uint8_t *port, uint8_t bit_i)
{
  *port &= ~(1 << bit_i);
}

inline void set_pins(volatile uint8_t *port, uint8_t bit_set_mask)
{
  *port |= bit_set_mask;
}

inline void clear_pins(volatile uint8_t *port, uint8_t bit_clear_mask)
{
  *port &= ~bit_clear_mask;
}

#endif
