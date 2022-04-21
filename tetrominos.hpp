#ifndef TETROMINOS_H_
#define TETROMINOS_H_

#include "dimensions.hpp"
#include "enums.hpp"

#include <array>
#include <chrono>
#include <iostream>
#include <utility>

class Playfield;

class Tetromino
{
protected:
    // rotation identifier 0-3, identifying each of the 4 possible rotations of a tetromino
    // used to index rotationBasicStates
    int rotationIdentifier = 0;

    // predefined logical positions after a rotation
    std::array<std::array<std::pair<int, int>, 4>, 4> rotationBasicStates;

    // kicks to be tested sequentially, according to SRS, for a clockwise rotation
    std::array<std::array<std::pair<int, int>, 5>, 4> kicks;

    // location on the playfield, initialized in each child class with the starting position
    std::array<std::pair<int, int>, 4> trueLocation;
    std::array<std::pair<int, int>, 4> defaultLocation;

    // tetrominos have standard colours
    Square colour;

    // a tetromino needs access to the playfield so that if it is possible for a piece to
    // rotate, whether it needs to kick etc.
    Playfield* playfield;

    // tracks whether there was a full square underneath the piece, on the last turn
    // a horizontal movement or rotation sets this to false, so infinite is possible
    // if there is a piece below it on the current turn & on the last turn (barring
    // rotations and movement), then the piece is added to the playfield
    bool set = false;

    // whether the tetromino has been added to the playfield
    bool added = false;

    // set to false after a hard drop
    bool moveable = true;

    // when the tetromino was last moved horizontally and rotated, as well as a timeout, so
    // that it is possible to react to rotations
    std::chrono::system_clock::time_point lastRotation;
    std::chrono::system_clock::time_point lastHorizontalMovement;
    std::chrono::duration<float> rotationTimeout = std::chrono::milliseconds(95);
    std::chrono::duration<float> movementTimeout = std::chrono::milliseconds(95);

public:
    Tetromino(Playfield* p);

    // specify whether to rotate clockwise or counter clockwise
    void rotate(Rotation);

    // move the piece down by one
    void moveDownOrAdd();

    // move the piece as far down as possible, then make it unmoveable
    void harddrop();

    // move the piece one horizontally, left or right
    // positive argument => right, negative => left
    void moveHorizontal(int);

    // get the colour of a piece
    Square getColour();

    // get the true location of the piece
    std::array<std::pair<int, int>, 4> getTrueLocation();

    // get whether the piece is set/on ground
    bool isAdded();

    // reset the position to the top of the board
    void resetPosition();
};

// Below are definitions for all 7 tetrominos

class IPiece : public Tetromino
{
public:
    IPiece(Playfield*);
};

class OPiece : public Tetromino
{
public:
    OPiece(Playfield*);
    void rotate(Rotation);
};

class JPiece : public Tetromino
{
public:
    JPiece(Playfield*);
};

class LPiece : public Tetromino
{
public:
    LPiece(Playfield*);
};

class ZPiece : public Tetromino
{
public:
    ZPiece(Playfield*);
};

class SPiece : public Tetromino
{
public:
    SPiece(Playfield*);
};

class TPiece : public Tetromino
{
public:
    TPiece(Playfield*);
};

#endif  // TETROMINOS_H_
