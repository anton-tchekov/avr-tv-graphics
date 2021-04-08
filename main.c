#include <avr/io.h>
#include "video.h"

int main(void)
{
	video_begin(NTSC);
	video_clear();
	video_rect(20, 20, 40, 40);
	video_line(0, 95, 127, 0);
	video_string(10, 70, "Hello!");

	for(;;)
	{
		
	}

	return 0;
}
