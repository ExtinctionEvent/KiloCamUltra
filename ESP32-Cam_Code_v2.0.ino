// To test this code, use H120211210141436 as a payload after O for on is displayed
// This should turn on the flash and take a photo labeled IMG_20211210141436.jpg

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>
#include "esp_camera.h"
#include "Arduino.h"
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "driver/rtc_io.h"

// Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

char PAYLOAD[18];
char TIMESTAMP[15];


void setup() {

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
 
  Serial.begin(57600);
  
  //Serial.println();
  Serial.println("Booting...");
  
  pinMode(33, OUTPUT);   // GPIO for red LED flash
  pinMode(4, OUTPUT); //GPIO for LED flash

  // Get timestamp and action flag 
  Serial.println("O"); // Initiate contact with kilocam "O" for on.
  
  // Wait for response 
  while(Serial.available() == 0) {
    }; 

  // Parse message
  int i = 0;
  if(Serial.available()>0) {    
     while (Serial.available()>0){
       char inByte = Serial.read();
       delay(10);
       
       if (inByte != '\n'){
         PAYLOAD[i] = inByte;
         i++;
        }
       else {
        PAYLOAD[i] = 0;
        }
       }                   
    }

  // Generate global variables for use later
  char ACTION = PAYLOAD[1];
  strcpy(TIMESTAMP, &PAYLOAD[2]); 

  // If Action == 1 turn on LED Flash
  if (ACTION == '1') {
    Serial.println("starting LED");
    digitalWrite(4, HIGH);
    }

  // Turn on onboard red LED
  //digitalWrite(33, LOW);
  
  rtc_gpio_hold_dis(GPIO_NUM_4);  //disable pin hold if it was enabled before sleeping

  WiFi.mode(WIFI_MODE_NULL); // Disable onboard wifi antenna

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; 
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_SXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // Set camera settings (note that this can be tricky, best to leave as-is. 
  // Use ESP32CAM examples > Camera > CameraWebServer to find the best settings for your situation
  sensor_t * s = esp_camera_sensor_get();
  
  //s->set_brightness(s, -2);     // -2 to 2
  //s->set_contrast(s, 0);       // -2 to 2
  //s->set_saturation(s, 0);     // -2 to 2
  //s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
  s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
  s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
  s->set_wb_mode(s, 1);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
  s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
  //s->set_aec2(s, 0);           // 0 = disable , 1 = enable
  //s->set_ae_level(s, 0);       // -2 to 2
  //s->set_aec_value(s, 300);    // 0 to 1200
  //s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
  //s->set_agc_gain(s, 0);       // 0 to 30
  //s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
  //s->set_bpc(s, 0);            // 0 = disable , 1 = enable
  s->set_wpc(s, 1);            // 0 = disable , 1 = enable
  s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
  s->set_lenc(s, 1);           // 0 = disable , 1 = enable
  //s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
  //s->set_vflip(s, 0);          // 0 = disable , 1 = enable
  //s->set_dcw(s, 1);            // 0 = disable , 1 = enable
  //s->set_colorbar(s, 0);       // 0 = disable , 1 = enable
  
  Serial.println("Starting SD Card");
  
  if(!SD_MMC.begin("/sdcard", true)){
    Serial.println("SD Card Mount Failed");
    return;
  }
  
  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
    Serial.println("No SD Card attached");
    return;
  }  

  // Take picture and obtain camera buffer  
  camera_fb_t * fb = NULL;

  // Delay 1 second or more to improve exposure of camera
  delay(2000);
  
  // Obtain buffer 
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    return;
  };
  
  // Turn off LED flash now photo is taken
  digitalWrite(4, LOW);

  // Create Path where new picture will be saved in SD Card
  char path[24] = "/IMG_";
  strcat(path,TIMESTAMP);
  strcat(path, ".jpg");
  
  fs::FS &fs = SD_MMC; 
  Serial.printf("Picture file name: %s\n", path);

  // Create new file
  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file in writing mode");
  } 
  else {
    file.write(fb->buf, fb->len); // payload (image), payload length
  }
  file.close();
  
  //return camera frame buffer
  esp_camera_fb_return(fb);
  //Serial.printf("Picture file saved to SD as: %s\n", path.c_str()); 
  
  // Turns off the ESP-CAM red onboard led
  //digitalWrite(33, HIGH);
  
  // Turns off the ESP32-CAM white on-board LED (flash) connected to GPIO 4
  rtc_gpio_hold_en(GPIO_NUM_4);
  
  delay(50);
  Serial.println("Going to sleep nowX"); // Kilocam is looking for "X" char in Serial to initiate powerdown
  delay(50);
  Serial.flush(); 
  esp_deep_sleep_start();
}

void loop() {
  
}
