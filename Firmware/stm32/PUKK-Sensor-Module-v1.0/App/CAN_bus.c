#include "CAN_bus.h"
#include "core_cm7.h"
#include <string.h>

#define TAG		"[ CAN Bus ] "


void handle_error(uint32_t error_count, uint32_t error_code){
	LOG_ITM_1(TAG "CAN1 TX Error #%lu: 0x%08lX\n", error_count, error_code);

	if (error_code & HAL_FDCAN_ERROR_FIFO_FULL) {
		LOG_ITM_1(TAG "CAN1: TX FIFO Full - waiting...\n");
		HAL_Delay(10);
	}
}

void build_can1_message(){

	/*CAN 1(PUKK DISPLAY MODULE CAN) Message 1: Indicators, Gears, Battery Volts, Tachometer Readings*/

//	Can1TxData1[0] = (uint8_t)sensor_data.indicators;
	Can1TxData1[0] = sensor_data.indicators.oil_pressure << 5 |
				     sensor_data.indicators.oil_temp     << 4 |
				     sensor_data.indicators.engine_temp  << 3 |
				     sensor_data.indicators.high_beam    << 2 |
				     sensor_data.indicators.right_turn   << 1 |
				     sensor_data.indicators.left_turn    << 0 ;

//	Can1TxData1[1] = (uint8_t)sensor_data.gear;
	Can1TxData1[1] = sensor_data.gear.neutral << 5 |
				     sensor_data.gear.gear_1  << 4 |
				     sensor_data.gear.gear_2  << 3 |
				     sensor_data.gear.gear_3  << 2 |
				     sensor_data.gear.gear_4  << 1 |
				     sensor_data.gear.gear_5  << 0 ;


	memcpy(&Can1TxData1[2], &sensor_data.battery_volt, 2);// Can1TxData1[2], Can1TxData1[3]
	memcpy(&Can1TxData1[4], &sensor_data.tacho, 2);		// Can1TxData1[4], Can1TxData1[5]

	/*CAN 1(PUKK DISPLAY MODULE CAN) Message 2: Speed, Coolant Temperature*/
	memcpy(&Can1TxData2[0], &sensor_data.speed, 4);		// Can1TxData2[0], Can1TxData2[1], Can1TxData2[2], Can1TxData2[3]
	memcpy(&Can1TxData2[4], &sensor_data.coolant_temp, 4);// Can1TxData2[4], Can1TxData2[5], Can1TxData2[6], Can1TxData2[7]

	/*CAN 1(PUKK DISPLAY MODULE CAN) Message 3: Fuel Level*/
	memcpy(&Can1TxData3[0], &sensor_data.fuel_lvl, 4);	// Can1TxData3[0], Can1TxData3[1], Can1TxData3[2], Can1TxData3[3]

	// Print All Messages
	LOG_ITM_1(TAG "========================\n");
	LOG_ITM_1(TAG "CAN Message Transmitting\n");
	LOG_ITM_1(TAG "========================\n");
	LOG_ITM_1(TAG "CAN Message 1: Indicators[0], Gears[1], Battery Volts[2][3], Tachometer Readings[4][5]: \n");
	LOG_ITM_1(TAG "");
	for(int i = 0; i < 7; i++){
	  LOG_ITM_1("%02X ", Can1TxData1[i]);
	}
	LOG_ITM_1("\n");

	LOG_ITM_1(TAG "CAN Message 2: Speed[0][1][2][3], Coolant Temperature[4][5][6][7]: \n");
	LOG_ITM_1(TAG "");
	for(int i = 0; i < 7; i++){
	  LOG_ITM_1("%02X ", Can1TxData2[i]);
	}
	LOG_ITM_1("\n");

	LOG_ITM_1(TAG "CAN Message 3: Fuel Level[0][1][2][3]: \n");
	LOG_ITM_1(TAG "");
	for(int i = 0; i < 7; i++){
	  LOG_ITM_1("%02X ", Can1TxData3[i]);
	}
	LOG_ITM_1("\n");
}

//void transmit_can_message(FDCAN_HandleTypeDef *hfdcan, const FDCAN_TxHeaderTypeDef *pTxHeader,
//        const uint8_t pTxData){
//	status1 = HAL_FDCAN_AddMessageToTxFifoQ(hfdcan, pTxHeader, pTxData);
//	if(status1 != HAL_OK) {
//		error_count1++;
//		handle_error(error_count1, hfdcan->ErrorCode);
//	} else {
//		LOG_ITM_1(TAG "CAN1: Message sent successfully\n");
//	}
//}

void can_transmit_data(){

	build_can1_message(); // A message that will be transmitted through display CAN to Receiver

//	HAL_GPIO_WritePin(DISPLAY_CAN_STB_GPIO_Port, DISPLAY_CAN_STB_Pin, GPIO_PIN_RESET); // Wake from StandBy Mode by STB Pin to LOW

	TxHeader1.Identifier = 0x101;
	status1 = HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader1, Can1TxData1);
	if(status1 != HAL_OK) {
		error_count1++;
		handle_error(error_count1, hfdcan1.ErrorCode);
	} else {
		LOG_ITM_1(TAG "CAN1: Message sent successfully\n");
	}
	TxHeader1.Identifier = 0x102;
	status1 = HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader1, Can1TxData2);
	if(status1 != HAL_OK) {
		error_count1++;
		handle_error(error_count1, hfdcan1.ErrorCode);
	} else {
		LOG_ITM_1(TAG "CAN1: Message sent successfully\n");
	}

	TxHeader1.Identifier = 0x103;
	status1 = HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader1, Can1TxData3);
	if(status1 != HAL_OK) {
		error_count1++;
		handle_error(error_count1, hfdcan1.ErrorCode);
	} else {
		LOG_ITM_1(TAG "CAN1: Message sent successfully\n");
	}

	RESET_FLAG(CAN_COMM_FLAG_BIT);
}
