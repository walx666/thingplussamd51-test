#include "config.h"
#include "io.h"

#include <Arduino.h>
#include <stdarg.h>
#include <Wire.h>
#if UseScheduler
#include <Scheduler.h>
#endif
#include <muTimer.h>
muTimer TimerLed1 = muTimer();
muTimer TimerSensor1 = muTimer();


#include "sys.h"

#define UsedSerial Serial   /* USB */
//#define UsedSerial Serial1  /* Default Uart */

//#include "I2C_eeprom.h"
//I2C_eeprom ee(0x50, I2C_DEVICESIZE_24LC512);
//bool EEPTestOk = false;

#define USE_I2C_EEPROM

#ifdef USE_I2C_EEPROM
#include "SparkFun_External_EEPROM.h" // Click here to get the library: http://librarymanager/All#SparkFun_External_EEPROM
ExternalEEPROM myMem;
uint8_t EEPROM_ADDRESS = 0xA0; //0b1010(A2 A1 A0): A standard I2C EEPROM with the ADR0 bit set to VCC
#endif /* #ifdef USE_I2C_EEPROM */

void loopSensor();
void loopHandleUart1();

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
/*
  pinMode(MODULE_WAKEUP, OUTPUT);
  digitalWrite(MODULE_WAKEUP, LOW);

  pinMode(MODULE_RESET, OUTPUT);
  digitalWrite(MODULE_RESET, HIGH);
*/
  /*ee.begin();
  EEPTestOk = ee.isConnected();
  if (!EEPTestOk) {
    USB_VBUS_ChkDisable = true;
    Serial1.println("\r\n>ERROR! : EEPROM cannot be found!");
    //while (1);
  }*/
 #if UseScheduler
  //Scheduler.startLoop(loopSensor);
 #endif

  UsedSerial.begin(115200);
  while(!Serial) {
    delay(50);
  } 

 #ifdef USE_I2C_EEPROM
  UsedSerial.println("Qwiic EEPROM example");
  Wire.begin();

  myMem.setMemoryType(2048); // Valid types: 0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1025, 2048

  if (myMem.begin(EEPROM_ADDRESS,Wire) == false)
  {
    UsedSerial.println("No memory detected. Freezing.");
    while (1)
      ;
  }
  UsedSerial.println("Memory detected!");

  //Write a series of bytes then test to see if auto-detect changes them
  //The detection functions *should not* modify the data on the EEPROM
  uint32_t maxBytes = 300; //Ideally we write this many bytes
  if (maxBytes > myMem.getMemorySizeBytes())
    maxBytes = myMem.getMemorySizeBytes();

  for (uint32_t x = 0 ; x < maxBytes ; x++)
    myMem.write(x, (uint8_t)('0' + x));

  //  UsedSerial.print("Detected number of address bytes: ");
  //UsedSerial.println(myMem.detectAddressBytes());

  UsedSerial.print("Detected pageSizeBytes: ");
  UsedSerial.println(myMem.detectPageSizeBytes());

  //  UsedSerial.print("Detected page write time (ms): ");
  //  UsedSerial.println(myMem.detectWriteTimeMs());

  //uint32_t eepromSizeBytes = myMem.detectMemorySizeBytes();
  uint32_t eepromSizeBytes = myMem.getMemorySizeBytes();
  UsedSerial.print("Detected EEPROM size (bytes): ");
  UsedSerial.print(eepromSizeBytes);
  UsedSerial.print(" bytes / ");
  if (eepromSizeBytes < 128)
  {
    UsedSerial.print(eepromSizeBytes * 8);
    UsedSerial.print(" Bits");
  }
  else
  {
    UsedSerial.print(eepromSizeBytes * 8 / 1024);
    UsedSerial.print(" kBits");
  }
  UsedSerial.print(" - 24XX");
  if (eepromSizeBytes == 16)
    UsedSerial.print("00");
  else
  {
    if ((eepromSizeBytes * 8 / 1024) < 10) UsedSerial.print("0");
    UsedSerial.print(eepromSizeBytes * 8 / 1024);
  }
  UsedSerial.println();

  UsedSerial.print("Checking memory: ");
  bool allTestsPassed = true;
  for (uint32_t x = 0 ; x < maxBytes ; x++)
  {
    byte readValue = myMem.read(x);
    if (readValue != (uint8_t)('0' + x))
    {
      UsedSerial.print("Error detected in location: ");
      UsedSerial.print(x);
      allTestsPassed = false;
      break;
    }
  }

  if (allTestsPassed == true)
    UsedSerial.print("Good");
  UsedSerial.println();

  //Do basic read/write tests
  byte myValue1 = 200;
  myMem.write(0, myValue1); //(location, data)

  byte myRead1 = myMem.read(0);
  UsedSerial.print("I read (should be 200): ");
  UsedSerial.println(myRead1);

  int myValue2 = -366;
  myMem.put(10, myValue2); //(location, data)
  int myRead2;
  myMem.get(10, myRead2); //location to read, thing to put data into
  UsedSerial.print("I read (should be -366): ");
  UsedSerial.println(myRead2);

  if (myMem.getMemorySizeBytes() > 16)
  {
    float myValue3 = -7.35;
    myMem.put(20, myValue3); //(location, data)
    float myRead3;
    myMem.get(20, myRead3); //location to read, thing to put data into
    UsedSerial.print("I read (should be -7.35): ");
    UsedSerial.println(myRead3);

    String myString = "Hi, I am just a simple test string"; //34 characters
    unsigned long nextEEPROMLocation = myMem.putString(70, myString);

    String myRead4 = "";
    myMem.getString(70, myRead4);
    UsedSerial.print("I read: ");
    UsedSerial.println(myRead4);
    UsedSerial.print("Next available EEPROM location: ");
    UsedSerial.println(nextEEPROMLocation);
  }
 #endif /*  #ifdef USE_I2C_EEPROM */
 #if UseScheduler
  //Scheduler.startLoop(loopHandleUart1);
 #endif
}

 #if UseScheduler
void loopHandleUart1() {
  yield();
 #else
void yield() {
  /* digitalWrite(LED_BUILTIN, TimerLed1.cycleOnOff(100, 500)); */
 #endif
  if (UsedSerial.available() > 0) {
    uint8_t rxdata = UsedSerial.read();
    prntf(Serial, " %02x", rxdata);
  }
}


void loopSensor(void) {
  static bool StateLED=HIGH;
  //digitalWrite(LED_BUILTIN, TimerLed1.cycleOnOff(100, 500));

  if (TimerSensor1.cycleTrigger(1000)) {
    digitalWrite(LED_BUILTIN, StateLED);
    if (StateLED==HIGH)
      StateLED=LOW;
    else  
      StateLED=HIGH;
  }
 #if UseScheduler
  delay(500);
  yield();
 #endif
}


void loop() {
  uint8_t seru_rxdata;

  if (UsedSerial.available() > 0) {
    seru_rxdata = UsedSerial.read();
    //prntf(UsedSerial, "%c", seru_rxdata);

    switch (seru_rxdata) {
      /*case 'i':
        prntf(UsedSerial, "\r\n%c:UsedSerial",seru_rxdata);
      break;*/
      case 'i' : {
      } break;
      case 'I': {
      } break;
      //
      case 'S' : {
      } break;
      //
      case 'r' : {
        prntf(UsedSerial,"\r\nReset: ok");
      } break;
      case 'R': {
      } break;
      case 'w' : {
        /*digitalWrite(MODULE_WAKEUP, LOW);
        delay(5);
        digitalWrite(MODULE_WAKEUP, HIGH); */
      } break;
      default: {
        prntf(UsedSerial, "\r\n%c", seru_rxdata);
      } break;
    }
  }
 #if UseScheduler
  delay(100);
 #else
  loopSensor();
 #endif
}

