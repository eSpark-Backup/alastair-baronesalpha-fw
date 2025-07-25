/*
 * App.cpp
 *
 *  Created on: Jul 22, 2025
 *      Author: Farhan
 */

#include "App.h"

#define APP_TAG		"[ APP ] "

FDCAN_HandleTypeDef* pukk_display_can_interface = NULL;
FDCAN_HandleTypeDef* ecu_can_interface = NULL;
I2C_HandleTypeDef*   ads_interface = NULL;
TIM_HandleTypeDef*	 base_timer = NULL;						// base 1ms timer

volatile uint32_t gearCheckTimestamp = 0;
volatile uint32_t speedTachoCheckTimestamp = 0;
volatile uint32_t coolantCheckTimestamp = 0;
volatile uint32_t fuelCheckTimestamp = 0;
volatile uint32_t voltageCheckTimestamp = 0;


void _timer_callback(TIM_HandleTypeDef* htim)
{
	if(htim == base_timer)
	{
		// LOG_ITM_1(APP_TAG "Timer callback\n");
		uint32_t now = HAL_GetTick();
		
		if((now - gearCheckTimestamp) > GEAR_CHECK_TIME)
		{
			LOG_ITM_1(APP_TAG "check gears\n");
			gearCheckTimestamp = now;
		}
		if((now - speedTachoCheckTimestamp) > SPEED_TACH_TIME)
		{
			LOG_ITM_1(APP_TAG "check speed/tacho\n");
			speedTachoCheckTimestamp = now;
		}
		if((now - coolantCheckTimestamp) > COOLANT_TIME)
		{
			LOG_ITM_1(APP_TAG "check coolant\n");
			coolantCheckTimestamp = now;
		}
		if((now - fuelCheckTimestamp) > FUEL_GAUGE_TIME)
		{
			LOG_ITM_1(APP_TAG "check fuel\n");
			fuelCheckTimestamp = now;
		}
		if((now - voltageCheckTimestamp) > VOLT_CALIB_TIME)
		{
			LOG_ITM_1(APP_TAG "check voltage\n");
			voltageCheckTimestamp = now;
		}

	}
}

void app_init(FDCAN_HandleTypeDef* pukk_can, FDCAN_HandleTypeDef* ecu_can, I2C_HandleTypeDef* ads_i2c, TIM_HandleTypeDef* timer)
{
	pukk_display_can_interface = pukk_can;
	ecu_can_interface = ecu_can;
	ads_interface = ads_i2c;
	base_timer = timer;

	// populate timer pointer first
	HAL_TIM_RegisterCallback(base_timer, HAL_TIM_PERIOD_ELAPSED_CB_ID, _timer_callback);
	HAL_TIM_Base_Start_IT(base_timer);


	// Initialize timer based approach for logging GPIO signals and populate them in a buffer

	// Initialize timers for capturing tachmeter/speedometer pulses
	// use a seperate timer for logging pulse counts

	// Initialize i2c for communication with ads1115 adc ic
	// ch1: coolant, ch2: fuel gauge, ch3: voltage feedback/callibration (3.36v ideal reading), ch4: connected to GND

	// Initialize can bus communication
}

void app_execute(void)
{
	if (console_line_ready)
	{
		console_line_ready = false;

		console_print_buffer();
	}
}
