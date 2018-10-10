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
			/* Set the next RGB LED color as reflection of new sensor data.
			* Broadcast to all. */
			SetNextColor();	
		}
}

/*******************************************************************************
* Function Name: SetNextColor
********************************************************************************
* Summary:
*        Sets next RGB color and intensity data
*
* Parameters:
*  Global variables for R,G,B and intensity
*
* Return:
*  void
*
*******************************************************************************/
void SetNextColor(void)
{
	/* State variable for controlling color */
	static uint8 color_state = COLOR_STATE_RED;
	uint8 red, green, blue, intensity;
	
	switch(color_state)
	{
		case COLOR_STATE_RED:
			red = 0xFF;
			green = 0x00;
			blue = 0x00;
			intensity = 0xFF;
		
			color_state = COLOR_STATE_GREEN;
		break;
		
		case COLOR_STATE_GREEN:
			red = 0x00;
			green = 0xFF;
			blue = 0x00;
			intensity = 0xFF;
		
			color_state = COLOR_STATE_BLUE;
		break;
		
		case COLOR_STATE_BLUE:
			red = 0x00;
			green = 0x00;
			blue = 0xFF;
			intensity = 0xFF;
		
			color_state = COLOR_STATE_YELLOW;
		break;
		
		case COLOR_STATE_YELLOW:
			red = 0xFF;
			green = 0xFF;
			blue = 0x00;
			intensity = 0xFF;
		
			color_state = COLOR_STATE_CYAN;
		break;
		
		case COLOR_STATE_CYAN:
			red = 0x00;
			green = 0xFF;
			blue = 0xFF;
			intensity = 0xFF;
		
			color_state = COLOR_STATE_PURPLE;
		break;
		
		case COLOR_STATE_PURPLE:
			red = 0xFF;
			green = 0x00;
			blue = 0xFF;
			intensity = 0xFF;
		
			color_state = COLOR_STATE_WHITE;
		break;
		
		case COLOR_STATE_WHITE:
			red = 0xFF;
			green = 0xFF;
			blue = 0xFF;
			intensity = 0xFF;
		
			color_state = COLOR_STATE_HALF_INT;
		break;
		
		case COLOR_STATE_HALF_INT:
			red = 0xFF;
			green = 0xFF;
			blue = 0xFF;
			intensity = 0x22;
		
			color_state = COLOR_STATE_RED;
		break;
		
		default:
			red = 0xFF;
			green = 0x00;
			blue = 0x00;
			intensity = 0xFF;
		
			color_state = COLOR_STATE_RED;	
		break;
	}
	
	/* Update PrISM parameters for new RGB LED color */
	RGBData[TEMP_FIRST_INDEX] = red;
	RGBData[TEMP_SECOND_INDEX] = green;
	RGBData[TEMP_THIRD_INDEX] = blue;
	RGBData[TEMP_FORTH_INDEX] = intensity;
	
	/* Update PrISM parameters for new  RGB LED data */
	UpdateRGBled(RGBData, 4);	
    
	/* Set the role switch flag so that main loop can switch to Central role */
	switch_Role = TRUE;
	#if (DEBUG_ENABLED == 1)
		UART_UartPutString("switch_Role called from SetNextColor() ");
		SendBLEStatetoUART(CyBle_GetState());
		UART_UartPutCRLF(' ');
	#endif
}

/* [] END OF FILE */
