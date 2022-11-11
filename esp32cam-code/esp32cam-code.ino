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

ESP32QRCodeReader reader(CAMERA_MODEL_AI_THINKER);
struct QRCodeData qrCodeData;

int pictureNumber = 0;


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
      String path = "/image" + String(pictureNumber++) + ".jpg";
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
