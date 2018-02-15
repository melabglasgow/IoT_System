# System Integration: Arm Mbed & Amazon Web Services

This repository contains all of the software resources used in the Project  for my Master Thesis.
* Section 1 -> __Documents__: Contains the report and presentation. ([Documents](/Documents) Folder)
* [Section 2](#system-evaluation) -> __System Evaluation__: Contains the report and presentation. ([SystemTest](/SystemTest) Folder)
* [Section 3](#smart-home-application) -> __Smart Home Application__: Contains the report and presentation. ([SmartHome](https://github.com/user/repo/blob/branch/other_file.md) Folder)

![System](/Documents/System.png)

# System Evaluation
This repository contains the code and resources to run the experiments described on the Chapter 3.2 of my X [Thesis](https://github.com/user/repo/blob/branch/other_file.md)

The hardware components used for the evaluation are:
* ST [Nucleo F401RE][1] development board
* ST [X-NUCLEO-IDB05A1][2] BLE shield
* ST [X-NUCLEO-IDW01M1][3]Wi-Fi shield
* RGB LED and Jumper Wires.
* Windows PC (haven’t been tested in other OS)

The Software required to be installed in the Windows PC is:
* [Mbed CLI][10]
* Python

The Accounts required are:
* Mbed [Account][4] (free)
* Amazon Web Services [Account][5] (free-trial) 

## Path I: Mbed Device and Gateway
This example is based on [this][6] project from the Arm Team.
1. Device A Setup (Nucleo board with BLE Shield)
	1. Download folder _.\SystemTest\Nucleo_Code\DeviceA_Program_ into the PC’s directory.
	1. Open the __main.cpp__ file and go through the test configuration as that of [section](#test-configuration)
	1. Run `mbed deploy` (downloads the necessary Libraries)
	1. Connect the Nucleo board to the PC and run `mbed compile -f` (this should flash the program to the device, you should see the RGB LED in the Nucleo Board).
	1. Debugging.
		1. Use the __nRF Connect__ App on your Android or iOS smartphone to find the BLE Device __PING__
		1. Use [Tera Term][11] to see the serial output of the device. (Baud Rate 115200)
1. Gateway Setup (Raspberry Pi):
The Pi is setup as shown in the [DeviceLink Project][7]. Skip section “Building a BLE device”. Once the two scripts __Linux_Devicelink__ and __Bluetooth_Devicelink__ are running and the local web application __localhost:3000__ can be opened from your web Browser, do as follows:
	1. Connect to the __PING__ BLE Device.
	1. Substitute the functions in the two squares with those from file __DeviceLink_Functions.txt__ in _.\SystemTest\Test_Resources_.
	1. Debug:
		1. Check messages in the Linux Terminals.
		1. Check Mbed Device Connector where you should see a new device connected.

## Path II: Mbed Device with Mbed Client
This code is based on the [mbed_os_example_client][8].
1. Download folder _.\SystemTest\Nucleo_Code\DeviceB_Program_ to your PC.
1. Get the credentials from Mbed Device Connector 
	1. Go to [Mbed Device Connector][9] and log in with your mbed account.
	1. On mbed Device Connector, go to My Devices > Security credentials, and get new credentials for your device by clicking the Get my device security credentials button.
	1. Store the credentials in __security.h__.
1. Open the __mbed_app.json__ file
	1. insert the SSID Name and password for your LAN or mobile hotspot.
1. Open the __main.cpp__ file and go through the test configuration as that of [section](#test-configuration)
1. Run `mbed deploy` in the cmd line to download the libraries
1. Connect the Nucleo board to the PC and run `mbed compile -f` (this should flash the program to the device, you should see the RGB LED in the Nucleo Board flash).
1. Debugging
	1. Use [Tera Term][11] to see the serial output of the device. (Baud Rate 115200)
	1. Use the Mbed Device Connector [Website][9] and the [Console][12] to observe the resources.

## Path III: Mbed Device connected IoT AWS
This Path follows the configuration steps from the [Project by Klikka][13]. Follow the tutorial to learn how to configure the device, specially to get the security credentials.
Device C Setup (Nucleo board with Wifi Shield)
1. Download folder _.\SystemTest\Nucleo_Code\DeviceC_Program_ into the PC’s directory.
1. Run `mbed deploy` (downloads the necessary Libraries)
1. Put your SSID and password for your LAN or hotspot as explained in the [Tutorial][13].
1. Put the Credentials as explained in the Tutorial.
1. Open the __main.cpp__ file and go through the test configuration as that of [section](#test-configuration)
1. Connect the Nucleo board to the PC and run `mbed compile -f` (this should flash the program to the device, you should see the RGB LED in the Nucleo Board).
1. Debugging:
	1. Use [Tera Term][11] to see the serial output of the device. (Baud Rate 115200)
	1. Note that the program has trouble connecting to the network and that it restarts several times before it actually manages to connect successfully. See the [Project][13] for more info.
	
The Topics to which the device is subscribed and publishes to is shown here:
```cpp
#define TOPIC_SUB_START	"Nucleo/direct/start"
#define TOPIC_SUB_RECV	"Nucleo/direct/ping-in"
#define TOPIC_PUB_SEND	"Nucleo/direct/ping-out"
#define TOPIC_PUB_TIME	"Nucleo/direct/time"
```

## Setup Bridge Connector.
Follow this [Project from Mbed Team][14] to connect Mbed Device Connector with AWS. Skip Section _Getting the sample application on your device_. Follow the rest of the steps to generate the AWS credentials and setup the Docker Application.
For a device registered in Mbed Device Connector, the Bridge Connector creates and registers a device in IoT AWS and publishes and subscribes to a set of MQTT Topics that allow you to interact with the Mbed Device. You can subscribe to the “mbed/#” to subscribe to all topics created by the Brige Connector.

## Cloud Actions for Lambda Functions
There is one Lambda function for Path I and II and a different one for Path III. Each must be attached to the appropriate Topic as described in the Table above. To learn more about how to setup the Lambda function, read the [Lambda AWS Documentation][15] and follow the [Quiklab Tutorial][16] and [here][17] on how to create a lambda function.

To set the actions in the AWS cloud you have to be familiar with [AWS IoT Rules][18] and [AWS IoT Actions][19].

In section 2.2.4 of the report, I show the Topics created by the Bridge Connector to which messages from the mbed device to AWS IoT.
1. Create Two Lambda Functions in _.\SystemTest\Test_Resources_ :
	1. Use the function in file _LambdaBouncer_1.txt_ for Path I
	1. Use the function in file _LambdaBouncer_2.txt_ for Path II
	1. Use the function in file _LambdaBouncer_3.txt_ for Path III
1. Create the IoT Rules. In the topics below substitute the _[endpoint-type]_ and _[endpoint-name]_
	1. For Path I
		1. Topic Filter: the notification Topic of the ping_out Topic:
		_mbed/notify/[endpoint-type]/[endpoint-name]/a000/0/a001_
		1. Action: trigger Lambda Function 1.
	1. For Path II
		1. Topic Filter: the notification Topic of the ping_send Topic:
		_mbed/notify/[endpoint-type]/[endpoint-name]/1/0/1_
		1. Action: trigger Lambda Function 2.
	1. For Path III
		1. Topic Filter: the notification Topic of the pub_send Topic:
		_Nucleo/direct/ping-out_
		1. Action: trigger Lambda Function 3.

## Python Scripts
The python scripts in the folder _\SystemTest\Test_Resources_, control the start of the test as well as take measurements or are part of the system by _bouncing the message back_. Other considerations to take are:
* Script 1,3 and 4 you must open the script and fill in the _token_ variable with your own. It can be obtained from the _Access Keys_ Tab in the [Mbed Device Connector][9].
* Script 2 and 3 IoT AWS credentials. This script is based on the “PubSub” [example code][20] from AWS and to get the credentials and how to run the script, see this [section][21]. Put the credentials obtained in the _.\SystemTest\Test_Resources\Certificates_ folder.
* For Script 3, you need to find the Certificate ID for the credentials we just created for this Scripts in this previous point.
	* Go to the _Thing_ created.
	* Click on the Security Tab 
	* Copy the Name of the Certificate (the Long ID).
	* Paste this name inside the Script in the _certificateId_ variable in the script.
* Script 4  can run in two modes: This script _bounces_ the messages in Test 4. When running the test with Device A (BLE), run the program with the argument _ble_ and when running the test with Device B (Wifi), then run the program with the argument _wifi_.


## Test Configuration
For the three device implementations (A,B and C) we have to configure the test parameters. The test runs through a set of modes, in which you can choose:
the periodicity with which ping times are sent (in milliseconds).
* The size of the data packet being sent:
	* False: minimum data packet (the counter of messages that have been sent)
	* True: maximum data packet (the maximum of this path)
		* Path I (BLE) – 26 bytes.
		* Path II (CoAP) – 1000 bytes.
		* Path III (MQTT) – 300 bytes.
The Values that can be modified (labelled in red in the example code below) are:
* VALUES_PER_MODE:
* TEST_N: The number of modes to test.
* period[TESTS_N]: the period at which messages are sent for each mode.
* bw[TESTS_N]: the data packet size for each mode.
So for the example below, the device will send 50 messages every second with minimum data packet and then another 50 messages every five seconds with maximum data packet.

```cpp
/************************ Test Configuration ************************//

#define TEST_DONE -1

#define VALUES_PER_MODE 50
#define TESTS_N 2
#define MIN_T 10

int period[TESTS_N]	= {	1000,	5000};
bool bw[TESTS_N]	= {	false,	true};

/*********************************************************************//
```

# Smart Home Application
This repository contains the code and resources to run the Smart Home Application described on the Chapter 4 of the report.

The hardware components used for the evaluation are:
* ST [Nucleo F401RE][1] development board
* ST [X-NUCLEO-IDB05A1][2] BLE shield
* ST [X-NUCLEO-IDW01M1][3] Wi-Fi shield
* ST [X-NUCLEO-IKS01A1][22] Sensor Shield 
* RGB LED and Jumper Wires.
* Windows PC (haven’t been tested in other OS)

The Software required to be installed in the Windows PC is:
* [Mbed CLI][10]
* Python

The Accounts required are:
* Mbed [Account][4] (free)
* Amazon Web Services [Account][5] (free-trial) 

## Device Configuration.
This code is based on the [mbed_os_example_client][8]
1. Download folder _.\SmartHome\m2A_MbedDevice_ to your PC.
1. Get the credentials from Mbed Device Connector 
	1. Go to mbed Device Connector [9] and log in with your mbed account.
	1. On mbed Device Connector, go to My Devices > Security credentials, and get new credentials for your device by clicking the Get my device security credentials button.
	1. Store the credentials as __security.h__ in this project's directory.
1. Open the __mbed_app.json__ file
	1. insert the SSID Name and password for your LAN or mobile hotspot.
1. Run `mbed deploy` in the cmd line to download the libraries.
1. Connect the Nucleo board to the PC and run `mbed compile -f` (this should flash the program to the device, you should see the RGB LED in the Nucleo Board flash).
1. Connect the RGB LED to pins PB_2(red), PB_15 (green) and PB_1 (blue) (Check pin layout here. [1]
1. Debugging
	1. Use [Tera Term][11] to see the serial output of the device. (Baud Rate 115200)
	1. Use the Mbed Device Connector [Website][9] and the [Console][12] to observe the resources.

## Application Configuration.

1. Open the __m2A_PythonApp.py__ file.
1. Get the Token from mbed Device Connector
	1. Go to [mbed Device Connector] [12] and log in with your mbed account.
	1. On mbed Device Connector, go to My Applications > Access keys, and generate Access Token and copy it into the _token_ variable.
1. Get the endpoint name of your device
	1. On [mbed Device Connector] [12], go to My Devices > Connected Devices, and you should see a device listed in the table.
	1. Copy the name in the list on the _endpoint_ variable of the file
1. On the root of the folder, run __python m2A_PythonApp.py__ in the terminal.

## Setup Bridge Connector.
Follow this [Project from Mbed Team][14] to connect Mbed Device Connector with AWS. Skip Section _Getting the sample application on your device_. Follow the rest of the steps to generate the AWS credentials and setup the Docker Application.
For a device registered in Mbed Device Connector, the Bridge Connector creates and registers a device in IoT AWS and publishes and subscribes to a set of MQTT Topics that allow you to interact with the Mbed Device. You can subscribe to the “mbed/#” to subscribe to all topics created by the Bridge Connector.

## Cloud Actions
To set the actions in the AWS cloud you have to be familiar with [AWS IoT Rules][18] and [AWS IoT Actions][19].
We also have to create a Lambda function for the comfort prediction. To learn more about how to setup the Lambda function, read the [Lambda AWS Documentation][15] and follow the [Quiklab Tutorial][16] and [here][17] on how to create a lambda function.
In section 2.2.4 of the report, I show the Topics created by the Bridge Connector to which messages from the Mbed device get to AWS IoT
1. Email Alert when motion is detected
	1. Topic Filter: the notification Topic of the motion resource.
	1. Condition: equals true
	1. Action: trigger [SNS AWS][23]
1. Store temperature in DynamoDB
	1. Topic Filter: the notification Topic of the motion resource.
	1. Action: trigger [DynamoDB][24]
1. Comfort Zone Prediction
	1. Train ML Model: follow [this example][25] by AWS if you are not familiar on how to. 
		1. Go to _.\SmartHome\m2A_CloudResources\ML_DataSets_ and use __EXL_TA_RH.csv__ to create your Model.
		1. Select all variables as numeric
		1. Select ‘ASH’ as the target
		1. When Generated find the 
	1. Create Lambda Function:
		1. [Activate real-time predictions][26]
		1. Create a function from scratch
		1. Copy the code from file __LambdaFunction_PredML.txt__ in _./MbedToAWS\m2A_CloudResources_
		1. Find the ID for your model and substitute it in the code.
	1. AWS IoT Rule.
		1. Topic Filter: the notification Topic of the current conditions prediction resource.
		1. Action: trigger Lambda Function.

## ML Dataset
The _.\MbedToAWS\m2A_CloudResources\ML_DataSets_ folder contains the Files from the [ASHRAE project][27] and the python scripts to manipulate the data using the [csv][28] Library.
1. 1_merge.py : merges all 52 files together, adds a climate field, fixes the times column and clears the files with dots.
1. 2_select.py: for a given excel, selects specific columns. To choose columns select the fields from “headers/categ_cl.csv” that you want to select and put them in “headers/categ_sel.csv”
1. 3_delete.py: For a given excel, deletes the rows that have a missing value.

[1]: https://os.mbed.com/platforms/ST-Nucleo-F401RE/
[2]: https://os.mbed.com/components/X-NUCLEO-IDB05A1-Bluetooth-Low-Energy/
[3]: https://os.mbed.com/components/X-NUCLEO-IDW01M1/
[4]: https://os.mbed.com/account/login/
[5]: https://aws.amazon.com/free/?sc_channel=PS&sc_campaign=acquisition_UK&sc_publisher=google&sc_medium=cloud_computing_b&sc_content=aws_account_e&sc_detail=aws%20account&sc_category=cloud_computing&sc_segment=67181347609&sc_matchtype=e&sc_country=UK&s_kwcid=AL!4422!3!67181347609!e!!g!!aws%20account&ef_id=WYXW2AAAAH-aE1MC:20180210145452:s
[6]: https://github.com/ARMmbed/mbed-os-example-ble
[7]: https://os.mbed.com/blog/entry/Connecting-BLE-devices-to-the-cloud
[8]: https://github.com/ARMmbed/mbed-os-example-client
[9]: https://connector.mbed.com/
[10]: https://github.com/ARMmbed/mbed-cli
[11]: https://os.mbed.com/docs/latest/tutorials/serial-comm.html
[12]: https://connector.mbed.com/#console
[13]: https://os.mbed.com/teams/Klika-Tech/code/Nucleo-AWS-IoT-mbed/
[14]: https://docs.mbed.com/docs/mbed-device-connector-web-interfaces/en/latest/cloud_amazon/
[15]: https://aws.amazon.com/documentation/lambda/
[16]: https://aws.qwiklab.com/
[17]: https://docs.aws.amazon.com/lambda/latest/dg/get-started-create-function.html
[18]: http://docs.aws.amazon.com/iot/latest/developerguide/iot-rules.html
[19]: http://docs.aws.amazon.com/iot/latest/developerguide/iot-rule-actions.html
[20]: https://github.com/aws/aws-iot-device-sdk-python
[21]: https://github.com/aws/aws-iot-device-sdk-python#examples
[23]: https://docs.aws.amazon.com/iot/latest/developerguide/sns-rule.html
[24]: https://docs.aws.amazon.com/iot/latest/developerguide/dynamodb-rule.html
[25]: https://docs.aws.amazon.com/machine-learning/latest/dg/tutorial.html
[26]: https://docs.aws.amazon.com/machine-learning/latest/dg/requesting-real-time-predictions.html
[27]: http://sydney.edu.au/architecture/staff/homepage/richard_de_dear/ashrae_rp-884.shtml
[28]: https://docs.python.org/2/library/csv.html