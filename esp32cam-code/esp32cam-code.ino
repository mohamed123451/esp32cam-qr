#include <Arduino.h>
#include <ESP32QRCodeReader.h>


// Camera libraries
#include "esp_camera.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "driver/rtc_io.h"
 
// MicroSD Libraries
#include "FS.h"
#include "SD_MMC.h"

//time libraries
#include <WiFi.h>
#include "time.h"


const char* ssid     = "TEdata8D4EBC";
const char* password = "21916283";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;


ESP32QRCodeReader reader(CAMERA_MODEL_AI_THINKER);
struct QRCodeData qrCodeData;


String printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    return "Failed to obtain time";
  }
  
  char timeYYYYMMDD[11];
  strftime(timeYYYYMMDD,11, "%F", &timeinfo);
  Serial.println(timeYYYYMMDD);

  char timeHH[3];
  strftime(timeHH,3, "%H", &timeinfo);
  char timeMM[3];
  strftime(timeMM,3, "%M", &timeinfo);

  String path = "/" + String(timeYYYYMMDD) +"-"+ String(timeHH)+ +"-"+ String(timeMM)+"QR"+ ".jpg";
  return path;
  
}

void initMicroSDCard() {
  // Start the MicroSD card
 
  Serial.println("Mounting MicroSD Card");
  if (!SD_MMC.begin()) {
    Serial.println("MicroSD Card Mount Failed");
    return;
  }
  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No MicroSD Card found");
    return;
  }
 
}
 


void takeNewPhoto(String path) {
  // Take Picture with Camera
 
  // Setup frame buffer
  camera_fb_t  * fb = esp_camera_fb_get();
 
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }


  if(fb->format != PIXFORMAT_JPEG){
    bool jpeg_converted = frame2jpg(fb, 80, &fb->buf, &fb->len);
    ESP_LOGI(TAG, "Converted into JPEG");
    if(!jpeg_converted){
      ESP_LOGI(TAG, "JPEG compression failed");
    }
  }

 
  // Save picture to microSD card
  fs::FS &fs = SD_MMC;
  File file = fs.open(path.c_str(), FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file in write mode");
  }
  else {
    file.write(fb->buf, fb->len); // payload (image), payload length
    Serial.printf("Saved file to path: %s\n", path.c_str());
  }

  
  // Close the file
  file.close();
 
  // Return the frame buffer back to the driver for reuse
  esp_camera_fb_return(fb);
}



void setup() {
  Serial.begin(115200);
  Serial.println();

  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  
  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  String path= printLocalTime();

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  reader.setup();
  Serial.println("Setup QRCode Reader");

  reader.begin();
  Serial.println("Begin QR Code reader");


  // Initialize the MicroSD
  Serial.print("Initializing the MicroSD card module... ");
  initMicroSDCard();
  

  
}

void loop() {
  if (reader.receiveQrCode(&qrCodeData, 100))
  {
    Serial.println("Found QRCode");
    if (qrCodeData.valid)
    {
      Serial.print("Payload: ");
      Serial.println((const char *)qrCodeData.payload);
      // Path where new picture will be saved in SD Card
      String path= printLocalTime();
      Serial.printf("Picture file name: %s\n", path.c_str());
      
      // Take and Save Photo
      takeNewPhoto(path);
      delay(1000);
    }
    else
    {
      Serial.print("Invalid: ");
      Serial.println((const char *)qrCodeData.payload);
    }
  }
  delay(300);

}
