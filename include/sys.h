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

#if defined(ARDUINO_ARCH_STM32)
 #if defined(STM32F4xx)
//#define UID_BASE                     0x1FFF7A10UL           /*!< Unique device ID register base address */
#define ID_UNIQUE_ADDRESS       0x1FFF7A10
#define ID_FLASH_ADDRESS        0x1FFF7A22
#define ID_DBGMCU_IDCODE        0xE0042000
/**
 * @brief  Get STM32F4xx device signature
 * @note   Defined as macro to get maximal response time
 * @param  None
 * @retval Device signature, bits 11:0 are valid, 15:12 are always 0.
 *           - 0x0413: STM32F405xx/07xx and STM32F415xx/17xx)
 *           - 0x0419: STM32F42xxx and STM32F43xxx
 *           - 0x0423: STM32F401xB/C
 *           - 0x0433: STM32F401xD/E
 *           - 0x0431: STM32F411xC/E
 */
#define ID_GetSignature()       ((*(uint16_t *) (ID_DBGMCU_IDCODE)) & 0x0FFF)

/**
 * @brief  Get STM32F4xx device revision
 * @note   Defined as macro to get maximal response time
 * @param  None
 * @retval Device revision value
 *           - 0x1000: Revision A
 *           - 0x1001: Revision Z
 *           - 0x1003: Revision Y
 *           - 0x1007: Revision 1
 *           - 0x2001: Revision 3
 */
#define ID_GetRevision()        (*(uint16_t *) (DBGMCU->IDCODE + 2))

/**
 * @brief  Get STM32F4xx device's flash size in kilo bytes
 * @note   Defined as macro to get maximal response time
 * @param  None
 * @retval Flash size in kilo bytes
 */
#define ID_GetFlashSize()       (*(uint16_t *) (ID_FLASH_ADDRESS))

/**
 * @brief  Get unique ID number in 8-bit format
 * @note   STM32F4xx has 96bits long unique ID, so 12 bytes are available for read in 8-bit format
 * @note   Defined as macro to get maximal response time
 * @param  x: Byte number: specify which part of 8 bits you want to read
 *               - Values between 0 and 11 are allowed
 * @retval Unique ID address
 */
#define ID_GetUnique8(x)        ((x >= 0 && x < 12) ? (*(uint8_t *) (ID_UNIQUE_ADDRESS + (x))) : 0)

/**
 * @brief  Get unique ID number in 16-bit format
 * @note   STM32F4xx has 96bits long unique ID, so 6 2-bytes values are available for read in 16-bit format
 * @note   Defined as macro to get maximal response time
 * @param  x: Byte number: specify which part of 16 bits you want to read
 *               - Values between 0 and 5 are allowed
 * @retval Unique ID address
 */
#define ID_GetUnique16(x)      ((x >= 0 && x < 6) ? (*(uint16_t *) (ID_UNIQUE_ADDRESS + 2 * (x))) : 0)

/**
 * @brief  Get unique ID number in 32-bit format
 * @note   STM32F4xx has 96bits long unique ID, so 3 4-bytes values are available for read in 32-bit format
 * @note   Defined as macro to get maximal response time
 * @param  x: Byte number: specify which part of 16 bits you want to read
 *               - Values between 0 and 2 are allowed
 * @retval Unique ID address
 */
#define ID_GetUnique32(x)       ((x >= 0 && x < 3) ? (*(uint32_t *) (ID_UNIQUE_ADDRESS + 4 * (x))) : 0)

 #endif /* #if defined(ARDUINO_ARCH_STM32) */
#endif /* #if defined(STM32F4xx) */

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
externSYS uint32_t GetUIDtoArray(uint32_t *CpuID);
