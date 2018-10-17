/*******************************************************************************
* File Name: sensor_process.c
*
* Version: 1.0
*
* Description:
* This file contains the definiton for sensor related functions.
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
/*******************************************************************************
* For this project, the sensor input is the button press and sensor data is
* a new RGB LED data to all nodes.
*******************************************************************************/

#include <main.h>
extern uint8 ble_gap_state;
extern uint8 RGBData[];
extern uint8 switch_Role;
extern uint8 dataADVCounter;
uint8 time_to_get_sensordata = FALSE;
uint8 check_pressed = FALSE;
extern uint8 scan_for_titan;

/*******************************************************************************
* Function Name: CheckSensorStatus
********************************************************************************
* Summary:
*        Scans for new sensor data and initiates BLE data transfer over Mesh, if
* new sensor data found. 
* For this project, the sensor input is User switch and sensor data is a new RGB 
* LED data sent to all the nodes (broadcast)
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void CheckSensorStatus(void)
{
		/* Change color (new sensor data) only when the node is in
		* peripheral mode and advertising */
		if((BLE_PERIPHERAL == ble_gap_state)
			&& (CYBLE_STATE_ADVERTISING == CyBle_GetState()))
		{
			#if (DEBUG_ENABLED == 1)
			UART_UartPutString("Time to get sensor data:  ");
			UART_UartPutCRLF(' ');
			#endif
			time_to_get_sensordata = TRUE;
            scan_for_titan = FALSE;
            /* Set the role switch flag so that main loop can switch to Central role */
	        switch_Role = TRUE;
            #if (DEBUG_ENABLED == 1)
    		UART_UartPutString("switch_Role called from CheckSensorStatus ");
    		SendBLEStatetoUART(CyBle_GetState());
    		UART_UartPutCRLF(' ');
    	    #endif
		}
}
/* [] END OF FILE */
