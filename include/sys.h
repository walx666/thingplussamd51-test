#include "Arduino.h"
#include "Printable.h"

#ifndef __SYS_C__
#define externSYS extern
#else
#define externSYS
#endif

#if !defined(SCANVALUE_TIMOUTMS_DEF)
#define SCANVALUE_TIMOUTMS_DEF 10000
#endif

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))

#ifdef ARDUINO_ARCH_SAMD
#if defined(__SAMD51__)
#define UID_W0 ((unsigned int *)0x008061FCUL)
#define UID_W1 ((unsigned int *)0x00806010UL)
#define UID_W2 ((unsigned int *)0x00806014UL)
#define UID_W3 ((unsigned int *)0x00806018UL)
#else
#define UID_W0 ((unsigned int *)0x0080A00CUL)
#define UID_W1 ((unsigned int *)0x0080A040UL)
#define UID_W2 ((unsigned int *)0x0080A044UL)
#define UID_W3 ((unsigned int *)0x0080A048UL)
#endif
#endif

#define ESC_CURSOR_ON "\x1b[?25h"
#define ESC_CURSOR_OFF "\x1b[?25l"
#define ESC_CURSOR_BLINK "\x1b[5m"
#define ESC_CURSOR_NOATT "\x1b[0m"

#define DelayBreakChar 0x1B /* <ESC> */

struct NullStream : public Stream
{
    NullStream(void) { return; }
    int available(void) { return 0; }
    void flush(void) { return; }
    int peek(void) { return -1; }
    int read(void) { return -1; }
    size_t write(uint8_t u_Data) { return u_Data; }
};

externSYS NullStream noStream;
externSYS size_t prntf(Stream &SerUart, const char *fmt, ...);
externSYS double ScanDouble(Stream &s, double *defValue = nullptr, uint32_t TimeOut = SCANVALUE_TIMOUTMS_DEF, LookaheadMode lookahead = SKIP_ALL, char ignore = '\x01');
externSYS int8_t delayMicrosWithStreamBreak(unsigned int us, Stream &mydev, char breakchr);
// externSYS size_t printComplex(Print& p, Complex c, int digits);
externSYS char *GetDeviceNameStringbyDID(void);
externSYS char *GetUIDString(void);
externSYS uint32_t *GetUID();
externSYS uint32_t *GetUIDtoArray(uint32_t *CpuID);
