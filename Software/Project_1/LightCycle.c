// Title: LightCycle.c
// Description: Implementation of LightCycle
// Author: Team SharkCat (Group 15) -- Alvin Hung
// January 25th 2013

#include "altera_up_avalon_video_pixel_buffer_dma.h"
#include "altera_up_avalon_video_character_buffer_with_dma.h"
#include <alt_types.h>
#include "alt_up_ps2_port.h"
#include "ps2_keyboard.h"

#include "sys/alt_timestamp.h"
#include <stdio.h>

#define Sws  (volidate char*) 0x0004460

//===================================================================================================	
//                               Struct that will hold individual player values
//===================================================================================================

struct Player {
	int CurrentPositionX,		// Current X position
		CurrentPositionY,		// Current Y Position 

		PreviousPositionX,		// Previous X Position
		PreviousPositionY,		// Previous Y Position

		DirectionX,				// Movement Directions
		DirectionY;
};

//===================================================================================================	
//===================================================================================================
	


//===================================================================================================	
//                               Declaring Functions
//===================================================================================================

		// This function checks the PS2 keyboard to see if the players wanted to change direction
void UpdatePlayerMovement( struct Player* Player1, struct Player* Player2 );

		// Returns 0 if no crash, 1 if player 1 crashes, 2 if player 2 crashes, 3 if both players crashed
int CheckPlayerCollision( struct Player* Player1, struct Player* Player2, int ** Grid);

void UpdatePositions( struct Player* Player1, struct Player* Player2, int ** Grid);

//===================================================================================================	
//===================================================================================================



int main() {	

//===================================================================================================	
//                               Initializing Pixel and VGA Things
//===================================================================================================

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

	// Draws the rectangle for the Game Grid (150 by 150)
	alt_up_pixel_buffer_dma_draw_rectangle(pixel_buffer, 149, 44, 300, 195, 0x3333, 0);

	alt_up_char_buffer_init(char_buffer);

	// Clear the FIFO for the PS2 port
	clear_FIFO();

	DECODE_MODE decode_mode;

	alt_u8 byte;

//===================================================================================================	
//===================================================================================================



//===================================================================================================	
//                               Game Grid and other initialization
//===================================================================================================

	int GameGrid[150][150] = {0};
	int UserHasQuit = 0;
	int GameStatus = 0;
	Struct Player Player1;
	Struct Player Player2;

//===================================================================================================	
//===================================================================================================
	


//===================================================================================================	
//                               Player Controls
//===================================================================================================

	alt_up_char_buffer_string(char_buffer, "Player 1: RED", 4, 46);
	alt_up_char_buffer_string(char_buffer, "W- Up", 8, 47);
	alt_up_char_buffer_string(char_buffer, "S- Down", 8, 48);
	alt_up_char_buffer_string(char_buffer, "A- Left", 8, 49);
	alt_up_char_buffer_string(char_buffer, "D- Right", 8, 50);


	alt_up_char_buffer_string(char_buffer, "Player 2: BLUE", 4, 53);
	alt_up_char_buffer_string(char_buffer, "Arrow Up- Up", 8, 54);
	alt_up_char_buffer_string(char_buffer, "Arrow Down - Down", 8, 55);
	alt_up_char_buffer_string(char_buffer, "Arrow Left- Left", 8, 56);
	alt_up_char_buffer_string(char_buffer, "Arrow Right- Right", 8, 57);

//===================================================================================================	
//===================================================================================================

	while(UserHasQuit != 1){


	// Function to read when user has pressed entered to start the game
	/*
	if( UserHasPressedEnter )
		GameStatus = 1;
	*/

	// This is the beginning of the game
		while( GameStatus != 0) {
			
			alt_timestamp_start();
			while( ((float) alt_timestamp() / (float) alt_timestamp_freq()) < 0.5 ){
				UpdatePlayerMovement(&Player1, &Player2);
			}
			GameStatus = CheckPlayerCollision(&Player1, &Player2, GameGrid);
			if( GameStatus > 0 ) {
				// Write winner and loser and tie functions
			}
			else {
				// Update Positions 
				UpdatePositions( &Player1, &Player2, GameGrid);
			}

		
		}


	}
	return 0;
}

		// This function checks the PS2 keyboard to see if the players wanted to change direction
void UpdatePlayerMovement( struct Player* Player1, struct Player* Player2 ) {
}

/* Predict whether the players will collide in the next movement.
 * Returns 0 if no crash, 1 if player 1 crashes, 2 if player 2 crashes, 3 if both players crashed
 */
int CheckPlayerCollision( struct Player* Player1, struct Player* Player2, int ** Grid) {
	// Assume we can only move either horizontal or vertical not diagonal.
	// Assume this function is called after the keyboard is being detected.
	// Current positions
	int x,y,result=0;

	//Check if Player1 will crash
	if (Player1 -> DirectionX != 0)
	{
		x= Player1-> CurrentPositionX +Player1 -> DirectionX;
		y = Player1-> CurrentPositionY;
	}
	else if (Player1 -> DirectionY != 0)
	{
		 x= Player1-> CurrentPositionX ;
		 y = Player1-> CurrentPositionY + Player1 -> DirectionY;
	}
	if (Grid[x][y] == 1)
	{
		printf("Play1 Crashes\n");
		result += 1;
	}

	// Check if Player2 will crash
	if (Player2 -> DirectionX != 0)
	{
		x= Player2-> CurrentPositionX +Player2 -> DirectionX;
		y = Player2-> CurrentPositionY;
	}
	else if (Player2 -> DirectionY != 0)
	{
		 x= Player2-> CurrentPositionX ;
		 y = Player2-> CurrentPositionY + Player2 -> DirectionY;
	}
	if (Grid[x][y] == 1)
	{
		printf("Play2 Crashes\n");
		result += 2;
	}

	// Check if both players crash
	if (result >2)
	{
		printf("Both players crash\n");
	}
	return result;
}

/* This function will update the positions of each character on
 * If one or two player will collide, this function will call another function for winning/losing animation.
 */

void UpdatePositions( struct Player* Player1, struct Player* Player2, int ** Grid) {
	int terminate =0;
	terminate = CheckPlayerCollision( Player1, Player2, Grid);
	if (terminate != 0)
	{
		// Display win or lose
	}
	else{
		// Update players positions
		if (Player1 -> DirectionX != 0)
			{
				Player1 ->PreviousPositionX = Player1-> CurrentPositionX;
				Player1 ->CurrentPositionX = Player1-> CurrentPositionX +Player1 -> DirectionX;
				Player1 ->PreviousPositionY = Player1-> CurrentPositionY;
				// Y current position not changed
			}
			else if (Player1 -> DirectionY != 0)
			{
				Player1 ->PreviousPositionY = Player1-> CurrentPositionY;
				Player1 ->CurrentPositionY = Player1-> CurrentPositionY +Player1 -> DirectionY;
				Player1 ->PreviousPositionX = Player1-> CurrentPositionX;
				// X current position not changed
			}

		if (Player2 -> DirectionX != 0)
			{
				Player2 ->PreviousPositionX = Player2-> CurrentPositionX;
				Player2 ->CurrentPositionX = Player2-> CurrentPositionX +Player2 -> DirectionX;
				Player2 ->PreviousPositionY = Player2-> CurrentPositionY;
				// Y current position not changed
			}
			else if (Player2 -> DirectionY != 0)
			{
				Player2 ->PreviousPositionY = Player2-> CurrentPositionY;
				Player2 ->CurrentPositionY = Player2-> CurrentPositionY +Player2 -> DirectionY;
				Player2 ->PreviousPositionX = Player2-> CurrentPositionX;
				// X current position not changed
			}
	}


}