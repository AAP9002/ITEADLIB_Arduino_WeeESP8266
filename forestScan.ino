/*
 *
 *
 *
 * Camera source code taken from
 *  camera.ino - Simple camera example sketch
 *  Copyright 2018, 2022 Sony Semiconductor Solutions Corporation
 */

#include <SDHCI.h>
#include <stdio.h> /* for sprintf */

#include <Camera.h>

#define TOTAL_PICTURE_COUNT     (10)

#include "ESP8266.h"

#define SSID        "Zacphone"
#define PASSWORD    "12345678"
#define HOST_NAME   "172.16.5.12"
#define HOST_PORT   (8090)

ESP8266 wifi(Serial1);

#define BAUDRATE (115200)

SDClass theSD;
int take_picture_count = 0;

/**
 * Print error message
 */

void printError(enum CamErr err) {
  Serial.print("Error: ");
  switch (err) {
    case CAM_ERR_NO_DEVICE:
      Serial.println("No Device");
      break;
    case CAM_ERR_ILLEGAL_DEVERR:
      Serial.println("Illegal device error");
      break;
    case CAM_ERR_ALREADY_INITIALIZED:
      Serial.println("Already initialized");
      break;
    case CAM_ERR_NOT_INITIALIZED:
      Serial.println("Not initialized");
      break;
    case CAM_ERR_NOT_STILL_INITIALIZED:
      Serial.println("Still picture not initialized");
      break;
    case CAM_ERR_CANT_CREATE_THREAD:
      Serial.println("Failed to create thread");
      break;
    case CAM_ERR_INVALID_PARAM:
      Serial.println("Invalid parameter");
      break;
    case CAM_ERR_NO_MEMORY:
      Serial.println("No memory");
      break;
    case CAM_ERR_USR_INUSED:
      Serial.println("Buffer already in use");
      break;
    case CAM_ERR_NOT_PERMITTED:
      Serial.println("Operation not permitted");
      break;
    default:
      break;
  }
}

/**
 * Callback from Camera library when video frame is captured.
 */

void CamCB(CamImage img) {

  /* Check the img instance is available or not. */

  if (img.isAvailable()) {

    /* If you want RGB565 data, convert image data format to RGB565 */

    img.convertPixFormat(CAM_IMAGE_PIX_FMT_RGB565);

    /* You can use image data directly by using getImgSize() and getImgBuff().
       * for displaying image to a display, etc. */

    Serial.print("Image data size = ");
    Serial.print(img.getImgSize(), DEC);
    Serial.print(" , ");

    Serial.print("buff addr = ");
    Serial.print((unsigned long)img.getImgBuff(), HEX);
    Serial.println("");
  } else {
    Serial.println("Failed to get video stream image");
  }
}

/**
 * @brief Initialize camera
 */
void setup() {

  /**
Set up wifi
*/
  Serial.begin(9600);
    Serial.print("setup begin\r\n");
    
    Serial.print("FW Version:");
    Serial.println(wifi.getVersion().c_str());
      
    if (wifi.setOprToStationSoftAP()) {
        Serial.print("to station + softap ok\r\n");
    } else {
        Serial.print("to station + softap err\r\n");
    }
 
    if (wifi.joinAP(SSID, PASSWORD)) {
        Serial.print("Join AP success\r\n");
        Serial.print("IP:");
        Serial.println( wifi.getLocalIP().c_str());       
    } else {
        Serial.print("Join AP failure\r\n");
    }
    
    if (wifi.disableMUX()) {
        Serial.print("single ok\r\n");
    } else {
        Serial.print("single err\r\n");
    }
    
    Serial.print("setup end\r\n");
 /**
Set up wifi end
*/

  CamErr err;

  /* Open serial communications and wait for port to open */

  Serial.begin(BAUDRATE);
  while (!Serial) {
    ; /* wait for serial port to connect. Needed for native USB port only */
  }

  /* Initialize SD */
  while (!theSD.begin()) {
    /* wait until SD card is mounted. */
    Serial.println("Insert SD card.");
  }

  /* begin() without parameters means that
   * number of buffers = 1, 30FPS, QVGA, YUV 4:2:2 format */

  Serial.println("Prepare camera");
  err = theCamera.begin();
  if (err != CAM_ERR_SUCCESS) {
    printError(err);
  }

  /* Start video stream.
   * If received video stream data from camera device,
   *  camera library call CamCB.
   */

  Serial.println("Start streaming");
  err = theCamera.startStreaming(true, CamCB);
  if (err != CAM_ERR_SUCCESS) {
    printError(err);
  }

  /* Auto white balance configuration */

  Serial.println("Set Auto white balance parameter");
  err = theCamera.setAutoWhiteBalanceMode(CAM_WHITE_BALANCE_DAYLIGHT);
  if (err != CAM_ERR_SUCCESS) {
    printError(err);
  }

  /* Set parameters about still picture.
   * In the following case, QUADVGA and JPEG.
   */

  Serial.println("Set still picture format");
  err = theCamera.setStillPictureImageFormat(
    CAM_IMGSIZE_QUADVGA_H,
    CAM_IMGSIZE_QUADVGA_V,
    CAM_IMAGE_PIX_FMT_JPG);
  if (err != CAM_ERR_SUCCESS) {
    printError(err);
  }
}

/**
 * @brief Take picture with format JPEG per second
 */

void loop()
{
  uint8_t buffer[128] = {0};
    
    if (wifi.createTCP(HOST_NAME, HOST_PORT)) {
        Serial.print("create tcp ok\r\n");
    } else {
        Serial.print("create tcp err\r\n");
    }
    
    char *hello = "Hello, this is client!";
    wifi.send((const uint8_t*)hello, strlen(hello));
    
    uint32_t len = wifi.recv(buffer, sizeof(buffer), 10000);
    if (len > 0) {
        Serial.print("Received:[");
        for(uint32_t i = 0; i < len; i++) {
            Serial.print((char)buffer[i]);
        }
        Serial.print("]\r\n");
    }
    
    if (wifi.releaseTCP()) {
        Serial.print("release tcp ok\r\n");
    } else {
        Serial.print("release tcp err\r\n");
    }
    delay(5000);
  sleep(1); /* wait for one second to take still picture. */

  /* You can change the format of still picture at here also, if you want. */

  /* theCamera.setStillPictureImageFormat(
   *   CAM_IMGSIZE_HD_H,
   *   CAM_IMGSIZE_HD_V,
   *   CAM_IMAGE_PIX_FMT_JPG);
   */

  /* This sample code can take pictures in every one second from starting. */

  if (take_picture_count < TOTAL_PICTURE_COUNT)
    {

      /* Take still picture.
      * Unlike video stream(startStreaming) , this API wait to receive image data
      *  from camera device.
      */
  
      Serial.println("call takePicture()");
      CamImage img = theCamera.takePicture();

      /* Check availability of the img instance. */
      /* If any errors occur, the img is not available. */

      if (img.isAvailable())
        {
          /* Create file name */
    
          char filename[16] = {0};
          sprintf(filename, "PICT%03d.JPG", take_picture_count);
    
          Serial.print("Save taken picture as ");
          Serial.print(filename);
          Serial.println("");

          /* Remove the old file with the same file name as new created file,
           * and create new file.
           */

          theSD.remove(filename);
          File myFile = theSD.open(filename, FILE_WRITE);
          myFile.write(img.getImgBuff(), img.getImgSize());
          myFile.close();
        }
      else
        {
          /* The size of a picture may exceed the allocated memory size.
           * Then, allocate the larger memory size and/or decrease the size of a picture.
           * [How to allocate the larger memory]
           * - Decrease jpgbufsize_divisor specified by setStillPictureImageFormat()
           * - Increase the Memory size from Arduino IDE tools Menu
           * [How to decrease the size of a picture]
           * - Decrease the JPEG quality by setJPEGQuality()
           */

          Serial.println("Failed to take picture");
        }
    }
  else if (take_picture_count == TOTAL_PICTURE_COUNT)
    {
      Serial.println("End.");
      theCamera.end();
    }

  take_picture_count++;
}
