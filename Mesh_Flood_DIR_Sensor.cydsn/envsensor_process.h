/* ========================================
 *
 * Copyright Sopicha S., 2018
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

/* [] END OF FILE */
#if !defined(ENVSENSOR_PROCESS_H)
#define ENVSENSOR_PROCESS_H
#include "project.h"    
    
/***************************************
* Exported structures and unions
***************************************/    
    
typedef struct
{
	/** Public Device Address or Random Device Address for
	   each device which responded to scanning. */
    uint8*	    				bleAddr;		

	/** sequen number of data */
    uint8       				seqNum;								

	/** temperature value */
    float 	    				temp;
    
    /** humidity value */
    float 	    				humi;
    
    /** light value */
    short 	    				light;
    
    /** uv value */
    float 	    				uv;
    
    /** pressure value */
    float 	    				pressure;
    
    /** noise value */
    float 	    				noise;
    
    /** dimention value */
    float 	    				di;
    
    /** heat value */
    float 	    				heat;
    
    /** acceleration at x axis value */
    float 	    				ax;
    
    /** acceleration at y axis value */
    float 	    				ay;
    
    /** acceleration at z axis value */
    float 	    				az;
    
    /** battery value */
    float 	    				battery;

	/** Rssi of the responding device.
				 * Range: -85 <= N <= 0
				 * Units: dBm */
    int8	    				rssi;	
    
    /** diatance value */
    float 	    				distance;
    
    uint8                        flagActive;
	
} ENV_SENSOR_T;    
/*****************************************************
*                  Function Declarations
*****************************************************/
    
void EnvSensorProcess(int8 rssi ,uint8* data);    
short Byte2Short(uint8 hi, uint8 low);   
unsigned short Byte2UShort(uint8 hi, uint8 lo);
ENV_SENSOR_T GetSensorData(void);
float ReturnAccuracy(int8 rssi);
#endif