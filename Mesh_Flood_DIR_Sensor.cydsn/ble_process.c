/*******************************************************************************
* File Name: ble_process.c
*
* Version: 1.0
*
* Description:
* This file contains the definiton for BLE related functions, such as event handler,
* GAP role switching, central scanning and connection to peripherals.
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <main.h>

uint8 RGBData[4];
uint8 tempData[4];
uint8 recvTempData[4];
uint8 dataADVCounter = 1;
uint8 switch_Role = FALSE;
uint8 scan_for_titan = FALSE;
extern uint8 time_to_get_sensordata;
uint16 node_address = 0;
extern uint16 sensorReceiveStartedTime;

#ifdef ENABLE_ADV_DATA_COUNTER
CYBLE_GAPP_DISC_DATA_T  new_advData;
uint8 potential_node_found = 0;
uint8 potential_node_bdAddr[6];
uint8 potential_node_bdAddrType = 0;
#endif

/* DO NOT CHANGE: This tag allows system to filter the correct nodes and resemble to 
* value set in BLE component Scan Response Packet settings */
uint8 scan_tag[SCAN_TAG_DATA_LEN] = {0x13, 0x21, 0x31, 0x01, 0x9B, 0x5F, 0x80, 0x00,
									0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xBB, 0xC2, 
									0x03, 0x00, 0xBB, 0xCB};

CYBLE_GAP_BD_ADDR_T				peripAddr;
volatile uint8 clientConnectToDevice = FALSE;
uint8 ble_gap_state = BLE_PERIPHERAL;

uint8 deviceConnected = FALSE;
uint8 restartScanning = FALSE;
volatile uint16 centralStartedTime = 0;
char val[31];
uint8 bt_addr[6] = {0x04,0x27,0xA8,0x1B,0x00,0xC4};
//uint8 bt_addr[6] = {0x1C,0x87,0x6A,0x4A,0x11,0xFA};
ENV_SENSOR_T                    result;
#ifdef RESTART_BLE_STACK
extern uint8 stackRestartIssued;
#endif
/*******************************************************************************
* Function Name: GenericEventHandler
********************************************************************************
* Summary:
*        Event handler function for the BLE stack. All the events by BLE stack
* are received by application through this function. For this, CyBle_ProcessEvents()
* should be called continuously in main loop and wherever required.
*
* Parameters:
*  event: 		event value
*  eventParame: pointer to the location where relevant event data is stored
*
* Return:
*  void
*
*******************************************************************************/
void GenericEventHandler(uint32 event, void * eventParam)
{
	/* Local variables and data structures*/
	CYBLE_GATTS_WRITE_REQ_PARAM_T 		writeReqData;
	CYBLE_GATTC_WRITE_REQ_T				writeADVcounterdata;
	CYBLE_GAPC_ADV_REPORT_T				scan_report;
	CYBLE_GATTS_WRITE_CMD_REQ_PARAM_T	writeCmdData;
	CYBLE_API_RESULT_T					apiResult;
	CYBLE_GATTC_WRITE_REQ_T 			writeRGBdata;
    CYBLE_GATTC_READ_BLOB_REQ_T         readBlobdata;
	static uint16 						received_node_addr ;
	static uint8 						received_node_addr_type ;

	
	#if (DEBUG_ENABLED == 1)
	CYBLE_GAP_BD_ADDR_T					gap_bdaddr;
	#endif
    	
	switch(event)
	{
		case CYBLE_EVT_STACK_ON:
			#if (DEBUG_ENABLED == 1)
			UART_UartPutString("CYBLE_EVT_STACK_ON ");
			UART_UartPutCRLF(' ');
			#endif
			
			#ifdef RESTART_BLE_STACK
			if(!stackRestartIssued)
			{
			#endif
				/* At the start of the BLE stack, start advertisement */
				apiResult = CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
				
				if(CYBLE_ERROR_OK == apiResult)
				{
					/* Set the BLE gap state flag */
					ble_gap_state = BLE_PERIPHERAL;
				}
			#ifdef RESTART_BLE_STACK
				
			}
			else
			{
				apiResult = CyBle_GapcStartScan(CYBLE_SCANNING_FAST);
					
				if(CYBLE_ERROR_OK == apiResult)
				{
					#if (DEBUG_ENABLED == 1)
					UART_UartPutString("Start scan after Stack RESET ");
					UART_UartPutCRLF(' ');
					#endif
					
					/* Record the time at which Central role was started. This will be 
					* used for timeout and switching to Peripheral operation*/
					centralStartedTime = WatchDog_CurrentCount();
					
					/* Update the current BLE role to Central */
					ble_gap_state = BLE_CENTRAL;
				}
			}
			
			/*Reset stackRestartIssued flag */
			stackRestartIssued = FALSE;
			#endif
		break;
			
		case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
			/* This event is received at every start or stop of peripheral advertisement*/
			#if (DEBUG_ENABLED == 1)
			UART_UartPutString("CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP :");
			UART_UartPutCRLF(' ');
			#endif
			
			if((CYBLE_STATE_DISCONNECTED == CyBle_GetState()) && (switch_Role == FALSE))
			{
				/* If the current state of the BLE is Disconnected, then restart advertisement.
				* Note that the advertisement should only be restarted if the switch flag is not
				* TRUE. If switch role flag is TRUE, then there is no need to start advertisement
				* as the GAP role has to be switched*/
				apiResult = CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);	
				
				if(apiResult == CYBLE_ERROR_OK)
				{
					#if (DEBUG_ENABLED == 1)
					UART_UartPutString("Restart Advertisement ");
					SendBLEStatetoUART(CyBle_GetState());
					UART_UartPutCRLF(' ');
					#endif
				}
			}
		break;
            
        case CYBLE_EVT_GATTC_READ_BLOB_RSP:
            //CyBle_ReadByTypeEventHandler((CYBLE_GATTC_READ_BY_TYPE_RSP_PARAM_T *)eventParam);
            UART_UartPutString("CYBLE_EVT_GATTC_READ_BLOB_RSP :");
            break;
			
		case CYBLE_EVT_GAPC_SCAN_START_STOP:
			/* This event is received at every start or stop of central scanning*/
			#if (DEBUG_ENABLED == 1)
			UART_UartPutString("CYBLE_EVT_GAPC_SCAN_START_STOP :");
			PrintHex(*(uint8*)eventParam);
			SendBLEStatetoUART(CyBle_GetState());
			UART_UartPutCRLF(' ');
			#endif
		break;
			
		case CYBLE_EVT_GATT_CONNECT_IND:
			/* This event is received at GATT connection with a device. This event
			* is received for both Client or Server role */
			#if (DEBUG_ENABLED == 1)
			UART_UartPutString("122 ");
			UART_UartPutCRLF(' ');
            /*
            writeReqData = *(CYBLE_GATTS_WRITE_REQ_PARAM_T*)eventParam;
                        int index = 0;
                        UART_UartPutString("writeReqData: ");
                        for(int i=0;i<writeReqData.handleValPair.value.len;i++){
                            char val = writeReqData.handleValPair.value.val[i];
                            PrintHex(val);
                        }
            */
			#endif
		break;
		
		case CYBLE_EVT_GATT_DISCONNECT_IND:
			/* This event is received at GATT disconnection with a device. This event
			* is received for both Client or Server role */
			#if (DEBUG_ENABLED == 1)
			UART_UartPutString("CYBLE_EVT_GATT_DISCONNECT_IND ");
			UART_UartPutCRLF(' ');
			#endif
		break;
			
		case CYBLE_EVT_GATTS_WRITE_REQ:
			/* This event is received at when Server receives a Write request from */
			
			/* Save the associated event parameter in local variable */
            writeReqData = *(CYBLE_GATTS_WRITE_REQ_PARAM_T*)eventParam;
            
            /* connected Client device */
            #if (DEBUG_ENABLED == 1)
                        UART_UartPutString("CYBLE_EVT_GATTS_WRITE_REQ ");  
                        PrintHex(writeReqData.handleValPair.attrHandle);
                        
                        UART_UartPutCRLF(' ');
			#endif
			if(writeReqData.handleValPair.attrHandle == CYBLE_NODE_ADDRESS_CHAR_HANDLE)
			{
				/* If the Write request is on Node Address Characteristic, then Client is 
				* trying to set a node address to this device */
				if(writeReqData.handleValPair.value.len == NODE_ADDR_DATA_LEN)
				{
					#if (DEBUG_ENABLED == 1)
					UART_UartPutString("Node Address Written: ");
					#endif
					/* Extract the node address from the event parameter */
					node_address = (uint16)(((uint16)writeReqData.handleValPair.value.val[NODE_ADDR_MSB_VAL_INDEX] << 8)
													| writeReqData.handleValPair.value.val[NODE_ADDR_LSB_VAL_INDEX]);
					
					if(node_address == 0x0000 || node_address == 0xFFFF)
					{
						/* A node address with value 0x0000 or 0xFFFF is invalid. Sent the 
						* appropriate error code to the Client device*/
						SendErrorCode(CYBLE_GATT_WRITE_REQ, 
										writeReqData.handleValPair.attrHandle, 
										ERR_INVALID_PDU);
						
						#if (DEBUG_ENABLED == 1)
							UART_UartPutString("Error ERR_INVALID_PDU ");
						#endif
						/* Reset Node Address and return */
						node_address = 0x0000;
						
						return;
					}
					else
					{
						/* If node address is valid, then save the value */
						#ifdef STORE_SFLASH_NODE_ADDRESS
							/* Save the set node address to user SFLASH to allow it to be
							* permanently set. This will still exist even on device reset. */
							UpdateSFlashNodeAddress(node_address);
						#endif
						
						/* Update the Node address Characteristic GATT DB value so that Client 
						* can read the current set node address */
						CyBle_GattsWriteAttributeValue(&writeReqData.handleValPair,
														0,
														&cyBle_connHandle,
														CYBLE_GATT_DB_LOCALLY_INITIATED);
						
						#if (DEBUG_ENABLED == 1)
							UART_UartPutCRLF(' ');
						#endif
						
						/* Reset the name of the device to 'Titan'. This indicates to the Central
						* device that the device has valie node address set */
						new_advData.advData[ADV_DATA_NAME_START_INDEX] = 'T';
						new_advData.advData[ADV_DATA_NAME_START_INDEX+1] = 'i';
						new_advData.advData[ADV_DATA_NAME_START_INDEX+2] = 't';
						new_advData.advData[ADV_DATA_NAME_START_INDEX+3] = 'a';
						new_advData.advData[ADV_DATA_NAME_START_INDEX+4] = 'n';
					}
				}
				else	/* if(writeReqData.handleValPair.value.len == NODE_ADDR_DATA_LEN) */
				{
						/* If the length of the data sent from the Client does not match the
						* expected data length on Node Address characteristic, then send a 
						* Error code indicating invalid length */
						SendErrorCode(CYBLE_GATT_WRITE_REQ, 
										writeReqData.handleValPair.attrHandle, 
										ERR_INVALID_ATT_LEN);
						node_address = 0x0000;
						UART_UartPutString("node_address = 0 ");
                        UART_UartPutCRLF(' ');
						/* Do not proceed further */
						return;
				}
			}
		if(writeReqData.handleValPair.attrHandle == CYBLE_TEMP_DATA_CONTROL_CHAR_HANDLE)
			{
				/* If the Write request is on RGB LED Control Characteristic, then Client is 
				* trying to set a new color to the device. */
				/* Proceed with color setting only if the node address has been set for the 
				* device. Else, send error code */
				if(node_address != 0)
				{
					/* If the data length of the received data meets the expected data 
					* length, then proceeed. Else send error code */
                    
					if(writeReqData.handleValPair.value.len == TEMP_DATA_LEN)
					{
						#if (DEBUG_ENABLED == 1)
							UART_UartPutString("TEMP CYBLE_EVT_GATTS_WRITE_REQ ");
							UART_UartPutCRLF(' ');
						#endif
						
						/* Extract the node address and its type sent as part of the RGB LED control data.  
						* The first byte is node address type (piconet addressing or not).
						* The third and fourth bytes of the received data has the desired node address */
						received_node_addr_type = writeReqData.handleValPair.value.val[DATA_NODE_ADDR_TYPE_INDEX];
						received_node_addr = (uint16)(((uint16)writeReqData.handleValPair.value.val[DATA_NODE_ADDR_MSB_INDEX] << 8)
													| writeReqData.handleValPair.value.val[DATA_NODE_ADDR_LSB_INDEX]);
						
						/* Next four bytes contain the color value. Store the temperature Value */
                        
						recvTempData[TEMP_FIRST_INDEX] = writeReqData.handleValPair.value.val[TEMP_FIRST_INDEX];
						recvTempData[TEMP_SECOND_INDEX] = writeReqData.handleValPair.value.val[TEMP_SECOND_INDEX];
						recvTempData[TEMP_THIRD_INDEX] = writeReqData.handleValPair.value.val[TEMP_THIRD_INDEX];
					    recvTempData[TEMP_FORTH_INDEX] = writeReqData.handleValPair.value.val[TEMP_FORTH_INDEX];
                        
                        UART_UartPutString("tempData: ");
                        for(int i=0;i<writeReqData.handleValPair.value.len;i++)
                            PrintHex(writeReqData.handleValPair.value.val[i]);
						UART_UartPutCRLF(' ');
                        
                        char* temperture = ftoa(Byte2Short(recvTempData[TEMP_FIRST_INDEX],recvTempData[TEMP_SECOND_INDEX])/100.0);
                        UART_UartPutString("receiving temperature: ");
                        UART_UartPutString(temperture);
                        UART_UartPutCRLF(' ');
						
						if(writeReqData.handleValPair.value.val[DATA_NODE_ADDR_TYPE_INDEX] == NODE_ADDR_TYPE_PICONET)
						{
							/* If the Address type in received packet is of Piconet type, then update data
							* for all nodes in a Piconet. Nodes in Piconet have the same MSB in address */
							if((received_node_addr & PICONET_SUBMASK) == (node_address & PICONET_SUBMASK))
							{
								
								/* Update the tempertaure data characteristic in GATT DB  to allow
								* Client to read the latest RGB LED color value set */
								CyBle_GattsWriteAttributeValue(&writeReqData.handleValPair,0,&cyBle_connHandle,CYBLE_GATT_DB_LOCALLY_INITIATED);
								
								#if (DEBUG_ENABLED == 1)
                                   
									UART_UartPutString("UpdateTempData ");
									SendBLEStatetoUART(CyBle_GetState());
									UART_UartPutCRLF(' ');
								#endif
							}
						}
						else
						{
							/* If the Address type in received packet is NOT of Piconet type, then update data
							* for particular node or all nodes, depending on address. */
							if((received_node_addr == node_address) ||
								(received_node_addr == BROADCAST_ADDR))
							{
				
								/* Update the tempertaure data characteristic in GATT DB  to allow
								* Client to read the latest RGB LED color value set */
								CyBle_GattsWriteAttributeValue(&writeReqData.handleValPair,0,&cyBle_connHandle,CYBLE_GATT_DB_LOCALLY_INITIATED);
								
							}
						}
						
						#ifdef ENABLE_ADV_DATA_COUNTER
						/* Increment the ADV data counter so that scanning Central device knows
						* if this device has updated RGB LED data or not */
						dataADVCounter++;
						#endif
						
						#if (DEBUG_ENABLED == 1)
						UART_UartPutString("incremented dataADVCounter value in CYBLE_EVT_GATTS_WRITE_REQ= ");
						PrintNum(dataADVCounter);
						UART_UartPutCRLF(' ');
                        
						#endif
						
						/* After receiveing the color value, set the switch role flag to allow the system
						* to switch role to Central role */
						switch_Role = TRUE;
						
						#if (DEBUG_ENABLED == 1)
						UART_UartPutString("switchRole to Central");
						UART_UartPutCRLF(' ');
						#endif
					}
					else	/* if(writeReqData.handleValPair.value.len == RGB_LED_DATA_LEN) */
					{
						/* Send the error code for invalid attribute length packet */
						SendErrorCode(CYBLE_GATT_WRITE_REQ, 
										writeReqData.handleValPair.attrHandle, 
										ERR_INVALID_ATT_LEN);
						return;
					}	/* if(writeReqData.handleValPair.value.len == RGB_LED_DATA_LEN) */
				}
				else	/* if(node_address != 0) */
				{
						/* As the node has not been assigned any address yet, send a error response to RGB
						* LED data Write request*/
						SendErrorCode(CYBLE_GATT_WRITE_REQ, 
										writeReqData.handleValPair.attrHandle, 
										ERR_INSUFF_RESOURCE);
						return;
				}
			}
			
			/* As part of every write request, the server needs to send a write response. Note
			* that this will be sent only if all the application layer conditions are met on a 
			* write request. Else, an appropriate error code is sent. */
	        CyBle_GattsWriteRsp(cyBle_connHandle);
		break;
			
		case CYBLE_EVT_GATTS_WRITE_CMD_REQ:
			/* This event is generated whenever a Client device sends a Write Command (Write 
			* without response) to a connected Server. Save the associated event parameter in
			* local variable. */
			writeCmdData = *(CYBLE_GATTS_WRITE_CMD_REQ_PARAM_T*)eventParam;
			
			/* Entertain the Write command only when the node address has been set to a valid 
			* value. Else, ignore the request. */
			if(node_address != 0)
			{
				/* Check if the Write command is for ADV Data counter characteristic */
				if(writeCmdData.handleValPair.attrHandle == CYBLE_RGB_DATA_COUNT_CHAR_HANDLE)
				{
					/* If the data sent is of one byte, then proceed. */
					if(writeCmdData.handleValPair.value.len == 1)
					{
						/* Extract and save the set ADV data counter value */
						dataADVCounter = *(writeCmdData.handleValPair.value.val);
						
						/* This increment is done to balance the ++ done as part of CYBLE_EVT_GATTS_WRITE_REQ */
						dataADVCounter--;
						
						/* Update the ADV data counter characteristic in GATT DB  to allow
						* Client to read the latest ADV data counter value */
						CyBle_GattsWriteAttributeValue(&writeCmdData.handleValPair,
														0,
														&cyBle_connHandle,
														CYBLE_GATT_DB_LOCALLY_INITIATED);
						
						#if (DEBUG_ENABLED == 1)
						UART_UartPutString("dataADVCounter from CYBLE_EVT_GATTS_WRITE_CMD_REQ = ");
						PrintNum(dataADVCounter);
						UART_UartPutCRLF(' ');
						#endif
					} /* if(writeCmdData.handleValPair.value.len == 1) */
                   
				}
			}	/* if(node_address != 0) */
			break;
		
		case CYBLE_EVT_GAPC_SCAN_PROGRESS_RESULT:
			/* This event is generated whenever there is a peripheral device found by 
			* while scanning */
			if(CYBLE_STATE_CONNECTED != CyBle_GetState())	
			{
				/* If we are not connected to any peripheral device, then save the new device  
				* information so to add it to our list */
				scan_report = *(CYBLE_GAPC_ADV_REPORT_T*)eventParam;
				
				#if (DEBUG_ENABLED == 1)
                    
					//UART_UartPutString("CYBLE_EVT_GAPC_SCAN_PROGRESS_RESULT ");
                    //UART_UartPutCRLF(' ');
                    //UART_UartPutCRLF(' ');
                    
				#endif
				
				#ifdef ENABLE_ADV_DATA_COUNTER
				/* If ADV DATA COUNTER is enabled, then the central device would check
				* if the counter in ADV packet of peripheral is less than its own counter
				* or not. If yes, then it will consider the peripheral has a older RGB LED
				* data and treat it as a potential device to connect to.*/
				if(scan_report.eventType == CYBLE_GAPC_CONN_UNDIRECTED_ADV)
				{                          
					/* If the scan report received is of advertising nature and the data 
					* length is as expected... */
                    
					if(scan_report.dataLen == new_advData.advDataLen)
					{
						/* If the second last value of the advertising data matches the custom 
						* marker, then the peripheral is a node of the network */
						if(scan_report.data[scan_report.dataLen-2] == CUSTOM_ADV_DATA_MARKER)
						{
							/* If the ADV counter data in Advertising data is less than that of
							* the value in this scanning device, then the node is a potential node 
							* whose color has to be updated. */
							if((scan_report.data[scan_report.dataLen-1] < dataADVCounter) ||
							((scan_report.data[scan_report.dataLen-1] == 255) && (dataADVCounter == 0)))
							{
								/* Potential node found*/
								potential_node_found = TRUE;
								/* Save the advertising peripheral address and type*/
								memcpy(potential_node_bdAddr, scan_report.peerBdAddr, 6);
								potential_node_bdAddrType = scan_report.peerAddrType;
								
								#if (DEBUG_ENABLED == 1)
								UART_UartPutString("potential_node_found ");
								UART_UartPutCRLF(' ');
								#endif
							}
							else
							{
								/* If the ADV data counter is equal or more than the data counter
								* in this scanning device, then the node has latest RGB LED data
								* and does not need to be connected to. Reset the potential node 
								* address */
								potential_node_found = FALSE;
								
								potential_node_bdAddrType = 0;
								
								potential_node_bdAddr[0] = 0x00;
								potential_node_bdAddr[1] = 0x00;
								potential_node_bdAddr[2] = 0x00;
								potential_node_bdAddr[3] = 0x00;
								potential_node_bdAddr[4] = 0x00;
								potential_node_bdAddr[5] = 0x00;
							}
						}
                        
                    }
                    else{
                    int check = 0;                   
                    if(scan_for_titan == FALSE || time_to_get_sensordata){
                        for(uint8 i=0u;i<6u;i++){
                            if(scan_report.peerBdAddr[i] == bt_addr[i]){
                                check++;   
                            }
                        }
                        time_to_get_sensordata = FALSE;
                    }
                    if(check == 6){
                        /* Stop existing scan */
    					CyBle_GapcStopScan();
    					
    					#if (DEBUG_ENABLED == 1)
    					UART_UartPutString("Stop Scan called ");
    					UART_UartPutCRLF(' ');
                        UART_UartPutCRLF(' ');
               
                        UART_UartPutString("EVENType: ");
                        char eventType[1];
                        sprintf(eventType, "%d", scan_report.eventType);
                        UART_UartPutString(eventType);
                        UART_UartPutString("\r\n");
                        UART_UartPutString("RSSI: ");
                        char rssi[12];
                        sprintf(rssi, "%d", scan_report.rssi);
                        UART_UartPutString(rssi);
                        UART_UartPutString("\r\n");
                        UART_UartPutString("BLE Address: 0x");

                        PrintHex(scan_report.peerBdAddr[5u]);
    				    PrintHex(scan_report.peerBdAddr[4u]);
    				    PrintHex(scan_report.peerBdAddr[3u]);
                        PrintHex(scan_report.peerBdAddr[2u]);
                        PrintHex(scan_report.peerBdAddr[1u]);
                        PrintHex(scan_report.peerBdAddr[0u]);
                        
                        UART_UartPutCRLF(' ');
                        UART_UartPutCRLF(' ');
    					
                        UART_UartPutString("****************Device Found***************\r\n");

                        int index = 0;

                        for(int i=0;i<scan_report.dataLen;i++){
                            val[i] = (uint8_t) scan_report.data[i];
                        }
              
                        UART_UartPutCRLF(' ');                     
                        
                        EnvSensorProcess(scan_report.rssi,scan_report.data);
                        result = GetSensorData();
                        char* temp = ftoa(result.temp);
                        UART_UartPutString("temperature: ");
                        UART_UartPutString(temp);
                        UART_UartPutCRLF(' ');
                        char li[1];
                        sprintf(li, "%hi", result.light);
                        UART_UartPutString("light: ");
                        PrintNum(result.light);
                        UART_UartPutCRLF(' ');
                        char* noise = ftoa(result.noise);
                        UART_UartPutString("noise: ");
                        UART_UartPutString(noise);
                        UART_UartPutCRLF(' ');
                        char* humi = ftoa(result.humi);
                        UART_UartPutString("humidity: ");
                        UART_UartPutString(humi);
                        UART_UartPutCRLF(' ');
                        char* uv = ftoa(result.uv);
                        UART_UartPutString("uv: ");
                        UART_UartPutString(uv);
                        UART_UartPutCRLF(' ');
                        char* press = ftoa(result.pressure);
                        UART_UartPutString("pressure: ");
                        UART_UartPutString(press);
                        UART_UartPutCRLF(' ');
                        char* di = ftoa(result.di);
                        UART_UartPutString("dimention: ");
                        UART_UartPutString(di);
                        UART_UartPutCRLF(' ');
                        char* heat = ftoa(result.heat);
                        UART_UartPutString("heat: ");
                        UART_UartPutString(heat);
                        UART_UartPutCRLF(' ');
                        char* battery = ftoa(result.battery);
                        UART_UartPutString("battery: ");
                        UART_UartPutString(battery);
                        UART_UartPutCRLF(' ');
                        char* distance = ftoa(result.distance);
                        UART_UartPutString("distance: ");
                        UART_UartPutString(distance);
                        UART_UartPutCRLF(' ');
                        UART_UartPutCRLF(' ');
                        
                       // if(result.flagActive == TRUE){
                        tempData[TEMP_FIRST_INDEX] = scan_report.data[9];
                        tempData[TEMP_SECOND_INDEX] = scan_report.data[8];
                        tempData[TEMP_THIRD_INDEX] = 0x00; //reserved
                        tempData[TEMP_FORTH_INDEX] = 0x00; //reserved
                       
                        UART_UartPutString("tempData: ");
                        for(int i=0;i<5;i++)
                            PrintHex(tempData[i]);
						UART_UartPutCRLF(' ');
                        
                        #ifdef ENABLE_ADV_DATA_COUNTER
                    	/* Increment the ADV data counter so that scanning Central device knows*/
                    	/* After receiveing the sensor data, set the switch role flag to allow the system
						* to switch role to Central role */
                        dataADVCounter++;
                    	#endif  
                        
                        #if (DEBUG_ENABLED == 1)
                        UART_UartPutString("timestamp: ");
                        PrintNum(sensorReceiveStartedTime);
						UART_UartPutCRLF(' ');
                        #endif
                       
                        restartScanning = TRUE;
				        scan_for_titan = TRUE;
        				#if (DEBUG_ENABLED == 1)
        				UART_UartPutString("Disconnect and restart scanning ");
        				SendBLEStatetoUART(CyBle_GetState());
        				UART_UartPutCRLF(' ');
                        #endif
                        
                        //result.flagActive = FALSE;
                       // }
                        #endif
                    }
                    //reset timer to receive sensor data every 1 min
                        sensorReceiveStartedTime = WatchDog_CurrentCount();
					}
				}
				#endif
				
				/* If the received scan data is part of scan response from a peripheral... */
				if(scan_report.eventType == CYBLE_GAPC_SCAN_RSP)
				{
					/* If the data lenght of the scan reponse packet is equal to expected
					* scan response data lenght...*/
					if(scan_report.dataLen == SCAN_TAG_DATA_LEN)
					{
						#ifdef ENABLE_ADV_DATA_COUNTER
						/* If a potential node had been found earlier as part of received 
						* advertising data, then compare the address of stored potential 
						* node and received address of the scan response */
						if(potential_node_found)
						{
							/* Compare the two addresses and type */
							if((!memcmp(scan_report.peerBdAddr, potential_node_bdAddr, 6))  
								&& (potential_node_bdAddrType == scan_report.peerAddrType))
							{
						#endif
								/* If the scan report data matches the expected data (scan_tag),
								* then it is our desired node */
								if(!memcmp(scan_report.data, scan_tag, scan_report.dataLen))
								{
									#if (DEBUG_ENABLED == 1)
									UART_UartPutString("*********** Titan Found ***********");
									UART_UartPutCRLF(' ');
									#endif
                              
                                    scan_for_titan = FALSE;
									/* Stop existing scan */
									CyBle_GapcStopScan();
									
									#if (DEBUG_ENABLED == 1)
									UART_UartPutString("Stop Scan called ");
									UART_UartPutCRLF(' ');
									#endif
									
									/* Save the peripheral BD address and type */
									peripAddr.type = scan_report.peerAddrType;
									peripAddr.bdAddr[0] = scan_report.peerBdAddr[0];
									peripAddr.bdAddr[1] = scan_report.peerBdAddr[1];
									peripAddr.bdAddr[2] = scan_report.peerBdAddr[2];
									peripAddr.bdAddr[3] = scan_report.peerBdAddr[3];
									peripAddr.bdAddr[4] = scan_report.peerBdAddr[4];
									peripAddr.bdAddr[5] = scan_report.peerBdAddr[5];
									
									/* Set the flag to allow application to connect to the
									* peripheral found */
									clientConnectToDevice = TRUE;
									
									#ifdef ENABLE_ADV_DATA_COUNTER
									/* Reset the potential node flag*/
									potential_node_found = FALSE;
									#endif
									
								}
						#ifdef ENABLE_ADV_DATA_COUNTER
							}
						}
						#endif
					}
				}
			}

		break;
			
		case CYBLE_EVT_GAP_DEVICE_CONNECTED:
			/* This event is received whenever the device connect on GAP layer */
			#if (DEBUG_ENABLED == 1)
				UART_UartPutString("CYBLE_EVT_GAP_DEVICE_CONNECTED: ");
				CyBle_GapGetPeerBdAddr(cyBle_connHandle.bdHandle, &gap_bdaddr);
				PrintHex(gap_bdaddr.bdAddr[0]);
				PrintHex(gap_bdaddr.bdAddr[1]);
				PrintHex(gap_bdaddr.bdAddr[2]);
				UART_UartPutCRLF(' ');
			#endif
			
			if(ble_gap_state == BLE_CENTRAL)
			{
				//uint8 rgb_data[RGB_LED_DATA_LEN];
				
				#ifdef ENABLE_CENTRAL_DISCOVERY
					/* The Device is connected now. Start Attributes discovery process.*/
					CyBle_GattcStartDiscovery(cyBle_connHandle);
					#if (DEBUG_ENABLED == 1)
						UART_UartPutString("CYBLE_EVT_GAP_DEVICE_CONNECTED ");
						SendBLEStatetoUART(CyBle_GetState());
						UART_UartPutCRLF(' ');                  
					#endif
				#else
					/* If this system is currently acting in Central role and has connected
					* to a peripheral device, then write directly the ADV counter data and
					* RGB LED control data using attribute handles */
					
					/* Set the device connected flag */
					deviceConnected = TRUE;
					#if (DEBUG_ENABLED == 1)
						UART_UartPutString("Directly write RGB using Attr handle ");
						SendBLEStatetoUART(CyBle_GetState());
						UART_UartPutCRLF(' ');
					#endif
					
					/* Write the Data Counter value */
					writeADVcounterdata.attrHandle = CYBLE_RGB_DATA_COUNT_CHAR_HANDLE;
					writeADVcounterdata.value.val = &dataADVCounter;
					writeADVcounterdata.value.len = 1;
					CyBle_GattcWriteWithoutResponse(cyBle_connHandle, &writeADVcounterdata);
				
					/* Write the RGB LED Data */
					rgb_data[DATA_NODE_ADDR_TYPE_INDEX] = 	received_node_addr_type;
					rgb_data[RESERVED_BYTE_INDEX] = 		RESERVED_BYTE_VALUE;
					rgb_data[DATA_NODE_ADDR_LSB_INDEX] = 	(uint8)(received_node_addr & 0x00FF);
					rgb_data[DATA_NODE_ADDR_MSB_INDEX] = 	(uint8)((received_node_addr & 0xFF00) >> 8);
					rgb_data[DATA_INDEX] = 					tempData[RGB_RED_INDEX];//RGBData[RGB_RED_INDEX];
					rgb_data[DATA_INDEX+1] = 				tempData[RGB_GREEN_INDEX];//RGBData[RGB_GREEN_INDEX];
					rgb_data[DATA_INDEX+2] = 				tempData[RGB_BLUE_INDEX];//RGBData[RGB_BLUE_INDEX];
					rgb_data[DATA_INDEX+3] = 				tempData[RGB_INTENSITY_INDEX];//RGBData[RGB_INTENSITY_INDEX];
					
					writeRGBdata.attrHandle = CYBLE_RGB_LED_CONTROL_CHAR_HANDLE;
					writeRGBdata.value.val = rgb_data;
					writeRGBdata.value.len = RGB_LED_DATA_LEN;
                    
                    for(int i=0;i<8;i++)
                            PrintHex(tempData[i]);
                    UART_UartPutCRLF(' ');        
                    char* temperture = ftoa(Byte2Short(tempData[RGB_RED_INDEX],tempData[RGB_GREEN_INDEX])/100.0);
                        UART_UartPutString("receive temperature: ");
                        UART_UartPutString(temperture);
                        UART_UartPutCRLF(' ');
					CyBle_GattcWriteCharacteristicDescriptors(cyBle_connHandle, &writeRGBdata);
                    //CyBle_GattcWriteWithoutResponse(cyBle_connHandle, &writeRGBdata);
				#endif
			}	
        break;
			
		case CYBLE_EVT_GATTC_DISCOVERY_COMPLETE:
			/* This event is generated whenever the discovery procedure is complete*/
			
			#ifdef ENABLE_CENTRAL_DISCOVERY
			deviceConnected = TRUE;
			#if (DEBUG_ENABLED == 1)
				UART_UartPutString("CYBLE_EVT_GATTC_DISCOVERY_COMPLET ");
				SendBLEStatetoUART(CyBle_GetState());
				UART_UartPutCRLF(' ');

			#endif
			
			/* Write the Data Counter value */
			writeADVcounterdata.attrHandle = CYBLE_RGB_DATA_COUNT_CHAR_HANDLE;
			writeADVcounterdata.value.val = &dataADVCounter;
			writeADVcounterdata.value.len = 1;
			CyBle_GattcWriteWithoutResponse(cyBle_connHandle, &writeADVcounterdata);
            
            char* tem = ftoa(Byte2Short(tempData[TEMP_FIRST_INDEX],tempData[TEMP_SECOND_INDEX])/100.0);
            UART_UartPutString("sending temperature: ");
            UART_UartPutString(tem);
            UART_UartPutCRLF(' ');
						
		
			/* Write the RGB LED Value */
			writeRGBdata.attrHandle = CYBLE_TEMP_DATA_CONTROL_CHAR_HANDLE;
			writeRGBdata.value.val = tempData;
			writeRGBdata.value.len = TEMP_DATA_LEN;
			CyBle_GattcWriteCharacteristicValue(cyBle_connHandle, &writeRGBdata);
            //CyBle_GattcWriteWithoutResponse(cyBle_connHandle, &writeRGBdata);
			#endif
            
		break;
			
		case CYBLE_EVT_GATTC_WRITE_RSP:
			/* This event is generated when the Client device receives a response
			* as part of the Write request sent earlier. This indicates that
			* the RGB LED data was written successfully */
			#if (DEBUG_ENABLED == 1)
				UART_UartPutString("CYBLE_EVT_GATTC_WRITE_RSP ");
				SendBLEStatetoUART(CyBle_GetState());
				UART_UartPutCRLF(' ');          
               
			#endif
			
			/* Disconnect the existing connection and restart scanning */
			if((cyBle_connHandle.bdHandle != 0))
			{
				CyBle_GapDisconnect(cyBle_connHandle.bdHandle);
				
				restartScanning = TRUE;
				
				#if (DEBUG_ENABLED == 1)
				UART_UartPutString("Disconnect and restart scanning ");
				SendBLEStatetoUART(CyBle_GetState());
				UART_UartPutCRLF(' ');
				#endif
			}
		break;
			
		case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
			/* This event is generated when the device disconnects from an 
			* existing connection */
			deviceConnected = FALSE;
			
			#if (DEBUG_ENABLED == 1)
				UART_UartPutString("CYBLE_EVT_GAP_DEVICE_DISCONNECTED ");
				SendBLEStatetoUART(CyBle_GetState());
				UART_UartPutCRLF(' ');
			#endif
			
			if((ble_gap_state == BLE_PERIPHERAL) && (switch_Role != TRUE))
			{
				/* If the current role of this system was Peripheral and the role
				* is not to be switched, then restart advertisement */
				if(CYBLE_STATE_DISCONNECTED == CyBle_GetState())
				{
					CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);	
					#if (DEBUG_ENABLED == 1)
					UART_UartPutString("Restart Advertisement ");
					SendBLEStatetoUART(CyBle_GetState());
					UART_UartPutCRLF(' ');
					#endif
				}
			}
			else if((ble_gap_state == BLE_CENTRAL) && (switch_Role != TRUE))
			{
				/* If the current role of this system was Central and the role
				* is not to be switched, then restart scanning */
				if(CYBLE_STATE_DISCONNECTED == CyBle_GetState())
				{
					CyBle_GapcStartScan(CYBLE_SCANNING_FAST);	
					#if (DEBUG_ENABLED == 1)
					UART_UartPutString("Restart Scanning ");
					SendBLEStatetoUART(CyBle_GetState());
					UART_UartPutCRLF(' ');
					#endif
				}	
			}
		break;
			
		default:
			eventParam = eventParam;
		break;
	}
}

/*******************************************************************************
* Function Name: SwitchRole
********************************************************************************
* Summary:
*        This function switches the role between Central and Peripheral. If device 
* is connected while switching role, then it is first disconnected.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void SwitchRole(void)
{
	CYBLE_API_RESULT_T  apiResult;
	
	/* if the switch role flag is set... */
	if(switch_Role == TRUE)
	{	
        UART_UartPutString("switch_Role");
		UART_UartPutCRLF(' ');
		/* Process pending BLE events */
		CyBle_ProcessEvents();
		
		/* If there is an existing connection, then disconnect before switching
		* role. */
		if((cyBle_connHandle.bdHandle != 0))
		{
			/* Disconnect the device and process the event */
			CyBle_GapDisconnect(cyBle_connHandle.bdHandle);
			CyBle_ProcessEvents();
			#if (DEBUG_ENABLED == 1)
			UART_UartPutString("Peripheral closed connection ");
			SendBLEStatetoUART(CyBle_GetState());
			UART_UartPutCRLF(' ');
			#endif
		}		
		
		switch(ble_gap_state)
		{
            
			case BLE_PERIPHERAL:
				/* If the current role is Peripheral and system is advertising,
				* then stop advertisement before switching role */
				if(CyBle_GetState() == CYBLE_STATE_ADVERTISING)
				{		
					CyBle_GappStopAdvertisement();
					CyBle_ProcessEvents();

					#if (DEBUG_ENABLED == 1)
						UART_UartPutString("Peripheral Advertisment Stopped ");
						SendBLEStatetoUART(CyBle_GetState());
						UART_UartPutCRLF(' ');
					#endif
				}
				
				if(CyBle_GetState() == CYBLE_STATE_DISCONNECTED)
				{
					/* Switch BLE role by starting scan. This way, the system is set
					* to Central role */
					apiResult = CyBle_GapcStartScan(CYBLE_SCANNING_FAST);
					
					if(CYBLE_ERROR_OK == apiResult)
					{
						#if (DEBUG_ENABLED == 1)
						UART_UartPutString("Start Scan API called ");
						SendBLEStatetoUART(CyBle_GetState());
						UART_UartPutCRLF(' ');
						#endif
						
						/* Record the time at which Central role was started. This will be 
						* used for timeout and switching to Peripheral operation*/
						centralStartedTime = WatchDog_CurrentCount();
						
						/* Update the current BLE role to Central */
						ble_gap_state = BLE_CENTRAL;
						
						/* Reset the switch role flag*/
						switch_Role = FALSE;
					}
					else
					{
						/* If scanning did not start, maintain the current role and retry later */
						ble_gap_state = BLE_PERIPHERAL;
						
						#if (DEBUG_ENABLED == 1)
						UART_UartPutString("Start Scan API failed ");
						SendBLEStatetoUART(CyBle_GetState());
						UART_UartPutCRLF(' ');
						#endif
					}
				}
				
				/* Process Pending BLE Events */
				CyBle_ProcessEvents();
				
			break;
			
			case BLE_CENTRAL:
				/* If the current role is Central and system is scanning,
				* then stop scanning before switching role */
				if(CyBle_GetState() == CYBLE_STATE_SCANNING)
				{
					CyBle_GapcStopScan();
					CyBle_ProcessEvents();
					
					#if (DEBUG_ENABLED == 1)
						UART_UartPutString("Central Scan stopped ");
						UART_UartPutCRLF(' ');
					#endif
				}
				
				if(CyBle_GetState() == CYBLE_STATE_DISCONNECTED)
				{
					#ifdef ENABLE_ADV_DATA_COUNTER
					/* Increment data counter */
					new_advData.advData[new_advData.advDataLen - 1] = dataADVCounter;
					
					cyBle_discoveryModeInfo.advData = &new_advData;
					
					#if (DEBUG_ENABLED == 1)
						UART_UartPutString("Updated ADV data = ");
						PrintNum(dataADVCounter);
						UART_UartPutCRLF(' ');
					#endif
					#endif
					
					/* Switch BLE role by starting advertisement. This way, the system is 
					* set to Peripheral role */
					apiResult = CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
					
					if(apiResult == CYBLE_ERROR_OK)
					{
						/* If advertisement started successfully, set the BLE state and
						* reset the switch role flag*/
						ble_gap_state = BLE_PERIPHERAL;
						clientConnectToDevice = FALSE;	
						switch_Role = FALSE;
						
						#if (DEBUG_ENABLED == 1)
						UART_UartPutString("Peripheral Advertisment called ");
						SendBLEStatetoUART(CyBle_GetState());
						UART_UartPutCRLF(' ');
						#endif
					}
					else
					{
						/* If advertisement did not start, maintain the current role and retry later */
						ble_gap_state = BLE_CENTRAL;
						
						#if (DEBUG_ENABLED == 1)
						UART_UartPutString("Start Peripheral Advertisment Failed ");
						SendBLEStatetoUART(CyBle_GetState());
						UART_UartPutCRLF(' ');
						#endif					
					}
				}
				
				/* Process Pending BLE Events */
				CyBle_ProcessEvents();
			break;
			
			default:
			
			break;
			
		}
	}
}

/*******************************************************************************
* Function Name: ConnectToPeripheralDevice
********************************************************************************
* Summary:
*        Connects to Peripheral device with given BD Address.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void ConnectToPeripheralDevice(void)
{
	CYBLE_API_RESULT_T  apiResult;
	
	/* If flag has been set to connect to a device... */
	if(clientConnectToDevice)
	{
		/* Process pending BLE events */
		CyBle_ProcessEvents();

		if(CYBLE_STATE_DISCONNECTED == CyBle_GetState())
		{
			/* Reset the flag to connect to a device*/
			clientConnectToDevice = FALSE;
			
			/* Connect to the device whose address has been saved as part of 
			* potential node search */
			apiResult = CyBle_GapcConnectDevice(&peripAddr);
			
			if(apiResult != CYBLE_ERROR_OK)
			{
				#if (DEBUG_ENABLED == 1)
					UART_UartPutString("Connect Request failed ");
					SendBLEStatetoUART(CyBle_GetState());
					UART_UartPutCRLF(' ');
				#endif
			}
			else
			{
				#if (DEBUG_ENABLED == 1)
					UART_UartPutString("Connect Request Sent: ");
					PrintHex(peripAddr.bdAddr[0]);
					PrintHex(peripAddr.bdAddr[1]);
					PrintHex(peripAddr.bdAddr[2]);
					SendBLEStatetoUART(CyBle_GetState());
					UART_UartPutCRLF(' ');
				#endif	
			}
			
			/* Process pending BLE events */
			CyBle_ProcessEvents();
		}
	}
}

/*******************************************************************************
* Function Name: RestartCentralScanning
********************************************************************************
* Summary:
*        Restarts Central scanning. Also, if the time that the device has remained
* in Central role exceeds pre-determined value, then the switch role flag is set.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void RestartCentralScanning(void)
{
	/* If the current role is Central and the Central time has exceeded the preset time,
	* then set the flag to switch role to Peripheral */
	if((BLE_CENTRAL == ble_gap_state) 
		&& (WatchDog_CurrentCount() - centralStartedTime > CENTRAL_STATE_SPAN) 
		&& ((CYBLE_STATE_DISCONNECTED == CyBle_GetState()) || (CYBLE_STATE_SCANNING == CyBle_GetState())))
	{
		/* Switch role flag set */
		switch_Role = TRUE;
		#if (DEBUG_ENABLED == 1)
		UART_UartPutString("switchRole from restart Scanning loop ");
		UART_UartPutCRLF(' ');
		#endif
		
		return;
	}
	
	/* If restart scanning flag is set, then restart the Central scanning */
	if(restartScanning)
	{
		/* Process pending BLE events */
		CyBle_ProcessEvents();
		
		/* Restart scanning only if role switch flag is not set, else reset the flag*/
		if(switch_Role !=  TRUE)		
		{
			if(CYBLE_STATE_DISCONNECTED == CyBle_GetState())
			{
				/* Reset the restart scanning flag */
				restartScanning = FALSE;

				/* Start Central scan and process the event */
				CyBle_ProcessEvents();
				
				#if (DEBUG_ENABLED == 1)
				UART_UartPutString("Start Scan from restart Scanning loop ");
				UART_UartPutCRLF(' ');
				#endif
							
				CyBle_GapcStartScan(CYBLE_SCANNING_FAST);
				CyBle_ProcessEvents();
			}
		}
		else
		{
			/* Reset the restart scanning flag */
			restartScanning = FALSE;
		}
	}
}	

/*******************************************************************************
* Function Name: SendErrorCode
********************************************************************************
* Summary:
*        Send Error code at GATT Layer
*
* Parameters:
*  uint8 opCode, 
*  CYBLE_GATT_DB_ATTR_HANDLE_T  attrHandle, 
*  CYBLE_GATT_ERR_CODE_T errorCode
*
* Return:
*  void
*
*******************************************************************************/
void SendErrorCode(uint8 opCode, CYBLE_GATT_DB_ATTR_HANDLE_T  attrHandle, CYBLE_GATT_ERR_CODE_T errorCode)
{
	CYBLE_GATTS_ERR_PARAM_T err_param;
		            
    err_param.opcode = opCode;
    err_param.attrHandle = attrHandle;
    err_param.errorCode = errorCode;

    /* Send Error Response */
    (void)CyBle_GattsErrorRsp(cyBle_connHandle, &err_param);
}
/* [] END OF FILE */
