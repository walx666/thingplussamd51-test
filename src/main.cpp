#include "config.h"
#include "io.h"

//#define UseScheduler
#define UseSPIFlash

#include <Arduino.h>
#include <stdarg.h>
#include <Wire.h>
#ifdef UseScheduler
#include <Scheduler.h>
#endif
#include <muTimer.h>
muTimer TimerLed1 = muTimer();
muTimer TimerSensor1 = muTimer();

#include "sys.h"

#define UsedSerial Serial   /* USB */
//#define UsedSerial Serial1  /* Default Uart */

#ifdef UseEEPI2C
#define EEPADDR_A0 0x50   //0b1010(A2 A1 A0): A standard I2C EEPROM with the ADR0 bit set to VCC
#define EEPADDR_A8 0x53
#define EEPADDR_AE 0x57
#define GETADR(MGADR, DEVADR) ((MGADR >> 16) | DEVADR)
 #if defined(UseEEPI2C) && (UseEEPI2C == LIB)
#include "SparkFun_External_EEPROM.h" // Click here to get the library: http://librarymanager/All#SparkFun_External_EEPROM
 #else
#include "External_EEPROM.h"
 #endif
uint8_t eepaddr=EEPADDR_A0;
uint8_t DetectI2CEeprom (Stream & mydev, uint8_t *eepaddr, ExternalEEPROM *eep=nullptr);
#include <SoftWire.h>
SoftWire sw(SDA, SCL);
// These buffers must be at least as large as the largest read or write you perform.
char swTxBuffer[32];
char swRxBuffer[32];

uint8_t DetectI2CEepromSW (Stream & mydev, uint8_t *eepaddr, SoftWire *swire);
#endif /* #ifdef UseEEPI2C */

#define TWI_CLOCK_100KHZ  100000
#define TWI_CLOCK_400KHZ  400000
#define TWI_CLOCK_1MHZ    1000000
 #ifdef UseSPIFlash
#include "SparkFun_SPI_SerialFlash.h" //Click here to get the library: http://librarymanager/All#SparkFun_SPI_SerialFlash
 #if defined(ARDUINO_ARCH_SAMD) && defined(ARDUINO_SAMD51_MICROMOD)
#define FLASH_SPI       SPI1
#define PIN_FLASH_CS    SS1
#define PIN_FLASH_WP    15
#define PIN_FLASH_HOLD  16
 #endif
 #if defined(ARDUINO_ARCH_SAMD) && defined(ARDUINO_SAMD51_THING_PLUS)
#define FLASH_SPI       SPI1
#define PIN_FLASH_CS    FLASH_SS
 #endif
 #if defined(ARDUINO_ARCH_SAMD) && defined(ARDUINO_SAMD_MKRWAN1310)
#define FLASH_SPI       SPI1
#define PIN_FLASH_CS    FLASH_CS
 #endif
 #if defined(ARDUINO_ARCH_SAMD) && defined(ARDUINO_QTPY_M0)

//#define FLASH_SPI       SPI1
//#define PIN_FLASH_CS    FLASH_CS
 #endif
 #if defined(ARDUINO_ARCH_SAMD) && defined(EXTERNAL_FLASH_USE_SPI) && defined(EXTERNAL_FLASH_USE_CS)
#define FLASH_SPI       EXTERNAL_FLASH_USE_SPI  /* SPI1 */
#define PIN_FLASH_CS    EXTERNAL_FLASH_USE_CS
 #endif
 #if defined(ARDUINO_ARCH_STM32) && defined(ARDUINO_SPARKFUN_MICROMOD_F405)
SPIClass SPIF(FLASH_SDI,FLASH_SDO,FLASH_SCK,FLASH_CS);
//SPIClass SPIF(FLASH_SDO,FLASH_SDI,FLASH_SCK,FLASH_CS);
#define FLASH_SPI       SPI /* SPIF */
#define PIN_FLASH_CS    FLASH_CS /* SS1 */
 #endif
 #if defined(ARDUINO_ARCH_STM32) && defined(_ARDUINO_NUCLEO_L432KC)
#define FLASH_SPI       SPI
#define PIN_FLASH_CS    PIN_SPI_SS /* SS1 */
 #endif
//
 #if !defined(FLASH_SPI) || !defined(PIN_FLASH_CS)
#warning !!! Unsupportet Board !!! Please define FLASH_SPI and PIN_FLASH_CS
#undef UseSPIFlash
 #endif
 #endif
 #ifdef UseSPIFlash
uint16_t DetectSPIFlash (Stream & mydev, SPIClass spi);
uint16_t FlashDeviceID=0;
 #endif

#ifdef PIN_NEOPIXEL
#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel pixels(1, PIN_NEOPIXEL);
#endif

void mydelay(unsigned long ms);
void loopHandleLED();
//void ShowInfo (Stream & mydev = UsedSerial);
void ShowInfo (Stream & mydev, const char *prestr=nullptr, const char *poststr=nullptr);

bool BlinkLED=false;

void setup() {
 #ifdef PIN_NEOPIXEL
  pixels.begin();
 #else
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
 #endif
 #ifdef UseScheduler
  Scheduler.startLoop(loopHandleLED);
 #endif

  UsedSerial.begin(115200);
  while(!UsedSerial) {
    mydelay(50);
  }
  BlinkLED=true;
  ShowInfo(UsedSerial,"\r\n>BOOT!\t");

 #ifdef UseScheduler
  Scheduler.startLoop(loopHandleLED);
 #endif

  Wire.begin();
  Wire.setClock(TWI_CLOCK_1MHZ);
}

void BlinkDefaultLED(bool blink) {
  if (blink==true) {
   #ifdef PIN_NEOPIXEL
    if (TimerLed1.cycleOnOff(100, 500)) {
      pixels.setPixelColor(0, pixels.Color(100, 0, 0));
    }
    else {
      pixels.clear();
    }
   #else
    digitalWrite(LED_BUILTIN, TimerLed1.cycleOnOff(100, 500));
   #endif
  }
  else {
   #ifdef PIN_NEOPIXEL
    pixels.setPixelColor(0, pixels.Color(100, 0, 0));
   #else
    digitalWrite(LED_BUILTIN, true);
   #endif
  }
 #ifdef PIN_NEOPIXEL
  pixels.show();
 #endif
}


 #ifdef UseScheduler
void loopHandleLED() {
  BlinkDefaultLED(BlinkLED);
}
void mydelay(unsigned long ms) {
  if (ms)
    delay(ms);
}
 #else
void mydelay(unsigned long ms) {
  if (ms)
    delay(ms);
  else
    yield();
 #ifndef UseScheduler
  BlinkDefaultLED(BlinkLED);
 #endif
}
/* void yield() {
  BlinkDefaultLED(BlinkLED);
} */
 #endif


void loop() {
 #ifndef UseScheduler
  BlinkDefaultLED(BlinkLED);
 #endif
 #ifdef UseSPIFlash
  if (FlashDeviceID==0) {
    FlashDeviceID = DetectSPIFlash(UsedSerial,FLASH_SPI);
  }
 #endif
 #ifdef UseEEPI2C
  if (eepaddr) {
    //DetectI2CEeprom(UsedSerial,&eepaddr);
  }
  prntf(UsedSerial,"\r\nPress:");
  if (eepaddr) {
    prntf(UsedSerial,"\t'ee' for EEPROM (sw)");
    prntf(UsedSerial,"\t'E' for EEPROM (hw)");
  }
 #endif
 #ifdef UseSPIFlash
  //if (eepaddr)
    prntf(UsedSerial,"\t'f' for FLASH");
 #endif
  while (UsedSerial.available()==0) {
    mydelay(10);
  };
  
  if (UsedSerial.available()>0)
  {
    uint8_t rxdata = UsedSerial.read();
    eepaddr=0;

    switch (rxdata) {
      case 'i' : {
        ShowInfo(UsedSerial,"\r\n");
      } break;
      //
      case 'r' : {
        UsedSerial.print("\r\nRESET!\r\n");
        delay(100);
        NVIC_SystemReset();
      } break;
      //
      case 'f' : {
        FlashDeviceID=0;
        prntfln(UsedSerial," ->'%c'",rxdata);
      } break;
      case 'E' : {
        eepaddr=EEPADDR_A0;
        prntfln(UsedSerial," ->'%c'",rxdata);
        DetectI2CEeprom(UsedSerial,&eepaddr);
      } break;
      case 'e' : {
        eepaddr=EEPADDR_A0;
        prntfln(UsedSerial," ->'%c'",rxdata);
        DetectI2CEepromSW(UsedSerial,&eepaddr,&sw);
      } break;
      default: {
       #if 0
        if ((rxdata >= ' ') && (rxdata <= 'z'))
          prntf(UsedSerial, "\r\n%c", rxdata);
        else
          prntf(UsedSerial, "\r\n0x%02x", rxdata);
       #endif
      } break;
    }
  }
}

void ShowInfo (Stream & mydev, const char *prestr, const char *poststr) {
  if (prestr!=nullptr)
    mydev.print(prestr);

  prntf(mydev,"V" FWVERSION_EXT_STR "\t(%s",GetDeviceNameStringbyDID());
 #if defined(ARDUINO_ARCH_SAMD)
  prntf(mydev," Rev:%c",'A'+DSU->DID.bit.REVISION);
 #endif
 #if defined(ARDUINO_ARCH_SAMD) || defined (ARDUINO_ARCH_STM32)
  prntf(mydev," ID=%s",GetUIDString());
 #endif
  prntf(mydev,")\r\n");

  if (poststr!=nullptr)
    mydev.print(poststr);
}


#ifdef UseEEPI2C

uint32_t detectMemorySizeNoWriteSW(SoftWire *swire,uint8_t i2cAddress)
{
  #define BUFSIZE (32)
  uint8_t x, req=2;
  uint32_t size=0;
  swire->setTxBuffer(swTxBuffer, sizeof(swTxBuffer));
  swire->setRxBuffer(swRxBuffer, sizeof(swRxBuffer));
  swire->setDelay_us(5);
  swire->setTimeout(1000);
  //swire->setTimeout_ms(50);
  swire->begin();
  //  try to read a byte to see if connected
  swire->beginTransmission(i2cAddress);
  if (swire->endTransmission(1) != 0) {
    swire->end();
    // Serial.println(__LINE__);
    return 0;
  }
  byte dataMatch[BUFSIZE];
  //  Read from largest to smallest size
 #if 1
  for (size = 0x40000; size >= 32; size /= 2)
  //for (size = 128; size < 0x40000; size *= 2)
 #elif 0
  size = 0x40000;
 #elif 0
  size = 0x20000;
 #endif
  {
    //  Try to read last byte of the block, should return length of 0 when fails for single byte devices
    //  Will return the same dataFirst as initially read on other devices 
    //  as the data pointer could not be moved to the requested position
    int8_t dec=1;
   #if 0
    while (dec>=0)
   #endif
   {
    delay(2);
    swire->beginTransmission((uint8_t)GETADR((size-dec), i2cAddress));
    swire->write((uint8_t)(((size-dec) & 0xFFFF) >> 8));
    swire->write((uint8_t)((size-dec) & 0xFF));
    if (swire->endTransmission() != 0) {
      //swire->end();
      Serial.println(__LINE__);
      return 0;
    }
   
    x = swire->requestFrom((uint8_t)GETADR((size-dec), i2cAddress), req);
    Serial.print("\r\nx=");
    Serial.print(x,HEX);
    Serial.println();

    if (x == req) {
      for (x=0; x < req;) {
        if (swire->available())
          dataMatch[x++] = swire->read();
        else
          break;
      }
    }
    else {
      swire->end();
      Serial.println(__LINE__);
      return 0;
    }
    dec--;
   }
   #ifdef DEBUG
    Serial.print("@ 0x");
    Serial.print(size,HEX);
    //
    Serial.print(" (-1,0)\tx=");
    Serial.print(x,HEX);
    Serial.print("/req=");
    Serial.print(req,HEX);
    //
    Serial.print("\r\ndataMatch[");
    Serial.print(BUFSIZE);
    Serial.print("] =\t");
    for (uint8_t pos = 0; pos < req; pos++) {
        Serial.print(" ");
        if (dataMatch[pos]<0x10)
            Serial.print("0");
        Serial.print(dataMatch[pos],HEX);
    }
    Serial.println("");
   #endif
    if (x == req) {
      //  Read is performed just over size (size + BUFSIZE), 
      //  this will only work for devices with mem > size;
      //  therefore return size * 2
      swire->end();
      return size;
    }
  }
  swire->end();
  Serial.println(__LINE__);
  return 0;
}

uint8_t DetectI2CEepromSW (Stream & mydev, uint8_t *eepaddr, SoftWire *swire) {
  uint8_t detected_eepaddr=0, tries=0, test_eppaddr=EEPADDR_A0;
  uint32_t eepromSizeBytes=0;
  ExternalEEPROM myMem;

  mydev.print("I2C-EEPROM(SW):\t");
  while (tries++<20) {
    while (test_eppaddr<=EEPADDR_AE) {
      swire->beginTransmission(test_eppaddr);
      if (swire->endTransmission(1) == 0) {
        detected_eepaddr=test_eppaddr;
        break;
      }
      test_eppaddr++;
    }
    
    if (detected_eepaddr)
      break;

    if (tries<=1)
      prntfln(mydev,"no memory detected");
    else
      mydev.write(".");
    
    mydelay(500);

    test_eppaddr=EEPADDR_A0;

    if (mydev.available()) {
      if (mydev.read()) {
        detected_eepaddr=0;
        break;
      }
    }
  }

  //eep->setMemoryType(2048); // Valid types: 0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1025, 2048
  if (detected_eepaddr) {
    *eepaddr=detected_eepaddr;
    //
    eepromSizeBytes=detectMemorySizeNoWriteSW(swire,detected_eepaddr);
    prntf(mydev,"detectMemorySizeNoWriteSW():\r\n");
    prntf(mydev,"24XX%02d 0x%02X (%02X):\tsize: %d bytes / ", (eepromSizeBytes * 8 / 1024),detected_eepaddr, (detected_eepaddr<<1),eepromSizeBytes);
  }

  return(detected_eepaddr);
}


uint8_t DetectI2CEeprom (Stream & mydev, uint8_t *eepaddr, ExternalEEPROM *eep) {
  uint8_t detected_eepaddr=0, tries=0, test_eppaddr=EEPADDR_A0;
  uint32_t eepromSizeBytes=0;
  ExternalEEPROM myMem;

  if (eep==nullptr) {
    eep=&myMem;
  }

  mydev.print("I2C-EEPROM:\t");
  while (tries++<20) {
    while (test_eppaddr<=EEPADDR_AE) {
      if (eep->begin(test_eppaddr) == true) {
        detected_eepaddr=test_eppaddr;
        break;
      }
      test_eppaddr++;
    }
    
    if (detected_eepaddr)
      break;

    if (tries<=1)
      prntfln(mydev,"no memory detected");
    else
      mydev.write(".");
    
    mydelay(500);

    test_eppaddr=EEPADDR_A0;

    if (mydev.available()) {
      if (mydev.read()) {
        detected_eepaddr=0;
        break;
      }
    }
  }

  //eep->setMemoryType(2048); // Valid types: 0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1025, 2048

  if (detected_eepaddr) {
    *eepaddr=detected_eepaddr;

    eep->setAddressBytes(0);
    eep->setMemorySize(0);
    eep->setMemorySizeBytes(0);
    eep->setPageSize(0);
    eep->setPageSizeBytes(0);
    //
    eepromSizeBytes=eep->detectMemorySizeNoWrite();
    prntf(mydev,"detectMemorySizeNoWrite():\r\n");
    prntf(mydev,"24XX%02d 0x%02X (%02X):\tsize: %d bytes / ", (eepromSizeBytes * 8 / 1024),detected_eepaddr, (detected_eepaddr<<1),eepromSizeBytes);
  #if 0
    eep->setAddressBytes(0);
    eep->setMemorySize(0);
    eep->setMemorySizeBytes(0);
    eep->setPageSize(0);
    eep->setPageSizeBytes(0);
    
    //eep->detectAddressBytes();
    eep->detectPageSizeBytes();
    //eep->detectMemorySizeBytes();
    eepromSizeBytes = eep->getMemorySizeBytes();
    prntf(mydev,"24XX%02d @ %02X:\tsize: %d bytes / ", (eepromSizeBytes * 8 / 1024), (*eepaddr<<1),eepromSizeBytes);
   
    if (eepromSizeBytes < 128)
      prntf(mydev,"%02d Bits", eepromSizeBytes * 8);
    else
      prntf(mydev,"%02d kBits",(eepromSizeBytes * 8 / 1024));
    prntf(mydev,"\tpageSize: %d",eep->getPageSize());
   #endif
  }

  return(detected_eepaddr);
}
 #endif /* #ifdef UseEEPI2C */


 #ifdef UseSPIFlash
uint16_t DetectSPIFlash (Stream & mydev, SPIClass spi)
{
  uint16_t deviceID=0;
  SFE_SPI_FLASH myFlash;

 #ifdef PIN_FLASH_WP
  pinMode(PIN_FLASH_WP, OUTPUT);
  digitalWrite(PIN_FLASH_WP, HIGH);
 #endif
 #ifdef PIN_FLASH_HOLD
  pinMode(PIN_FLASH_HOLD, OUTPUT);
  digitalWrite(PIN_FLASH_HOLD, HIGH);
 #endif

  //myFlash.enableDebugging(); // Uncomment this line to enable helpful debug messages on Serial
  mydev.print("SPI-FLASH:\t");

  if (myFlash.begin(PIN_FLASH_CS,2000000U,spi,SPI_MODE0) == false) {
    mydev.println("not detected!");
  }
  else {
    sfe_flash_manufacturer_e mfgID = myFlash.getManufacturerID();
    if (mfgID != SFE_FLASH_MFG_UNKNOWN) {
      prntf(mydev,"%s (0x%02X)",myFlash.manufacturerIDString(mfgID), myFlash.getRawManufacturerID());
    }
    else {
      uint8_t unknownID = myFlash.getRawManufacturerID(); // Read the raw manufacturer ID
      prntf(mydev,"unknown manufacturer ID: 0x%02X",unknownID);
    }
    deviceID=myFlash.getDeviceID();
    prntfln(mydev, "\tID: 0x%04X",deviceID);
  }
  return deviceID;
}
#endif /* #ifdef UseSPIFlash */