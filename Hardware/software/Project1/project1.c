// Title: LightCycle.c
// Description: Implementation of LightCycle
// Author: Team SharkCat (Group 15) -- Alvin Hung, Kenneth Song, Vincent Chen, Victor Song, Ali Hossain
// January 25th 2013

//===================================================================================================
//                                   INCLUDE HEADER FILES
//===================================================================================================
#include <stdio.h>
#include "altera_up_avalon_character_lcd.h"
#include "altera_up_sd_card_avalon_interface.h"
#include "altera_up_avalon_video_pixel_buffer_dma.h"
#include "altera_up_avalon_video_character_buffer_with_dma.h"
#include "sys/alt_alarm.h"
#include "sys/alt_timestamp.h"
#include "io.h"
#include "altera_up_avalon_ps2.h"
#include "altera_up_ps2_keyboard.h"
#include "background.h"

//===================================================================================================
//	                          DEFINE CONSTANTS AND GLOBAL VARIABLES
//===================================================================================================

//Constants for DE2 built-in I/O
#define switches (volatile char *) SWITCHES_BASE
#define leds (char *) LEDS_BASE
#define keys (volatile char *) KEYS_BASE

//In-game constants
#define GAMESPEED 0.5 //seconds
#define WIDTH 320
#define HEIGHT 240
#define GRID_WIDTH 150
#define GRID_HEIGHT 150

//Keyboard key constants
#define W_KEY 0x1d
#define S_KEY 0x1b
#define A_KEY 0x1c
#define D_KEY 0x23
#define LEFT_ARROW 0x6b
#define DOWN_ARROW 0x75
#define RIGHT_ARROW 0x72
#define UP_ARROW 0x74

//Direction constants
#define LEFT 1
#define DOWN 2
#define RIGHT 3
#define UP 4

//Global variables for PS/2 Keyboard
KB_CODE_TYPE decode_mode;
alt_u8 buffer[4];
char ascii;
alt_up_ps2_dev* ps2_dev;

//Global variables for VGA pixel buffer
alt_up_pixel_buffer_dma_dev* pixel_buffer;

//Global variables for character buffer
alt_up_char_buffer_dev *char_buffer;

//===================================================================================================
//           					STRUCT HOLDING PLAYER ATTRIBUTES
//===================================================================================================

//Global variables for player direction
int player1_direction;
int player2_direction;

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
//                               	FUNCTION PROTOTYPES
//===================================================================================================




void UpdatePlayerMovement( struct Player* Player1, struct Player* Player2 );
	//=======================================================================================
	// This function checks the PS2 keyboard to see if the players wanted to change direction


int CheckPlayerCollision( struct Player* Player1, struct Player* Player2, int ** Grid);
	// Returns 0 if no crash, 1 if player 1 crashes, 2 if player 2 crashes, 3 if both players crashed
	//=======================================================================================


void GameReset( struct Player* Player1, struct Player* Player2, int ** Grid);
	// This function resets the game round
	//=======================================================================================


void UpdatePositions( struct Player* Player1, struct Player* Player2, int ** Grid);
	// This function updates the position of all players
	//=======================================================================================


void UpdateScreen( struct Player* Player1, struct Player* Player2, alt_up_pixel_buffer_dma_dev* pixel_buffer);
	// This function draws the position of all players to screen as well as the wall
	//=======================================================================================


void UpdateGame( struct Player* Player1, struct Player* Player2, int ** Grid, int *GameStatus, alt_up_pixel_buffer_dma_dev* pixel_buffer);
	// This function uses other functions to check collision, update position and screens
	//=======================================================================================

void detect_keys(void);
	// This function reads the PS2 buffers for predetermined player movement keys
	//=======================================================================================


void update_direction(struct Player* Player, int* new_direction);
	// Updates player's direction with new values from keyboard
	//=======================================================================================

void initialize_hardware(void);
	// Initializes all hardware before loading the game - VGA, LEDs, PS/2 etc
	//=======================================================================================

//===================================================================================================
//===================================================================================================



int main() {

	//Print status message
	printf("Program Started...\n");
//===================================================================================================
//                              		INITIALIZE HARDWARE
//===================================================================================================

	initialize_hardware();

//	//Print Background
//	int i,j;
//	for (i=0; i<240; i++){
//		for (j=0; j<320; j++){
//			alt_up_pixel_buffer_dma_draw(pixel_buffer, world[i][j], j, i);
//		}
//	}

	//Draws the rectangle for the Game Grid (150 by 150)
	alt_up_pixel_buffer_dma_draw_rectangle(pixel_buffer, 149, 44, 300, 195, 0x3333, 0);

	alt_up_char_buffer_clear(char_buffer);
	alt_up_char_buffer_init(char_buffer);


//===================================================================================================
//===================================================================================================



//===================================================================================================
//                               Game Grid and other initialization
//===================================================================================================

	int GameGrid[GRID_WIDTH][GRID_HEIGHT] = {0};
	int UserHasQuit = 0;
	int GameStatus = 0;

	//===========================================================
	//Create Player 1 struct and initialize positions
	struct Player Player1;
	Player1.CurrentPositionX = 37;
	Player1.CurrentPositionY = 75;

	Player1.PreviousPositionX = 37;
	Player1.PreviousPositionY = 75;

	Player1.DirectionX = 1;
	Player1.DirectionY = 0;


	//===========================================================
	//Create Player 2 struct and initialize positions
	struct Player Player2;
	Player2.CurrentPositionX = 113;
	Player2.CurrentPositionY = 75;

	Player2.PreviousPositionX = 113;
	Player2.PreviousPositionY = 75;

	Player2.DirectionX = -1;
	Player2.DirectionY = 0;


	//===========================================================
	//  Sets the starting position as being filled
	GameGrid[Player1.CurrentPositionX][Player1.CurrentPositionY] = 1;
	GameGrid[Player2.CurrentPositionX][Player2.CurrentPositionY] = 1;

//===================================================================================================
//===================================================================================================



//===================================================================================================
//                               Controls
//===================================================================================================

	alt_up_char_buffer_string(char_buffer, "Press Enter to Start!", 4, 40);
	alt_up_char_buffer_string(char_buffer, "Press 'P' to Pause", 4, 42);

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
//									START GAME
//===================================================================================================

	while(UserHasQuit != 1){


	// Function to read when user has pressed entered to start the game
	/*
	if( UserHasPressedEnter )
		GameStatus = 1;
	*/
	// This is the beginning of the game
		while( GameStatus != 1) {

			//Starts the timer to read keyboard inputs until we update the player positions and collisions
			UpdatePlayerMovement(&Player1, &Player2);

			//Updating the collision detection, movement, and screen
			//UpdateGame(&Player1, &Player2, &GameGrid, GameStatus, pixel_buffer);


		}


	}
	return 0;
}



//===================================================================================================




//===================================================================================================
//===================================================================================================
/* Draws the current and previous position onto the screen
*/
void UpdatePlayerMovement( struct Player* Player1, struct Player* Player2 ) {

	//Detect which keyboard keys are being pressed
	detect_keys();

	//Updates player's direction of motion
	update_direction(Player1, &player1_direction); //Player 1
	update_direction(Player2, &player2_direction); //Player 2

	//Set player's next position
}





//===================================================================================================




//===================================================================================================
//===================================================================================================
/* Checks collision.
		Return 0 if no collision
		Return 1 if Player 1 collided
		Return 2 if Player 2 collided
		Return 3 if both players collided

*/
int CheckPlayerCollision( struct Player* Player1, struct Player* Player2, int ** Grid) {
	int x,
		y,
		result=0;

	//===========================================================
	//Check if Player1 will crash
	if (Player1 -> DirectionX != 0){

		x= Player1-> CurrentPositionX +Player1 -> DirectionX;
		y = Player1-> CurrentPositionY;

		//===========================================================
		// Checking if player tries to move beyond edge of map, which actually takes it to the opposite side
		if( x == -1)
			x = 149;
		else if( x == 150)
			x = 0;
	}

	else if (Player1 -> DirectionY != 0){

		x= Player1-> CurrentPositionX ;
		y = Player1-> CurrentPositionY + Player1 -> DirectionY;

		//===========================================================
		// Checking if player tries to move beyond edge of map, which actually takes it to the opposite side
		if( y == -1)
			y = 149;
		else if( y == 150)
			y = 0;
	}

	//===========================================================
	// Checks if there is a wall existing here
	if (Grid[x][y] == 1)
		result += 1;

	//===========================================================
	// Check if Player2 will crash
	if (Player2 -> DirectionX != 0){

		x= Player2-> CurrentPositionX +Player2 -> DirectionX;
		y = Player2-> CurrentPositionY;

		//===========================================================
		// Checking if player tries to move beyond edge of map, which actually takes it to the opposite side
		if( x == -1)
			x = 149;
		else if( x == 150)
			x = 0;
	}

	else if (Player2 -> DirectionY != 0){

		 x= Player2-> CurrentPositionX ;
		 y = Player2-> CurrentPositionY + Player2 -> DirectionY;

		 //===========================================================
		 // Checking if player tries to move beyond edge of map, which actually takes it to the opposite side
		if( y == -1)
			y = 149;
		else if( y == 150)
			y = 0;
	}

	//===========================================================
	// Checks if there is a wall existing here
	if (Grid[x][y] == 1)
		result += 2;
	//===========================================================

	return result;
}

//===================================================================================================
//===================================================================================================
/* Reads the PS2 buffers for predetermined player movement keys
 * and updates global player direction variables
 */
void detect_keys(void) {

	//Detect make and break code
	if( ! decode_scancode(ps2_dev, &decode_mode, buffer, &ascii)){

		// Detect if any predetermined key is pressed
		if (decode_mode == KB_ASCII_MAKE_CODE || decode_mode == KB_BINARY_MAKE_CODE){

			switch (buffer[0]){

			case A_KEY: //If 'A' is pressed
				player1_direction = LEFT;
				printf("Play1 detected: %i\n",player1_direction);
				break;

			case S_KEY: //If 'S' is pressed
				player1_direction = DOWN;
				printf("Play1 detected: %i\n",player1_direction);
				break;

			case D_KEY: //If 'D' is pressed
				player1_direction = RIGHT;
				printf("Play1 detected: %i\n",player1_direction);
				break;

			case W_KEY: //If 'W' is pressed
				player1_direction = UP;
				printf("Play1 detected: %i\n",player1_direction);
				break;

			case LEFT_ARROW: //If left arrow is pressed
				player2_direction = 1;
				printf("Play2 detected: %i\n",player2_direction);
				break;

			case DOWN_ARROW: //If down arrow is pressed
				player2_direction = 4;
				printf("Play2 detected: %i\n",player2_direction);
				break;

			case RIGHT_ARROW: //If right arrow is pressed
				player2_direction = 2;
				printf("Play2 detected: %i\n",player2_direction);
				break;

			case UP_ARROW: //If up arrow is pressed
				player2_direction = 3;
				printf("Play2 detected: %i\n",player2_direction);
				break;

			default:
				break;
			}

		}
	}
}


//===================================================================================================
//===================================================================================================
/* Updates player's direction with new values from keyboard
 */
void update_direction(struct Player* Player, int* new_direction){

	// Change the direction for player 1
	// Current direction is either left or right, change to up or down
	if(Player -> DirectionX != 0)
	{
		// Change direction to either up or down
		switch (*new_direction){

		case UP:
			// Go up
			Player -> DirectionX = 0;
			Player -> DirectionY = 1;
			printf("Player1_DirectionX:%i   Player1_DirectionY: %i", Player -> DirectionX, Player -> DirectionY);
			break;

		case DOWN:
			// Go down
			Player -> DirectionX = 0;
			Player -> DirectionY = -1;
			printf("Player1_DirectionX:%i   Player1_DirectionY: %i", Player -> DirectionX,Player -> DirectionY);
			break;

		default:
			break;
		}
	}
	// Current direction is either up or down, change to left or right
	else if (Player -> DirectionY != 0)
	{
		// Change direction to either up or down
		switch (*new_direction){

		case LEFT:
			// Go left
			Player -> DirectionX = -1;
			Player -> DirectionY = 0;
			printf("Player1_DirectionX:%i   Player1_DirectionY: %i", Player -> DirectionX, Player -> DirectionY);
			break;

		case RIGHT:
			// Go right
			Player -> DirectionX = 1;
			Player -> DirectionY = 0;
			printf("Player1_DirectionX:%i   Player1_DirectionY: %i", Player -> DirectionX, Player -> DirectionY);
			break;

		default:
			break;
		}
	}
}





//===================================================================================================
//===================================================================================================
/* Reset game round

*/
void GameReset( struct Player* Player1, struct Player* Player2, int ** Grid) {

		//===========================================================
		// Resetting all the starting positions for Player 1
		Player1->CurrentPositionX = 37;
		Player1->CurrentPositionY = 75;

		Player1->PreviousPositionX = 37;
		Player1->PreviousPositionY = 75;

		Player1->DirectionX = 1;
		Player1->DirectionY = 0;

		//===========================================================
		// Resetting all the starting positions for Player 2
		Player2->CurrentPositionX = 113;
		Player2->CurrentPositionY = 75;

		Player2->PreviousPositionX = 113;
		Player2->PreviousPositionY = 75;

		Player2->DirectionX = -1;
		Player2->DirectionY = 0;

		//===========================================================
		// Resetting the map
		int x,y;
		for(x=0; x <150; x++) {
			for(y=0; y<150; y++) {
				Grid[x][y] = 0;
			}
		}

		//===========================================================
		// Making sure starting position is value 1 in the grid
		Grid[Player1 -> CurrentPositionX][Player1 -> CurrentPositionY] = 1;
		Grid[Player2 -> CurrentPositionX][Player2 -> CurrentPositionY] = 1;
}





//===================================================================================================




//===================================================================================================
//===================================================================================================
/* Updates the appropriate player positions

*/
void UpdatePositions( struct Player* Player1, struct Player* Player2, int ** Grid) {

		//===========================================================
		// Updates the location of Player 1
		if (Player1 -> DirectionX != 0){

				Player1 ->PreviousPositionX = Player1-> CurrentPositionX;
				Player1 ->CurrentPositionX = Player1-> CurrentPositionX +Player1 -> DirectionX;

				//==================================
				// Creates the pacman map edges
				if( Player1 -> CurrentPositionX == -1)
					Player1 -> CurrentPositionX  = 149;
				else if( Player1 -> CurrentPositionX  == 150)
					Player1 -> CurrentPositionX  = 0;

				Player1 ->PreviousPositionY = Player1-> CurrentPositionY;
				// Y current position not changed
			}
		else if (Player1 -> DirectionY != 0){

				Player1 ->PreviousPositionY = Player1-> CurrentPositionY;
				Player1 ->CurrentPositionY = Player1-> CurrentPositionY +Player1 -> DirectionY;

				//==================================
				// Creates the pacman map edges
				if( Player1 -> CurrentPositionY == -1)
					Player1 -> CurrentPositionY  = 149;
				else if( Player1 -> CurrentPositionY  == 150)
					Player1 -> CurrentPositionY  = 0;

				Player1 ->PreviousPositionX = Player1-> CurrentPositionX;
				// X current position not changed
			}


		//===========================================================
		// Updates the position of Player 2
		if (Player2 -> DirectionX != 0){

				Player2 ->PreviousPositionX = Player2-> CurrentPositionX;
				Player2 ->CurrentPositionX = Player2-> CurrentPositionX +Player2 -> DirectionX;

				//==================================
				// Creates the pacman map edges
				if( Player2 -> CurrentPositionX == -1)
					Player2 -> CurrentPositionX  = 149;
				else if( Player2 -> CurrentPositionX  == 150)
					Player2 -> CurrentPositionX  = 0;

				Player2 ->PreviousPositionY = Player2-> CurrentPositionY;
				// Y current position not changed
			}
		else if (Player2 -> DirectionY != 0){

				Player2 ->PreviousPositionY = Player2-> CurrentPositionY;
				Player2 ->CurrentPositionY = Player2-> CurrentPositionY +Player2 -> DirectionY;

				//==================================
				// Creates the pacman map edges
				if( Player2 -> CurrentPositionY == -1)
					Player2 -> CurrentPositionY  = 149;
				else if( Player2 -> CurrentPositionY  == 150)
					Player2 -> CurrentPositionY  = 0;

				Player2 ->PreviousPositionX = Player2-> CurrentPositionX;
				// X current position not changed
			}

		//===========================================================
		// Updating the grid
		Grid[Player1 -> CurrentPositionX][Player1 -> CurrentPositionY] = 1;
		Grid[Player2 -> CurrentPositionX][Player2 -> CurrentPositionY] = 1;
}


//===================================================================================================




//===================================================================================================
//===================================================================================================
/* Draws the current and previous position onto the screen

*/
void UpdateScreen( struct Player* Player1, struct Player* Player2, alt_up_pixel_buffer_dma_dev* pixel_buffer) {

	alt_up_pixel_buffer_dma_draw( pixel_buffer, 4, Player1->PreviousPositionX +150, Player1->PreviousPositionY +45);
	alt_up_pixel_buffer_dma_draw( pixel_buffer, 5, Player1->CurrentPositionX +150, Player1->CurrentPositionY +45);

	alt_up_pixel_buffer_dma_draw( pixel_buffer, 6, Player2->PreviousPositionX +150, Player1->PreviousPositionY +45);
	alt_up_pixel_buffer_dma_draw( pixel_buffer, 7, Player2->CurrentPositionX +150, Player1->CurrentPositionY +45);

}
//===================================================================================================





//===================================================================================================
//===================================================================================================
/* If one or two player will collide, this function will call another function for winning/losing animation.
	Check Collision
		Resets game if collision detected
		Draw Box of black to wipe game screen
	Else
		Update Player Positions

	Update Screen

*/
void UpdateGame( struct Player* Player1, struct Player* Player2, int** Grid, int *GameStatus, alt_up_pixel_buffer_dma_dev* pixel_buffer) {
	int terminate =0;
	//===========================================================
	// Check Collision
	terminate = CheckPlayerCollision( Player1, Player2, Grid);


	if (terminate != 0){

		//===========================================================
		// If collision detected
		*GameStatus = 0;
		GameReset(Player1, Player2, Grid);

		//===========================================================
		// Draws black box to clear the positions
		alt_up_pixel_buffer_dma_draw_box(pixel_buffer, 150, 45, 299, 194, 0, 0);

		// Display win or lose

	}

	else{
		//===========================================================
		// Updating the grid
		UpdatePositions( Player1, Player2, Grid);

		//===========================================================
		// Draw pixels where the players are
		UpdateScreen(Player1, Player2, pixel_buffer);
	}
}
//===================================================================================================
//===================================================================================================
//===================================================================================================
/* Initializes all hardware before loading the game - VGA, LEDs, PS/2, etc.
*/
void initialize_hardware(void){

	// Use the name of your pixel buffer DMA core
	pixel_buffer = alt_up_pixel_buffer_dma_open_dev(BUFFER_DMA_NAME);

	// Set the background buffer address – Although we don’t use the background,
	// they only provide a function to change the background buffer address, so
	// we must set that, and then swap it to the foreground.
	alt_up_pixel_buffer_dma_change_back_buffer_address(pixel_buffer, PIXEL_BUFFER_BASE);

	// Swap background and foreground buffers
	alt_up_pixel_buffer_dma_swap_buffers(pixel_buffer);

	// Wait for the swap to complete while
	(alt_up_pixel_buffer_dma_check_swap_buffers_status(pixel_buffer));

	// Initialize character buffer
	char_buffer = alt_up_char_buffer_open_dev("/dev/character_buffer");

	// Clear the screen
	alt_up_pixel_buffer_dma_clear_screen(pixel_buffer, 0);

	//Initialize the PS2 controller
	ps2_dev = alt_up_ps2_open_dev(PS2_0_NAME);

}

