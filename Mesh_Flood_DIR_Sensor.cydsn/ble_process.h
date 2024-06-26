/*******************************************************************************
* File Name: ble_process.h
*
* Version: 1.0
*
* Description:
*  This file contains the headers and constants for BLE functions, such as
* event call back function, changing GAP roles and error codes.
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#if !defined(BLE_PROCESS_H)
#define BLE_PROCESS_H
#include "project.h"

/*****************************************************
*                  Enums and macros
*****************************************************/ 
//#define RGB_LED_DATA_LEN					8
#define TEMP_DATA_LEN					    8
#define RGB_PAYLOAD_LEN						4

#define TEMP_FIRST_INDEX					0
#define TEMP_SECOND_INDEX					1
#define TEMP_THIRD_INDEX					2
#define TEMP_FORTH_INDEX					3
#define RGB_LED_MAX_VAL						255
#define TEMP_OFF_THRESHOLD					2
#define NODE_ADDR_DATA_LEN					2
	
#define BROADCAST_ADDR						0x0000
#define PICONET_SUBMASK						0xFF00

#define CUSTOM_ADV_DATA_MARKER				0x02
#define ADV_DATA_NAME_START_INDEX			5
	
#define DATA_NODE_ADDR_TYPE_INDEX			0x00
#define RESERVED_BYTE_INDEX					0x01
#define DATA_NODE_ADDR_LSB_INDEX			0x02
#define DATA_NODE_ADDR_MSB_INDEX			0x03
#define DATA_INDEX							0x04
	
#define NODE_ADDR_MSB_VAL_INDEX				0x01
#define NODE_ADDR_LSB_VAL_INDEX				0x00
	
#define NODE_ADDR_TYPE_PICONET				0x01
#define RESERVED_BYTE_VALUE					0x00

/*****************************************************
*                  GATT Error codes
*****************************************************/
#define ERR_INVALID_HANDLE				0x01
#define ERR_INVALID_PDU					0x04
#define ERR_REQ_NOT_SUPP				0x06
#define ERR_ATTR_NOT_FOUND				0x0A
#define ERR_UNLIKELY_ERR				0x0E
#define ERR_INVALID_ATT_LEN				0x0D
#define ERR_INSUFF_RESOURCE				0x11
#define ERR_APPL_ERR_1					0x80

/*****************************************************
*                  Function Declarations
*****************************************************/
void GenericEventHandler(uint32 event, void * eventParam);
void SwitchRole(void);
void ConnectToPeripheralDevice(void);
void RestartCentralScanning(void);
void SendErrorCode(uint8 opCode, CYBLE_GATT_DB_ATTR_HANDLE_T  attrHandle, CYBLE_GATT_ERR_CODE_T errorCode);

#endif
/* [] END OF FILE */
