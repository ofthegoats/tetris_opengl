#ifndef GENERATOR_H_
#define GENERATOR_H_

#include "enums.hpp"

#include <algorithm>
#include <array>
#include <random>

// the tetris piece generator has to follow a specific set of rules, so we define the
// generator functions here in a class

class RandomGenerator
{
private:
    std::array<Piece, 7> pieces = {I, J, L, O, S, T, Z};
    std::array<Piece, 7> lastBag;
    std::array<Piece, 7> currentBag;
    short index = 0;

public:
    RandomGenerator();
    Piece getNextPiece();
    void generateNextBag();
};

#endif  // GENERATOR_H_
