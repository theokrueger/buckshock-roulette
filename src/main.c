#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "pico/time.h"

#define RF_PIN 15


int main() {
	gpio_set_function(RF_PIN, GPIO_FUNC_PWM);
	uint slice_num = pwm_gpio_to_slice_num(RF_PIN);

	// set 10.65khz pwm
	pwm_set_wrap(slice_num, 11737); // 125MHz clock / 11737 wrap ~= 10.65Khz wrap
	pwm_set_chan_level(slice_num, PWM_CHAN_B, 5869);

	while(true) {
		pwm_set_enabled(slice_num, false);
		sleep_ms(15);

		pwm_set_enabled(slice_num, true);
		sleep_ms(20);
	}
}
