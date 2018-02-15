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

  
//************************ ble declarations ************************//
 
#include <events/mbed_events.h>
#include <mbed.h>
#include "ble/BLE.h"
#include "ping.h"
PING *ping;

//**************************************************************************//

#define VALUES_PER_MODE 50
#define DONE -1
#define TESTS_N 2
#define MIN_T 10

int period[2]	= {	10,	5000	};
bool bw[2]		= {	false,	false	};

osThreadId mainThread;

//**************************** DeviceLink globals ************************//

bool connected = false;

const static char     DEVICE_NAME[] = "PING";
static const uint16_t uuid16_list[] = {PING::PING_SERVICE_UUID};

Thread t;
static EventQueue queue(32 * EVENTS_EVENT_SIZE);

DigitalOut alivenessLED(LED1, 0);
void blinkCallback(void) { alivenessLED = !alivenessLED; }

//RtosTimer updater;
volatile bool update = false;
void update_handle(void const *n)	{ update = true; osSignalSet(mainThread, 0x1);}	//updates_global.release(); }
RtosTimer updater(&update_handle, osTimerPeriodic, (void *)0);

volatile bool pushed = false;
InterruptIn button(USER_BUTTON);
void button_handle(void)	{ pushed = true; osSignalSet(mainThread, 0x1);}

//**************************** DeviceLink functions ************************//
void connectionCallback(const Gap::ConnectionCallbackParams_t *params)
{
	printf("\r\n{connectionCallback}\r\n");
	connected = true;
}

void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *params)
{
	printf("\r\n{disconnectionCallback]}\r\n");
	connected = false;
    (void) params;
    BLE::Instance().gap().startAdvertising();
}

/**
 * This callback allows the PING to receive updates to the ledState Characteristic.
 *
 * @param[in] params
 *     Information about the characterisitc being updated.
 */
void onDataWrittenCallback(const GattWriteCallbackParams *params) {
	printf("\r\n{onDataWrittenCallback}\r\n");
}

/**
 * This function is called when the ble initialization process has failled
 */
void onBleInitError(BLE &ble, ble_error_t error)
{
	printf("\r\n{onBleInitError}\r\n");
    /* Initialization error handling should go here */
}

void printMacAddress()
{
	printf("\r\n{printMacAddress}\r\n");
    /* Print out device MAC address to the console*/
    Gap::AddressType_t addr_type;
    Gap::Address_t address;
    BLE::Instance().gap().getAddress(&addr_type, address);
    printf("DEVICE MAC ADDRESS: ");
    for (int i = 5; i >= 1; i--){
        printf("%02x:", address[i]);
    }
    printf("%02x\r\n", address[0]);
}

/**
 * Callback triggered when the ble initialization process has finished
 */
void bleInitComplete(BLE::InitializationCompleteCallbackContext *params)
{
	printf("\r\n[bleInitComplete]\r\n");
    BLE&        ble   = params->ble;
    ble_error_t error = params->error;

    if (error != BLE_ERROR_NONE) {
        /* In case of error, forward the error handling to onBleInitError */
        onBleInitError(ble, error);
        return;
    }

    /* Ensure that it is the default instance of BLE */
    if(ble.getInstanceID() != BLE::DEFAULT_INSTANCE) {
        return;
    }

    ble.gap().onDisconnection(disconnectionCallback);
	ble.gap().onConnection(connectionCallback);
    //ble.gattServer().onDataWritten(onDataWrittenCallback); // do it in the Service

    ping = new PING(ble);

    /* setup advertising */
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, (uint8_t *)uuid16_list, sizeof(uuid16_list));
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME));
    ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.gap().setAdvertisingInterval(1000); /* 1000ms. */
    ble.gap().startAdvertising();

    printMacAddress();
}

void scheduleBleEventsProcessing(BLE::OnEventsToProcessCallbackContext* context) {
	printf(" {sBEP} ");
	
	BLE &ble = BLE::Instance();
    queue.call(Callback<void()>(&ble, &BLE::processEvents));
}

int main() {
	
	printf("\r\n------ main ------\r\n"); 
	mainThread = osThreadGetId();
	
	//**************************** DeviceLink init ***********************//
	printf("\nStarting DeviceLink test\n");
	
    BLE &ble = BLE::Instance();
	
	ble.onEventsToProcess(scheduleBleEventsProcessing);
	ble.init(bleInitComplete);
	
	t.set_priority(osPriorityRealtime);
	t.start(callback(&queue, &EventQueue::dispatch_forever));
	queue.call_every(500, blinkCallback);
	
	printf("\r\nWaiting for Device to connect.");
	while(!connected){
		printf(".");
		wait(0.5);
	}
	
	//**************************** DeviceLink Initial Values ************************//
	
	ping->count = VALUES_PER_MODE;
	int mode = 0;
	
	updater.start(1000);
	button.fall(&button_handle);
	
	while(mode != DONE){
		
		osSignalWait(0x1, osWaitForever);
		
		if (connected && ping->count == VALUES_PER_MODE && ping->ping_flag && ping->ping_enable){
			printf("\r\n------- Mode Change -------\r\n");
			
			printf("New mode is: %i\n",mode);
			
			if(mode < TESTS_N){
				
				int wait_time = period[mode];
				ping->max_bw = bw[mode];
				updater.stop();
				
				if(wait_time != 0)
					updater.start(wait_time);
				else
					updater.start(MIN_T);
				mode++;
				ping->count = 0;
				ping->ping_flag = true;
			
				wait(wait_time/1000);
				
			} else {
				mode = DONE;
			}
		}
		
		if(connected && (mode != DONE) && update && ping->ping_enable && ping->ping_flag && (ping->count<VALUES_PER_MODE)){
			printf("\n\r------ event ------\n\r");
			ping->send();
			update = false;
		}
		
		/*if( (ping->read_timer() > 10000) && ping->ping_enable ) {
			printf("\n\r------ Ping timeout ------\n\r");
			ping->ping_flag = true;
			ping->count--;
			printf("\n\r********* Retry Send *********\n\r");			
		}*/
		
		if(pushed) {
			printf("\n\r------ button ------\n\r");
			printf("ping_enable(%i),ping_flag(%i),update(%i),connected(%i)\n\r",ping->ping_enable,ping->ping_flag,update,connected);
			printf("Restarting Test!\n");
			ping->batch_time();
			mode = 0;
			ping->count = VALUES_PER_MODE;
			ping->ping_enable = false;
			ping->ping_flag = true;
			pushed = false;
		}
		
		if( (ping->read_timer() >  20000) && !ping->ping_flag && update && ping->ping_enable) {
			printf("\n\r------ timeout ------\n\r");			
		}

		Thread::yield();
		ble.waitForEvent();
	}
	
	//ble.disconnect(Gap::REMOTE_DEV_TERMINATION_DUE_TO_LOW_RESOURCES);
	
	wait(10.0);
	
	ping->batch_time();
	
	
	while(1){
		printf("\rTest done!\n");
		wait(1.0);
	}
}
