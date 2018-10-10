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

#include <main.h>
ENV_SENSOR_T sensor;
#define BEACON_MEASURED_POWER  -59  //BEACON Measured power (RSSI at 1m distance)
/*******************************************************************************
* Function Name: EnvSensorProcess
********************************************************************************
* Summary:
*        Get Omron sensor data from BLE, then interpret a paylaod as enironment
* sensor data 
* For this project, we get only temperature data from Omron sensor and transfer  
* temperature data from its node to the mesh network
*
* Parameters:
*  uint8* data (Length: 31)
*
* Return:
*  void
*
*******************************************************************************/
void EnvSensorProcess(int8 rssi ,uint8* data){       
     sensor.seqNum = data[7];
     sensor.temp = Byte2Short(data[9],data[8])/100.0;
     sensor.humi = Byte2UShort(data[10],data[11])/100.0;
     sensor.light = Byte2UShort(data[13],data[12]);
     sensor.uv = Byte2UShort(data[15],data[14])/100.0;
     sensor.pressure = Byte2UShort(data[17],data[16])/10.0;
     sensor.noise = Byte2UShort(data[19],data[18])/100.0;
     sensor.battery = (data[26]+100)* 10.0;
     sensor.ax = 0.0;
     sensor.ay = 0.0;
     sensor.az = 0.0;
     sensor.di = Byte2Short(data[21],data[20])/100.0;
     sensor.heat = Byte2Short(data[23],data[22])/100.0;
     sensor.rssi = rssi;
     sensor.distance = ReturnAccuracy(sensor.rssi);
     sensor.flagActive = TRUE;
}
/*******************************************************************************
* Function Name: ReturnAccuracy
********************************************************************************
* Summary:
*        This function will calculate the distance between CypressBoard and 
* Omron sensor with rssi and BLE measured power
*
* Parameters:
*  void
*
* Return:
*  short
*
*******************************************************************************/
float ReturnAccuracy(int8 rssi){
        int8 RSSI = abs(rssi);
        if (RSSI == 0)
            return -1;
        if (BEACON_MEASURED_POWER == 0)
            return -1;

        float ratio = RSSI * 1.0 / abs(BEACON_MEASURED_POWER);
        if (ratio < 1.0)
            return pow(ratio, 8.0);
        float accuracy = 0.69976 * pow(ratio, 7.7095) + 0.111;
        // accuracy = 0.89976 * pow(ratio, 7.7095) + 0.111

        return accuracy;
}
/*******************************************************************************
* Function Name: GetTemperature
********************************************************************************
* Summary:
*        When this function is called, it will transfer the temperature data that
* get from the Omron environment sensor
*
* Parameters:
*  void
*
* Return:
*  short
*
*******************************************************************************/
ENV_SENSOR_T GetSensorData(){
    return sensor;
}
/*******************************************************************************
* Function Name: Byte2Short
********************************************************************************
* Summary:
*        This function converts byte data to short data and return it
*
* Parameters:
*  uint8 high paylaod and uint8 low payload
*
* Return:
*  short
*
*******************************************************************************/
short Byte2Short(uint8 hi, uint8 lo){
    short short_val;
    short val = (hi << 8) | lo;
    if ((hi & 0b10000000) == 0b10000000){
       short val_inv = val ^ 0b1111111111111111;
        val_inv = val_inv + 1;
        short_val = val_inv * (-1);
    }
    else{
        short_val = val;
    }
    return short_val;
}
/*******************************************************************************
* Function Name: Byte2UShort
********************************************************************************
* Summary:
*        This function converts byte data to unsigned short data and return it
*
* Parameters:
*  uint8 high paylaod and uint8 low payload
*
* Return:
*  unsigned short
*
*******************************************************************************/
unsigned short Byte2UShort(uint8 hi, uint8 lo){
    unsigned short ushort_val  = (hi << 8) | lo;
    return ushort_val ;
}
