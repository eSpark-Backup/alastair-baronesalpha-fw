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

#define FW_VERSION      "v0.1.1"
#define BOARD_NAME      "Alastair-BaronessAlpha"

// tiem delay after which sample should be taken
#define GEAR_CHECK_TIME     1000
#define SPEED_TACH_TIME     1000
#define COOLANT_TIME        1500
#define FUEL_GAUGE_TIME     2000
#define VOLT_CALIB_TIME     10000           // check voltage every 10secs


extern volatile bool console_line_ready;

extern FDCAN_HandleTypeDef* pukk_display_can_interface;
extern FDCAN_HandleTypeDef* ecu_can_interface;
extern I2C_HandleTypeDef*   ads_interface;
extern TIM_HandleTypeDef*	base_timer;
extern UART_HandleTypeDef*  console_interface;


void console_init(UART_HandleTypeDef* huart);
void console_print(const char *format, ...);
void console_print_buffer(void);
int  console_read_line(char *buffer, int max_len);

void app_init(FDCAN_HandleTypeDef* pukk_can, FDCAN_HandleTypeDef* ecu_can, I2C_HandleTypeDef* ads_i2c, TIM_HandleTypeDef* timer);
void app_execute(void);

void _timer_callback(TIM_HandleTypeDef* htim);


#endif /* APP_H_ */
