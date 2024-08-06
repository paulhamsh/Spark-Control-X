// USES SPARK BOX TO EMULATE A SPARK CONTROL
// Library dependenct: NimBLE 1.3.8
// Edit NimBLEDevice.cpp
// Probably here: C:\Users\XXXXX\Documents\Arduino\libraries\NimBLE-Arduino\src\NimBLEDevice.cpp

/*
  #ifdef SPARK_CONTROL
  uint8_t                     NimBLEDevice::m_own_addr_type = BLE_OWN_ADDR_RANDOM;
  #else
  uint8_t                     NimBLEDevice::m_own_addr_type = BLE_OWN_ADDR_PUBLIC;
  #endif
*/

// Set this if you have switches attached to your GPIOS
#define USE_GPIOS

// ACTIVE_HIGH define should be set if your pedal sets GPIOs to +ve when switch pressed 
//    uint8_t SCswitchPins[]{33,27,14,26}; 
// for your GPIO setup
// note SCswitchPins is in a strange order - [0][2][1][3]
//#define ACTIVE_HIGH

// Set your board type- if nothing set then generic ESP32 is assumed
//#define HELTEC
//#define M5CORE
//#define M5CORE2
//#define M5STICK


#ifdef HELTEC
#include "heltec.h"
#endif

#ifdef M5CORE2
#include <M5Core2.h>
#endif

#ifdef M5CORE
#include <M5Stack.h>
#endif

#ifdef M5STICK
#include <M5StickC.h>
#endif

void SparkControlStart();
void InitialiseGPIO();
void SparkControlLoop();
bool spark_box;

void setup() {
// general setup
#ifdef HELTEC
  Heltec.begin(true , false , true );
  Heltec.display->setFont(ArialMT_Plain_24); // 10, 16 and 24
  Heltec.display->clear();
  Heltec.display->drawString(20, 0,  " Spark");
  Heltec.display->drawString(20, 30, "Control");  
  Heltec.display->display();
#elif defined M5CORE2 || defined M5CORE || defined M5STICK
  M5.begin();
#ifdef M5CORE
  M5.Power.begin();
#endif
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.println("Spk Control");
#else
  //Serial.begin(115200);
#endif

  Serial.begin(115200);
  delay(400); 
  
  InitialiseGPIO();
  SparkControlStart();

  Serial.println("=========================================");
  Serial.println("Started");
}

void loop() {
  SparkControlLoop();
}
