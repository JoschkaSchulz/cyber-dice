#include <Arduino.h>
#include "esp_camera.h"
#include "wifi_config.h"
#include <WiFi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#define CAMERA_MODEL_AI_THINKER
#define FLASH_PIN 4

#include "camera_pins.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

WiFiClient client;
const int timerInterval = 30000;
unsigned long previousMillis = 0;

sensor_t * s;
camera_fb_t * fb;

const int buttonPin = 2;
int buttonState = 0;
int buttonDelay = 0;

// debugging
int lastHeap = 0;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  setupCamera();

  pinMode(buttonPin, INPUT);
  pinMode(FLASH_PIN, OUTPUT);

  setupWifi();
}


void loop() {
  // put your main code here, to run repeatedly:
  buttonState = digitalRead(buttonPin);
  if(buttonState == HIGH && buttonDelay == 0) {
    Serial.printf("Button wurde gedrückt\n");
    digitalWrite(FLASH_PIN, HIGH);
    buttonDelay = 50;
    sendPhoto();
  } else if(buttonDelay > 0) {
    reduceButtonDelay(1);
    if(buttonDelay == 0) {
      digitalWrite(FLASH_PIN, HIGH);
      delay(50);
      digitalWrite(FLASH_PIN, LOW);
    }
  }
  delay(100);
  checkHeap();
}

void checkHeap() {
  if(lastHeap != ESP.getFreeHeap()) {
    lastHeap = ESP.getFreeHeap();
    Serial.printf("Heap:  %i",ESP.getFreeHeap());
    Serial.println();
    Serial.printf("Psram: %i",ESP.getFreePsram());
    Serial.printf("---");
    Serial.println();
  }
}

void reduceButtonDelay(int step) {
  if(buttonDelay - step > 0) {
    buttonDelay -= step;
  } else {
    buttonDelay = 0;
  }
}

void setupWifi() {
  WiFi.begin(ssid, password);

  int counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    counter++;
    if(counter >= 15) {
      Serial.println();
      ESP.restart();
    }
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
 
  //startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}

void setupCamera() {
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
  //init with high specs to pre-allocate larger buffers
  config.frame_size = FRAMESIZE_UXGA;
  config.jpeg_quality = 10;
  config.fb_count = 1;

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  s = esp_camera_sensor_get();

  //initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);//flip it back
    s->set_brightness(s, 1);//up the blightness just a bit
    s->set_saturation(s, -2);//lower the saturation
  }
  //drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);
  s->set_quality(s, 40);
}

String sendPhoto() {
  String getAll;
  String getBody;
  String serverName = "192.168.178.39";
  int serverPort = 5000;
  String serverPath = "/dices";

  s->set_framesize(s, FRAMESIZE_SXGA);
  s->set_quality(s, 10);
  delay(500);

  fb = NULL;
  fb = esp_camera_fb_get();
  if(!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
  }
  
  digitalWrite(FLASH_PIN, LOW);
  Serial.println("Connecting to server: " + serverName);

  if (client.connect(serverName.c_str(), serverPort)) {
    Serial.println("Connection successful!");    
    String head = "--CyberDice\r\nContent-Disposition: form-data; name=\"dice\"; filename=\"dice.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--CyberDice--\r\n";

    uint32_t imageLen = fb->len;
    uint32_t extraLen = head.length() + tail.length();
    uint32_t totalLen = imageLen + extraLen;
  
    client.println("POST " + serverPath + " HTTP/1.1");
    client.println("Host: " + serverName);
    client.println("Content-Length: " + String(totalLen));
    client.println("Content-Type: multipart/form-data; boundary=CyberDice");
    client.println();
    client.print(head);

    Serial.print("writing buffer:");
    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
    for (size_t n=0; n<fbLen; n=n+1024) {
      Serial.print(".");
      if (n+1024 < fbLen) {
        client.write(fbBuf, 1024);
        fbBuf += 1024;
      }
      else if (fbLen%1024>0) {
        size_t remainder = fbLen%1024;
        client.write(fbBuf, remainder);
      }
    }   
    client.print(tail);

    fbBuf = NULL;
    esp_camera_fb_return(fb);
    
    int timoutTimer = 10000;
    long startTimer = millis();
    boolean state = false;

    Serial.print("\nWaiting for response:");
    while ((startTimer + timoutTimer) > millis()) {
      Serial.print(".");
      delay(100);      
      while (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (getAll.length()==0) { state=true; }
          getAll = "";
        }
        else if (c != '\r') { getAll += String(c); }
        if (state==true) { getBody += String(c); }
        startTimer = millis();
      }
      if (getBody.length()>0) { break; }
    }
    Serial.println();
    client.stop();
    Serial.println(getBody);
  }
  else {
    getBody = "Connection to " + serverName +  " failed.";
    Serial.println(getBody);
  }

  s->set_framesize(s, FRAMESIZE_QVGA);
  s->set_quality(s, 40);
  
  return getBody;
}
