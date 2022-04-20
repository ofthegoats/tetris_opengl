#ifndef PLAYFIELD_H_
#define PLAYFIELD_H_

#include "dimensions.hpp"
#include "enums.hpp"

#include <array>

class Tetromino;

class Playfield
{
public:
    // given an x and a y, with 0 <= x < width and 0 <= y < height, return if there is
    // already a square in that position
    bool squareFull(int, int);

    // getter for gameOver
    bool isGameOver();

    // given 4 positions, add blocks in these positions with the specified square type/colour
    void addTetromino(Tetromino*);

    // pretty print to stdout, including tetromino (active piece)
    void print(Tetromino*);

    // check for full lines and clear them, returning the score gained
    int handleFullLines();

    // get the grid
    std::array<std::array<Square, HEIGHT>, WIDTH> getGrid();

private:
    // initialise the grid with empty squares
    std::array<std::array<Square, HEIGHT>, WIDTH> grid = {Empty};

    // whether the game is over, should be set when a tetromino is placed
    bool gameOver = false;

    // number of consecutive clears
    int combo = 0;
};

#endif  // PLAYFIELD_H_
