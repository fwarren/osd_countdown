/*
 * osd_countdown
 *
 * Copyright (C) 2018, Daniel González Cabanelas <dgcbueu@gmail.com>
 *               Based on osd_clock by Jon Beckham <leftorium@<leftorium@leftorium.net>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xosd.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <sys/time.h>

static struct option long_options[] = {
  {"font",     1, NULL, 'f'},
  {"color",    1, NULL, 'c'},
  {"delay",    1, NULL, 'd'},
  {"execute",  1, NULL, 'e'},
  {"message",  1, NULL, 'M'},
  {"format",   1, NULL, 'F'},
  {"interval", 1, NULL, 'i'},
  {"shadow",   1, NULL, 's'},
  {"top",      0, NULL, 't'},
  {"middle",   0, NULL, 'm'},
  {"bottom",   0, NULL, 'b'},
  {"right",    0, NULL, 'r'},
  {"left",     0, NULL, 'l'},
  {"center",   0, NULL, 'C'},
  {"xoffset",  1, NULL, 'x'},
  {"yoffset",  1, NULL, 'y'},
  {"wait",     1, NULL, 'w'},
  {"help",     0, NULL, 'h'},
  {NULL,       0, NULL, 0}
};

int main (int argc, char *argv[])
{
  char c;

  static const char *format;
  static const char *command;
  static const char *message;

  xosd *osd;
  xosd_pos pos = XOSD_bottom;
  xosd_align align = XOSD_left;

  const char *font = "-*-freemono-*-r-*-*-*-1000-*-*-*-*-*-*";
  const char *color = "red";
  int delay = -1; 
  int xoffset = 0;
  int yoffset = 0;
  int shadow = 2;
  int interval = 1;
  unsigned int wait = 30;

  while ((c = getopt_long(argc ,argv,"f:c:d:e:M:F:i:s:x:y:w:tmbrlCh",
			  long_options, NULL)) != -1)
  {
    switch(c)
    {
      case 'f':
	font = optarg;
	break;
      case 'F':
	format = optarg;
	break;
      case 'c':
	color = optarg;
	break;
      case 'e':
	command = optarg;
	break;
      case 'M':
	message = optarg;
	break;
      case 'd':
	delay = atoi(optarg);
	break;
      case 'i':
	interval = atoi(optarg);
       break;
      case 's':
	shadow = atoi(optarg);
	break;
      case 'x':
	xoffset = atoi(optarg);
	break;
      case 'y':
	yoffset = atoi(optarg);
	break;
      case 'w':
	wait = atoi(optarg);
	break;
      case 't':
	pos = XOSD_top;
	break;
      case 'm':
	pos = XOSD_middle;
	break;
      case 'b':
	pos = XOSD_bottom;
	break;
      case 'r':
	align = XOSD_right;
	break;
      case 'l':
	align = XOSD_left;
	break;
      case 'C':
        align = XOSD_center;
        break;
      case 'h':
	printf("USAGE: %s [-flag args]\n"
		"\t-w --wait    \tduration, in seconds, of the countdown\n"
		"\t-e --execute \texecute a command after the countdown timeout\n"
		"\t-M --message \tdisplay an OSD text message after the countdown timeout\n"
		"\t-f --font    \tfully qualified font. Use 'xfontsel'. default: fixed\n"
		"\t-c --color   \tcolor. Name or hex e.g. '#B03060'. See X11 rgb.txt. default: red\n"
		"\t-s --shadow  \tdrop shadow offset.  default: 2\n"
		"\t-t --top     \tlocate countdown at top (default: bottom)\n"
		"\t-m --middle  \tlocate countdown at middle (default: bottom)\n"
		"\t-b --bottom  \tlocate countdown at bottom (default)\n"
		"\t-r --right   \tlocate countdown at right (default: left)\n"
		"\t-l --left    \tlocate countdown at left (default: left)\n"
		"\t-C --center  \tlocate countdown at center (default: left)\n"
		"\t-x --xoffset \thorizontal offset (default: 0)\n"
		"\t-y --yoffset \tvertical offset (default: 0)\n"
		"\t-F --format  \tSpecify time/date format (in strftime(3) style)\n"
		"\t-d --delay   \tDelay (time the countdown stays on screen when it's updated)\n"
	        "\t             \tin seconds (default: -1 (never))\n"
		"\t-i --interval\tInterval (time between updates) in seconds\n"
		"\t-h --help    \tthis help message\n",
		argv[0]);
	return EXIT_SUCCESS;
	break;
    }
  }

  osd = xosd_create(1);

  /* Set the position of the display. */
  xosd_set_pos(osd, pos);
  xosd_set_align(osd, align);
  xosd_set_vertical_offset(osd,yoffset);
  xosd_set_horizontal_offset(osd,xoffset);

  /* Set timeout */
  xosd_set_timeout(osd, delay);

  /* Set the font and the colours. */
  xosd_set_font(osd, font);
  xosd_set_colour(osd, color);

  /* Set the font shadow */
  xosd_set_shadow_offset(osd, shadow);

  if (!osd)
  {
    fprintf (stderr, "Error initializing osd\n");
    return EXIT_FAILURE;
  }

  /* If no format is specified, we revert to ctime-ish display */ 
  if(!format) format = "%H:%M:%S";

  struct tm mytime;
  struct timeval start, now;
  unsigned int elapsed = 0, time_left = 0;
  char output[255], output_l[255];
  int time = 0, days = 0;

  gettimeofday(&start, NULL); //first time stamp
  time_left = wait - elapsed; //set time left

  while (time_left > 0) 
	{
		gettimeofday(&now, NULL); //actual time stamp
		elapsed = now.tv_sec - start.tv_sec;
		time_left = wait - elapsed; //update time left

		time 		= time_left;
		days 		= time / 86400;
		time 		= time % 86400;
		mytime.tm_hour 	= time / 3600;
		time 		= time % 3600;
		mytime.tm_min 	= time / 60;
		time 		= time % 60;
		mytime.tm_sec 	= time;

		strftime(output, 255, format, &mytime);

		if (days > 0) {
			sprintf(output_l, "%d days %s", days, output);
			xosd_display(osd, 0, XOSD_string, output_l);
		}
		else
			xosd_display(osd, 0, XOSD_string, output);

		sleep(interval);
	}

  if(message) xosd_display(osd, 0, XOSD_string, message);
  if(command) system(command);
  if(message) pause();

  xosd_destroy(osd);

  return EXIT_SUCCESS;
}
