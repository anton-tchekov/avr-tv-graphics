#ifndef __VIDEO_H__
#define __VIDEO_H__

#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "video_conf.h"

int16_t abs(int16_t v);
void video_begin(uint8_t mode);
void video_sp(uint8_t x, uint8_t y);

void video_set_color(uint8_t color);
void video_set_pixel(uint8_t x, uint8_t y);
uint8_t video_get_pixel(uint8_t x, uint8_t y);
void video_clear(void);
void video_rect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void video_circle(int16_t x0, int16_t y0, int16_t radius);
void video_hline(uint8_t x, uint8_t y, uint8_t l);
void video_vline(uint8_t x, uint8_t y, uint8_t l);
void video_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
void video_char(uint8_t x, uint8_t y, char c);
void video_string(uint8_t x, uint8_t y, char *s);
void video_bitmap
	(uint8_t *img, uint8_t x0, uint8_t y0, uint8_t x1, uint16_t len);
void video_shift(uint8_t distance, uint8_t dir);

#endif /* __VIDEO_H__ */
