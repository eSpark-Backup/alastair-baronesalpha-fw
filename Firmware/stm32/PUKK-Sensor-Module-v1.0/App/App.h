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

// --------------------------------------------------------------
// time delay after which sample should be taken   (in ms)
#define GEAR_CHECK_TIME             10
#define SPEED_TACH_TIME             50
#define COOLANT_TIME                500
#define FUEL_GAUGE_TIME             1000
#define VOLT_CALIB_TIME             5000            // check voltage every 5secs
#define INDICATIONS_TIME            100

#define PRINT_DATA_TIME             5000            // print debug data every 5secs

extern FDCAN_HandleTypeDef* pukk_display_can_interface;
extern FDCAN_HandleTypeDef* ecu_can_interface;
extern I2C_HandleTypeDef*   ads_interface;
extern TIM_HandleTypeDef*	base_timer;

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

void app_init(FDCAN_HandleTypeDef* pukk_can, FDCAN_HandleTypeDef* ecu_can, I2C_HandleTypeDef* ads_i2c, TIM_HandleTypeDef* timer);
void app_execute(void);

void _timer_callback(TIM_HandleTypeDef* htim);


// --------------------------------------------------------------
extern UART_HandleTypeDef*  console_interface;
extern volatile bool console_line_ready;

void console_init(UART_HandleTypeDef* huart);
void console_print(const char *format, ...);
void console_print_buffer(void);
int  console_read_line(char *buffer, int max_len);


#endif /* APP_H_ */
