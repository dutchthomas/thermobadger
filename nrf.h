
// Registers
#define NRF_CONFIG 0x00
#define NRF_EN_AA 0x01
#define NRF_EN_RXADDR 0x02
#define NRF_SETUP_AW 0x03
#define NRF_SETUP_RETR 0x04
#define NRF_RF_CH 0x05
#define NRF_RF_SETUP 0x06
#define NRF_STATUS 0x07
#define NRF_OBSERVE_TX 0x08
#define NRF_CD 0x09

// Read/write register
#define NRF_REG_W 0x20
#define NRF_REG_R 0x00

// Radio function
#define NRF_FLUSH_RX 0xE2
#define NRF_FLUSH_TX 0xE1

// Config
#define NRF_SETUP_RETR 0x04

// Pipeline settings
#define NRF_RX_ADDR_P0 0x0A
#define NRF_RX_PW_P0 0x11
#define NRF_TX_ADDR 0x10

// Payload
#define NRF_R_RX_PAYLOAD 0x61
#define NRF_W_TX_PAYLOAD 0xA0