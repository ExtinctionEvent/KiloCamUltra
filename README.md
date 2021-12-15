# KiloCamUltra
Ultra low power trail cam

This is a fork of software for the amazing kiloCam Arduino and ESP32 cam boards.

Specifically this fork adds in the following features.
- Reduced camera cycle time (by ~50%)
- Enables the kilocam to communicate with the camera via serial. This allows different functionality to be programmed into the ESP32Cam which can be controlled by a  serial message from kilocam (e.g. 5min cycle times with no flash during the day, followed by hourly cycles with flash at night)
- ESP32 cam stores the images with the following format IMG_YYYYMMDDHHMMSS.jpg so timestamps can be esily infered

### The KiloCam
The Kilocam is an ultralow power arduino based PCB for controlling ESP32Cameras. It is available from https://www.ecologisconsulting.com/products/p/kilocam-pcb-v10 and is currently in beta. It enables you to create ultra low power envriromental/trail/timelapse cameras for <£50, which can be deployed for upto 3months on a single battery.

### The ESP32Cam
A cheap and easy to use camera board.

### How to use
This pair of sketches is a direct replacement for the original sketches by ecologis. There is a great youtube video walking you step by step through the set up process. You must set the clock by running the setserial sketch in the clock examples folder.

In the Kilocam sketch you can change the Dawn and Dusk variables. These control the time the kilocam swithes from one behaviour to the next. You can change the alarmInterval and darkAlarmInterval variables to control the interval between ESP32cam cycles during the two different periods.
Currently is ACTION variable is set to 1 this will enable the front facing LED on the ESP to power up.

These sketches are set to from 0600 to 1700 the camera will cycle every 5mins, from 1700 to 0600 the camera will cycle hourly with front facing flash.

Additionally you will need to solder 2 jumpers from the serial terminals into a twin dupont female terminal. And to allow the terminals to fit the ESP32board you need to remove the right side bottom three terminal blocks. This can be done with a dremmel.

![KilocamPCB](https://github.com/ExtinctionEvent/KiloCamUltra/blob/main/images/kilocamPCB.jpg?raw=true)

To finish with a connection like this...

![Kilocam and ESP32cam with jumpers](https://github.com/ExtinctionEvent/KiloCamUltra/blob/main/images/kilocamwithesp32camandjumpers.jpg?raw=true)

### How does it work?

Step by step guide to the code interactions. 

1. KiloCam alarm triggers and initiates runCamera() function
2. The runCamera() function powers up pin5 and the ESP32Cam boots
3. The runCamera() function then waits to recieve a "O" for On from the ESP32cam
4. When the ESP32 has booted it sends a O for on by serial to kilocam
5. When kiloCam recieves the O for On, it reciprocates by sending a data payload. The payload consists of a 18byte string - "H" for header, an int to drive a specific Action in the camera, and then the timestamp in form YYYYMMDDHHMMSS.
6. When ESP32cam recieves this payload it adjusts its activity based upon the Action int (currently this turns on the front LED flash, when Action==1). This can be programmed to drive any specific activity or camera setup you wish.
7. When the camera has taken the photo and stored it to the SD card, it then sends a X for Off back to the kilocam and powers down
8. KiloCam waits for the serial X, pauses for a 0.5sec then powers down waiting for the next alarm

### Troubleshooting

Once the jumpers are soldered inplace to the Serial sockets, it becomes more difficult to update the kilocam. I created a bespoke connecting cable that uses deupont connectors to connect to the jumperwires and the board.

Some ESP32Cams, specifically those that come with a programming PCB board have a localalised Right side GND (see image below). These cameras wont work with the original kiloCam unless this GND/R pin is removed. This however makes reprogramming the ESP more difficult as this GND is needed for programming the board. With the With kiloCamUlta, you can keep the pin in place as the corresponding dupont connector on the kiloCam is removed (see top image).

![Detail of ESP32Cam boards that come with a programming board that stops kilocam working due to the GND/R pin](https://github.com/ExtinctionEvent/KiloCamUltra/blob/main/images/ESP32Cam%20with%20GND:R.jpg?raw=true)


