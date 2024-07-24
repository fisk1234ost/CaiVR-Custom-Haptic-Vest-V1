//VestCode project by fisk1234ost (https://github.com/fisk1234ost)
//based on CaiVR's Custom Haptic Vest V1  (Raspi Vest Script) 
//(https://github.com/CaiVR/CaiVR-Custom-Haptic-Vest-V1/tree/main/Raspi%20Vest%20Script)

// Please include ArduinoOSCWiFi.h to use ArduinoOSC on the platform
// which can use both WiFi and Ethernet
#include <ArduinoOSCWiFi.h>
// this is also valid for other platforms which can use only WiFi
// #include <ArduinoOSC.h>

#include "PCA9685.h"


// WiFi stuff
const char* ssid = "your-ssid";
const char* pwd = "your-password";
// for ArduinoOSC
const int recv_port = 1025;

PCA9685 pwmFront(B000000);
PCA9685 pwmBack(B000001);
// Not a real device, will act as a proxy to pwmFront and pwmBack, using all-call i2c address 0xE0, and default Wire @400kHz
PCA9685 pwmControllerAll(PCA9685_I2C_DEF_ALLCALL_PROXYADR);

//Motor Index Mapping (index used to send motor data to correct motor)
int motorMap [] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31}; //Index Mapping (index used to send motor data to correct motor)


void setup() {

  Serial.begin(115200);

  Wire.begin();
    pwmControllerAll.resetDevices();    // Resets all PCA9685 devices on i2c line
    pwmFront.init();
    pwmBack.init();

  delay(2000);

  Serial.println("Program: VestCode");

  // WiFi stuff (no timeout setting for WiFi)
#if defined(ESP_PLATFORM) || defined(ARDUINO_ARCH_RP2040)
#ifdef ESP_PLATFORM
  WiFi.disconnect(true, true);  // disable wifi, erase ap info
#else
  WiFi.disconnect(true);  // disable wifi
#endif
  delay(1000);
  WiFi.mode(WIFI_STA);
#endif
  WiFi.begin(ssid, pwd);

  while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
#ifdef ARDUINO_UNOR4_WIFI
      static int count = 0;
      if (count++ > 20) {
          Serial.println("WiFi connection timeout, retry");
          WiFi.begin(ssid, pwd);
          count = 0;
      }
#endif
  }
  Serial.print("WiFi connected, IP = ");
  Serial.println(WiFi.localIP());
  // subscribe osc messages
  OscWiFi.subscribe(recv_port, "/h", onOscReceived);
  Serial.print("starting server");

//ignore this, its literally just the startup chime
  pwmFront.setAllChannelsPWM(4096);
  pwmBack.setAllChannelsPWM(4096);
  delay(500);
  pwmFront.setAllChannelsPWM(0);
  pwmBack.setAllChannelsPWM(0);
  delay(50);
  pwmFront.setAllChannelsPWM(floatToDuty(0.6));
  pwmBack.setAllChannelsPWM(floatToDuty(0.6));
  delay(150);
  pwmFront.setAllChannelsPWM(0);
  pwmBack.setAllChannelsPWM(0);
  delay(100);
  pwmFront.setAllChannelsPWM(4096);
  pwmBack.setAllChannelsPWM(4096);
  delay(500);
  pwmFront.setAllChannelsPWM(0);
  pwmBack.setAllChannelsPWM(0);
}

uint16_t floatToDuty(float e){
  return static_cast<uint16_t>(e*4096);
}

void onOscReceived(const OscMessage& m) {
  handle_values(m.arg<String>(0));
}


void handle_values(String args){
  uint16_t valArray [32];
  
  String temp = "";
  int index = 0;
  for (int i = 0; i < args.length(); i++)
  {
    if (args[i] != ',')
    {
      temp += args[i];
    }
    else{
    valArray[index] = floatToDuty(temp.toFloat());
      index++;
      temp = "";
    }
  }
  valArray[index] = floatToDuty(temp.toFloat());
  for (int i = 0; i < 16; i++) {
    pwmFront.setChannelPWM(i, valArray[i]);
  }
  for (int i = 0; i < 16; i++) {
    pwmBack.setChannelPWM(i, valArray[i+16]);
  }
}

void loop() {
    OscWiFi.parse(); // to receive osc
}
