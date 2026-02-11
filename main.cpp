#include <M5Unified.h>
#include "../lib/bejeweled.h"
#include <EEPROM.h>


// these are some settings
#define CELL_SIZE 20		// each ston is 20*20 pixels, 
#define BOARD_WIDTH 7		// 7 stones width
#define BOARD_HEIGHT 6		// 7 stones height
#define INITIAL_MOVES 20	// initial number of moves
#define MIN_TILT 0.15		// minimal tilt to move cursor, i got it from exercise 4 of wpo9
#define MOVE_DELAY 200 		// add some delay between moves when tilting

// some colors 

int colors[] = {RED, GREEN, BLUE, YELLOW, MAGENTA, ORANGE, CYAN, SILVER};
GameBoard *game;


// function to draw a single stone
// we give the coordinate and the color index (0-4)
void draw_cell(int x, int y, int color_index) {

	int color;
	if (color_index == 255) {	// EMPTY_CELL assigned to BLACK
		color = BLACK;

	} else {
			color = colors[color_index % game->num_colors];	// actually we do a mod to be sure the index is not out of range, itshoud work without it too 
	}

	//draw the stone
	M5.Lcd.fillRect(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, color);

	// draw a contour ("rand" in dutch), so that we can see the stones as independent indivduals
	M5.Lcd.drawRect(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, BLACK);

}


// function to draw the whole board
// we loop over all cells and call the function draw_cell (above this one)
void draw_game_board() {

	for(int	y = 0; y < game->height; y++) {
		for(int	x = 0; x < game->width; x++) {
			uint8_t cell = game->grid[y][x].color;		// we use again uint8_t cause we did that also in the struct (headerfile of bejewled)
				draw_cell(x, y, cell);
		}
	}
}

9/20

// function to draw the cursor
// it is a rectangle around 2 stones, either horizontal or vertical
void draw_cursor() {

	// convert to pixels
	int x = game->cursor_x * CELL_SIZE;
	int y = game->cursor_y * CELL_SIZE;

	// it is horizontal
	if (game->cursor_vertical_horizontal == 0) {

	// draw a white line around 2 stones (horizontal)
	M5.Lcd.drawRect(x, y, CELL_SIZE * 2, CELL_SIZE, WHITE);
	
	// to make a thicker line, we draw it again with 1 pixel offset
	// + 2 pixels in width and height to cover all corners
	M5.Lcd.drawRect(x-1, y-1, CELL_SIZE * 2 +2, CELL_SIZE +2, WHITE);
 
	}

	// it is vertical
	else {
	M5.Lcd.drawRect(x, y, CELL_SIZE, CELL_SIZE * 2, WHITE);
	M5.Lcd.drawRect(x-1, y-1, CELL_SIZE +2, CELL_SIZE * 2 +2, WHITE);
	}
}


// function to setup the game at the start of the program
// we initialize M5, ask the player which mode he wants (easy or hard)
// and we fill the board with random stones
// we also make sure there are no matches at the start of the game
void setup() {

	// some configurations, these are default values 
	// then start M5 with these configurations
    
	auto cfg = M5.config();
    M5.begin(cfg);

	// eeprom is for saving and loading the game state, we initialize it with 512 bytes as in exercises
	EEPROM.begin(512);	// initialize EEPROM with 512 bytes, also in oef 2  wpo 9

	// fix the memory allocation
    game = init_game_board(BOARD_WIDTH, BOARD_HEIGHT, INITIAL_MOVES);

	// lets ask the player which mode he wants
	M5.Lcd.fillScreen(BLACK);		// clear screen
	M5.Lcd.setCursor(10, 10);		// position for text
	M5.Lcd.printf("HARD MODE = A");

	M5.Lcd.setCursor(10, 30);
	M5.Lcd.printf("EASY MODE = B");		

	// wait for player to press A or B
	while (true) {
		M5.update();
		if (M5.BtnA.wasPressed()) {
			game->easy_mode = 0;		// hard mode
			break;
		}
		if (M5.BtnB.wasPressed()) {
			game->easy_mode = 1;		// easy mode
			break;
		}
	}

	// fill the grid with random colors of stones
    fill_random_game_board(game);
	
	// after filling the board, there might be some matches already present
	// we remove them by checking for matches, applying gravity and filling again
	while (check_matches(game) > 0) {	
		apply_gravity(game);
		fill_random_game_board(game);
	}

	game->score = 0;					// reset score to 0, so that player doesnt get free point

	M5.Lcd.fillScreen(BLACK);		// clear screen
}


// function to save a game with EEPROM, based on exercise 6 of wpo9 
// based on wpo9 oef 6
void save_game() {

	// we start at address 0 of eeprom, we can go until 512 bytes but that would probaly be not needed
	int address = 0;

	// first save some general data
	// we use Byte cause all values are small, so we can spare memory
	EEPROM.writeInt(address, game->score); 
	address+(s);
	EEPROM.writeByte(address, game->moves_left);
	address++;
	EEPROM.writeByte(address, game->cursor_x);
	address++;
	EEPROM.writeByte(address, game->cursor_y);
	address++;
	EEPROM.writeByte(address, game->cursor_vertical_horizontal);
	address++;
	EEPROM.writeByte(address, game->easy_mode);
	address++;
	EEPROM.writeByte(address, game->num_colors);
	address++;

	// noow we gonna save the whole grid, it is width * height cells
	// we loop over all cells and save the color value of each cell
	for (int y = 0; y < game->height; y++) {
		for (int x = 0; x < game->width; x++) {
			EEPROM.writeByte(address, game->grid[y][x].color);
			address++;
		}
	}
	EEPROM.commit();		// the commit is important, otherwise the data is not really saved
}


// function to load a game from EEPROM, its really the reverse of save_game, and also based on wpo9 oef 6
void load_game() {
	int address = 0;

	// again read the general data in the right order as save function (save_game)
	game->score = EEPROM.readByte(address);
	address++;
	game->moves_left = EEPROM.readByte(address);
	address++;
	game->cursor_x = EEPROM.readByte(address);
	address++;
	game->cursor_y = EEPROM.readByte(address);
	address++;
	game->cursor_vertical_horizontal = EEPROM.readByte(address);
	address++;
	game->easy_mode = EEPROM.readByte(address);
	address++;
	game->num_colors = EEPROM.readByte(address);
	address++;


	for (int y = 0; y < game->height; y++) {
		for (int x = 0; x < game->width; x++) {
			game->grid[y][x].color = EEPROM.readByte(address);
			address++;
		}
	}
}

// function to show the score and moves left
// we gonna show these stats beneath the board
void show_stats() {

	int text_y = BOARD_HEIGHT * CELL_SIZE + 65;		// this is postion under the board

	M5.Lcd.fillRect(0, text_y, 135, 30, BLACK);	// clear previous stats

	M5.Lcd.setTextColor(WHITE);					// white text
	M5.Lcd.setCursor(5, text_y);				// position for score
	M5.Lcd.printf("Score: %d", game->score);	// print score

	M5.Lcd.setCursor(5, text_y + 10);
	M5.Lcd.printf("Moves left: %d      ", game->moves_left);
}




//first we make an array with menu items
const char* menu_items[] = {
    "BACK",
    "SAVE",
    "LOAD",
    "RESET LEVEL"
};

int selected_menu_item = 0;		// this keeps track of which option in the menu is selected
bool in_menu = false;			// whether we are in the menu or in the game


// function to draw the menu
void draw_menu() {
	M5.Lcd.fillScreen(BLACK);		// clear screen

	// draw all menu items
	for (int i = 0; i < 4; i++) {

		M5.Lcd.setCursor(10, 10 + i * 30);			// set cursor position, so that items are under each other and not above each other

		if (i == selected_menu_item) {
			M5.Lcd.setTextColor(BLUE);				// selected item
			M5.Lcd.printf("%s", menu_items[i]);		// print the item with blue color (selected)
		} else {
			M5.Lcd.setTextColor(WHITE);				// unselected item
			M5.Lcd.printf("%s", menu_items[i]);
		}
	}
}


// this function is if we win (hit the target score), we go to the next level
// we make the grid bigger and reset everything(score, moves, etc)
void next_level() {
	
	int saved_mode = game->easy_mode;	// save the easy/hard mode for next level
	int next_color = game->num_colors;	// keep the same number of colors
	int current_score = game->score;	// keep current score, so that player doesnt lose it

	// increase the height of the board by 1, if we hit max height, we reset to initial height
	int new_height = game->height + 1;
	if (new_height > 9) {				// max height
		new_height = BOARD_HEIGHT;		
	}

	if (next_color < 8) {		// max 8 colors
		next_color++;			// increase number of colors to make it harder
	}

	int new_width = game->width;     	// keep width same

	free_game_board(game);				//free the memory of the old board, this is important to avoid memory leaks

	game = init_game_board(new_width, new_height, INITIAL_MOVES);	// init new game board, with the new height and less moves, cause board is bigger so less difficult

	game->score = current_score;	// reset score to 0, so that player
	game->num_colors = next_color;	// set the number of colors
	game->easy_mode = saved_mode;	// restore easy/hard mode

	// fill the grid with random colors of stones
	fill_random_game_board(game);
	while (check_matches(game) > 0) {	// just to be sure there are no matches at the start
		apply_gravity(game);
		fill_random_game_board(game);

	}


	M5.Lcd.fillScreen(BLACK);		// clear screen
}

//  this variable is the memory of the last time we moved the cursor with tilting
// we use as type unsigned long cause we compare it with millis() function and int is too small for that
unsigned long lastMoveTime = 0;
int next_target_score = 20; // score to reach to win the game

// the main loop of the program
// we check for button presses to move the cursor and swap stones
// we also check for tilt to move the cursor
// we also check for matches after each move
// we also handle the menu logic
void loop() {
    M5.update();	// this is standard used to update the buttons


	// --MENU LOGIC--, we check if we are in the menu
	if (in_menu){
		
		// if button B is pressed, we select the menu item
		// wasPressed means that we only trigger once when button is pressed
		// so that we dont move multiple items with one press with isPressed
		if (M5.BtnA.wasPressed()) {

			// move up in menu, and wrap around if needed
			selected_menu_item--;
			if (selected_menu_item < 0) {
				selected_menu_item = 3;		// wrap around to last item
			}
			draw_menu();
		}

		// if button B is pressed, we select the menu item
		if (M5.BtnB.wasPressed()) {

			//0. BACK -> close menu
			if(selected_menu_item == 0) {	// this one is really obvious, just close the menu
				in_menu = false;
				M5.Lcd.fillScreen(BLACK);
			}


			// 1. SAVE -> save game
			else if (selected_menu_item == 1) { 	// if SAVE, we save the game
				save_game();

				M5.Lcd.fillScreen(BLACK); 			// this is for myself, to make sure the player sees its saved
				M5.Lcd.printf("Game succesfully saved");
				delay(1000);						// wait a second so player can see its saved

				in_menu = false;
				M5.Lcd.fillScreen(BLACK);
			}

			// 2. LOAD -> load game
			else if (selected_menu_item == 2){	//if load than we load game
				load_game();

				M5.Lcd.fillScreen(BLACK);
				M5.Lcd.printf("Game succesfully loaded!");
				delay(1000);		

				in_menu = false;
				M5.Lcd.fillScreen(BLACK);
			}

			// 3. RESET LEVEL -> reset everything
			else if (selected_menu_item == 3) { 
				M5.Lcd.fillScreen(BLACK);
				selected_menu_item = 0;		// reset menu selection
				setup();		// we just call setup again to reset everything
				in_menu = false;
			}
			

			draw_menu();
		}
	}

	// if we are not in the menu, we do the game logic
	else {
	// --GAME LOGIC--
		// press A en B buttons together to enter menu
	if (M5.BtnA.isPressed() && M5.BtnB.isPressed()) {
		in_menu = true;
		draw_menu(); 		// draw the menu, so we see what to select
		return; 			// we return so that rest of the game logic is not executed and we stay in the menu

	}

	// if no moves left, we stop here
	if (game->moves_left <= 0) {
		M5.Lcd.fillScreen(BLACK);
		M5.Lcd.setCursor(10, 50);
		M5.Lcd.setTextColor(RED);
		M5.Lcd.printf("Game over ur Score=: %d", game->score);
		M5.Lcd.setCursor(10, 90);
		M5.Lcd.printf("press A+B to go to menu");
		delay(100);			// so that it doesnt spam the screen, 
		return;				
	}

	// --1. BUTTONS ---
	// if button A is pressed, change cursor orientation
    if (M5.BtnA.wasPressed()) {
        game->cursor_vertical_horizontal = !game->cursor_vertical_horizontal;


		// check for out of bounds after changing orientation
		if (game->cursor_vertical_horizontal == 0) {
			if (game->cursor_x >= BOARD_WIDTH -2) {		// if x is greater of equal to max - 2
				game->cursor_x = BOARD_WIDTH -2;		// force it to max -2
			}
    }
		else {
			if (game->cursor_y >= BOARD_HEIGHT -2) {		
				game->cursor_y = BOARD_HEIGHT -2;
			}
		}
	}

	// if button B is pressed we gonna swap the stones
	if (M5.BtnB.wasPressed()) {
		int x1 = game->cursor_x;		// get cursor position
		int y1 = game->cursor_y;
		int x2, y2;

		// depending on the cursor orientation we swap horziontally or vertically
		// first we have to calculate the second stone position

		if (game->cursor_vertical_horizontal == 0) { // horizontal
			x2 = x1 + 1;
			y2 = y1;
		} else { // vertical
			x2 = x1;
			y2 = y1 + 1;
		}

		// swap the stones
		swap_cells(game, x1, y1, x2, y2);

		// check for matches
		if (check_matches(game) > 0) {	// if there are matches

				int matches_found = 1; 		// there is minimum 1 match

				// this is some 'kettingreactie' logic
				while (matches_found > 0) {

				draw_game_board();		// draw the board with the gaps
				delay(250);				// we wait a bit so we see the actual gap

				apply_gravity(game);		// make the stones fall down
				refill_gameboard(game);		// refill the empty spaces

				draw_game_board();
				delay(250);				// wait a bit so the player can see the 'falling' stones

				matches_found = check_matches(game); // check for new matches
			}

			game->moves_left--;		// decrease moves left
		}

		// no matches found, we swap back the stones or do nothing based on mode
		else {
			// no matches found, we check whether we play easy or hard mode
			// in easy mode we do nothing, in hard mode we swap back the stones

			if(!game->easy_mode){
				// swap back the stones
				swap_cells(game, x1, y1, x2, y2);
			} else {
				// in easy mode the swap is valid and we decrease moves left
				game->moves_left--;
			}

		}
	}


	// --2. Tilting, fully based on exercise 4 and 5 from wpo9
    float accX = 0, accY = 0, accZ = 0;
    M5.Imu.getAccelData(&accX, &accY, &accZ); // get IMU data

	// we use a timer so the cursor does not move every 20ms
	// millis = time since start of program, lastMoveTime = last time we moved the cursor, and then we compare with MOVE_DELAY
	// so that there is some delay between moves
    if (millis() - lastMoveTime > MOVE_DELAY) {
        bool moved = false;

		// logic from Exercise 4, we check tilt directions
        // horzizontal movement X-as
        if (accX > MIN_TILT) { // bend to left

            if (game->cursor_x > 0) { // this is a check so that we do not go out of the board
                game->cursor_x--;
                moved = true;
            }
        } else if (accX < -MIN_TILT) { // bend to right
            int max_x;
			

			// if the cursor is horizontal or vertical, the max x is different
			// for example if the cursor is horizontal it cant go to the last column cause it needs 2 columns to show
			if (game->cursor_vertical_horizontal == 0) { // horizontal cursor
				max_x = game->width - 2;
			} else { // vertical cursor
				max_x = game->width - 1;
			}

			// check for out of bounds
            if (game->cursor_x < max_x) {
                game->cursor_x++;
                moved = true;
            }
        }

        // Vertical movement Y-as
        if (accY > MIN_TILT) { // bend forward down on screen
            int max_y;
			

			if (game->cursor_vertical_horizontal == 1) { // vertical cursor
				max_y = game->height - 2;
			} else { // horizontal cursor
				max_y = game->height - 1;
			}

            if (game->cursor_y < max_y) {
                game->cursor_y++;
                moved = true;
            }
        } else if (accY < -MIN_TILT) { // bend backward up on screen
            if (game->cursor_y > 0) {
                game->cursor_y--;
                moved = true;
            }
        }

		// update last move time if we moved, otherwise we keep the old time
        if (moved) {
            lastMoveTime = millis();
        }
    }
	
	// we habe to increase the target score each level, otherwise we will have infinite loop when we reach the target score
	if (game->score >= next_target_score) {
		next_target_score += 30;
		next_level();
	}

    // --3. drawing the board and cursor
    draw_game_board(); 
    draw_cursor();
	show_stats();
	
}
    
    delay(20); // this delay so that the cursors does not flicker too much
				// in the exercise it was 20ms but here it looks better with 30ms
}




