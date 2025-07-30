/*
 * App.cpp
 *
 *  Created on: Jul 22, 2025
 *      Author: Farhan
 */

#include "App.h"
#include "ADS1115.h"
#include "core_cm7.h"

#define TAG		"[ APP ] "

#define CHECK_AND_SET_FLAG(timestamp, interval, flag1, flag2, log_statement, print_log)  \
    do {                                                                          		 \
        if ((now - (timestamp)) > (interval)) {                                  		 \
            if (!CHECK_FLAG(flag1) && !CHECK_FLAG(flag2)) {                              \
                if (print_log)  (void)log_statement;                                     \
                (timestamp) = now;                                               		 \
                SET_FLAG(flag1);                                                  		 \
            }                                                                    		 \
        }                                                                        		 \
    } while(0)


#define PIN_CHECK(port, bit, pin_base)  (sensor_data.port.bit = !HAL_GPIO_ReadPin(pin_base##_GPIO_Port, pin_base##_Pin))

#define ADS_GAIN					(6144.0f)				// using PGA_6_144 ads gain
#define LSB_VOLTS					(ADS_GAIN/32768.0f)
#define ADS_ERROR_COMPENSATION		24						// this much mV error is found between actual and read values by the ADS
#define SCHOTTKY_DIODE_COMPENSATION	350						// voltage drop on D1 diode

#define BATTERY_DIVIDER				((10000.0f+3900.0f)/(3900.0f))		// 	Ra = 10kohm, Rb = 3.9kohm;	Vin = Vread * ((Ra+Rb)/Rb)

#define FUEL_FLOAT_SWITCH_VMIN		1.2f
#define FUEL_FLOAT_SWITCH_VMAX		3.3f

#define NTC_R_FIXED       			49900.0f     // 49.9kΩ resistor
#define NTC_BETA          			3950.0f      // Beta constant for your NTC
#define NTC_R0            			10000.0f     // Resistance at 25°C
#define NTC_T0            			298.15f      // Reference temperature in Kelvin (25°C)
#define NTC_VIN_MV        			5000.0f      // Supply voltage in millivolts on NTC coolant divider network

#define KM_PER_HOUR					(1000.0 * 3600.0f)

static const char *gear_names[] = {
    "Neutral",  // bit 0
    "Gear 1",   // bit 1
    "Gear 2",   // bit 2
    "Gear 3",   // bit 3
    "Gear 4",   // bit 4
    "Gear 5",   // bit 5
	"UNKNOWN STATE"
};

FDCAN_HandleTypeDef* pukk_display_can_interface = NULL;
FDCAN_HandleTypeDef* ecu_can_interface = NULL;
I2C_HandleTypeDef*   ads_i2c_interface = NULL;
TIM_HandleTypeDef*	 tacho_timer = NULL;
TIM_HandleTypeDef*	 speedo_timer = NULL;
#if(!USE_SYSTICK_HANDLER)
TIM_HandleTypeDef*	 base_timer = NULL;						// base 1ms timer
#endif
ADS1115_Handle_t*	 ads_handle = NULL;

volatile uint16_t flags = GEAR_FLAG_BIT | SPEED_TACHO_FLAG_BIT | COOLANT_FLAG_BIT | FUEL_FLAG_BIT | VOLTAGE_FLAG_BIT | INDICATIONS_FLAG_BIT;

struct
{
	indicators_t indicators;
	gears_t	gear;
	uint16_t tacho;
	float speed;
	float coolant_temp;
	float fuel_lvl;
	uint16_t battery_volt;
} sensor_data;


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == ADC_RDY_Pin)
	{
		SET_FLAG(ADS_DATA_READY_FLAG_BIT);
	}
}

#if(!USE_SYSTICK_HANDLER)
void _tick_callback(TIM_HandleTypeDef* htim)
#else
void _tick_callback(void)
#endif
{
	static uint32_t gearCheckTimestamp = 0;
	static uint32_t speedTachoCheckTimestamp = 0;
	static uint32_t coolantCheckTimestamp = 0;
	static uint32_t fuelCheckTimestamp = 0;
	static uint32_t voltageCheckTimestamp = 0;
	static uint32_t indicationsCheckTimestamp = 0;
	static uint32_t printDataTimestamp = 0;
	
#if(!USE_SYSTICK_HANDLER)
	if(htim == base_timer)
	{
#endif
		// LOG_ITM_2(TAG "Timer callback\n");
		uint32_t now = HAL_GetTick();
		
		CHECK_AND_SET_FLAG(gearCheckTimestamp, GEAR_CHECK_TIME, GEAR_FLAG_BIT, 0, LOG_ITM_2(TAG "check gears\n"), false);
		CHECK_AND_SET_FLAG(speedTachoCheckTimestamp, SPEED_TACH_TIME, SPEED_TACHO_FLAG_BIT, 0, LOG_ITM_2(TAG "check speed/tacho\n"), false);
		CHECK_AND_SET_FLAG(coolantCheckTimestamp, COOLANT_TIME, COOLANT_FLAG_BIT, OP_RUNNING_FLAG_BIT, LOG_ITM_2(TAG "check coolant\n"), false);
		CHECK_AND_SET_FLAG(fuelCheckTimestamp, FUEL_GAUGE_TIME, FUEL_FLAG_BIT, OP_RUNNING_FLAG_BIT, LOG_ITM_2(TAG "check fuel\n"), false);
		CHECK_AND_SET_FLAG(voltageCheckTimestamp, VOLT_CALIB_TIME, VOLTAGE_FLAG_BIT, OP_RUNNING_FLAG_BIT, LOG_ITM_2(TAG "check voltage\n"), false);
		CHECK_AND_SET_FLAG(indicationsCheckTimestamp, INDICATIONS_TIME, INDICATIONS_FLAG_BIT, 0, LOG_ITM_2(TAG "check indicators\n"), false);
		CHECK_AND_SET_FLAG(printDataTimestamp, PRINT_DATA_TIME, PRINT_INFO_FLAG_BIT, 0, LOG_ITM_2(TAG "print sensor data\n"), false);
#if(!USE_SYSTICK_HANDLER)
	}
#endif
}

float voltage_to_percentage(float Vin, float Vmin, float Vmax)
{
    if (Vin <= Vmin) return 0.0f;
    if (Vin >= Vmax) return 100.0f;
    return ((Vin - Vmin) / (Vmax - Vmin)) * 100.0f;
}

float ntc_mv_to_temperature(float vout_mv)
{
    // Prevent divide-by-zero or invalid voltage range
    if (vout_mv <= 0.0f || vout_mv >= NTC_VIN_MV)
        return -273.15f;

    // Calculate NTC resistance using millivolts
    float r_ntc = (vout_mv * NTC_R_FIXED) / (NTC_VIN_MV - vout_mv);

    // Apply Beta formula
    float inv_T = (1.0f / NTC_T0) + (1.0f / NTC_BETA) * logf(r_ntc / NTC_R0);
    float temperature_K = 1.0f / inv_T;

    return temperature_K - 273.15f;  // Return °C
}

void check_gears()
{
	// sensor_data.gear.neutral = !HAL_GPIO_ReadPin(NEUTRAL_SIG_GPIO_Port, NEUTRAL_SIG_Pin);
	PIN_CHECK(gear, neutral, NEUTRAL_SIG);
	PIN_CHECK(gear, gear_1, GEAR_1_SIG);
	PIN_CHECK(gear, gear_2, GEAR_2_SIG);
	PIN_CHECK(gear, gear_3, GEAR_3_SIG);
	PIN_CHECK(gear, gear_4, GEAR_4_SIG);
	PIN_CHECK(gear, gear_5, GEAR_5_SIG);

	uint8_t gear_val = *((uint8_t *)&sensor_data.gear);
	int i;
    for (i=0; i < 6; i++)
    {
        if (gear_val & (1 << i))
        {
			break;  // stop after first match (assumes only one gear is active)
        }
    }
	
	LOG_ITM_2(TAG "Current gear: %s\n", gear_names[i]);
	RESET_FLAG(GEAR_FLAG_BIT);
}

void check_tachospeed()
{
	uint32_t tacho_pulses = __HAL_TIM_GET_COUNTER(tacho_timer);
	uint32_t speedo_pulses = __HAL_TIM_GET_COUNTER(speedo_timer);
	__HAL_TIM_SET_COUNTER(tacho_timer, 0);
	__HAL_TIM_SET_COUNTER(speedo_timer, 0);


	float tacho_hz = (tacho_pulses / ((float)(SPEED_TACH_TIME)/1000.0f));
	float speedo_hz = (speedo_pulses / ((float)(SPEED_TACH_TIME)/1000.0f));

	float rpm = tacho_hz * 60.0f / TACHO_PULSES_PER_REV;
	float speed_kph = speedo_hz * (VEHICLE_WHEEL_CIRCUM/1000.0) / KM_PER_HOUR;

	sensor_data.tacho = rpm;
	sensor_data.speed = speed_kph;

	LOG_ITM_2(TAG "[ TIM1 ] Counter: %d\tFreq: %f Hz\tRPM: %f\n", tacho_pulses, tacho_hz, rpm);
	LOG_ITM_2(TAG "[ TIM5 ] Counter: %d\tFreq: %f Hz\tSpeed: %f Km/H\n", speedo_pulses, speedo_hz, speed_kph);

	RESET_FLAG(SPEED_TACHO_FLAG_BIT);
}

void read_ads()
{
	uint16_t ads_data = ADS1115_getData(ads_handle);
	float volts = ads_data * LSB_VOLTS;						// in mV
	switch(default_ads1115_config.channel)
	{
		case CHANNEL_AIN0_GND:
			sensor_data.coolant_temp = ntc_mv_to_temperature(volts);
			LOG_ITM_2(TAG "AIN0: %d(0x%X) volts: %f mV\n", ads_data, ads_data, volts);
			LOG_ITM_2(TAG "Coolent Temp: %f °C\n", sensor_data.coolant_temp);
			RESET_FLAG(COOLANT_FLAG_BIT);
			break;
		case CHANNEL_AIN1_GND:
			sensor_data.fuel_lvl = voltage_to_percentage(volts, FUEL_FLOAT_SWITCH_VMIN, FUEL_FLOAT_SWITCH_VMAX);
			LOG_ITM_2(TAG "AIN1: %d(0x%X) volts: %f mV\n", ads_data, ads_data, volts);
			LOG_ITM_2(TAG "Fuel level: %d%%\n", sensor_data.fuel_lvl);
			RESET_FLAG(FUEL_FLAG_BIT);
			break;
		case CHANNEL_AIN2_GND:
			sensor_data.battery_volt = ((volts+ADS_ERROR_COMPENSATION) * BATTERY_DIVIDER) + SCHOTTKY_DIODE_COMPENSATION;
			LOG_ITM_2(TAG "AIN2: %d(0x%X) volts: %f mV\n", ads_data, ads_data, volts);
			LOG_ITM_2(TAG "Battery voltage: %d mV\n", sensor_data.battery_volt);
			RESET_FLAG(VOLTAGE_FLAG_BIT);
			break;
		case CHANNEL_AIN3_GND:
			LOG_ITM_2(TAG "AIN3: %d(0x%X) volts: %f mV\n", ads_data, ads_data, volts);
			if(volts>100)		// if read voltage on AIN3 is above thsi much voltage than there is error
			{
				LOG_ITM_2(TAG "Invalid voltage read on AIN3(GND) line\n");
			}
			// RESET_FLAG();
			break;
		default:
			LOG_ITM_2(TAG "UNKNOWN ADS CHANNEL\n");
			break;
	}

	RESET_FLAG(ADS_DATA_READY_FLAG_BIT | OP_RUNNING_FLAG_BIT);
}

void check_ads()
{
	if(!CHECK_FLAG(OP_RUNNING_FLAG_BIT) && CHECK_FLAG(COOLANT_FLAG_BIT))
	{
		// start ads conversion, setup readout on RDY interrupt
		default_ads1115_config.channel = CHANNEL_AIN0_GND;
		ADS1115_updateConfig(ads_handle, default_ads1115_config);
		ADS1115_oneShotMeasure(ads_handle);
		SET_FLAG(OP_RUNNING_FLAG_BIT);
	}
	if(!CHECK_FLAG(OP_RUNNING_FLAG_BIT) && CHECK_FLAG(FUEL_FLAG_BIT))
	{
		// start ads conversion, setup readout on RDY interrupt
		default_ads1115_config.channel = CHANNEL_AIN1_GND;
		ADS1115_updateConfig(ads_handle, default_ads1115_config);
		ADS1115_oneShotMeasure(ads_handle);
		SET_FLAG(OP_RUNNING_FLAG_BIT);
	}
	if(!CHECK_FLAG(OP_RUNNING_FLAG_BIT) && CHECK_FLAG(VOLTAGE_FLAG_BIT))
	{
		// start ads conversion, setup readout on RDY interrupt
		default_ads1115_config.channel = CHANNEL_AIN2_GND;
		ADS1115_updateConfig(ads_handle, default_ads1115_config);
		ADS1115_oneShotMeasure(ads_handle);
		SET_FLAG(OP_RUNNING_FLAG_BIT);
	}
}

void check_indicators()
{
	PIN_CHECK(indicators, oil_pressure, OIL_PRESURE_SIG);
	PIN_CHECK(indicators, oil_temp, OIL_TEMP_SIG);
	PIN_CHECK(indicators, engine_temp, ENGINE_TEMP_SIG);
	PIN_CHECK(indicators, high_beam, HIGH_BEAM_SIG);
	PIN_CHECK(indicators, right_turn, RIGHT_TURN_SIG);
	PIN_CHECK(indicators, left_turn, LEFT_TURN_SIG);

	LOG_ITM_2(TAG "Indicators:\n");
	LOG_ITM_2(TAG "\tOIL_PRESURE: %s\n", (sensor_data.indicators.oil_pressure) ? "true" : "false");
	LOG_ITM_2(TAG "\tOIL_TEMP: %s\n", (sensor_data.indicators.oil_temp) ? "true" : "false");
	LOG_ITM_2(TAG "\tENGINE_TEMP: %s\n", (sensor_data.indicators.engine_temp) ? "true" : "false");
	LOG_ITM_2(TAG "\tHIGH_BEAM: %s\n", (sensor_data.indicators.high_beam) ? "true" : "false");
	LOG_ITM_2(TAG "\tRIGHT_TURN: %s\n", (sensor_data.indicators.right_turn) ? "true" : "false");
	LOG_ITM_2(TAG "\tLEFT_TURN: %s\n", (sensor_data.indicators.left_turn) ? "true" : "false");

	RESET_FLAG(INDICATIONS_FLAG_BIT);
}

void print_data()
{
	LOG_ITM_1(TAG "=============SENSOR DATA==========\n");
	LOG_ITM_1(TAG "==================================\n");
	LOG_ITM_1(TAG "Indicators:\n");
	LOG_ITM_1(TAG "\tOIL_PRESURE: %s\n", (sensor_data.indicators.oil_pressure) ? "true" : "false");
	LOG_ITM_1(TAG "\tOIL_TEMP: %s\n", (sensor_data.indicators.oil_temp) ? "true" : "false");
	LOG_ITM_1(TAG "\tENGINE_TEMP: %s\n", (sensor_data.indicators.engine_temp) ? "true" : "false");
	LOG_ITM_1(TAG "\tHIGH_BEAM: %s\n", (sensor_data.indicators.high_beam) ? "true" : "false");
	LOG_ITM_1(TAG "\tRIGHT_TURN: %s\n", (sensor_data.indicators.right_turn) ? "true" : "false");
	LOG_ITM_1(TAG "\tLEFT_TURN: %s\n", (sensor_data.indicators.left_turn) ? "true" : "false");

	uint8_t gear_val = *((uint8_t *)&sensor_data.gear);
	int i;
    for (i=0; i < 6; i++)
    {
        if (gear_val & (1 << i))
        {
			break;  // stop after first match (assumes only one gear is active)
        }
    }
	LOG_ITM_1(TAG "Gear: %s\n", gear_names[i]);
	
	LOG_ITM_1(TAG "Revs: %d\n", sensor_data.tacho);
	LOG_ITM_1(TAG "Speed: %f\n", sensor_data.speed);
	LOG_ITM_1(TAG "Coolant Temp: %f °C\n", sensor_data.coolant_temp);
	LOG_ITM_1(TAG "Fuel level: %d%%\n", sensor_data.fuel_lvl);
	LOG_ITM_1(TAG "Battery: %d mV\n\n", sensor_data.battery_volt);

	RESET_FLAG(PRINT_INFO_FLAG_BIT);
}

void app_init(FDCAN_HandleTypeDef* pukk_can, FDCAN_HandleTypeDef* ecu_can, I2C_HandleTypeDef* ads_i2c, TIM_HandleTypeDef* tacho, TIM_HandleTypeDef* speedo, TIM_HandleTypeDef* timer)
{
	pukk_display_can_interface = pukk_can;
	ecu_can_interface = ecu_can;
	ads_i2c_interface = ads_i2c;
	tacho_timer = tacho;
	speedo_timer = speedo;
#if(!USE_SYSTICK_HANDLER)
	base_timer = timer;
#endif
	
	// Initialize timers for capturing tachmeter/speedometer pulses
	// use a seperate timer for logging pulse counts, use external sensor pulses as clock source
	HAL_TIM_Base_Start_IT(tacho_timer);
	HAL_TIM_Base_Start_IT(speedo_timer);
	
	// Initialize i2c for communication with ads1115 adc ic
	// ch1: coolant, ch2: fuel gauge, ch3: voltage feedback/callibration (3.36v ideal reading) (battery voltage), ch4: connected to GND
	ads_handle = ADS1115_init(ads_i2c_interface, default_ads1115_addr, default_ads1115_config);
	ADS1115_oneShotMeasure(ads_handle);
	SET_FLAG(OP_RUNNING_FLAG_BIT);

	// Initialize can bus communication

#if(!USE_SYSTICK_HANDLER)
	// setup all peripherals first, than initiate readout timer callback
	HAL_TIM_RegisterCallback(base_timer, HAL_TIM_PERIOD_ELAPSED_CB_ID, _tick_callback);
	HAL_TIM_Base_Start_IT(base_timer);
#endif
}

void app_execute(void)
{
	if (console_line_ready)
	{
		console_line_ready = false;

		console_print_buffer();
	}

	CHECK_FLAG(GEAR_FLAG_BIT) ? (void)check_gears() : (void)0;
	CHECK_FLAG(SPEED_TACHO_FLAG_BIT) ? (void)check_tachospeed() : (void)0;
	CHECK_FLAG(COOLANT_FLAG_BIT | FUEL_FLAG_BIT | VOLTAGE_FLAG_BIT) ? (void)check_ads() : (void)0;
	CHECK_FLAG(INDICATIONS_FLAG_BIT) ? (void)check_indicators() : (void)0;
	
	CHECK_FLAG(ADS_DATA_READY_FLAG_BIT) ? (void)read_ads() : (void)0;

	CHECK_FLAG(PRINT_INFO_FLAG_BIT) ? (void)print_data() : (void)0;
}

