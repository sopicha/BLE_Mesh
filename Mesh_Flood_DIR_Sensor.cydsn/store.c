/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

/* [] END OF FILE */
#include <main.h>
NODE_TABLE_T nodeTable[MAX_NODE];
uint8 number_of_node = 0;
uint8 nodeIndex;
/*******************************************************************************
* Function Name: nodeInit
********************************************************************************
* Summary:
*       This function initailzes the data node
*
* Parameters: 
*  uint8 index 
*
* Return:
*  null
*
*******************************************************************************/
void nodeInit(uint8 index){
    nodeTable[index].total = 0;
    nodeTable[index].fistByteAvgData = 0;
    nodeTable[index].secondByteAvgData = 0;
}
/*******************************************************************************
* Function Name: updateNodeTable
********************************************************************************
* Summary:
*        After receving temperature data from each node, this function will 
* collect sensor data and store it as a table 
*        The maximum value to store temperature data of 1 node is 800. If it's  
* full, we need to clear all data
*
* Parameters:
*  uint16 nodeAddr 
*  uint8 advdata 
*  uint8 firstByteTemp
*  uint8 secondByteTemp
*  uint16 timestamp
*
* Return:
*  TRUE when update success, and FALSE vice versa
*
*******************************************************************************/
uint8 updateNodeTable(uint16 nodeAddr, uint8 advdata, uint8 firstByteTemp, uint8 secondByteTemp, uint16 timestamp){
 
    if(number_of_node != 0){
        uint8 foundStoredNode = FALSE;
        /** check if the node has already stored */
        for(int i= 0;i<MAX_NODE;i++){
           if(nodeTable[i].nodeAddr == nodeAddr){
                /** if found node address, get the old index */
                nodeIndex = i;
                foundStoredNode = TRUE;
                break;
            }          
        }
        
        if(!foundStoredNode){
                /** update new node */
                nodeIndex = number_of_node;
                number_of_node++;
                nodeInit(nodeIndex);
        }
    }else{
        /** first node update */
        nodeIndex = 0;
        number_of_node++;
        nodeInit(nodeIndex);
    }
    
    /** check if it doesn't reach the max data */
    if(nodeTable[nodeIndex].total < MAX_TEMP_DATA){      
        nodeTable[nodeIndex].nodeAddr = nodeAddr;
        nodeTable[nodeIndex].adv = advdata;
        nodeTable[nodeIndex].fistByteTempData[nodeTable[nodeIndex].total] = firstByteTemp;
        nodeTable[nodeIndex].secondByteTempData[nodeTable[nodeIndex].total] = secondByteTemp;
        nodeTable[nodeIndex].timestamp[nodeTable[nodeIndex].total] = timestamp;
        calAverage(nodeTable[nodeIndex].fistByteAvgData,nodeTable[nodeIndex].secondByteAvgData);
        nodeTable[nodeIndex].total++;
        nodeTable[nodeIndex].adv = advdata;
        
        return TRUE;
    }else{
        /** node table reached the maximum data */
        return FALSE;
    }
}
/*******************************************************************************
* Function Name: calAverage
********************************************************************************
* Summary:
*       This function calculates the average temperature data
*
* Parameters: 
*  uint8 firstByteTemp, uint8 secondByteTemp
*
* Return:
*  null
*
*******************************************************************************/
void calAverage(uint8 firstByteTemp, uint8 secondByteTemp){
     /** if this ir the first data */
     if(nodeTable[nodeIndex].fistByteAvgData == 0 && nodeTable[nodeIndex].secondByteAvgData == 0){
        nodeTable[nodeIndex].fistByteAvgData = firstByteTemp;
        nodeTable[nodeIndex].secondByteAvgData = secondByteTemp;
     }
     else{
        float temp = Byte2Short(firstByteTemp,secondByteTemp)/100.0;
        float avgTemp = Byte2Short(firstByteTemp,secondByteTemp)/100.0;
        float sum = (avgTemp*nodeTable[nodeIndex].total) + temp;
        float newAvgTemp = sum/(float)(nodeTable[nodeIndex].total+1);       
        float2Bytes(newAvgTemp,&firstByteTemp,&secondByteTemp);
    }
}
/*******************************************************************************
* Function Name: float2Bytes
********************************************************************************
* Summary:
*       This function converts float value to byte value
*
* Parameters: 
*  float value, uint8* fistByte, uint8* secondByte
*
* Return:
*  null
*
*******************************************************************************/
void float2Bytes(float value, uint8* fistByte, uint8* secondByte){
 
  union {
    float a;
    uint8 bytes[2];
  } thing;
  thing.a = value;
  *fistByte = thing.bytes[0];
  *secondByte = thing.bytes[1];

    #if (DEBUG_ENABLED == 1)
                UART_UartPutString("*************** convert test *************** ");
                char* varBefore = ftoa(value);
                UART_UartPutString("float value: ");
                UART_UartPutString(varBefore);
                UART_UartPutCRLF(' ');
				UART_UartPutString("convert to Byte: ");
				PrintHex(*fistByte);
                PrintHex(*secondByte);
				UART_UartPutCRLF(' ');
                float tempAf = Byte2Short(*fistByte,*secondByte)/100.0;
                char* varAfter = ftoa(tempAf);
                UART_UartPutString("float after convert value: ");
                UART_UartPutString(varAfter);
                UART_UartPutCRLF(' ');

	#endif
}
/*******************************************************************************
* Function Name: getNodeFromTable
********************************************************************************
* Summary:
*       This function will return node data that stored in a node table
*
* Parameters: 
*  uint16 nodeAddr
*
* Return:
*  NODE_TABLE_T node
*
*******************************************************************************/
uint8 getNodeFromTable(uint16 nodeAddr,NODE_TABLE_T* node){
    uint8 found = FALSE;
    for(int i= 0;i<MAX_NODE;i++){
           if(nodeTable[i].nodeAddr == nodeAddr){
                /** if found node address, get the old index */
                *node = nodeTable[i];
                found = TRUE;
                break;
            }          
        }   
    
    return found;
}