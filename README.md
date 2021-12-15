# KiloCamUltra
Ultra low power trail cam

This is a fork of software for the amazing kiloCam Arduino and ESP32 cam boards.

Specifically this fork adds in the following features.
- Reduced camera cycle time (by ~50%)
- Enables the kilocam to communicate with the camera via serial. This allows different functionality to be programmed into the ESP32Cam which can be controlled by a  serial message from kilocam (e.g. 5min cycle times with no flash during the day, followed by hourly cycles with flash at night)
- ESP32 cam stores the images with the following format IMG_YYYYMMDDHHMMSS.jpg so timestamps can be esily infered

## The KiloCam
The Kilocam is an ultralow power arduino based PCB for controlling ESP32Cameras. It is available from https://www.ecologisconsulting.com/products/p/kilocam-pcb-v10 and is currently in beta. It enables you to create ultra low power envriromental/trail/timelapse cameras for <£50, which can be deployed for upto 3months on a single battery.

## The ESP32Cam
A cheap and easy to use camera board.

### How to use
This pair of sketches is a direct replacement for the original sketches by ecologis. There is a great youtube video walking you step by step trhough the set up process. You must set the clock by running the setserial sketch in the clock examples folder.

In the Kilocam sketch you can change the Dawn and Dusk variables. These control the time the kilocam swithes from one behaviour to the next. You can change the alarmInterval and darkAlarmInterval variables to control the interval between ESP32cam cycles during the two different periods.
Currently is ACTION variable is set to 1 this will enable the front facing LED on the ESP to power up.

These sketches are set to from 0600 to 1700 the camera will cycle every 5mins, from 1700 to 0600 the camera will cycle hourly with front facing flash.

Additionally you will need to solder 2 jumpers from the serial terminals into a twin dupont female terminal. And to allow the terminals to fit the ESP32board you need to remove the right side bottom three terminal blocks. This can be done with a dremmel.

![KilocamPCB](https://github.com/ExtinctionEvent/KiloCamUltra/images/blob/main/KilocamPCB.jpg?raw=true)

To finish with a connection like this...

![Kilocam and ESP32cam with jumpers](https://github.com/ExtinctionEvent/KiloCamUltra/images/blob/main/kilocamwithesp32camandjumpers..jpg?raw=true)





