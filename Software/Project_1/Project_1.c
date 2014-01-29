#include <stdio.h>
#include "altera_up_avalon_character_lcd.h"
#include "altera_up_sd_card_avalon_interface.h"
#include "altera_up_avalon_video_pixel_buffer_dma.h"
#include "altera_up_avalon_video_character_buffer_with_dma.h"
#include "sys/alt_alarm.h"
#include "sys/alt_timestamp.h"
#include "io.h"

#define switches (volatile char *) SWITCHES_BASE
#define ledr1s (char*) LEDS_BASE
#define ledr2s (char*) LEDS_BASE
#define leds (char *) LEDS_BASE
#define keys (volatile char *) KEYS_BASE
void initializeVGA(void);
void displayVGA(void);
void multiply_matrix();
alt_up_character_lcd_dev* init_lcd();
void lcd_to_key(alt_up_character_lcd_dev* char_lcd_dev);
void blink(void);
void readfile(void);
int main(void) {

	printf("Starting Matrix Multiplication\n");
	alt_timestamp_start();
	multiply_matrix();
	printf("Done! Time Elapsed : %f seconds\n" , (float) alt_timestamp() / (float) alt_timestamp_freq());



	// Init for SD_card

	alt_up_sd_card_dev *device_reference = NULL;
	int connected = 0;






	// Initialize pixel buffer
	alt_up_pixel_buffer_dma_dev* pixel_buffer;
	// Use the name of your pixel buffer DMA core
	pixel_buffer = alt_up_pixel_buffer_dma_open_dev(BUFFER_DMA_NAME);
	if (pixel_buffer == NULL)
	{
		printf("pixel_buffer failed \n");
	}
	// Set the background buffer address – Although we don’t use thebackground,
	// they only provide a function to change the background buffer address, so
	// we must set that, and then swap it to the foreground.
	alt_up_pixel_buffer_dma_change_back_buffer_address(pixel_buffer, PIXEL_BUFFER_BASE);
	// Swap background and foreground buffers
	alt_up_pixel_buffer_dma_swap_buffers(pixel_buffer);

	// Wait for the swap to complete while
	(alt_up_pixel_buffer_dma_check_swap_buffers_status(pixel_buffer));

	// Initialize for char
	alt_up_char_buffer_dev *char_buffer;
	char_buffer = alt_up_char_buffer_open_dev("/dev/character_buffer");
	if ( char_buffer == NULL)
	printf("Char_buffer failed \n");

	while(1){
		// Clear the screen
			alt_up_pixel_buffer_dma_clear_screen(pixel_buffer, 0);
			// Draw a blue line to the foreground buffer
			alt_up_pixel_buffer_dma_draw_line(pixel_buffer, 0, 120, 320, 120, 0x1234, 0);

			alt_up_pixel_buffer_dma_draw_box(pixel_buffer, 50, 50, 60, 60, 0x2222, 0);
			alt_up_pixel_buffer_dma_draw_rectangle(pixel_buffer, 70, 70, 80, 80, 0x3333, 0);
			//helper_plot_pixel(register unsigned int buffer_start, register int line_size, register int x, register int y, register int color, register int mode)


			alt_up_char_buffer_init(char_buffer);
			// Display Game Name, Team Name, Scores
			alt_up_char_buffer_string(char_buffer, "Team SharkCat", 0, 5);
			alt_up_char_buffer_string(char_buffer, "Game: Sneak", 20, 5);
			alt_up_char_buffer_string(char_buffer, "Highest Score: 0", 40, 5);
			alt_up_char_buffer_string(char_buffer, "Current Score: 0", 60, 5);


			device_reference = alt_up_sd_card_open_dev(SDCARD_INTERFACE_NAME);
			if (device_reference != NULL) {
					if ((connected == 0) && (alt_up_sd_card_is_Present())) {
						printf("Card connected.\n");
						if (alt_up_sd_card_is_FAT16()) {
							printf("FAT16 file system detected.\n");
							readfile();
						} else {
							printf("Unknown file system.\n");
						}
						connected = 1;
					} else if ((connected == 1) && (alt_up_sd_card_is_Present() == false)) {
						printf("Card disconnected.\n");
						connected = 0;
					}
			}


	}

	return 0;
}

void displayVGA(void){

}
void initializeVGA(void)
{
	// Initialize pixel buffer
	alt_up_pixel_buffer_dma_dev* pixel_buffer;
	// Use the name of your pixel buffer DMA core
	pixel_buffer = alt_up_pixel_buffer_dma_open_dev("/dev/pixel_buffer_dma");

	// Set the background buffer address – Although we don’t use thebackground,
	// they only provide a function to change the background buffer address, so
	// we must set that, and then swap it to the foreground.
	alt_up_pixel_buffer_dma_change_back_buffer_address(pixel_buffer, PIXEL_BUFFER_BASE);
	// Swap background and foreground buffers
	alt_up_pixel_buffer_dma_swap_buffers(pixel_buffer);

	// Wait for the swap to complete while
	(alt_up_pixel_buffer_dma_check_swap_buffers_status(pixel_buffer));

	// Initialize for char
	alt_up_char_buffer_dev *char_buffer;
	char_buffer = alt_up_char_buffer_open_dev("/dev/char_drawer");

	// Clear the screen
	alt_up_pixel_buffer_dma_clear_screen(pixel_buffer, 0);
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


void readfile(void)
{
	short error;
	char file_name[50];
	error = alt_up_sd_card_find_first('.', file_name);
	if (error != 0){
		printf("Error find first\n");
		return;
	}
	printf("First file detected: %s\n",file_name);
	while ( 1)
	{
		error = alt_up_sd_card_find_next(file_name);
		if (error) break;
		printf("File Detected: %s\n",file_name);
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
