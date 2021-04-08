#ifndef __VIDEO_SETTINGS_H__
#define __VIDEO_SETTINGS_H__

#define NTSC     0
#define PAL      1

#define BLACK    0
#define WHITE    1
#define INVERT   2

#define UP       0
#define DOWN     1
#define LEFT     2
#define RIGHT    3

#define WIDTH   20
#define PWIDTH    (8 * WIDTH)
#define HEIGHT  96

#define CYCLES_PER_US \
(F_CPU / 1000000)

#define TIME_HORZ_SYNC           4.7
#define TIME_VIRT_SYNC          58.85
#define TIME_ACTIVE             46

#define CYCLES_VIRT_SYNC \
((TIME_VIRT_SYNC * CYCLES_PER_US) - 1)

#define CYCLES_HORZ_SYNC \
((TIME_HORZ_SYNC * CYCLES_PER_US) - 1)

/* timing settings for NTSC */
#define NTSC_TIME_SCANLINE      63.55
#define NTSC_TIME_OUTPUT_START  12
#define NTSC_LINE_FRAME        262
#define NTSC_LINE_START_VSYNC    0
#define NTSC_LINE_STOP_VSYNC     3
#define NTSC_LINE_DISPLAY      216

#define NTSC_LINE_MID \
((NTSC_LINE_FRAME - NTSC_LINE_DISPLAY) / 2 + NTSC_LINE_DISPLAY / 2)

#define NTSC_CYCLES_SCANLINE \
((NTSC_TIME_SCANLINE * CYCLES_PER_US) - 1)

#define NTSC_CYCLES_OUTPUT_START \
((NTSC_TIME_OUTPUT_START * CYCLES_PER_US) - 1)

#define VSCALE_NTSC \
(NTSC_LINE_DISPLAY / HEIGHT - 1)

#define START_RENDER_NTSC \
(NTSC_LINE_MID - ((HEIGHT * (VSCALE_NTSC + 1)) / 2) + 8)

/* timing settings for PAL */
#define PAL_TIME_SCANLINE       64
#define PAL_TIME_OUTPUT_START   12.5
#define PAL_LINE_FRAME         312
#define PAL_LINE_START_VSYNC     0
#define PAL_LINE_STOP_VSYNC      7
#define PAL_LINE_DISPLAY       260

#define PAL_LINE_MID \
((PAL_LINE_FRAME - PAL_LINE_DISPLAY) / 2 + PAL_LINE_DISPLAY / 2)

#define PAL_CYCLES_SCANLINE \
((PAL_TIME_SCANLINE * CYCLES_PER_US) - 1)

#define PAL_CYCLES_OUTPUT_START \
((PAL_TIME_OUTPUT_START * CYCLES_PER_US) - 1)

#define VSCALE_PAL \
(PAL_LINE_DISPLAY / HEIGHT - 1)

#define START_RENDER_PAL \
(PAL_LINE_MID - ((HEIGHT * (VSCALE_PAL + 1)) / 2))

#define RMETHOD (TIME_ACTIVE * CYCLES_PER_US) / PWIDTH

/* sync output is on OC1A */
#define ENABLE_FAST_OUTPUT

/* device specific settings */
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega1281__) || \
defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__)

/* video */
#if defined(ENABLE_FAST_OUTPUT)

#define VID_PORT   PORTA
#define VID_DDR    DDRA
#define VID_PIN   7

#else

#define VID_PORT   PORTB
#define VID_DDR    DDRB
#define VID_PIN   6

#endif

/* sync */
#define SYNC_PORT  PORTB
#define SYNC_DDR   DDRB
#define SYNC_PIN  5

#elif defined(__AVR_ATmega644__) || defined(__AVR_ATmega644P__) || \
defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__)

/* video */
#if defined(ENABLE_FAST_OUTPUT)

#define VID_PORT   PORTA
#define VID_DDR    DDRA
#define VID_PIN   7

#else

#define VID_PORT   PORTD
#define VID_DDR    DDRD
#define VID_PIN   4

#endif

/* sync */
#define SYNC_PORT  PORTD
#define SYNC_DDR   DDRD
#define SYNC_PIN  5

#elif defined(__AVR_ATmega8__) || defined(__AVR_ATmega88__) || \
defined(__AVR_ATmega168P__) || defined(__AVR_ATmega168__) || \
defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328__)

/* video */
#if defined(ENABLE_FAST_OUTPUT)

#define VID_PORT   PORTD
#define VID_DDR    DDRD
#define VID_PIN   7

#else

#define VID_PORT   PORTB
#define VID_DDR    DDRB
#define VID_PIN   0

#endif

/* sync */
#define SYNC_PORT  PORTB
#define SYNC_DDR   DDRB
#define SYNC_PIN  1

#elif defined (__AVR_AT90USB1286__)

/* video */
#define VID_PORT   PORTF
#define VID_DDR    DDRF
#define VID_PIN   7

/* sync */
#define SYNC_PORT  PORTB
#define SYNC_DDR   DDRB
#define SYNC_PIN  5

#endif

#if VID_PIN == 0

#define HWS_BLD   "bld  r16, 0    \n"
#define HWS_BST   "bst  r16, 0    \n"
#define HWS_ANDI  "andi r16, 0xFE \n"

#elif VID_PIN == 1

#define HWS_BLD   "bld  r16, 1    \n"
#define HWS_BST   "bst  r16, 1    \n"
#define HWS_ANDI  "andi r16, 0xFD \n"

#elif VID_PIN == 2

#define HWS_BLD   "bld  r16, 2    \n"
#define HWS_BST   "bst  r16, 2    \n"
#define HWS_ANDI  "andi r16, 0xFB \n"

#elif VID_PIN == 3

#define HWS_BLD   "bld  r16, 3    \n"
#define HWS_BST   "bst  r16, 3    \n"
#define HWS_ANDI  "andi r16, 0xF7 \n"

#elif VID_PIN == 4

#define HWS_BLD   "bld  r16, 4    \n"
#define HWS_BST   "bst  r16, 4    \n"
#define HWS_ANDI  "andi r16, 0xEF \n"

#elif VID_PIN == 5

#define HWS_BLD   "bld  r16, 5    \n"
#define HWS_BST   "bst  r16, 5    \n"
#define HWS_ANDI  "andi r16, 0xDF \n"

#elif VID_PIN == 6

#define HWS_BLD   "bld  r16, 6    \n"
#define HWS_BST   "bst  r16, 6    \n"
#define HWS_ANDI  "andi r16, 0xBF \n"

#elif VID_PIN == 7

#define HWS_BLD   "bld  r16, 7    \n"
#define HWS_BST   "bst  r16, 7    \n"
#define HWS_ANDI  "andi r16, 0x7F \n"

#endif

#endif /* __VIDEO_SETTINGS_H__ */
