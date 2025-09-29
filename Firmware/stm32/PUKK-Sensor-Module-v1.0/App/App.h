/*
 * App.h
 *
 *  Created on: Jul 22, 2025
 *      Author: Farhan
 */

#ifndef APP_H_
#define APP_H_

#include "main.h"
#include <stdio.h>
#include <stdbool.h>

#define FW_VERSION      "v0.1.2"
#define BOARD_NAME      "Alastair-BaronessAlpha"


#define GEAR_FLAG_BIT				(1U << 0)
#define SPEED_TACHO_FLAG_BIT		(1U << 1)
#define COOLANT_FLAG_BIT			(1U << 2)
#define FUEL_FLAG_BIT				(1U << 3)
#define VOLTAGE_FLAG_BIT			(1U << 4)
#define INDICATIONS_FLAG_BIT		(1U << 5)
#define CAN_COMM_FLAG_BIT           (1U << 12)
#define PRINT_INFO_FLAG_BIT			(1U << 13)
#define ADS_DATA_READY_FLAG_BIT		(1U << 14)
#define OP_RUNNING_FLAG_BIT			(1U << 15)

#define RESET_FLAG(x)				(flags &= ~(x))
#define SET_FLAG(x)					(flags |= (x))
#define CHECK_FLAG(x)				(flags & (x))

extern volatile uint16_t flags;

// --------------------------------------------------------------
#define USE_SYSTICK_HANDLER         1

// time delay after which sample should be taken   (in ms)
#define GEAR_CHECK_TIME             10
#define SPEED_TACH_TIME             100             // might need seperate time periods for tacho and speed readings
#define COOLANT_TIME                500
#define FUEL_GAUGE_TIME             1000
#define VOLT_CALIB_TIME             5000            // check battery voltage every 5secs
#define INDICATIONS_TIME            100

#define PRINT_DATA_TIME             5000            // print debug data every 5secs
#define CAN_TRANSMIT_TIME           4000            // CAN transmit data every 4secs


// use a variable style format? or store these values in NVS?
// should these parameters be changeble from console/CAN bus?
#define TACHO_PULSES_PER_REV        60.0f           // check actual sensor value
#define SPEEDO_PULSES_PER_REV       4.0f            // check actual sensor value
#define VEHICLE_WHEEL_CIRCUM        2105.0f         // whel circumference in mm

// float switch resistance range
#define FUEL_SENSOR_FIXED_OHM       75.0f           // constant resistance R79
#define FUEL_SENSOR_MAX_OHM         91.6f           // resistance of sensor when lever in full position
#define FUEL_SENSOR_MIN_OHM         7.1f            // resistance of sensor when lever in empty position
#define FUEL_SENSOR_VOLTAGE         12.0f           // voltage applied on voltage regulator


extern FDCAN_HandleTypeDef* pukk_display_can_interface;
extern FDCAN_HandleTypeDef* ecu_can_interface;
extern I2C_HandleTypeDef*   ads_interface;
extern TIM_HandleTypeDef*	tacho_timer;
extern TIM_HandleTypeDef*	speedo_timer;
extern TIM_HandleTypeDef*	base_timer;

extern uint16_t pulses_per_rev;             // 
extern float wheel_circumference_m;         // wheel circumference in meters

typedef struct
{
    uint8_t oil_pressure    :1;
    uint8_t oil_temp        :1;
    uint8_t engine_temp     :1;
    uint8_t high_beam       :1;
    uint8_t right_turn      :1;
    uint8_t left_turn       :1;
    uint8_t                 :2;     // reserved for later use
} indicators_t;

typedef struct
{
    uint8_t neutral         :1;
    uint8_t gear_1          :1;
    uint8_t gear_2          :1;
    uint8_t gear_3          :1;
    uint8_t gear_4          :1;
    uint8_t gear_5          :1;
    uint8_t                 :2;     // reserved for later use
} gears_t;

typedef struct {
    indicators_t indicators;
    gears_t gear;
    uint16_t battery_volt;
    uint16_t tacho;
    float speed;
    float coolant_temp;
    float fuel_lvl;
} sensor_data_t;

extern sensor_data_t sensor_data;


extern FDCAN_HandleTypeDef hfdcan1;
extern FDCAN_HandleTypeDef hfdcan2;

extern FDCAN_TxHeaderTypeDef TxHeader1;
extern FDCAN_RxHeaderTypeDef RxHeader1;

extern FDCAN_TxHeaderTypeDef TxHeader2;
extern FDCAN_RxHeaderTypeDef RxHeader2;


extern uint8_t Can1TxData1[8];

extern uint8_t Can1TxData2[8];

extern uint8_t Can1TxData3[8];

extern uint8_t Can1TxData4[8];


void app_init(FDCAN_HandleTypeDef* pukk_can, FDCAN_HandleTypeDef* ecu_can, I2C_HandleTypeDef* ads_i2c, TIM_HandleTypeDef* tacho, TIM_HandleTypeDef* speedo, TIM_HandleTypeDef* timer);
void app_execute(void);

#if(!USE_SYSTICK_HANDLER)
void _tick_callback(TIM_HandleTypeDef* htim);
#else
void _tick_callback(void);
#endif


// --------------------------------------------------------------
extern UART_HandleTypeDef*  console_interface;
extern volatile bool console_line_ready;
extern int8_t send_data;
extern HAL_StatusTypeDef ret;

void console_init(UART_HandleTypeDef* huart);
void console_print(const char *format, ...);
void console_print_buffer(void);
int  console_read_line(char *buffer, int max_len);
void can_transmit_data();

extern HAL_StatusTypeDef status1, status2;
extern uint32_t error_count1, error_count2;

#endif /* APP_H_ */
