#include "playfield.hpp"

#include "tetrominos.hpp"

#include <iostream>

bool Playfield::squareFull(int x, int y)
{
    return (y < HEIGHT && y >= 0 && x < WIDTH && x >= 0) ? grid.at(x).at(y) != Empty : true;
}

bool Playfield::isGameOver() { return gameOver; }

void Playfield::addTetromino(Tetromino* t)
{
    auto squares = t->getTrueLocation();
    Square colour = t->getColour();
    for (auto coord : squares) {
        if (coord.second >= HEIGHT) {
            gameOver = true;
        } else if (coord.second < 0 || coord.first < 0 || coord.first >= WIDTH) {
            // somehow this has gotten out of bounds, which should not happen
            std::cerr << "tetromino is out of bounds" << std::endl;
            std::exit(1);
        } else {
            grid.at(coord.first).at(coord.second) = colour;
        }
    }
}

void Playfield::print(Tetromino* t)
{
    auto board = grid;
    auto tetrLoc = t->getTrueLocation();
    auto tetrCol = t->getColour();
    for (auto coord : tetrLoc) {
        if (coord.second < HEIGHT && coord.second >= 0)
            board.at(coord.first).at(coord.second) = tetrCol;
        std::cout << coord.first << " " << coord.second << std::endl;
    }
    std::cout << "------------" << std::endl;
    // don't display the top 2 rows
    for (int i = HEIGHT - 1; i >= 0; i--) {
        std::cout << "|";
        for (int j = 0; j < WIDTH; j++) {
            if (board.at(j).at(i) == Empty) {
                std::cout << " ";
            } else {
                std::cout << "X";
            }
        }
        std::cout << "|" << std::endl;
    }
    std::cout << "------------" << std::endl;
}

int Playfield::handleFullLines()
{
    int linesCleared = 0;
    for (int y = 0; y < HEIGHT; y++) {
        bool fullLine = true;
        for (int x = 0; x < WIDTH; x++) {
            if (grid.at(x).at(y) == Empty) fullLine = false;
        }
        if (fullLine) {
            linesCleared++;
            // lower all lines by one
            for (int z = y; z < HEIGHT - 1; z++) {
                for (int x = 0; x < WIDTH; x++) {
                    grid.at(x).at(z) = grid.at(x).at(z + 1);
                }
            }
            for (int x = 0; x < WIDTH; x++) {
                grid.at(x).at(HEIGHT - 1) = Empty;
            }
        }
    }
    if (linesCleared > 0)
        combo++;
    else
        combo = 0;
    switch (linesCleared) {
    case 1: return 100 + 50 * combo;
    case 2: return 300 + 50 * combo;
    case 3: return 500 + 50 * combo;
    case 4: return 800 + 50 * combo;
    default: return 0;
    }
}

std::array<std::array<Square, HEIGHT>, WIDTH> Playfield::getGrid() { return grid; }
