#ifndef HOST_NDS_H
#define HOST_NDS_H
#include <cstdint>
#include <cstdio>
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef unsigned int uint;
#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 192
#define TRUE true
#define FALSE false
#define BIT(n) (1u<<(n))
#define RGB15(r,g,b) ((r)|((g)<<5)|((b)<<10))
inline void swiWaitForVBlank() {}
inline int iprintf(const char*,...) {return 0;}
#define BG_BMP_RAM_SUB(n) ((void*)0)
#define BG_BMP_RAM(n) ((void*)0)
#endif
