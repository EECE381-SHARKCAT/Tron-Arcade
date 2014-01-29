#include <stdio.h>
#include "altera_up_avalon_character_lcd.h"
#include "altera_up_sd_card_avalon_interface.h"
#include "altera_up_avalon_video_pixel_buffer_dma.h"
#include "altera_up_avalon_video_character_buffer_with_dma.h"
#include "sys/alt_alarm.h"
#include "sys/alt_timestamp.h"
#include "io.h"

#define switches (volatile char *) SWITCHES_BASE
#define leds (char *) LEDS_BASE
#define keys (volatile char *) KEYS_BASE

void multiply_matrix();
alt_up_character_lcd_dev* init_lcd();
void lcd_to_key(alt_up_character_lcd_dev* char_lcd_dev);

int main(void) {

	printf("Starting Matrix Multiplication\n");
	alt_timestamp_start();
	multiply_matrix();
	printf("Done! Time Elapsed : %f seconds\n" , (float) alt_timestamp() / (float) alt_timestamp_freq());

	while(1){
		*leds = *switches;
	}

	return 0;
}

void multiply_matrix(){

	//declare variables
	int a[100][100], b[100][100], c[100][100];
	int i,j,k,sum;

	//initialize arrays with 1s
	for (i=0; i<100; i++){
		for(j=0; j<100; j++){
			a[i][j] = 1;
			b[i][j] = 1;
		}
	}

	//multiply matrix
	for (i=0; i<100; i++){
			for(j=0; j<100; j++){
				sum = 0;
				for(k=0; k<100; k++){
					sum = sum + a[i][k] * b[k][j];
				}
				c[i][j] = sum;
			}
		}
}

void timer_test(void) {
	int freq;
	int cycles;
	float duration;
	int ticks_start;
	int ticks_end;
	int ticks_per_s;
	int ticks_duration;
	int timer_period;
	int status;
	int done;

	printf("Timers\n");

	//TEST SYSTEM CLOCK
	printf(" Sys Clock Timer\n");
	ticks_per_s = alt_ticks_per_second();
	printf(" Tick Freq: %d\n", ticks_per_s);
	printf(" Recording starting ticks\n");
	ticks_start = alt_nticks();
	printf(" Sleeping for 5 seconds\n");
	usleep(5000000);
	printf(" Recording ending ticks\n");
	ticks_end = alt_nticks();
	ticks_duration = ticks_end - ticks_start;
	duration = (float) ticks_duration / (float) ticks_per_s;
	printf(" The program slept for %d ticks (%f seconds)\n\n", ticks_duration, duration);

	//TEST TIMESTAMP TIMER
	printf(" Timestamp Timer\n");
	freq = alt_timestamp_freq();
	printf(" CPU Freq: %.2fMHz\n", (float) freq / (float) 1000000);
	printf(" Resetting Timestamp timer\n");
	alt_timestamp_start();
	printf(" ...Timing the print of this statement...\n");
	cycles = alt_timestamp();
	duration = (float) cycles / (float) freq;
	printf(" It took %d cycles (%f seconds) to print the statement\n\n", cycles, duration);

	//TEST HARDWARE ONLY TIMER
	printf(" Hardware-Only Timer\n");
	printf(" Setting timer period to 5 seconds.\n");
	timer_period = 5 * 50000000;
	IOWR_16DIRECT(HARDWARE_TIMER_BASE, 8, timer_period & 0xFFFF);
	IOWR_16DIRECT(HARDWARE_TIMER_BASE, 12, timer_period >> 16);
	printf(" Stopping Timer\n");
	status = IORD_16DIRECT(HARDWARE_TIMER_BASE, 0);
	if (status & 0x2) {
	IOWR_16DIRECT(HARDWARE_TIMER_BASE, 4, 1 << 3);
	}
	printf(" Starting Timer\n");
	IOWR_16DIRECT(HARDWARE_TIMER_BASE, 4, 1 << 2);
	printf(" Waiting for timer to expire...\n");
	done = 0;
	while (! done) {
	status = IORD_16DIRECT(HARDWARE_TIMER_BASE, 0);
	done = status & 0x1;
	}
	printf(" 5 seconds timer is done\n");
}

alt_up_character_lcd_dev* init_lcd(){
	// open the Character LCD port
	alt_up_character_lcd_dev* char_lcd_dev;
	char_lcd_dev = alt_up_character_lcd_open_dev (CHARACTER_LCD_0_NAME);
	if ( char_lcd_dev == NULL)
	  alt_printf ("Error: could not open character LCD device\n");
	else
	  alt_printf ("Opened character LCD device\n");
	return char_lcd_dev;
}

void lcd_to_key(alt_up_character_lcd_dev* char_lcd_dev){
	//KEY 0 is pressed
	if ((*keys) == 0xE){
		//Initialize LCD - clears display
		alt_up_character_lcd_init (char_lcd_dev);
		//Print
		alt_up_character_lcd_string(char_lcd_dev, "KEY0");
	}
	//KEY 1 is pressed
	else if ((*keys) == 0xD){
		//Initialize LCD - clears display
		alt_up_character_lcd_init (char_lcd_dev);
		//Print
		alt_up_character_lcd_string(char_lcd_dev, "KEY1");
	}
	//KEY 2 is pressed
	else if ((*keys) == 0xB){
		//Initialize LCD - clears display
		alt_up_character_lcd_init (char_lcd_dev);
		//Print
		alt_up_character_lcd_string(char_lcd_dev, "KEY2");
	}
	//KEY 3 is pressed
	else if ((*keys) == 0x7){
		//Initialize LCD - clears display
		alt_up_character_lcd_init (char_lcd_dev);
		//Print
		alt_up_character_lcd_string(char_lcd_dev, "KEY3");
	}
}
