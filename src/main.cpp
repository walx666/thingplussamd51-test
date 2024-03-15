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
#define EEPADR_A0 0x50   //0b1010(A2 A1 A0): A standard I2C EEPROM with the ADR0 bit set to VCC
#define EEPADR_A8 0x54   //0b1010(A2 A1 A0): A standard I2C EEPROM with the ADR0 bit set to VCC
#include "SparkFun_External_EEPROM.h" // Click here to get the library: http://librarymanager/All#SparkFun_External_EEPROM
ExternalEEPROM myMem;
uint8_t eepadr=EEPADR_A0;
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
//void DetectSPIFlash (Stream & mydev = UsedSerial);
void DetectSPIFlash (Stream & mydev, SPIClass spi);
 #endif

void loopHandleLED();
//void ShowInfo (Stream & mydev = UsedSerial);
void ShowInfo (Stream & mydev, const char *prestr=nullptr, const char *poststr=nullptr);

bool BlinkLED=false;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

 #ifdef UseScheduler
  Scheduler.startLoop(loopHandleLED);
 #endif

  UsedSerial.begin(115200);
  while(!UsedSerial) {
    delay(50);
  } 

  BlinkLED=true;
  ShowInfo(UsedSerial,"\r\n>BOOT!\t");

 #ifdef UseScheduler
  Scheduler.startLoop(loopHandleLED);
 #endif

 #ifdef UseSPIFlash
  DetectSPIFlash(UsedSerial,FLASH_SPI);
 #endif

  Wire.begin();
  Wire.setClock(TWI_CLOCK_1MHZ);
}

 #ifdef UseScheduler
void loopHandleLED() {
  if (BlinkLED)
    digitalWrite(LED_BUILTIN, TimerLed1.cycleOnOff(100, 500));
  else
    digitalWrite(LED_BUILTIN, true);
}
 #else
void yield() {
  if (BlinkLED)
    digitalWrite(LED_BUILTIN, TimerLed1.cycleOnOff(100, 500));
  else
    digitalWrite(LED_BUILTIN, true);
}
 #endif


void loop() {
#ifdef UseEEPI2C
  if (eepadr) {
    myMem.setMemoryType(2048); // Valid types: 0, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1025, 2048
    UsedSerial.println("I2C-EEPROM Test");
    while (1) {
      if (myMem.begin(eepadr) == true) {
        break;
      }
      prntf(UsedSerial,"No memory @ %02X detected !\r\n", (eepadr<<1));
      delay(1000);

      if (eepadr==EEPADR_A0)
        eepadr=EEPADR_A8;
      else
        eepadr=EEPADR_A0;

      yield();
    }
    prntf(UsedSerial,"Memory detected @ %02X !\r\n", (eepadr<<1));

    //Write a series of bytes then test to see if auto-detect changes them
    //The detection functions *should not* modify the data on the EEPROM
    uint32_t maxBytes = 300; //Ideally we write this many bytes
    if (maxBytes > myMem.getMemorySizeBytes())
      maxBytes = myMem.getMemorySizeBytes();
   
    //UsedSerial.print("Detected number of address bytes: ");
    //UsedSerial.println(myMem.detectAddressBytes());
   
    prntf(UsedSerial, "Detected pageSizeBytes: %d\r\n",myMem.detectPageSizeBytes());
   
    //UsedSerial.print("Detected page write time (ms): ");
    //UsedSerial.println(myMem.detectWriteTimeMs());
   
    //uint32_t eepromSizeBytes = myMem.detectMemorySizeBytes();
    uint32_t eepromSizeBytes = myMem.getMemorySizeBytes();
    prntf(UsedSerial,"Detected EEPROM size (bytes): %d bytes / ",eepromSizeBytes);
   
    if (eepromSizeBytes < 128)
      prntf(UsedSerial,"%02d Bits", eepromSizeBytes * 8);
    else
      prntf(UsedSerial,"%02d kBits",(eepromSizeBytes * 8 / 1024));
    
    prntf(UsedSerial," - 24XX%02d\r\n",(eepromSizeBytes * 8 / 1024));
  }
 #endif /* #ifdef UseEEPI2C */
   
  if (eepadr)
    prntf(UsedSerial,"\r\nPress 'c' to continue...");

  while (UsedSerial.available()==0) {
    delay(10);
    yield();
  };
  
  if (UsedSerial.available()>0)
  {
    uint8_t rxdata = UsedSerial.read();
    eepadr=0;

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
      case 'c' : {
        eepadr=EEPADR_A0;
        prntf(UsedSerial,"\t->\r\n");
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

 #ifdef UseSPIFlash
void DetectSPIFlash (Stream & mydev, SPIClass spi)
{
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

  // Begin the flash using the chosen CS pin. Default to: spiPortSpeed=2000000, spiPort=SPI and spiMode=SPI_MODE0
  if (myFlash.begin(PIN_FLASH_CS,2000000U,spi,SPI_MODE0) == false) {
    mydev.println("SPI Flash not detected!");
  }
  else {
    sfe_flash_manufacturer_e mfgID = myFlash.getManufacturerID();
    if (mfgID != SFE_FLASH_MFG_UNKNOWN) {
      prntf(mydev,"SPI Flash: %s (0x%02X)",myFlash.manufacturerIDString(mfgID), myFlash.getRawManufacturerID());
    }
    else {
      uint8_t unknownID = myFlash.getRawManufacturerID(); // Read the raw manufacturer ID
      prntf(mydev,"SPI Flash: Unknown manufacturer ID: 0x%02X",unknownID);
    }
    prntf(mydev, "\tDevice ID: 0x%04X\r\n",myFlash.getDeviceID());
  }
}
#endif /* #ifdef UseSPIFlash */