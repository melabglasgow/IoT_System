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
#include "easy-resources.h"

#define PYTHON_WAIT 1	// A wait between updating resources so that the python script can be properly notified for all resources.

//**************************** easy-connect globals ************************//

// These are example resource values for the Device Object
struct MbedClientDevice device = {
    "Manufacturer_String",      // Manufacturer
    "Type_String",              // Type
    "ModelNumber_String",       // ModelNumber
    "SerialNumber_String"       // SerialNumber
};

// Instantiate the class which implements LWM2M Client API (from simpleclient.h)
MbedClient mbed_client(device);
volatile bool registered = false;
// Network interaction must be performed outside of interrupt context
osThreadId mainThread; //Semaphore updates_global(0);


//RtosTimer updater;
volatile bool update = false;
void update_handle(void const *n)	{ update = true; osSignalSet(mainThread, 0x1);}	//updates_global.release(); }
RtosTimer updater(&update_handle, osTimerPeriodic, (void *)0);

//RtosTimer sensors_updater;
volatile bool sensors_update = false;
void sensors_update_handle(void const *n)	{ sensors_update = true; osSignalSet(mainThread, 0x1);}	//updates_global.release(); }
RtosTimer sensors_updater(&sensors_update_handle, osTimerPeriodic, (void *)0);


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

//***********************************************************************//

// Entry point to the program
int main() {
	
	// Keep track of the main thread
    mainThread = osThreadGetId();
	
	printf("\nStarting mbed Client example\n");
	
	//**************************** easy-connect init ***********************//
	
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
	
	RES res(&object_list);
	
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
	
	//*********************************************************************//
	
 	updater.start(30000);
	sensors_updater.start(20000);
	
	
	#if SENSE_RESOURCE == true
	
		res.sense->thread_enable();
	#endif
	
	#if BUTTON_RESOURCE != true
	
		int timer_count = 0;
	#endif
	
	while (1) {
		
		osSignalWait(0x1, osWaitForever); //osWaitForever //Thread::wait(100); //updates_global.wait(25000);
		
		if(update) {
			printf("\r\n------- Client Update Action -------\r\n");
			
			if(registered)
				mbed_client.test_update_register();
				
			update = false;
		}	
		
		if(sensors_update) {
			printf("\r\n------- Sensors Update Action -------\r\n");
			
			#if SENSE_RESOURCE == true
				
				res.sense->update_sensors(ALL);
				res.sense->print();
				res.sense->mdc_set_motion();
				
				res.sense->mdc_set_sensors(TEMP | HUMI);
				wait(PYTHON_WAIT);
			#endif
			
			
			//#if TIMER_RESOURCE == true
			
			//	res.e_time->mdc_get_value();
			//#endif
				
			
			#if RGB_RESOURCE == true
			
				#if PRED_RESOURCE == true
					if(res.pred->comf == NO_PRED) {
						res.rgb->flash();
					} else {
						res.rgb->write((color)res.pred->comf_level);
					}
				#else
					res.rgb->flash();
				#endif
				wait(PYTHON_WAIT);
			#endif
			
			#if BUTTON_RESOURCE != true
			
				timer_count++;
			#endif
			
			printf("\n");
			sensors_update = false;
		}
		
		#if BUTTON_RESOURCE == true
		
			if(res.button->clicked) {
		#else
			if(timer_count == 6) {
		#endif
				printf("\r\n------- Button Action -------\r\n");
				
				#if RGB_RESOURCE == true
				
					res.rgb->write(white);
					wait(PYTHON_WAIT);
				#endif

				#if TIMER_RESOURCE == true
				
					res.e_time->mdc_update_value();
					wait(PYTHON_WAIT);
				#endif
				
				#if SENSE_RESOURCE == true
				
					res.sense->mdc_set_sensors(ALL);
					wait(PYTHON_WAIT);
				#endif
				
				#if PRED_RESOURCE == true
				
					res.pred->mdc_set_cur(res.sense->temp,res.sense->humi);
					wait(PYTHON_WAIT);
				#endif
				
				#if BUTTON_RESOURCE == true
				
					res.button->clicked = false;
					wait(PYTHON_WAIT);
				#else
					timer_count = 0;
				#endif
				
				printf("\n");
			}
		
		
		Thread::yield();
	}
	
	mbed_client.test_unregister();
}