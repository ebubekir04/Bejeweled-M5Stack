#include "../lib/bejeweled.h"
#include <cstdio>

// I actualy took this function from the preporject, and did some really small adjustmenst
// so (again) this function allocates memory and setts the standard start values of the game
// mines_count replaced by moves (number of moves left)
// pointer to  array with pointers with size = height, each pointer in the vector points to an other vector where the actual cells are in
// I could have used a simpler method ex: calloc(height * width, sizeof(cell)), but then the acces to a cell would be difficlutier to acces
// ex; grid[(2 * width) + 3] in stead of grid[2][3]
// and in general we see grids as matrices, and that is vector of vectors, anod not a single vector or list


GameBoard* init_game_board(int width, int height, int moves) {

    // we need to allocate memory for the GameBoard struct, data as width height and movesLeft are stored
    GameBoard* game_board = (GameBoard*) calloc(1, sizeof(GameBoard));

    //check if memory allocation was succesfull
    if (game_board == NULL) {
        fprintf(stderr, "FAIL by allocating memory for the gameboard\n");
        return NULL;
    }

    //array of pointers to rows, each pointer will point to an array of Cells
    game_board->grid = (Cell**) calloc(height, sizeof(Cell*));
    // allocate memory for the rows of the grid, we use calloc so that memory is initialized to zero
    if (game_board->grid == NULL) {
        fprintf(stderr, "FAIL by allocating memory for the grid rows\n");
        free(game_board); // free previously allocated memory
        return NULL;
    }


    // allocate memory for each rows columns
    for (int i = 0; i < height; i++) {

        // using of calloc to initialize memory to zero, otherwhise we could use malloc and manually set each cells values to zero
        game_board->grid[i] = (Cell*) calloc(width, sizeof(Cell));
        if (game_board->grid[i] == NULL) {
            fprintf(stderr, "FAIL by allocating memory for the grid columns\n");
            // free previously allocated memory
            for (int j = 0; j < i; j++) {
                free(game_board->grid[j]);
            }
            free(game_board->grid);
            free(game_board);
            return NULL;
        }

    }

    //we use game_board->... instead of (*game_board.)
    // because game_board is a pointer to a struct, so we can use the arrow operator -> to acces its members
    game_board->width = width;
    game_board->height = height;
    game_board->moves_left = moves;
    game_board->score = 0;
    game_board->num_colors = 5;          // default number of colors
    return game_board;
}


// now visaversa, function to free the memory allocated for the game board
// this is really important to avoid the system crashing because of memory leaks
void free_game_board(GameBoard* board) {

    // first free each row of the grid
    for (int i = 0; i < board->height; i++) {
        free(board->grid[i]);
    }

    // then free the array of row pointers
    free(board->grid);

    // finally free the GameBoard struct itself
    free(board);
}

// fil the board with random colors for each cell,
void fill_random_game_board(GameBoard* board) {
    for (int i = 0; i < board->height; i++) {
        for (int j = 0; j < board->width; j++) {
            board->grid[i][j].color = rand() % board->num_colors;      // randomizing the color, we take the modulo so we stay in the range of the colors
        }
    }
}


//--------------------------- LOGIC OF THE GAME-----------------------------------

// function to swapp two cells
// we give the board and the coordinates of the 2 cells to swap
void swap_cells(GameBoard* board, int x1, int y1, int x2, int y2) {
    
    //check wether we are not outside the grid
    // the return causes that we justs exit the function without doin anythin
    if (x1 < 0 || x1 >= board->width || y1 < 0 || y1 >= board->height) return;
    if (x2 < 0 || x2 >= board->width || y2 < 0 || y2 >= board->height) return;

    Cell temp = board->grid[y1][x1];                 // place cell A in temp
    board->grid[y1][x1] = board->grid[y2][x2];       // place cell B in A
    board->grid[y2][x2] = temp;                      // place Temp in B
}


// function to check for matches of 3 or more stones in a row or column
// we dont directly bring changes on the board , because we can miss some cross scenarios when u directly make adjustments
// ex; if you have a cross shape of same color, and you gonna remove the horizontal first, then u will miss the vertical match
// so we first mark all stones that have to be removed, and after that we remove them
int check_matches(GameBoard* board) {

    // make a new 'rooster' that is as big as the original gameboard
    int to_remove[board->height][board->width];


    // fill the new rooster with 0 everywhere
    for (int y = 0; y < board->height; y++) {
        for (int x = 0; x < board->width; x++) {
            to_remove[y][x] = 0;
        }
    }

    //horizontal search, (rows)
    for (int y = 0; y < board->height; y++) {
        for (int x = 0; x < board->width - 2; x++) {    //-2 because we look 2 steps ahead, otherwise we can go out of the board
            uint8_t c1 = board->grid[y][x].color;       // we use uint8_t because color is also uint8_t, and yeah it saves memory, 
            uint8_t c2 = board->grid[y][x + 1].color;   // unint8_t is 8 bits = 1 byte, and int is often 4 bytes =  32 bits
            uint8_t c3 = board->grid[y][x + 2].color;

            // if they are all three the same AND NOT empty
            // this does not mean that there cant be more than 3 in a row
            // we will just mark all of them in the next iterations
            if (c1 != EMPTY_CELL && c1 == c2 && c1 == c3) {
                to_remove[y][x] = 1;
                to_remove[y][x + 1] = 1;
                to_remove[y][x + 2] = 1;
            }
        }
    }

    //vertical search (columns)
    for (int x = 0; x < board->width; x++) {
        for (int y = 0; y < board->height - 2; y++) {   //-2 again , we go downstairs, otherwise we can go out of the board
            uint8_t c1 = board->grid[y][x].color;
            uint8_t c2 = board->grid[y + 1][x].color;
            uint8_t c3 = board->grid[y + 2][x].color;
            if (c1 != EMPTY_CELL && c1 == c2 && c1 == c3) {
                to_remove[y][x] = 1;
                to_remove[y + 1][x] = 1;
                to_remove[y + 2][x] = 1;
            }
        }
    }

    //remove everything that is marked 1
    int score_increment = 0;
    for (int y = 0; y < board->height; y++) {
        for (int x = 0; x < board->width; x++) {
            if (to_remove[y][x] == 1) {
                board->grid[y][x].color = EMPTY_CELL;      // make empty
                score_increment++;  // 1 point per blok
            }
        }
    }

    //update the totalscore that we already had by adding now this socre
    board->score += score_increment;

    return score_increment;
}




// this function applies gravity to the board, so that stones fall down to fill empty spaces
// we have to implement some 'gravity', not the actual physics but the illusion of it
// we dont want gaps between 2 blocks ex; [A] [EMPTY] [B] -> [EMPTY] [A] [B]
// basically, there are 3 loops: 1) X-loop loops over all columns
//                               2) Y-Loop searchs for an emptycell
//                               3) K-loop searach for a stond above the empty cell
void apply_gravity(GameBoard* board) {

    //loop over each column
    for (int x = 0; x < board->width; x++) {

        // for each column go from under to high, this feels a bit weird in programming, because we work actually the opposite way often
        for (int y = board->height - 1; y >= 0; y--) {

            //is that place empty?
            if (board->grid[y][x].color == EMPTY_CELL) {

                // if empty, search for the first place above that
                for (int k = y - 1; k >= 0; k--) {

                    // did we found a non-empty block?
                    if (board->grid[k][x].color != EMPTY_CELL) {

                        // replace the block (k) on that empty place beneath it
                        board->grid[y][x] = board->grid[k][x];

                        // make the old place of k then empty
                        board->grid[k][x].color = EMPTY_CELL;
                        break; //break so that we do not loop the whole thing for a place we have already filled in

                    }

                }
            }
        }

    }
}

// this function refills the gameboard after gravity has been applied
// okay, we have implemented to remove stones and fill the place empty okay
// but now the empty spaces has to be filled to continue with the game

void refill_gameboard(GameBoard* board) {
    for (int y = 0; y < board->height; y++) {
        for (int x = 0; x < board->width; x++) {

            // is that cell empty?
            if (board->grid[y][x].color == EMPTY_CELL) {

                // give the new stone a random color
                board->grid[y][x].color = rand() % board->num_colors;

                // make sure that the new stone is not a bom/special
            }
        }
    }
}