#pragma once
//! mcp23008.h
//!
//! communication with mcp23008 GPIO expander over i2c

#include "pico/stdlib.h"

#define MCP23008_IODIR 		0x00
//#define MCP23008_IPOL 		0x01
//#define MCP23008_GPINTEN 	0x02
//#define MCP23008_DEFVAL 	0x03
//#define MCP23008_INTCON 	0x04
//#define MCP23008_IOCON 		0x05
///#define MCP23008_GPPU 		0x06
//#define MCP23008_INTF 		0x07
//#define MCP23008_INTCAP 	0x08
#define MCP23008_GPIO 		0x09
#define MCP23008_OLAT 		0x0A

struct MCP23008 {
	char address;
	char sda_pin;
	char scl_pin;
	i2c_inst_t *i2c_instance;
	char state;
};

void setup_mcp23008(struct MCP23008 *mcp);

void update_mcp23008_state(struct MCP23008 *mcp);

