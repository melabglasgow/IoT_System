'''
/*
 * Copyright 2010-2017 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */
 '''
 
'''
/* Credits
 * This application uses Open Source components.
 * Created by Sergio Martin (Github: @smysergio)
 * Inspired on Project: https://github.com/aws/aws-iot-device-sdk-python/tree/master/samples/basicPubSub
 * License: Apache-2.0
*/
'''

from AWSIoTPythonSDK.MQTTLib import AWSIoTMQTTClient
import logging
import time
import argparse
import csv
import mbed_connector_api
import sys
from base64 import standard_b64decode as b64decode
import boto3
import json

#////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////#
#////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////#

client = boto3.client('iot')

def reactivate_cert():
	response = client.update_certificate(
		certificateId='9560fd3e883fb5b58bb2880f425f2d801f6043c920703e81a008f4a4a37584bd',
		newStatus='ACTIVE'
	)

reactivate_cert()

#////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////#
#////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////#

token = "YDWYb01rNn4hHfMUIhfwV48QMVbJxeKWHamZB1kRyTFxec4MBXIlUNzCeMaMl4m6WKbrXKTRsAQS6lYVtF7bAUIfBmzeTgSPUrJ7"
Endpoint = '83a4d6e0-fcc4-469f-96d4-6f3a33ecd636'
#---------- Initialisations ----------#

x = mbed_connector_api.connector(token)									# Instantiation of the connector object

x.debug(True,'ERROR')    # Set level of displayed messages in the terminal DEBUG INFO WARNING ERROR	

x.startLongPolling()	# Required for async_handling, can't do anything without this
#***************************************************#


#////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////#
#////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////#

#---------- Initialisations ----------#

# Read in command-line parameters
parser = argparse.ArgumentParser()
parser.add_argument("-e", "--endpoint", action="store", required=True, dest="host", help="Your AWS IoT custom endpoint")
parser.add_argument("-r", "--rootCA", action="store", required=True, dest="rootCAPath", help="Root CA file path")
parser.add_argument("-c", "--cert", action="store", dest="certificatePath", help="Certificate file path")
parser.add_argument("-k", "--key", action="store", dest="privateKeyPath", help="Private key file path")
parser.add_argument("-w", "--websocket", action="store_true", dest="useWebsocket", default=False,
                    help="Use MQTT over WebSocket")
parser.add_argument("-id", "--clientId", action="store", dest="clientId", default="basicPubSub",
                    help="Targeted client id")
parser.add_argument("-t", "--topic", action="store", dest="topic", default="sdk/test/Python/Pub", help="Targeted topic")

args = parser.parse_args()
host = args.host
rootCAPath = args.rootCAPath
certificatePath = args.certificatePath
privateKeyPath = args.privateKeyPath
useWebsocket = args.useWebsocket
clientId = args.clientId
Pubtopic = args.topic
SubTopic = "sdk/test/Python/Sub"

if args.useWebsocket and args.certificatePath and args.privateKeyPath:
    parser.error("X.509 cert authentication and WebSocket are mutual exclusive. Please pick one.")
    exit(2)

if not args.useWebsocket and (not args.certificatePath or not args.privateKeyPath):
    parser.error("Missing credentials for authentication.")
    exit(2)

# Init AWSIoTMQTTClient
myAWSIoTMQTTClient = None
if useWebsocket:
    myAWSIoTMQTTClient = AWSIoTMQTTClient(clientId, useWebsocket=True)
    myAWSIoTMQTTClient.configureEndpoint(host, 443)
    myAWSIoTMQTTClient.configureCredentials(rootCAPath)
else:
    myAWSIoTMQTTClient = AWSIoTMQTTClient(clientId)
    myAWSIoTMQTTClient.configureEndpoint(host, 8883)
    myAWSIoTMQTTClient.configureCredentials(rootCAPath, privateKeyPath, certificatePath)

# AWSIoTMQTTClient connection configuration
myAWSIoTMQTTClient.configureAutoReconnectBackoffTime(1, 32, 20)
myAWSIoTMQTTClient.configureOfflinePublishQueueing(-1)  # Infinite offline Publish queueing
myAWSIoTMQTTClient.configureDrainingFrequency(2)  # Draining: 2 Hz
myAWSIoTMQTTClient.configureConnectDisconnectTimeout(10)  # 10 sec
myAWSIoTMQTTClient.configureMQTTOperationTimeout(5)  # 5 sec


#////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////#
#////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////#

into_AWS_IoT_topic = "mbed/notify/test/83b4d6e0-fcc4-469f-96d4-6f3a33ecd636/1/0/1"
outof_AWS_IoT_topic = "mbed/put/test/83b4d6e0-fcc4-469f-96d4-6f3a33ecd636/1/0/2"

into_bridge_resource = '/1/0/1'	#ping send
time_resource = '/1/0/3'



id_flag_error = False

t0 = time.time()
t1 = time.time()
bridge_time = t1-t0
t2 = time.time()
lambda_time = t2-t1
id = 0

#===============================================================================#
#===============================================================================#

#-------------- csv to store ping data --------------#

f_time = open("ping_data.csv", 'wt')

writer_time = csv.writer(f_time, delimiter=',',quoting=csv.QUOTE_MINIMAL,lineterminator='\r')

def add_time_to_csv(data):
	index_current = 0
	while (index_current < len(data)):
		index_start = index_current
		index_current = data.find(',', index_current)
		print(', found at', index_current)
		input = data[index_start:index_current]
		print(input)
		writer_time.writerow([input])
		index_current += 1
	
#***************************************************#

#-------------- csv to store timer data --------------#

f_timer = open("mdc_timer.csv", 'wt')

header_timer = ['count']+['bridge timer']+['lambda timer']

writer_timer = csv.writer(f_timer, delimiter=',',quoting=csv.QUOTE_MINIMAL,lineterminator='\r')
writer_timer.writerow(header_timer)
count = 0
def add_timers_to_csv():
	print("add_timers_to_csv")
	line = [str(id)]+[str(bridge_time)]+[str(lambda_time)]
	writer_timer.writerow(line)
	global count
	count = count + 1

#***************************************************#

#===============================================================================#
#===============================================================================#

#***************************************************#

def handler_notification(data):								#everytime a value changes, this is sent!
	global id
	global t0
	#print("---------------------------")
	#print("into_bridge")				# contatins the resource that changed, and the new value! check async_Handler to decode base64
	#print(data)
	payload = b64decode(data['notifications'][0]['payload'])
	if(data['notifications'][0]['path'] == into_bridge_resource):
		#print("time_resource")
		id = str(payload)
		#print id
		t0 = time.time()
	elif(data['notifications'][0]['path'] == time_resource):
		print("amount of timers stored " + count)
		add_time_to_csv(payload)
		
	else:
		print data['notifications'][0]['path']
	#print("+++++++++++++++++++++++++++")

def into_AWS_callback(client, userdata, message):
	global id_flag_error
	global t1
	global bridge_time
	#print("---------------------------")
	#print("into_AWS_callback")
	#print(message.payload)
	d = json.loads(str(message.payload))
	message_id = str(d["value"])
	#print message_id		
	if(message_id != id):
		id_flag_error = True
		print("into_AWS_callback - not same message id!")
	global bridge_time
	t1 = time.time()
	bridge_time = str((t1-t0)*1000)
	#print(bridge_time)
	
def outof_AWS_callback(client, userdata, message):
	global id_flag_error
	global t2
	global lambda_time
	global bridge_time
	
	#print("---------------------------")
	#print("outof_AWS_callback")
	#print(message.payload)
	d = json.loads(str(message.payload))
	message_id = str(d["new_value"])
	#print message_id
	if(message_id != id):
		id_flag_error = True
		print("outof_AWS_callback - not same message id!")
	
	if(id_flag_error):
		lambda_time = 0
		bridge_time = 0
		id_flag_error = False
	else:
		t2 = time.time()
		lambda_time = str(((t2-t1)/2)*1000)	# This counts the time back and forth in the lambda so divide by 2.
		#print(lambda_time)
	add_timers_to_csv()
	
		
	#print("+++++++++++++++++++++++++++")
#***************************************************#



# Connect and subscribe to AWS IoT
myAWSIoTMQTTClient.connect()
myAWSIoTMQTTClient.subscribe(into_AWS_IoT_topic, 0, into_AWS_callback)
myAWSIoTMQTTClient.subscribe(outof_AWS_IoT_topic, 0, outof_AWS_callback)

#***************************************************#


	
x.setHandler('notifications', handler_notification)
r = x.putResourceSubscription(Endpoint,into_bridge_resource) # IF YOU DONT SUBSCRIBE HERE IT DOESNT COME IN TO THE HANDLER_NOTIFICATION!!!
r = x.putResourceSubscription(Endpoint,time_resource) # IF YOU DONT SUBSCRIBE HERE IT DOESNT COME IN TO THE HANDLER_NOTIFICATION!!!
x.postResource(Endpoint,time_resource,"START") # send the "Value" to the Resource over a POST request

#***************************************************#

time.sleep(2)

while True:
	reactivate_cert()
	time.sleep(10)

