#include "video.h"

extern const uint8_t font5x7[] PROGMEM;

static void active_line(void);
static void vsync_line(void);
static void blank_line(void);

volatile int scanLine;
int renderLine, lines_frame;
char vscale_const, vscale, vsync_end;
uint8_t frame[WIDTH * HEIGHT];
uint8_t start_render, output_delay, video_color = 0;
void (*line_handler)(void);

int16_t abs(int16_t v)
{
	return (v < 0) ? (v * -1) : v;
}

void video_begin(uint8_t mode)
{
	VID_DDR |= (1 << VID_PIN);
	VID_PORT &= ~(1 << VID_PIN);
	SYNC_DDR |= (1 << SYNC_PIN);
	SYNC_PORT |= (1 << SYNC_PIN);

	TCCR1A = (1 << COM1A1) | (1 << COM1A0) | (1 << WGM11);
	TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10);

	if(mode)
	{
		vscale_const = VSCALE_PAL;
		start_render = START_RENDER_PAL;
		output_delay = PAL_CYCLES_OUTPUT_START;
		vsync_end = PAL_LINE_STOP_VSYNC;
		lines_frame = PAL_LINE_FRAME;
		ICR1 = PAL_CYCLES_SCANLINE;
	}
	else
	{
		vscale_const = VSCALE_NTSC;
		start_render = START_RENDER_NTSC;
		output_delay = NTSC_CYCLES_OUTPUT_START;
		vsync_end = NTSC_LINE_STOP_VSYNC;
		lines_frame = NTSC_LINE_FRAME;
		ICR1 = NTSC_CYCLES_SCANLINE;
	}

	vscale = vscale_const;
	OCR1A = CYCLES_HORZ_SYNC;
	scanLine = lines_frame + 1;
	line_handler = &vsync_line;
	TIMSK1 = (1 << TOIE1);
	sei();
}

void video_sp(uint8_t x, uint8_t y)
{
	switch(video_color)
	{
		case 0:
		{
			frame[(x >> 3) + (y * WIDTH)] &= (~0x80 >> (x & 7));
			break;
		}

		case 1:
		{
			frame[(x >> 3) + (y * WIDTH)] |= (0x80 >> (x & 7));
			break;
		}

		case 2:
		{
			frame[(x >> 3) + (y * WIDTH)] ^= (0x80 >> (x & 7));
			break;
		}
	}
}

void video_set_color(uint8_t color)
{
	video_color = color;
}

void video_set_pixel(uint8_t x, uint8_t y)
{
	if(x < PWIDTH && y < HEIGHT)
	{
		video_sp(x, y);
	}
}

uint8_t video_get_pixel(uint8_t x, uint8_t y)
{
	return(x < PWIDTH && y < HEIGHT &&
			(frame[x / 8 + y * WIDTH] & (0x80 >> (x & 7))));
}

void video_clear(void)
{
	uint16_t i;
	uint8_t val;
	switch(video_color)
	{
		case BLACK:
		{
			val = 0x00;
			break;
		}

		case WHITE:
		{
			val = 0xFF;
			break;
		}

		case INVERT:
		{
			for(i = 0; i < WIDTH * HEIGHT; ++i)
			{
				frame[i] = ~frame[i];
			}
		}

		default:
		{
			return;
		}
	}

	for(i = 0; i < WIDTH * HEIGHT; ++i)
	{
		frame[i] = val;
	}
}

void video_rect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
	uint8_t ox = x0;
	if(x0 >= PWIDTH) { x0 = PWIDTH - 1; }
	if(y0 >= HEIGHT) { y0 = HEIGHT - 1; }
	if(x1 >= PWIDTH) { x1 = PWIDTH - 1; }
	if(y1 >= HEIGHT) { y1 = HEIGHT - 1; }
	while(y0 < y1)
	{
		while(x0 < x1)
		{
			video_sp(x0, y0);
			++x0;
		}

		x0 = ox;
		++y0;
	}

	frame[0] = 0xFF;
}

void video_circle(int16_t x0, int16_t y0, int16_t radius)
{
	int16_t x = radius - 1, y = 0, dx = 1, dy = 1,
	err = dx - (radius << 1);
	while(x >= y)
	{
		video_set_pixel(x0 + x, y0 + y);
		video_set_pixel(x0 + y, y0 + x);
		video_set_pixel(x0 - y, y0 + x);
		video_set_pixel(x0 - x, y0 + y);
		video_set_pixel(x0 - x, y0 - y);
		video_set_pixel(x0 - y, y0 - x);
		video_set_pixel(x0 + y, y0 - x);
		video_set_pixel(x0 + x, y0 - y);

		if(err <= 0)
		{
			++y;
			err += dy;
			dy += 2;
		}

		if(err > 0)
		{
			--x;
			dx += 2;
			err += dx - (radius << 1);
		}
	}
}

void video_hline(uint8_t x, uint8_t y, uint8_t l)
{
	uint8_t e;
	if(x < PWIDTH && y < HEIGHT && (e = x + l) < PWIDTH)
	{
		for(; x < e; ++x)
		{
			video_sp(x, y);
		}
	}
}

void video_vline(uint8_t x, uint8_t y, uint8_t l)
{
	uint8_t e;
	if(x < PWIDTH && y < HEIGHT && (e = y + l) < HEIGHT)
	{
		for(; y < e; ++y)
		{
			video_sp(x, y);
		}
	}
}

void video_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
	int16_t dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int16_t dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int16_t err = (dx > dy ? dx : -dy) / 2, e2;
	while(x0 != x1 && y0 != y1)
	{
		video_set_pixel(x0, y0);

		e2 = err;

		if(e2 > -dx)
		{
			err -= dy;
			x0 += sx;
		}

		if(e2 < dy)
		{
			err += dx;
			y0 += sy;
		}
	}
}

void video_char(uint8_t x, uint8_t y, char c)
{
	uint8_t a, b, p, ex, ey;
	const uint8_t *v;
	a = y;
	ex = x + 5;
	ey = y + 7;
	v = font5x7 + 5 * (c - 32);
	if(ex < PWIDTH && ey < HEIGHT)
	{
		for(; x < ex; ++x, ++v)
		{
			p = pgm_read_byte(v);
			for(y = a, b = 1; y < ey; ++y, b <<= 1)
			{
				if(p & b)
				{
					video_sp(x, y);
				}
			}
		}
	}
}

void video_string(uint8_t x, uint8_t y, char *s)
{
	while(*s)
	{
		video_char(x, y, *s++);
		x += 6;
	}
}

void video_bitmap
(uint8_t *img, uint8_t x0, uint8_t y0, uint8_t x1, uint16_t len)
{
	uint8_t *end, mask, sx = x0;
	for(end = img + len; img < end; ++img)
	{
		for(mask = 1; mask < 0x80; mask <<= 1)
		{
			if(*img & mask)
			{
				video_sp(x0, y0);
			}

			if(++x0 == x1)
			{
				x0 = sx;
				++y0;
			}
		}
	}
}

void video_shift(uint8_t distance, uint8_t dir)
{
	switch(dir)
	{
		case UP:
		{
			uint8_t *src, *dst, *end;
			dst = frame;
			src = frame + distance * WIDTH;
			end = frame + HEIGHT * WIDTH;
			while(src <= end)
			{
				*dst = *src;
				*src = 0;
				++dst;
				++src;
			}
			break;
		}

		case DOWN:
		{
			uint8_t *src, *dst, *end;
			dst = frame + HEIGHT * WIDTH;
			src = dst - distance * WIDTH;
			end = frame;
			while(src >= end)
			{
				*dst = *src;
				*src = 0;
				--dst;
				--src;
			}
			break;
		}

		case LEFT:
		{
			uint8_t *src, *dst, *end, shift, tmp, line;
			shift = distance & 7;
			for(line = 0; line < HEIGHT; ++line)
			{
				dst = frame + WIDTH * line;
				src = dst + distance / 8;
				end = dst + WIDTH - 2;
				while(src <= end)
				{
					tmp = 0;
					tmp = *src << shift;
					*src = 0;
					++src;
					tmp |= *src >> (8 - shift);
					*dst = tmp;
					++dst;
				}

				tmp = 0;
				tmp = *src << shift;
				*src = 0;
				*dst = tmp;
			}
			break;
		}

		case RIGHT:
		{
			uint8_t *src, *dst, *end, shift, tmp, line;
			shift = distance & 7;
			for(line = 0; line < HEIGHT; ++line)
			{
				dst = frame + WIDTH - 1 + WIDTH * line;
				src = dst - distance / 8;
				end = dst - WIDTH + 2;
				while(src >= end)
				{
					tmp = 0;
					tmp = *src >> shift;
					*src = 0;
					--src;
					tmp |= *src << (8 - shift);
					*dst = tmp;
					--dst;
				}

				tmp = 0;
				tmp = *src >> shift;
				*src = 0;
				*dst = tmp;
			}
			break;
		}
	}
}

static void blank_line(void)
{
	if(scanLine == start_render)
	{
		renderLine = 0;
		vscale = vscale_const;
		line_handler = &active_line;
	}
	else if(scanLine == lines_frame)
	{
		line_handler = &vsync_line;
	}

	++scanLine;
}

static void vsync_line(void)
{
	if(scanLine >= lines_frame)
	{
		OCR1A = CYCLES_VIRT_SYNC;
		scanLine = 0;
	}
	else if(scanLine == vsync_end)
	{
		OCR1A = CYCLES_HORZ_SYNC;
		line_handler = &blank_line;
	}

	++scanLine;
}

static void active_line(void)
{
	__asm__ __volatile__
	(
		"subi %[time], 10              \n"
		"sub  %[time], %[tcnt1l]       \n\t"
		"100:                          \n\t"
		"    subi %[time], 3           \n\t"
		"    brcc 100b                 \n\t"
		"    subi %[time], 0-3         \n\t"
		"    breq 101f                 \n\t"
		"    dec  %[time]              \n\t"
		"    breq 102f                 \n\t"
		"    rjmp 102f                 \n"
		"101:                          \n\t"
		"    nop                       \n"
		"102:                          \n"
		:: [time] "a" (output_delay), [tcnt1l] "a" (TCNT1L)
	);

	#if RMETHOD >= 6

	__asm__ __volatile__
	(
		"ADD  r26, r28                 \n\t"
		"ADC  r27, r29                 \n\t"
		"in   r16, %[port]             \n\t"
		HWS_ANDI
		"rjmp enter6                     \n"
		"loop6:                        \n\t"
		"    bst  __tmp_reg__, 0       \n\t"
		HWS_BLD
		"    out  %[port],r16            \n"
		"enter6:                       \n\t"
		"    LD   __tmp_reg__, X+      \n\t"
		"    nop                       \n\t"
		"    bst  __tmp_reg__, 7       \n\t"
		HWS_BLD
		"    out  %[port], r16           \n"
		"    nop                       \n\t"
		"    nop                       \n\t"
		"    nop                       \n\t"
		"    bst  __tmp_reg__, 6       \n\t"
		HWS_BLD
		"    out  %[port], r16           \n"
		"    nop                       \n\t"
		"    nop                       \n\t"
		"    nop                       \n\t"
		"    bst  __tmp_reg__, 5       \n\t"
		HWS_BLD
		"    out  %[port], r16           \n"
		"    nop                       \n\t"
		"    nop                       \n\t"
		"    nop                       \n\t"
		"    bst  __tmp_reg__, 4       \n\t"
		HWS_BLD
		"    out  %[port], r16           \n"
		"    nop                       \n\t"
		"    nop                       \n\t"
		"    nop                       \n\t"
		"    bst  __tmp_reg__, 3       \n\t"
		HWS_BLD
		"    out  %[port], r16           \n"
		"    nop                       \n\t"
		"    nop                       \n\t"
		"    nop                       \n\t"
		"    bst  __tmp_reg__, 2       \n\t"
		HWS_BLD
		"    out  %[port], r16           \n"
		"    nop                       \n\t"
		"    nop                       \n\t"
		"    nop                       \n\t"
		"    bst  __tmp_reg__, 1       \n\t"
		HWS_BLD
		"    out  %[port], r16           \n"
		"    dec  %[hres]              \n\t"
		"    brne loop6                \n\t"
		"    nop                       \n\t"
		"    nop                       \n\t"
		"    bst  __tmp_reg__, 0       \n\t"
		HWS_BLD
		"    out  %[port], r16           \n"
		"    in   r16, %[port]         \n\t"
		HWS_ANDI
		HWS_BST
		HWS_BLD
		"    out  %[port], r16           \n"
		:: [port] "i" (_SFR_IO_ADDR(VID_PORT)), "x" (frame),
		"y" (renderLine), [hres] "d" (WIDTH) : "r16"
	);

	#elif RMETHOD == 5

	__asm__ __volatile__
	(
		"ADD  r26, r28                 \n\t"
		"ADC  r27, r29                 \n\t"
		"in   r16, %[port]             \n\t"
		HWS_ANDI
		"rjmp enter5                     \n"
		"loop5:                        \n\t"
		"    bst  __tmp_reg__, 0       \n\t"
		HWS_BLD
		"    out  %[port], r16           \n"
		"enter5:                       \n\t"
		"    LD   __tmp_reg__, X+      \n\t"
		"    bst  __tmp_reg__, 7       \n\t"
		HWS_BLD
		"    out  %[port], r16           \n"
		"    nop                       \n\t"
		"    nop                       \n\t"
		"    bst  __tmp_reg__, 6       \n\t"
		HWS_BLD
		"    out  %[port], r16           \n"
		"    nop                       \n\t"
		"    nop                       \n\t"
		"    bst  __tmp_reg__, 5       \n\t"
		HWS_BLD
		"    out  %[port], r16           \n"
		"    nop                       \n\t"
		"    nop                       \n\t"
		"    bst  __tmp_reg__, 4       \n\t"
		HWS_BLD
		"    out  %[port], r16           \n"
		"    nop                       \n\t"
		"    nop                       \n\t"
		"    bst  __tmp_reg__, 3       \n\t"
		HWS_BLD
		"    out  %[port], r16           \n"
		"    nop                       \n\t"
		"    nop                       \n\t"
		"    bst  __tmp_reg__, 2       \n\t"
		HWS_BLD
		"    out  %[port], r16           \n"
		"    nop                       \n\t"
		"    dec  %[hres]              \n\t"
		"    bst  __tmp_reg__, 1       \n\t"
		HWS_BLD
		"    out  %[port], r16           \n"
		"    brne loop5                \n\t"
		"    nop                       \n\t"
		"    bst  __tmp_reg__, 0       \n\t"
		HWS_BLD
		"    out  %[port], r16           \n"
		"    in   r16, %[port]         \n\t"
		HWS_ANDI
		HWS_BST
		HWS_BLD
		"    out   %[port], r16          \n"
		:: [port] "i" (_SFR_IO_ADDR(VID_PORT)), "x" (frame),
		"y" (renderLine), [hres] "d" (WIDTH) : "r16"
	);

	#elif RMETHOD == 4

	__asm__ __volatile__
	(
		"ADD  r26, r28                 \n\t"
		"ADC  r27, r29                 \n\t"
		"rjmp enter4                     \n"
		"loop4:                        \n\t"
		"    lsl  __tmp_reg__          \n\t"
		"    out  %[port], __tmp_reg__ \n\t"
		"enter4:                       \n\t"
		"    LD   __tmp_reg__, X+      \n\t"
		"    nop                       \n\t"
		"    out  %[port], __tmp_reg__ \n\t"
		"    nop                       \n\t"
		"    nop                       \n\t"
		"    lsl  __tmp_reg__          \n\t"
		"    out  %[port], __tmp_reg__ \n\t"
		"    nop                       \n\t"
		"    nop                       \n\t"
		"    lsl  __tmp_reg__          \n\t"
		"    out  %[port], __tmp_reg__ \n\t"
		"    nop                       \n\t"
		"    nop                       \n\t"
		"    lsl  __tmp_reg__          \n\t"
		"    out  %[port], __tmp_reg__ \n\t"
		"    nop                       \n\t"
		"    nop                       \n\t"
		"    lsl  __tmp_reg__          \n\t"
		"    out  %[port], __tmp_reg__ \n\t"
		"    nop                       \n\t"
		"    nop                       \n\t"
		"    lsl  __tmp_reg__          \n\t"
		"    out  %[port], __tmp_reg__ \n\t"
		"    nop                       \n\t"
		"    lsl  __tmp_reg__          \n\t"
		"    dec  %[hres]              \n\t"
		"    out  %[port], __tmp_reg__ \n\t"
		"    brne loop4                \n\t"
		"    nop                       \n\t"
		"    lsl  __tmp_reg__          \n\t"
		"    out  %[port], __tmp_reg__ \n\t"
		"    nop                       \n\t"
		"    nop                       \n\t"
		"    nop                       \n\t"
		"    cbi  %[port], 7           \n\t"
		:: [port] "i" (_SFR_IO_ADDR(VID_PORT)), "x" (frame),
		"y" (renderLine), [hres] "d" (WIDTH) : "r16"
	);

	#elif RMETHOD <= 3


	__asm__ __volatile__
	(
		".macro byteshift              \n\t"
		"    LD   __tmp_reg__, X+      \n\t"
		"    out  %[port], __tmp_reg__ \n\t"
		"    nop                       \n\t"
		"    lsl  __tmp_reg__          \n\t"
		"    out  %[port], __tmp_reg__ \n\t"
		"    nop                       \n\t"
		"    lsl  __tmp_reg__          \n\t"
		"    out  %[port], __tmp_reg__ \n\t"
		"    nop                       \n\t"
		"    lsl  __tmp_reg__          \n\t"
		"    out  %[port], __tmp_reg__ \n\t"
		"    nop                       \n\t"
		"    lsl  __tmp_reg__          \n\t"
		"    out  %[port], __tmp_reg__ \n\t"
		"    nop                       \n\t"
		"    lsl  __tmp_reg__          \n\t"
		"    out  %[port], __tmp_reg__ \n\t"
		"    nop                       \n\t"
		"    lsl  __tmp_reg__          \n\t"
		"    out  %[port], __tmp_reg__ \n\t"
		"    nop                       \n\t"
		"    lsl  __tmp_reg__          \n\t"
		"    out  %[port], __tmp_reg__ \n\t"
		".endm                         \n\t"
		"ADD  r26, r28                 \n\t"
		"ADC  r27, r29                 \n\t"
		"cpi  %[hres], 30              \n\t"
		"breq skip0                    \n\t"
		"cpi  %[hres], 29              \n\t"
		"breq jumpto1                  \n\t"
		"cpi  %[hres], 28              \n\t"
		"breq jumpto2                  \n\t"
		"cpi  %[hres], 27              \n\t"
		"breq jumpto3                  \n\t"
		"cpi  %[hres], 26              \n\t"
		"breq jumpto4                  \n\t"
		"cpi  %[hres], 25              \n\t"
		"breq jumpto5                  \n\t"
		"cpi  %[hres], 24              \n\t"
		"breq jumpto6                  \n\t"
		"jumpto1:                      \n\t"
		"    rjmp skip1                \n\t"
		"jumpto2:                      \n\t"
		"    rjmp skip2                \n\t"
		"jumpto3:                      \n\t"
		"    rjmp skip3                \n\t"
		"jumpto4:                      \n\t"
		"    rjmp skip4                \n\t"
		"jumpto5:                      \n\t"
		"    rjmp skip5                \n\t"
		"jumpto6:                      \n\t"
		"    rjmp skip6                \n\t"
		"skip0:                        \n\t"
		"    byteshift                 \n\t"
		"skip1:                        \n\t"
		"    byteshift                 \n\t"
		"skip2:                        \n\t"
		"    byteshift                 \n\t"
		"skip3:                        \n\t"
		"    byteshift                 \n\t"
		"skip4:                        \n\t"
		"    byteshift                 \n\t"
		"skip5:                        \n\t"
		"    byteshift                 \n\t"
		"skip6:                        \n\t"
		"    byteshift                 \n\t"
		"    byteshift                 \n\t"
		"    byteshift                 \n\t"
		"    byteshift                 \n\t"
		"    byteshift                 \n\t"
		"    byteshift                 \n\t"
		"    byteshift                 \n\t"
		"    byteshift                 \n\t"
		"    byteshift                 \n\t"
		"    byteshift                 \n\t"
		"    byteshift                 \n\t"
		"    byteshift                 \n\t"
		"    byteshift                 \n\t"
		"    byteshift                 \n\t"
		"    byteshift                 \n\t"
		"    byteshift                 \n\t"
		"    byteshift                 \n\t"
		"    byteshift                 \n\t"
		"    byteshift                 \n\t"
		"    byteshift                 \n\t"
		"    byteshift                 \n\t"
		"    byteshift                 \n\t"
		"    byteshift                 \n\t"
		"    byteshift                 \n\t"
		"    nop                       \n\t"
		"    nop                       \n\t"
		"    cbi %[port], 7            \n\t"
		:: [port] "i" (_SFR_IO_ADDR(VID_PORT)), "x" (frame),
		"y" (renderLine), [hres] "d" (WIDTH) : "r16"
	);

	#endif

	if(!vscale)
	{
		vscale = vscale_const;
		renderLine += WIDTH;
	}
	else
	{
		--vscale;
	}

	if(scanLine++ == (int)(start_render + HEIGHT * (vscale_const + 1)))
	{
		line_handler = &blank_line;
	}
}

ISR(TIMER1_OVF_vect)
{
	line_handler();
}
