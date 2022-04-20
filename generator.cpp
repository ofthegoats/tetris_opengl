#include "generator.hpp"

RandomGenerator::RandomGenerator()
{
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(pieces.begin(), pieces.end(), g);
    lastBag = pieces;
    currentBag = pieces;
}

Piece RandomGenerator::getNextPiece()
{
    Piece piece = currentBag.at(index);
    index++;
    if (index == 7) {
        lastBag = currentBag;
        generateNextBag();
        index = 0;
    }
    return piece;
}

void RandomGenerator::generateNextBag()
{
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(pieces.begin(), pieces.end(), g);
    currentBag = pieces;
}
