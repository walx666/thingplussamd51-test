#include <Arduino.h>
#include "Printable.h"

#include <stdio.h>
#include <stdarg.h>

#define __SYS_C__
#include "sys.h"

size_t prntf(Stream &SerUart, const char *fmt, ...)
{
  size_t size;
  char buf[256];
  va_list args;
  va_start(args, fmt);
  size = vsnprintf(buf, 256, fmt, args);
  va_end(args);
  SerUart.print(buf);
  return size;
}

int timedPeek(Stream &s, uint32_t Timeout)
{
  int c;
  unsigned long _startMillis = millis();
  do
  {
    c = s.peek();
    if (c >= 0)
      return c;
  } while (millis() - _startMillis < Timeout);
  return -1; // -1 indicates timeout
}

// returns peek of the next digit in the stream or -1 if timeout
// discards non-numeric characters
int peekNextDigit(Stream &s, LookaheadMode lookahead, bool detectDecimal, uint32_t Timeout)
{
  int c;
  while (1)
  {
    c = timedPeek(s, Timeout);

    if (c < 0 ||
        c == '-' || c == '+' ||
        (c >= '0' && c <= '9') ||
        (detectDecimal && c == '.'))
      return c;

    switch (lookahead)
    {
    case SKIP_NONE:
      return -1; // Fail code.
    case SKIP_WHITESPACE:
      switch (c)
      {
      case ' ':
      case '\t':
      case '\r':
      case '\n':
        break;
      default:
        return -1; // Fail code.
      }
    case SKIP_ALL:
      if (c == '\x1b')
      {
        s.read();
        return -1;
      }
      break;
    }
    s.read(); // discard non-numeric
  }
}

/*double ScanDouble(Stream & s, double *defValue, uint32_t TimeOut, LookaheadMode lookahead, char ignore) {
  bool isNegative = false;
  bool isFraction = false;
  long long value = 0;
  int c, digitcnt=0;
  double fraction = 1.0;

  c = peekNextDigit(s, lookahead, true, TimeOut);
    // ignore non numeric leading characters
  if(c < 0) {
    if (defValue!=nullptr) {
      return *defValue;
    }
    return 0; // zero returned if timeout
  }

  do{
    if(c == ignore)
      ; // ignore
    else if(c == '-') {
      isNegative = true;
    }
    else if (c == '.') {
      isFraction = true;
    }
    else if(c >= '0' && c <= '9')  {      // is c a digit?
      digitcnt++;
      value = value * 10 + c - '0';
      if(isFraction)
         fraction *= 0.1;
    }
    s.read();  // consume the character we got with peek
    s.write(c);
    c = timedPeek(s,TimeOut);
  }
  while( (c >= '0' && c <= '9')  || (c == '.' && !isFraction) || c == ignore );

  if (defValue!=nullptr) {
    if (((digitcnt==0) && (c<0)) || (c=='\x1b')) {
      return *defValue;
    }
  }

  if(isNegative)
    value = -value;
  if(isFraction)
    return value * fraction;
  else
    return value;
}

int8_t delayMicrosWithStreamBreak(unsigned int us, Stream &mydev, char breakchr)
{
  uint32_t start, elapsed;
  uint32_t count;

  if (us == 0)
    return 0;

  count = us * (VARIANT_MCK / 1000000) - 20; // convert us to cycles.
  start = DWT->CYCCNT;                       // CYCCNT is 32bits, takes 37s or so to wrap.
  while (1)
  {
    elapsed = DWT->CYCCNT - start;
    if (elapsed >= count)
      return 0;

    if (us > 10000)
      yield();

    if (mydev.available())
      if (mydev.read() == breakchr)
        return -1;
  }
}
*/

char *GetUIDString(void)
{
  static char str[33] = {0};
  sprintf(str, "%08X%08X%08X%08X", *UID_W0, *UID_W1, *UID_W2, *UID_W3);
  return str;
}

uint32_t *GetUIDtoArray(uint32_t *CpuID)
{
  CpuID[0] = *UID_W0;
  CpuID[1] = *UID_W1;
  CpuID[2] = *UID_W2;
  CpuID[3] = *UID_W3;
  return CpuID;
}

uint32_t *GetUID()
{
  static uint32_t CpuID[4];
  CpuID[0] = *UID_W0;
  CpuID[1] = *UID_W1;
  CpuID[2] = *UID_W2;
  CpuID[3] = *UID_W3;
  return CpuID;
}

char *GetDeviceNameStringbyDID(void)
{
  static char str[20];
  strcpy(str, "???");
#ifdef ARDUINO_ARCH_SAMD
  switch (DSU->DID.reg)
  {
  case 0x61810306:
    strcpy(str, "ATSAME51G18A");
    break;
  case 0x61810305:
    strcpy(str, "ATSAME51G19A");
    break;
  }
  switch (DSU->DID.reg & 0xfffff0ff)
  {
  case 0x61840001:
    strcpy(str, "ATSAME54P19A");
    break;
  case 0x61840000:
    strcpy(str, "ATSAME54P20A");
    break;
  case 0x61840003:
    strcpy(str, "ATSAME54N19A");
    break;
  case 0x61840002:
    strcpy(str, "ATSAME54N20A");
    break;
  case 0x61830002:
    strcpy(str, "ATSAME53N20A");
    break;
  case 0x61830003:
    strcpy(str, "ATSAME53N19A");
    break;
  case 0x61830006:
    strcpy(str, "ATSAME53J18A");
    break;
  case 0x61830005:
    strcpy(str, "ATSAME53J19A");
    break;
  case 0x61830004:
    strcpy(str, "ATSAME53J20A");
    break;
  case 0x61810001:
    strcpy(str, "ATSAME51N19A");
    break;
  case 0x61810000:
    strcpy(str, "ATSAME51N20A");
    break;
  case 0x61810003:
    strcpy(str, "ATSAME51J18A");
    break;
  case 0x61810002:
    strcpy(str, "ATSAME51J19A");
    break;
  case 0x61810004:
    strcpy(str, "ATSAME51J20A");
    break;
  case 0x60060000:
    strcpy(str, "ATSAMD51P20A");
    break;
  case 0x60060001:
    strcpy(str, "ATSAMD51P19A");
    break;
  case 0x60060003:
    strcpy(str, "ATSAMD51N19A");
    break;
  case 0x60060002:
    strcpy(str, "ATSAMD51N20A");
    break;
  case 0x60060006:
    strcpy(str, "ATSAMD51J18A");
    break;
  case 0x60060005:
    strcpy(str, "ATSAMD51J19A");
    break;
  case 0x60060004:
    strcpy(str, "ATSAMD51J20A");
    break;
  case 0x60060008:
    strcpy(str, "ATSAMD51G18A");
    break;
  case 0x60060007:
    strcpy(str, "ATSAMD51G19A");
    break;
  case 0x61810306:
    strcpy(str, "ATSAME51G18A");
    break;
  case 0x61810305:
    strcpy(str, "ATSAME51G19A");
    break;
  }
#else
#ifdef ARDUINO_ARCH_STM32
#ifdef STM32F405xx
  strcpy(str, "STM32F405xx");
#else
  strcpy(str, "STM32??????");
#endif
#else
#endif
#endif
  return str;
}