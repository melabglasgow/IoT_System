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
  * Inspired on Project: https://github.com/ARMmbed/mbed-os-example-client
  * License: Apache-2.0
  */
 
//************************ easy-connect declarations ************************//

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <string>
#include <sstream>
#include <vector>

#include "mbed.h"

#include "security.h"
#include "mbed-trace/mbed_trace.h"
#include "mbedtls/entropy_poll.h"

#include "easy-connect/easy-connect.h"
#include "simpleclient.h"

#include "ping.h"
PING ping;

//************************ Test Configuration ************************//

#define TEST_DONE -1

#define VALUES_PER_MODE 50
#define TESTS_N 2
#define MIN_T 10

int period[TESTS_N]	= {	10,	5000};
bool bw[TESTS_N]	= {	false,	false};

osThreadId mainThread;

//**************************** easy-connect globals ************************//

volatile bool registered = false;

struct MbedClientDevice device = { // These are example resource values for the Device Object
    "Manufacturer_String",      // Manufacturer
    "Type_String",              // Type
    "ModelNumber_String",       // ModelNumber
    "SerialNumber_String"       // SerialNumber
};

//Push Button
InterruptIn button(USER_BUTTON);
volatile bool pushed = false;
void button_handle(void)	{ pushed = true; osSignalSet(mainThread, 0x1);}	//updates_global.release(); } 

//RtosTimer updater;
volatile bool registered_check = false;
void register_handle(void const *n)	{ registered_check = true; osSignalSet(mainThread, 0x1);}	//updates_global.release(); }
RtosTimer register_checker(&register_handle, osTimerPeriodic, (void *)0);

//RtosTimer updater;
volatile bool update = false;
void update_handle(void const *n)	{ update = true; osSignalSet(mainThread, 0x1);}	//updates_global.release(); }
RtosTimer updater(&update_handle, osTimerPeriodic, (void *)0);

//**************************** easy-connect functions ************************//

void entropy_init() {
	
	unsigned int seed;
    size_t len;

	#ifdef MBEDTLS_ENTROPY_HARDWARE_ALT
		// Used to randomize source port
		mbedtls_hardware_poll(NULL, (unsigned char *) &seed, sizeof seed, &len);

	#elif defined MBEDTLS_TEST_NULL_ENTROPY

	#warning "mbedTLS security feature is disabled. Connection will not be secure !! Implement proper hardware entropy for your selected hardware."
		// Used to randomize source port
		mbedtls_null_entropy_poll( NULL,(unsigned char *) &seed, sizeof seed, &len);

	#else

	#error "This hardware does not have entropy, endpoint will not register to Connector.\
	You need to enable NULL ENTROPY for your application, but if this configuration change is made then no security is offered by mbed TLS.\
	Add MBEDTLS_NO_DEFAULT_ENTROPY_SOURCES and MBEDTLS_TEST_NULL_ENTROPY in mbed_app.json macros to register your endpoint."

	#endif

    srand(seed);
}

//**************************************** main ****************************************//

int main() {
	
	printf("\r\n------ main ------\r\n");
    mainThread = osThreadGetId();
	
	//**************************** easy-connect init ***********************//
	printf("\nStarting easy-connect test\n");
	
	MbedClient mbed_client(device); // Instantiate the class which implements LWM2M Client API (from simpleclient.h)
	
	entropy_init();
	
	mbed_trace_init();

    NetworkInterface* network = easy_connect(true);
    if(network == NULL) {
        printf("\nConnection to Network Failed - exiting application...\n");
        return -1;
    }
	
    // Create endpoint interface to manage register and unregister
    mbed_client.create_interface(MBED_SERVER_ADDRESS, network);

    // Create Objects of varying types, see simpleclient.h for more details on implementation.
    M2MSecurity* register_object = mbed_client.create_register_object(); // server object specifying connector info
    M2MDevice*   device_object   = mbed_client.create_device_object();   // device resources object

    // Create list of Objects to register
    M2MObjectList object_list;

    // Add objects to list
    object_list.push_back(device_object);
	object_list.push_back(ping.get_object());
	// Set endpoint registration object
    mbed_client.set_register_object(register_object);

    // Register with mbed Device Connector
    mbed_client.test_register(register_object, object_list);

	printf("\r\nWaiting for mbed Client Registering.");
	while(!registered){
		registered = mbed_client.register_successful();
		printf(".");
		wait(0.5);
	}
	printf("\n");
	
	//**************************** easy-connect Initial Values ************************//
	
	ping.count = VALUES_PER_MODE;
	int mode = 0;
	
 	register_checker.start(20000);
	updater.start(1000);
	button.fall(&button_handle);
	
	//**************************** Test Loop ************************//
	
	while (mode != TEST_DONE) {
		
		osSignalWait(0x1, osWaitForever);
		
		if (registered && ping.count == VALUES_PER_MODE && ping.ping_enable && ping.ping_flag){
			printf("\r\n------- Mode Change -------\r\n");
			
			printf("New mode is: %i\n",mode);
			
			if(mode < TESTS_N){
				
				int wait_time = period[mode];
				ping.max_bw = bw[mode];
				updater.stop();
				
				if(wait_time != 0)
					updater.start(wait_time);
				else
					updater.start(MIN_T);
				mode++;
				ping.count = 0;
				ping.ping_flag = true;
			
				wait(wait_time/1000);
				
			} else {
				mode = TEST_DONE;
			}
		}
		
		if(registered && update && ping.ping_enable && ping.ping_flag && (ping.count<VALUES_PER_MODE) ) {
			ping.send();
			update = false;
		}
		
		if(pushed) {		// Used to restart the test when a problem is encountered
			printf("\r\n------- Button Pushed -------\r\n");
			//the values of the control variables are printed
			printf("ping_enable(%i),ping_flag(%i),update(%i),registered(%i)\n\r",ping.ping_enable,ping.ping_flag,update,registered);
			
			//The control variables are restarted
			printf("Restarting Test!\n");
			mode = 0;
			ping.count = VALUES_PER_MODE;
			ping.ping_enable = false;
			ping.ping_flag = true;
			pushed = false;
			//ping.mdc_batch_time();
		}
		
		if(registered_check) {		// Ensure that the device is still registered in Mbed Device Connector
			printf("\r\n------- Client Register Check Action -------\r\n");
			
			registered = mbed_client.register_successful();
			
			if(registered)
				mbed_client.test_update_register();
				
			registered_check = false;
		}
		
		Thread::yield();
	}
	
	ping.batch_time();				// Send all of the ping times recorded to time resource
	
	//mbed_client.test_unregister(); //if you wish to unregister the device after test
	
	while(1){
		printf("Test done!\n");
		wait(1);
	}
}
