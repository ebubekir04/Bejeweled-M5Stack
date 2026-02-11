#ifndef M5_PROJECT_BEJEWELED_H
#define M5_PROJECT_BEJEWELED_H
#include <stdint.h>

// some settings
#define NUM_COLORS 5
#define EMPTY_CELL 255      // this value means that the cell is empty, later we assign is to BLACK in the draw_game_board function


// we define some Structs
// we use uint8_t as type, so that we can be 'greedy' about the memory
//int = 4 bytes en uint8_t is 1 byte
typedef struct {
    uint8_t color;                //which color (0-4)
    uint8_t is_special;         // is it a special stone (triangle), didnt have time to implement yet
}   Cell;


// we use normal int, because there is just one instance of it per game, so it wont waste much memory
typedef struct {
    int width;       // width and length of the board
    int height;
    Cell **grid;    // 2D array of Cells , it is the grid itself

    int score;
    int moves_left;

    int easy_mode;                       // is easy mode on or off
    int num_colors;                       // number of different colors in the game

    int cursor_x;                        // coordinate of the cursor
    int cursor_y;
    int cursor_vertical_horizontal;     // is the cursor vertical or horizontal, this is later important for the swapping of 2 cells
}   GameBoard;


// those are the function prototypes
GameBoard* init_game_board(int width, int height, int moves);
void free_game_board(GameBoard* board);
void fill_random_game_board(GameBoard* board);
void swap_cells(GameBoard* board, int x1, int y1, int x2, int y2);
int check_matches(GameBoard* board);
void apply_gravity(GameBoard* board);
void refill_gameboard(GameBoard* board);

#endif
