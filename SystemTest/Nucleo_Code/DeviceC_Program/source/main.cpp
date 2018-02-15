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

/* SpwfInterface NetworkSocketAPI Example Program
 * Copyright (c) 2015 ARM Limited
 * Copyright (c) 2017 KLIKA TECH, LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.

 
/* Credits
* This application uses Open Source components.
* Modified by Sergio Martin (Github: @smysergio)
* Inspired on Project: https://os.mbed.com/teams/Klika-Tech/code/Nucleo-AWS-IoT-mbed/ https://github.com/Klika-Tech/nucleo-aws-iot-demo
* License: Apache-2.0
*/
 
//************************ direct-nucleo declarations ************************//

#include "mbed.h"
#include "SpwfInterface.h"
#include "TCPSocket.h"
#include "MQTTClient.h"
#include "MQTTWiFi.h"
#include <ctype.h>
#include "ping.h"


Serial pc(SERIAL_TX, SERIAL_RX);
DigitalOut led(PB_2);
   

#define AWS_IOT_MQTT_HOST              "a36y0j7pvueqfk.iot.eu-west-1.amazonaws.com" //Use your own host.
#define AWS_IOT_MQTT_PORT              8883
#define AWS_IOT_MQTT_CLIENT_ID         "Nucleo" //Should be kept if you are using same device clent.
#define AWS_IOT_MY_THING_NAME          "Nucleo" //Should be kept if you are using same device thing name.
#define AWS_IOT_MQTT_TOPIC_TEST		   "Nucleo/test"
#define AWS_IOT_MQTT_TOPIC_DATA		   "Nucleo/data"
#define AWS_IOT_MQTT_TOPIC_SHADOW	   "$aws/things/Nucleo/shadow/update"
#define AWS_IOT_ID ""
#define AWS_IOT_AUTH_TOKEN ""

// WiFi network credential
#define SSID   "PLUSNET-G73R"  // Network must be visible otherwise it can't connect
#define PASSW  "85869e43c6"
//#error "Wifi SSID & password empty"

#include "stdint.h"
#include "security.h"

//************************ Test Configuration ************************//

#define TEST_DONE -1

#define VALUES_PER_MODE 100
#define TESTS_N 1
#define MIN_T 10

int period[1]	= {	1000};
bool bw[1]		= {	false};


//**************************** direct-nucleo globals ************************//

bool connected = false;

int connack_rc = 0; // MQTT connack return code
int connectTimeout = 1000;
int retryAttempt = 0;


//Push Button
InterruptIn button(USER_BUTTON);
volatile bool pushed = false;
void button_handle(void)	{ pushed = true; }

//**************************** direct-nucleo functions ************************//

int connect(MQTT::Client<MQTTWiFi, Countdown, MQTT_MAX_PACKET_SIZE>* client, MQTTWiFi* ipstack)
{ 
	SpwfSAInterface& WiFi = ipstack->getWiFi();

	// Network debug statements
	printf("=====================================\n\r");
	LOG("Connecting WiFi.\n\r");
	LOG("Nucleo IP ADDRESS: %s\n\r", WiFi.get_ip_address());
	LOG("Nucleo MAC ADDRESS: %s\n\r", WiFi.get_mac_address());
	LOG("Server Hostname: %s port: %d\n\r", AWS_IOT_MQTT_HOST, AWS_IOT_MQTT_PORT);
	LOG("Client ID: %s\n\r", AWS_IOT_MQTT_CLIENT_ID);
	//LOG("Topic: %s\n\r", AWS_IOT_MQTT_TOPIC_TEST);
	//LOG("Subscription URL: %s\n\r", subscription_url);
	LOG("=====================================\n\r");
    
    ipstack->open(&ipstack->getWiFi());

    int rc=ipstack->getNTPtime();

    if (rc != 0)
	{
    	WARN("Get NTP time error: %d\n", rc);
		return rc;
	}

    rc = WiFi.setSocketClientSecurity((uint8_t *)"m", (uint8_t *)rootCA, (uint8_t *)clientCRT, (uint8_t *)clientKey, (uint8_t *)AWS_IOT_MQTT_HOST, ipstack->getTime());

    if (rc != 0)
	{
		WARN("Set security params error: %d\n", rc);
		return rc;
	}

    rc = ipstack->connect(AWS_IOT_MQTT_HOST, AWS_IOT_MQTT_PORT, connectTimeout);

    if (rc != 0)
    {
    	WARN("IP Stack connect returned: %d\n\r", rc);
        return rc;
    }

    printf ("--->TCP Connected\n\r");

    // MQTT Connect
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 4;
    data.struct_version=0;
    data.clientID.cstring = AWS_IOT_MQTT_CLIENT_ID;
    //data.username.cstring = "use-token-auth";
    //data.password.cstring = AWS_IOT_AUTH_TOKEN;

    if ((rc = client->connect(data)) == 0) 
    {
        printf ("--->MQTT Connected\n\r");
		connected = true;
        //if (!subscribe(client, ipstack)) printf ("--->>>MQTT subscribed to: %s\n\r",AWS_IOT_MQTT_TOPIC_TEST);
    }
    else
    {
        WARN("MQTT connect returned %d\n", rc);        
    }
    if (rc >= 0)
        connack_rc = rc;
    return rc;
}

int getConnTimeout(int attemptNumber)
{  // First 10 attempts try within 3 seconds, next 10 attempts retry after every 1 minute
   // after 20 attempts, retry every 10 minutes
    return (attemptNumber < 10) ? 3 : (attemptNumber < 20) ? 60 : 600;
}

void attemptConnect(MQTT::Client<MQTTWiFi, Countdown, MQTT_MAX_PACKET_SIZE>* client, MQTTWiFi* ipstack)
{
    while (connect(client, ipstack) != MQTT_CONNECTION_ACCEPTED) 
    {    
        if (connack_rc == MQTT_NOT_AUTHORIZED || connack_rc == MQTT_BAD_USERNAME_OR_PASSWORD)
        {
            printf ("File: %s, Line: %d Error: %d\n\r",__FILE__,__LINE__, connack_rc);        
            return; // don't reattempt to connect if credentials are wrong
        } 

        int timeout = getConnTimeout(++retryAttempt);
        WARN("Retry attempt number %d waiting %d\n\r", retryAttempt, timeout);
        
        // if ipstack and client were on the heap we could deconstruct and goto a label where they are constructed
        //  or maybe just add the proper members to do this disconnect and call attemptConnect(...)        
        // this works - reset the system when the retry count gets to a threshold
        if (retryAttempt == 2)
        {
        	ipstack->getWiFi().reset_chip();
            NVIC_SystemReset();
        }
        else
            wait(timeout);
    }
}

//**************************************** main ****************************************//

int main() {
		
	printf("\nStarting direct-nucleo test\n");
	
	//**************************** direct-nucleo init ***********************//
	
	ping_timer.start();
		
    const char * ssid = SSID; // Network must be visible otherwise it can't connect
    const char * seckey = PASSW;

    pc.baud(115200);

    SpwfSAInterface spwf(D8, D2, true);
    
    led=0;	
    
    printf("\r\nX-NUCLEO-IDW01M1 mbed Application\r\n");     
    printf("\r\nconnecting to AP\r\n");            
	MQTTWiFi ipstack(spwf, ssid, seckey, NSAPI_SECURITY_WPA2);
	

	LOG("Connected to WiFI.\r\n");
	
	spwf.set_debug(false);

	MQTT::Client<MQTTWiFi, Countdown, MQTT_MAX_PACKET_SIZE> client(ipstack, 5000);

	attemptConnect(&client, &ipstack);

	if (connack_rc == MQTT_NOT_AUTHORIZED || connack_rc == MQTT_BAD_USERNAME_OR_PASSWORD) {
	  while (true)
		wait(1.0); // Permanent failures - don't retry
	}

	led=1;
	
	if(connected){
		ping_init(&client, &ipstack);
	}else{
		printf("NOT CONNECTED");
	}
	
	//**************************** direct-nucleo Initial Values ************************//
	count = VALUES_PER_MODE;
	int mode = 0;
	int wait_time = 1000;
	
	button.fall(&button_handle);
	bool sensors_update = true;
	
	//**************************** Test Loop ************************//
	
	while (mode != TEST_DONE) {
		
		if (count == VALUES_PER_MODE && ping_enable && ping_flag){
			printf("\r\n------- Mode Change -------\r\n");
			
			printf("New mode is: %i\n",mode);
			
			if(mode < TESTS_N){
					
				max_bw = bw[mode];
				
				if(period[mode] != 0)
					wait_time = period[mode];
				else
					wait_time = MIN_T;
				
				mode++;
				count = 0;
				ping_flag = true;
			
				wait(wait_time/1000);
				
			} else {
				mode = TEST_DONE;
			}	
		}
		
		if(sensors_update && ping_enable && ping_flag && (count<VALUES_PER_MODE) ) {
			printf("\r\n------- Sending Ping -------\r\n");
			if (ping_send(&client, &ipstack) != 0) {
				printf("error sending");
				ipstack.getWiFi().reset_chip();
				NVIC_SystemReset();
				attemptConnect(&client, &ipstack);   // if we have lost the connection
			}
		}
		
		if(pushed) {
			printf("\r\n------- Button Pushed -------\r\n");
			printf("Restarting Test!\n");
			mode = 0;
			count = VALUES_PER_MODE;
			ping_enable = false;
			ping_flag = true;
			pushed = false;
		}
		
		client.yield(wait_time);
	}
	
	ping_batch_time(&client, &ipstack);
	
	while(1){
		printf("Test done!\n\r");
		wait(1);
	}
}
