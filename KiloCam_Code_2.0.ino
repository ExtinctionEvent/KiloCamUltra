
/*
WELCOME TO THE CODE FOR KILOCAM v2.0

This code is based on a collection of forum posts at Arduino.cc, along with information
on the DS3232RTC.h library. This updated code enables users to do a couple things...
- Set a specific start time and date for the camera to first boot up after deployment.
KiloCam will remain inactive and in a low-power sleep state (0.14 mA) until this start time/date occurs. 
- Set a sampling frequency (in seconds) of how often to collect data AFTER the start time/date has occurred.  
- Define two activity states a day time activity state with a specific sampling frequency, and a dark state activity with a specific frequency
- Define a range of hours, for example 09:00 (represented as "9") to 14:00 (represented as "14"), that represent
the times at which behvaiour changes between the two states. 
This is helpful if, for example, you want videos to flash the LED at night but not during the day.

Instructions to program your KiloCam: 
1. Search for "STEP 1:", go to the following line: "RTC.setAlarm(ALM1_MATCH_HOURS, 15, 45, 20, 0);"

  Adjust the time/date above as needed. Above, 45 represents minutes, 20 represents hours, 
  and 15 represents seconds. So KiloCam will initiate its capture interval from the next time it encounters 
  20:45:15 (8:45:15 PM) after programming. If the sampling interval is set to 3600 (one hour in seconds), then the second 
  data capture will occur at 21:45:15 or (9:45:15 PM).

  Alternatively, replace the line with "RTC.setAlarm(ALM1_MATCH_DATE, 48, 14, 10);", which would allow you to set KiloCam to activate on
  the tenth day of the month, at 14 hours and 48 minutes. So if KiloCam is programmed on September 1st, it will not
  activate until September 10th at 14:48 (2:48 PM). This is useful if you want to deploy cameras well before data capture. 
  Note that the low-power sleep state of KiloCam is approximately 0.14 mA prior to the first activation and between scheduled data captures.

2. Search for "STEP 2:", go to the following line: "unsigned long alarmInterval = 3600;"

  Adjust the value to whatever sampling interval, in seconds, you want between KiloCam data collections during the day period.
  For example, one hour is 3600 seconds, six hours is 21600 seconds, twelve hours is 43200 seconds, twenty-four hours is 86400 seconds.
  When writing your value, do not include any commas for values in the thousands etc. 
  Repeated sampling by KiloCam will occur at this interval AFTER the initial alarm time (Step 1). So, if 
  the initial alarm is set for September 3 at 14:00, and we have an interval of 180 seconds (3 minutes), then
  KiloCam will collect data at 14:00, 14:03, 14:06, 14:09, 15:02, etc until it runs out of camera battery. 

  NOTE: The shortest reccomended capture frequency is 10 seconds. Current while capturing an image is approximately 50 mA but 
  may spike as high as 300 mA.  

  Adjust the "unsigned long darkAlarmInterval = 3600000" to the sampling frequency during the dark period, as above. 

3. Search for "unsigned int Dawn =" or "unsigned int Dusk ="

  Adjust these values to represent the earliest and latest hours of the day (respectively) that you want 
  KiloCam to capture data. Setting Dawn = 0 and Dusk = 24 with a 3600 second sampling interval (1 hour) will 
  enable KiloCam to capture data every hour, 24 hours a day.Setting Dawn = 8 and Dusk = 18
  with a 7200 sampling interval will enable KiloCam to capture data every 2 hours from 08:00 to 18:00 each day.

  These times will reflect the time at which kilocams behaviour changes. If you want the original behaviour please 
  comment out the lines in the else statement below 

4. BEFORE UPLOADING! 
  In the Arduino IDE under "Tools" set the Board to "Arduino Pro or Pro Mini", set the Processor to "ATMEGA328P 3.3V 8MHz). 
  Plug in your USB FTDI programmer, and select the appropriate serial port under "Tools" in the Arduino IDE. 
  Set your FTDI programmer to 3.3V by adjusting the jumper, located near the pins on most FTDI boards. 
  Attach your FTDI programmer to the KiloCam PCB so that the GND and DTR pins on the programmer align with those marked on the PCB. 
  Hold the FTDI programmer gently in place to maintain a good connection, then hit "Upload" in the Arduino IDE. 
  Wait until the Arduino IDE reports "Done Uploading" before you disconnect the FTDI programmer from the KiloCam PCB.  
  
NOTES: 
- Before attaching your ESP32-CAM to the KiloCam PCB, make sure you have uploaded the "ESP32CAM_PictureOnReset" code to your ESP32-CAM board.
- When programming KiloCam you should have a serial monitor open and set to 57600 baud. KiloCam will 
report back the current time, make sure this is correct before deploying. If it is not, use the "SetTime" or "SetTime_serial" 
examples from the Arduino RTCLib library to reprogram your PCB's real-time clock.     
- Power consumption of the KiloCam PCB is low, about 0.14 mA when sleeping and 5 mA when active, and 50 mA when capturing a photo. 
Be mindful of how long you can have KiloCam deployed based on your chosen power source and capture schedule(see above).
- Please make sure you have the DS3232RTC and LowPower libraries installed before attempting to load this
code onto your KiloCam. If you do not have these installed from the github directories linked below, the code will fail. 
- For more information on how to get more creative with your initial KiloCam activation time or repeated 
sampling invervals, check out the DS3232RTC library repository on Github at the links below.
- You must set the time of your KiloCam PCB real-time clock before  deployment. Do this
by running the SetTime_serial script (and following its instructions) before loading your KiloCam deployment
code.  The RTC enables files to be labeled with a specific timestamp.
   
*/

#include <DS3232RTC.h>        // https://github.com/JChristensen/DS3232RTC
#include <LowPower.h>         // https://github.com/rocketscream/Low-Power
#include <Wire.h>
#include <SPI.h>

// Defined constants
#define RTC_ALARM_PIN 2 //pin for interrupt 

// Object instantiations
time_t          t, alarmTime;
tmElements_t    tm;

//  Global variable declarations
volatile bool alarmIsrWasCalled = true;    // DS3231 RTC alarm interrupt service routine (ISR) flag. Set to true to allow first iteration of loop to take place and sleep the system.
unsigned int ACTION = 1;
unsigned long alarmInt;

// User defined global variable declarations
unsigned long alarmInterval = 300;          // STEP 2: Set sleep duration (in seconds) between data samples ... 300 = 5min
unsigned long darkAlarmInterval = 3600;  // Set Dark period sleep duration 3600 = 1 hr
unsigned int Dawn = 6; // Specify the earliest hour (24 hour clock) you want captures to occur in. Or the time to move to dark action
unsigned int Dusk = 17; // Specify the latest hour (24 hour clock) you want captures to occur in Or the time to move to day action

void setup()
{
  // DS3231 Real-time clock (RTC)
  RTC.setAlarm(ALM1_MATCH_DATE, 0, 0, 0, 1);    // Initialize alarm 1 to known value
  RTC.setAlarm(ALM2_MATCH_DATE, 0, 0, 0, 1);    // Initialize alarm 2 to known value
  RTC.alarm(ALARM_1);                           // Clear alarm 1 interrupt flag
  RTC.alarm(ALARM_2);                           // Clear alarm 2 interrupt flag
  RTC.alarmInterrupt(ALARM_1, false);           // Disable interrupt output for alarm 1
  RTC.alarmInterrupt(ALARM_2, false);           // Disable interrupt output for alarm 2
  RTC.squareWave(SQWAVE_NONE);                  // Configure INT/SQW pin for interrupt operation by disabling default square wave output

  // Initialize serial monitor
  Serial.begin(57600);

  // Initializing shield pins
  pinMode(5, OUTPUT); // Pin D5 controlling power to ESP32-CAM
  digitalWrite(5, LOW); // Set D5 to LOW, power to ESP32-CAM is off
  
  pinMode(RTC_ALARM_PIN, INPUT);           // Enable internal pull-up resistor on external interrupt pin
  digitalWrite(RTC_ALARM_PIN, HIGH);
  attachInterrupt(0, alarmIsr, FALLING);       // Wake on falling edge of RTC_ALARM_PIN

  // Set initial RTC alarm
  RTC.setAlarm(ALM1_MATCH_HOURS, 0, 0, 6, 0);  // STEP 1: Set initial alarm 1 here. Camera inactive until this time/date.
  RTC.alarm(ALARM_1);                             // Ensure alarm 1 interrupt flag is cleared
  RTC.alarmInterrupt(ALARM_1, true);              // Enable interrupt output for alarm

  // flash two short times to show KiloCam is powered on
  digitalWrite(13, HIGH);
  delay(250); 
  digitalWrite(13, LOW);   
  delay(250); 
  digitalWrite(13, HIGH);
  delay(250); 
  digitalWrite(13, LOW);   
  delay(250); 
    
  runCamera(); // do a demo photo cycle before going to sleep to verify all buttons work. Comment out this line with "//" to disable
}

// Loop
void loop()
{
  if (alarmIsrWasCalled)
  {
    t = RTC.get();            // Read current date and time from RTC
    if (RTC.alarm(ALARM_1))   // Check alarm 1 and clear flag if set
    { 
      if  (hour(t) >= Dawn && hour(t) <= Dusk) { //Only run camera for capture if current RTC time is between dawn and dusk
          ACTION = 0; // Turns LED Flash off
          runCamera();
          alarmInt = alarmInterval;
      }
      else {
        // ********** Comment out the below if you do not want dark action *********
        ACTION = 1; // Turns LED Flash on
        runCamera();
        alarmInt = darkAlarmInterval;
      }
      
      // Set alarm
      alarmTime = t + alarmInt;  // Calculate next alarm
      RTC.setAlarm(ALM1_MATCH_DATE, second(alarmTime), minute(alarmTime), hour(alarmTime), day(alarmTime));   // Set alarm

      // Check if alarm was set in the past
      if (RTC.get() >= alarmTime)
      {
        t = RTC.get();                    // Read current date and time from RTC
        alarmTime = t + alarmInt;    // Calculate next alarm
        RTC.setAlarm(ALM1_MATCH_DATE, second(alarmTime), minute(alarmTime), hour(alarmTime), day(alarmTime));
        RTC.alarm(ALARM_1);               // Ensure alarm 1 interrupt flag is cleared
      }
    }
    
    alarmIsrWasCalled = false;  // Reset RTC ISR flag
    goToSleep();                // Sleep
  }
}

// Enable sleep and await RTC alarm interrupt
void goToSleep()
{
  Serial.println(F("Going to sleep..."));
  Serial.flush();
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);  // Enter sleep and await external interrupt 
}

// Real-time clock alarm interrupt service routine (ISR)
void alarmIsr()
{
  alarmIsrWasCalled = true;
}

// Sends a package of data, H- Header, Action (int) and date/time
void sendDateTime(time_t t)
{ 
  Serial.print('H'); Serial.print(ACTION);
  Serial.print(year(t), DEC); 
  Serial.print((month(t) < 10) ? "0" : ""); Serial.print(month(t), DEC); 
  Serial.print((day(t) < 10) ? "0" : ""); Serial.print(day(t), DEC); 
  Serial.print((hour(t) < 10) ? "0" : ""); Serial.print(hour(t), DEC); 
  Serial.print((minute(t) < 10) ? "0" : ""); Serial.print(minute(t), DEC);
  Serial.print((second(t) < 10) ? "0" : ""); Serial.print(second(t), DEC); Serial.print('\n');
}

// Funtion to run the camera
void runCamera() {
  
  // flash two short times to show unit is awake
    t = RTC.get();  
    Serial.println(F("KiloCam awake, capturing..."));
    delay(50);
    
    Serial.flush();
    delay(1);
    
    digitalWrite(5, HIGH);   // Pull pin 5 high to power on ESP32-CAM

    // Wait for ESP32 to wake and send "O"n signal
    while(Serial.read() != 'O') {
    };

    // Send the payload containing action and date time stamp
    sendDateTime(t);

    // Wait for ESP32 to finish and send Off "X" signal
    while(Serial.read() != 'X') {
    };
    
    delay(500); // Give chance for ESP to enter sleep before power off
    digitalWrite(5, LOW);    // Pull pin 5 low to power off ESP32-CAM 
    
    //digitalWrite(13, HIGH);  // Flash the blue LED to show cycle has completed.
    //delay(500);
    //digitalWrite(13, LOW);
    
    delay(500); // wait 1 seconds before powering down KiloCam
}
