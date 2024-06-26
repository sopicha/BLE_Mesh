/*******************************************************************************
* File Name: main.h
*
* Version: 1.0
*
* Description:
* This is header file containing function declarations for all application related
* functions as well as Macros. Also contains pre-processor directives to enable
* or disable a particular feature.
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#if !defined(MAIN_H)
#define MAIN_H
	
#include <project.h>
#include <debug.h>
#include "stdio.h"
#include "math.h"
#include <stdlib.h>
#include <ble_process.h>
#include <WDT.h>
#include <low_power.h>
#include <WriteUserSFlash.h>
#include <sensor_process.h>
#include <envsensor_process.h>    
#include <store.h>    

/*****************************************************
*                  Variables
*****************************************************/ 
extern volatile uint16 current_watchdog_counter;

/*****************************************************
*             Pre-processor Directives
*****************************************************/ 
#define ENABLE_ADV_DATA_COUNTER
//#define ENABLE_LOW_POWER_MODE
#define ENABLE_CENTRAL_DISCOVERY
#define STORE_SFLASH_NODE_ADDRESS
#define SENSOR_DATA_ROUTING

#define DEBUG_ENABLED						1
	
#define RESTART_BLE_STACK						//CDT 214662 workaround
#define NODE2
	
/*****************************************************
*                  Enums and macros
*****************************************************/ 
#define TRUE								1
#define FALSE								0
#define BLE_PERIPHERAL						1
#define BLE_CENTRAL							2
#define SCAN_TAG_DATA_LEN					20
	
#ifdef NODE1    
#define CENTRAL_STATE_SPAN					0.5
#endif

#ifdef NODE2
#define CENTRAL_STATE_SPAN					1
#endif    

#define WatchDog_CurrentCount()				(current_watchdog_counter)
#define SENSOR_RECEIVE_TIME                 600 //1 mins 

/*****************************************************
*                  Function Declarations
*****************************************************/
void InitializeSystem(void);
#endif

/* [] END OF FILE */
