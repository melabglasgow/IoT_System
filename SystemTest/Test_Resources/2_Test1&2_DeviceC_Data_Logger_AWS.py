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

ping_time_IoT_topic = "Nucleo/direct/time"


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
#values ="264,251,265,216,157,188,214,161,192,211,239,203,822,221,169,206,476,354,1196,140,208,225,194,209,224,229,185,189,197,165,214,195,206,125,177,129,253,137,219,137,201,173,150,228,164,224,174,192,132,196,192,187,159,224,128,165,194,156,241,222,154,138,285,156,153,192,137,134,160,147,126,158,159,188,170,136,142,221,133,128,168,144,125,186,125,129,124,130,154,141,187,170,195,137,206,133,218,153,181,125,248,178,169,126,257,125,236,130,241,138,217,139,147,246,134,233,193,165,125,223,225,128,130,187,165,155,126,124,143,217,1439,132,151,129,196,125,127,229,217,318,157,132,133,301,128,225,145,174,124,130,146,138,128,186,128,128,194,148,214,130,133,"
#add_time_to_csv(values)
#===============================================================================#
#===============================================================================#

#***************************************************#

def ping_time_callback(client, userdata, message):
	print message.payload
	add_time_to_csv(str(message.payload))

#***************************************************#

# Connect and subscribe to AWS IoT
myAWSIoTMQTTClient.connect()
myAWSIoTMQTTClient.subscribe(ping_time_IoT_topic, 0, ping_time_callback)

#***************************************************#

time.sleep(2)

while True:
	time.sleep(10)

