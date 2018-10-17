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
#if !defined(STORE_H)
#define STORE_H
#include "project.h"    
#define MAX_TEMP_DATA               50     
#define MAX_NODE                    9
/***************************************
* Exported structures and unions
***************************************/    
    
/** struct as a table for collecting node data */
    
typedef struct
{	
    
    /** node address */
    CYBLE_GAP_BD_ADDR_T*	    				nodeAddr;		

	/** temperature data in short uint */
    uint8                       fistByteTempData[MAX_TEMP_DATA];
    
    uint8                       secondByteTempData[MAX_TEMP_DATA];	

	/** timestamp */
    uint16 	    				timestamp[MAX_TEMP_DATA];
    
    /** average temperature in short uint */
    uint8                       fistByteAvgData;
    
    uint8                       secondByteAvgData;	
    
    /** Number of all receiving temperature data */
    uint16 	    				total;
    
    /** ADV latest data */
    uint8 	    				adv;
} NODE_TABLE_T;    
/*****************************************************
*                  Function Declarations
*****************************************************/
void nodeInit(uint8 index);
uint8 updateNodeTable(CYBLE_GAP_BD_ADDR_T* nodeAddr, uint8 advdata, uint8 firstByteTemp, uint8 secondByteTemp, uint16 timestamp);
void calAverage(uint8 firstByteTemp, uint8 secondByteTemp);
void float2Bytes(float value, uint8* fistByte, uint8* secondByte);
uint8 getNodeFromTable(uint8 nodeAddr[6u],NODE_TABLE_T* node);
#endif