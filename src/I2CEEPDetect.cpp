#include "Arduino.h"
#include "Wire.h"
#include "I2CEEPDetect.h"
#include "SparkFun_External_EEPROM.h"


/*
    if (i2cAddress == 255)
        i2cAddress = settings.deviceAddress; // We can't set the default to settings.deviceAddress so we use 255 instead

    settings.i2cPort->beginTransmission((uint8_t)i2cAddress);
    if (settings.i2cPort->endTransmission() == 0)
        return (true);
  return (false);
}
*/
/*
ExternalEEPROM *eep;

void begin(ExternalEEPROM &myeep) {
  eep = myeep;
}

bool PingI2C(uint8_t deviceAddress) {
  return(eep->begin(deviceAddress));
}

uint8_t TestI2C(uint8_t address) {
  uint8_t ee_save, ee_test, c;
// routine that tests writing at a address then changes it back
  ee_save = eep->ReadI2C(address);
  if(ee_save!=0x55) {
    //eep->write(address,0x55,1);
    eep->write(address,0x55);
    ee_test = 0x55;
  } else {
    eep->WriteI2C(address,0xCC);
    ee_test = 0xCC;
  }
  c = ReadI2C(address);
  if(c == ee_test) {
    // address is working
    eep->WriteI2C(address,ee_save);  // restore old data
    return 1;
  } else {
    if(c==ee_save)
      return 0;
    else {
      eep->WriteI2C(address,ee_save);  // try to restore old data
      return 0xff;  // strange results
    }
  }
}

//Determining one or two byte addresses:
uint8_t get_address_size() {
uint8_t ee_test_address, ee_save, ee_test, c, i;
   ee_test_address = 1;            // small memory chips actually work either way
   i = ReadI2C(ee_test_address);     // Read memory address 00 01
   c = ReadI2C(ee_test_address);     // Read memory address 01
   if(c!=i) {
      ee_test_address = 64;         // page boundry works if Address 1 doesn't
      i = ReadI2C(ee_test_address);     // Read memory address 00 64
      c = ReadI2C(ee_test_address-1);     // Read memory address 63
      c = ReadI2C(ee_test_address);     // Read memory address 64
      //serial_print_hex(i);
      //serial_printf( ". 16 bit address model\r\n ");
      //serial_print_hex(c);
      //serial_printf( ". 8 bit address model\r\n ");
   }
   if(c!=i) {
      //serial_printf( "Undefined problem can not verify value at address\r\n ");
      //serial_printf( "unable to determine address size terminate analysis.\r\n ");
      return 0xff;
   } else {
      ee_save = c;   // save origional value
   }
   if(c!=0x55) {
      WriteLI2C(ee_test_address,0x55);
      ee_test = 0x55;
      //serial_printf( "Writing 55 using large model\r\n ");
   } else {
      WriteLI2C(ee_test_address,0xCC);
      ee_test = 0xCC;
      //serial_printf( "Writing CC using large model\r\n ");
   }
   // Read memory address with new detectable test pattern
   // using 2 byte address then with one byte address
   i = ReadI2C(ee_test_address);     
   c = ReadI2C(ee_test_address);
   if(c!=i) {
      if (i == ee_test) {
         //serial_printf( "EEPROM is LARGE address model\r\n ");
         ee_address_size = 2;
      } else {
         //serial_printf( "Error determining address size\r\n ");
         //serial_printf( "EEPROM assumed to be Write Protected\r\n ");
         return 0xfe;
      }
      //serial_print_hex(i);
      //serial_printf( ". large model\r\n ");
      //serial_print_hex(c);
      //serial_printf( ". small model\r\n ");
   } else {
      if (c == ee_test) {
         //serial_printf( "EEPROM is small address size\r\n ");
         ee_address_size = 1;
      } else {
         //serial_printf( "Error determining address size\r\n ");
         //serial_printf( "EEPROM assumed to be Write Protected\r\n ");
         return 0xfe;
      }
   }
   // restore origional value
   if(ee_address_size == 2)
      WriteLI2C(ee_test_address,ee_save);
   else
      eep->WriteI2C(ee_test_address,ee_save);
   // now that we know address size we can determine total size of chip
   // via a combination of testing I2C control address and 
   // modifing highest possible memory address
   return ee_address_size;
}

Finally how to get total memory size:
uint8_t get_total_mem_size() {
uint8_t ee_test_address, ee_save, ee_test, c, i;
   // first determine 8bit address total memsize
   if(ee_address_size == 1) {
      if(TestI2C(0x0f) != 1)      // largest mem location for24LC00
         return 0xfe;
      if(TestI2C(0x10) != 1) {   // fail 16 bytes therefore 24LC00
         ee_page_size = 1;
         ee_segment_size = 16;
         ee_total_mem = 16;
         return 0;
      }
      if(TestI2C(0x80) != 1) {   // fail 129 bytes therefore 24LC01B
         ee_page_size = 8;
         ee_segment_size = 128;
         ee_total_mem = 128;
         return 1;
      }
      c = 0;
      for (i=0xA0; i < 0xB0; i=i+2) {
         if(PingI2C(i))
            c++;
      }
      if(c==1) {               // only one address means 24LC02B
         ee_page_size = 8;
         ee_segment_size = 256;
         ee_total_mem = 256;
         return 2;
      }
      if(c==2) {               // 2 addresses means 24LC04B
         ee_page_size = 16;
         ee_segment_size = 256;
         ee_total_mem = 512;
         return 4;
      }
      if(c==4) {               // 4 addresses means 24LC08B
         ee_page_size = 16;
         ee_segment_size = 256;
         ee_total_mem = 1024;
         return 8;
      }
      if(c==8) {               // 8 addresses means 24LC16B
         ee_page_size = 16;
         ee_segment_size = 256;
         ee_total_mem = 2048;
         return 16;
      }
      // failure for single address
      ee_page_size = 0;
      ee_segment_size = 0;
      ee_total_mem = 0;
      return 0xff;
   } else {
   // now testing 16 bit addresses
      if(testL_i2c(0xfff) != 1)      // largest mem location for 24LC32B
         return 0;
      if(testL_i2c(0x1000) != 1) {   // fail 4K+1 bytes therefore 24LC32B
         ee_page_size = 32;
         ee_segment_size = 4096;
         ee_total_mem = 4096;
         return 32;
      }
      if(testL_i2c(0x2000) != 1) {   // fail 8K+1 bytes therefore 24LC65
         ee_page_size = 64;
         ee_segment_size = 8192;
         ee_total_mem = 8192;
         return 65;
      }
      if(testL_i2c(0x4000) != 1) {   // fail 16K+1 bytes therefore 24LC128
         ee_page_size = 64;
         ee_segment_size = 16384;
         ee_total_mem = 16384;
         return 128;
      }
      if(testL_i2c(0x8000) != 1) {   // fail 32K+1 bytes therefore 24LC256
         ee_page_size = 64;
         ee_segment_size = 32768;
         ee_total_mem = 32768;
         return 256;
      }
      c = 0;
      for (i=0xA8; i < 0xB0; i=i+2) {
         if(PingI2C(i))
            c++;
      }
      if(c == 0) {
         ee_page_size = 64;         // only one address might mean 24LC512
         ee_segment_size = 65536;
         ee_total_mem = 65536;
         return 512;
      }
      if(c == 1) {
         ee_page_size = 128;         // two addresses should mean 24LC1025
         ee_segment_size = 65536;
         ee_total_mem = 131072;
         return 1024;
      }
   // failure for 16 bit addresses
   ee_page_size = 0;
   ee_segment_size = 0;
   ee_total_mem = 0;
   return 0xff;
}*/