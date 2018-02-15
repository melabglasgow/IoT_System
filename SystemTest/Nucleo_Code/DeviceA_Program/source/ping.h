/*
 * Copyright (c) 2015, 2016 ARM Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
 /* Credits
  * This application uses Open Source components.
  * Created by Sergio Martin (Github: @smysergio)
  * Inspired on Project: https://github.com/ARMmbed/mbed-os-example-ble
  * License: Apache-2.0
  */

#ifndef __PING_H__
#define __PING_H__

#include <string>

#define MAX_BW	20

//extern Serial pc;
extern osThreadId mainThread;

class PING {

public:
    const static uint16_t PING_SERVICE_UUID              	= 0xA000;
	const static uint16_t PING_OUT_CHARACTERISTIC_UUID		= 0xA001;
	const static uint16_t PING_IN_CHARACTERISTIC_UUID		= 0xA002;
	const static uint16_t PING_START_CHARACTERISTIC_UUID	= 0xA003;
	//const static uint16_t READ_CHARACTERISTIC_UUID		= 0xA011;
	//const static uint16_t WRITE_CHARACTERISTIC_UUID		= 0xA012;
	//const static uint16_t READ_WRITE_CHARACTERISTIC_UUID	= 0xA013;
	   
	
	PING(BLEDevice &_ble) : 
	
		count(0), ping_time(0), ping_enable(false), ping_flag(true), max_bw(true),
		
		ble(_ble), inInitValue(1), outInitValue(2), startInitValue(3),
		
		ping_out(	PING_OUT_CHARACTERISTIC_UUID,	&outInitValue	, sizeof(uint8_t), MAX_BW,	GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY	| GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ),
		ping_in(	PING_IN_CHARACTERISTIC_UUID,	&inInitValue	, sizeof(uint8_t), MAX_BW,	GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE ),
		start(		PING_START_CHARACTERISTIC_UUID,	&startInitValue	, sizeof(uint8_t), 1, 	GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE )
	
	{
		printf("\r[PING:constructor]: ");
		//**************************** ble service init ****************************//
		
        GattCharacteristic *charTable[] = {&ping_in,&ping_out,&start};
		
        GattService pingService(0xA000, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));
        ble.addService(pingService);
		ble.gattServer().onDataWritten(this, &PING::onDataWritten);
		//****************************************************************************//
		
		ping_timer.start();
		
		for(int i = 0; i < MAX_BW; i++) {
			buf_mbw[i] = 48 + i%10;
		}
		printf("\n\r");
    }

public:
	void batch_time(){
		printf("[PING::batch_time]: ");
		printf("time_batch (%s): ",time_batch.c_str());	
	}
	
	void store_time(){
		printf("\n\r\t\t[PING::store_time]: ");
		ping_flag = true;
		char buffer[5];
		sprintf(buffer,"%i",ping_time);
		string str(buffer);
		time_batch = time_batch + str + ",";
		printf("time_batch size %i ",time_batch.length());
		printf("ping_flag %i",ping_flag);
		//printf("time_batch (size %i) :(%s) ",time_batch.length(),time_batch.c_str());
		//osSignalSet(mainThread, 0x1);
	}
		
	void send() {
		printf("\r[PING:send]: ");
		count++;
		printf("(%i) ",count);
		ping_timer.reset();
		ping_flag = false;
		
		ble_error_t flag = BLE_ERROR_NONE;
		
		
		if(max_bw){
			buf_mbw[0] = 48 + count%10;
			printf("(max BW) ");
			flag = ble.gattServer().write(ping_out.getValueHandle(), (uint8_t *)&buf_mbw,MAX_BW);
		}else {
			uint8_t value = 48 + count%10;
			printf("(min BW) ");
			flag = ble.gattServer().write(ping_out.getValueHandle(), (uint8_t *)&value, sizeof(value));
		}

		
		if (BLE_ERROR_NONE == flag){
			printf("ble_error_t (OK) ");
		}
		else {
			printf("ble_error_t (ERROR:%i) ");
		}
		//Thread::yield();
		//ble.waitForEvent();
		printf("(%i)",count);
		printf("\n\r");
    }
	
	void onDataWritten(const GattWriteCallbackParams *params) {
		printf("\r[PING:onDataWritten]: ");
		printf("param[%i] = { ",params->len);
		for(int i = 0; (i < params->len); i++) {
			printf("%i, ",params->data[i]);
		}
		printf("}");
		
		if (params->handle == ping_in.getValueAttribute().getHandle()) {
			printf("\n\r[recv]: ");
			printf("(%i) ",params->data[0]);
			ping_time = ping_timer.read_ms();
			printf("ping time: %i",ping_time);
			store_time(); //ping_flag = true;
			if(params->data[0] != count)
				printf("\n\rERROR (Mismatch)",ping_time);
        }
		
		if (params->handle == start.getValueAttribute().getHandle()) {
			printf("\n\r[start_handle]: ");
			if( (params->data[0] == 1) || (params->data[0] == 49) ) {
				ping_enable = true;
				printf("START!");
			}
			else if( (params->data[0] == 0) || (params->data[0] == 48) ) {
				ping_enable = false;
				printf("STOP!");
			}
			else {
				printf("wrong command message");
			}
        }
		
		printf("\n\r");
    }
	
	int read_timer(void){
		return ping_timer.read_ms();
	}
	
public:
	bool ping_enable;
	bool ping_flag;
	uint8_t count;
	bool max_bw;
	
private:
	Timer ping_timer;
	string time_batch;
	uint16_t ping_time;
	uint8_t buf_mbw[MAX_BW];
	
	BLEDevice			&ble;
	
	uint8_t             inInitValue;
	uint8_t             outInitValue;
	uint8_t				startInitValue;
	
    GattCharacteristic	ping_out;
    GattCharacteristic	ping_in;
	GattCharacteristic	start;
	
};

#endif /* #ifndef __PING_H__ */
