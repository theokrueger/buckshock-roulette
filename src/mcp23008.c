//! mcp23008.c
//!
//! communication with mcp23008 GPIO expander over i2c

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "../include/mcp23008.h"

void setup_mcp23008(struct MCP23008 *mcp)
{
	printf("Setup MCP23008 with SDA %i and SCL %i at %X\n", mcp->sda_pin, mcp->scl_pin, mcp->address);
	// pins
	i2c_init(mcp->i2c_instance, 100 * 1000); // 100khz baud
	gpio_set_function(mcp->sda_pin, GPIO_FUNC_I2C);
	gpio_set_function(mcp->scl_pin, GPIO_FUNC_I2C);
	gpio_pull_up(mcp->sda_pin);
	gpio_pull_up(mcp->scl_pin);

	// set all pins as output
        char buf[2] = {
		MCP23008_IODIR,
		0b00000000,
	};
	i2c_write_blocking(mcp->i2c_instance, mcp->address, buf, sizeof(char) * 2, false);

	printf("Done setting up MCP23008\n");
}

void update_mcp23008_state(struct MCP23008 *mcp)
{
	char buf[2] = {
		MCP23008_GPIO,
	        mcp->state,
	};
	i2c_write_blocking(mcp->i2c_instance, mcp->address, buf, sizeof(char) * 2, false);
}
