#include "tetrominos.hpp"

#include "enums.hpp"
#include "playfield.hpp"

#include <chrono>
#include <utility>

Tetromino::Tetromino(Playfield* p) { playfield = p; }

void Tetromino::rotate(Rotation r)
{
    if (!moveable) return;
    // do nothing if the last rotation was too recent to be intentional
    auto currentTime = std::chrono::system_clock::now();
    if (currentTime - lastRotation < rotationTimeout)
        return;
    else
        lastRotation = currentTime;
    auto newR = r == Clockwise              ? (rotationIdentifier + 1) % 4
                : (rotationIdentifier == 0) ? 3
                                            : rotationIdentifier - 1;
    auto oldLayout = rotationBasicStates[rotationIdentifier];
    auto newLayout = rotationBasicStates[newR];
    std::array<std::pair<int, int>, 4> Dlayout;
    for (int i = 0; i < 4; i++) {
        Dlayout.at(i) = std::make_pair<int, int>(newLayout.at(i).first - oldLayout.at(i).first,
          newLayout.at(i).second - oldLayout.at(i).second);
    }
    std::array<std::pair<int, int>, 4> newLocation;
    for (int i = 0; i < 4; i++) {
        newLocation.at(i) = std::make_pair<int, int>(trueLocation.at(i).first + Dlayout.at(i).first,
          trueLocation.at(i).second + Dlayout.at(i).second);
    }
    bool illegal;
    int kickTry;
    std::array<std::pair<int, int>, 4> kickedNewLocation;
    for (kickTry = 0; kickTry < 5; kickTry++) {
        int dX =
          r == Clockwise ? kicks.at(r).at(kickTry).first : -1 * kicks.at(newR).at(kickTry).first;
        int dY =
          r == Clockwise ? kicks.at(r).at(kickTry).second : -1 * kicks.at(newR).at(kickTry).second;
        for (int i = 0; i < 4; i++) {
            kickedNewLocation.at(i) =
              std::make_pair<int, int>(newLocation.at(i).first + dX, newLocation.at(i).second + dY);
        }
        illegal = false;
        for (auto coord : kickedNewLocation) {
            if (playfield->squareFull(coord.first, coord.second)) illegal = true;
        }
        if (!illegal) {
            newLocation = kickedNewLocation;
            break;
        }
    }
    if (!illegal) {
        // to allow for intinite rotation, rotation must unset the piece
        trueLocation = newLocation;
        rotationIdentifier = newR;
        set = false;
    }
}

void Tetromino::moveDownOrAdd()
{
    // check if there is a solid beneath any of the solid squares
    bool squareBelow = false;
    for (auto coord : trueLocation) {
        if (coord.second == 0 || playfield->squareFull(coord.first, coord.second - 1))
            squareBelow = true;
    }
    // if there isn't, move everything down by one
    if (set && squareBelow) {
        playfield->addTetromino(this);
        added = true;
    } else if (!squareBelow) {
        for (auto i = trueLocation.begin(); i < trueLocation.end(); i++) {
            i->second--;
        }
    }
    for (auto coord : trueLocation) {
        if (playfield->squareFull(coord.first, coord.second - 1)) set = true;
    }
}

void Tetromino::harddrop()
{
    while (!set)
        moveDownOrAdd();
}

void Tetromino::moveHorizontal(int dir)
{
    if (!moveable) return;
    // do nothing if the last movement was too recent to be intentional
    auto currentTime = std::chrono::system_clock::now();
    if (currentTime - lastHorizontalMovement < movementTimeout)
        return;
    else
        lastHorizontalMovement = currentTime;
    std::array<std::pair<int, int>, 4> newTrueLocation;
    int d = (dir > 0) ? 1 : -1;
    for (int i = 0; i < 4; i++) {
        newTrueLocation.at(i) = std::make_pair<int, int>(
          trueLocation.at(i).first + d, std::move(trueLocation.at(i).second));
    }
    bool legal = true;
    for (int i = 0; i < 4; i++) {
        if (playfield->squareFull(newTrueLocation.at(i).first, newTrueLocation.at(i).second))
            legal = false;
    }
    if (legal) {
        trueLocation = newTrueLocation;
        set = false;
    }
}

Square Tetromino::getColour() { return colour; }

std::array<std::pair<int, int>, 4> Tetromino::getTrueLocation() { return trueLocation; }

bool Tetromino::isAdded() { return added; }

IPiece::IPiece(Playfield* p) : Tetromino(p)
{
    playfield = p;
    colour = Cyan;
    trueLocation = {std::make_pair<int, int>((WIDTH / 2) - 2, HEIGHT - 1),
      std::make_pair<int, int>((WIDTH / 2) - 1, HEIGHT - 1),
      std::make_pair<int, int>((WIDTH / 2), HEIGHT - 1),
      std::make_pair<int, int>((WIDTH / 2) + 1, HEIGHT - 1)};
    rotationBasicStates = {{
      {
        std::make_pair<int, int>(0, 2),
        std::make_pair<int, int>(1, 2),
        std::make_pair<int, int>(2, 2),
        std::make_pair<int, int>(3, 2),
      },
      {
        std::make_pair<int, int>(2, 0),
        std::make_pair<int, int>(2, 1),
        std::make_pair<int, int>(2, 2),
        std::make_pair<int, int>(2, 3),
      },
      {
        std::make_pair<int, int>(3, 1),
        std::make_pair<int, int>(2, 1),
        std::make_pair<int, int>(1, 1),
        std::make_pair<int, int>(0, 1),
      },
      {
        std::make_pair<int, int>(1, 3),
        std::make_pair<int, int>(1, 2),
        std::make_pair<int, int>(1, 1),
        std::make_pair<int, int>(1, 0),
      },
    }};
    kicks = {{
      {
        // r : 0 >> 1
        std::make_pair<int, int>(0, 0),
        std::make_pair<int, int>(-2, 0),
        std::make_pair<int, int>(1, 0),
        std::make_pair<int, int>(-2, -1),
        std::make_pair<int, int>(1, 2),
      },
      {
        // r : 1 >> 2
        std::make_pair<int, int>(0, 0),
        std::make_pair<int, int>(-1, 0),
        std::make_pair<int, int>(2, 0),
        std::make_pair<int, int>(-1, 2),
        std::make_pair<int, int>(2, -1),
      },
      {
        // r : 2 >> 3
        std::make_pair<int, int>(0, 0),
        std::make_pair<int, int>(2, 0),
        std::make_pair<int, int>(-1, 0),
        std::make_pair<int, int>(2, 1),
        std::make_pair<int, int>(-1, -2),
      },
      {
        // r : 3 >> 0
        std::make_pair<int, int>(0, 0),
        std::make_pair<int, int>(1, 0),
        std::make_pair<int, int>(-2, 0),
        std::make_pair<int, int>(1, -2),
        std::make_pair<int, int>(-2, 1),
      },
    }};
};

OPiece::OPiece(Playfield* p) : Tetromino(p)
{
    playfield = p;
    colour = Yellow;
    trueLocation = {std::make_pair<int, int>(WIDTH / 2, HEIGHT - 1),
      std::make_pair<int, int>((WIDTH / 2) - 1, HEIGHT - 1),
      std::make_pair<int, int>((WIDTH / 2), HEIGHT),
      std::make_pair<int, int>((WIDTH / 2) - 1, HEIGHT)};
};

// The O piece does not rotate, but infinite is still possible
void OPiece::rotate(Rotation r) { set = false; }

// all kicks are for clockwise rotations
std::array<std::array<std::pair<int, int>, 5>, 4> JLZSTKicks = {{
  {
    // r : 0 >> 1
    std::make_pair<int, int>(0, 0),
    std::make_pair<int, int>(-1, 0),
    std::make_pair<int, int>(-1, 1),
    std::make_pair<int, int>(0, -2),
    std::make_pair<int, int>(-1, -2),
  },
  {
    // r : 1 >> 2
    std::make_pair<int, int>(0, 0),
    std::make_pair<int, int>(1, 0),
    std::make_pair<int, int>(1, -1),
    std::make_pair<int, int>(0, 2),
    std::make_pair<int, int>(1, 2),
  },
  {
    // r : 2 >> 3
    std::make_pair<int, int>(0, 0),
    std::make_pair<int, int>(1, 0),
    std::make_pair<int, int>(1, 1),
    std::make_pair<int, int>(0, -2),
    std::make_pair<int, int>(1, -2),
  },
  {
    // r : 3 >> 0
    std::make_pair<int, int>(0, 0),
    std::make_pair<int, int>(-1, 0),
    std::make_pair<int, int>(-1, -1),
    std::make_pair<int, int>(0, 2),
    std::make_pair<int, int>(1, -2),
  },
}};

JPiece::JPiece(Playfield* p) : Tetromino(p)
{
    playfield = p;
    colour = Blue;
    trueLocation = {std::make_pair<int, int>((WIDTH / 2) - 2, HEIGHT),
      std::make_pair<int, int>((WIDTH / 2) - 2, HEIGHT - 1),
      std::make_pair<int, int>((WIDTH / 2) - 1, HEIGHT - 1),
      std::make_pair<int, int>((WIDTH / 2), HEIGHT - 1)};
    rotationBasicStates = {{
      {
        std::make_pair<int, int>(-1, 1),
        std::make_pair<int, int>(-1, 0),
        std::make_pair<int, int>(0, 0),
        std::make_pair<int, int>(1, 0),
      },
      {
        std::make_pair<int, int>(1, 1),
        std::make_pair<int, int>(0, 1),
        std::make_pair<int, int>(0, 0),
        std::make_pair<int, int>(0, -1),
      },
      {
        std::make_pair<int, int>(1, -1),
        std::make_pair<int, int>(1, 0),
        std::make_pair<int, int>(0, 0),
        std::make_pair<int, int>(-1, 0),
      },
      {
        std::make_pair<int, int>(-1, -1),
        std::make_pair<int, int>(0, -1),
        std::make_pair<int, int>(0, 0),
        std::make_pair<int, int>(0, 1),
      },
    }};
    kicks = JLZSTKicks;
};

LPiece::LPiece(Playfield* p) : Tetromino(p)
{
    playfield = p;
    colour = Orange;
    trueLocation = {std::make_pair<int, int>((WIDTH / 2) - 2, HEIGHT - 1),
      std::make_pair<int, int>((WIDTH / 2) - 1, HEIGHT - 1),
      std::make_pair<int, int>((WIDTH / 2), HEIGHT - 1),
      std::make_pair<int, int>((WIDTH / 2), HEIGHT)};
    rotationBasicStates = {{
      {
        std::make_pair<int, int>(-1, 0),
        std::make_pair<int, int>(0, 0),
        std::make_pair<int, int>(1, 0),
        std::make_pair<int, int>(1, 1),
      },
      {
        std::make_pair<int, int>(0, 1),
        std::make_pair<int, int>(0, 0),
        std::make_pair<int, int>(0, -1),
        std::make_pair<int, int>(1, -1),
      },
      {
        std::make_pair<int, int>(1, 0),
        std::make_pair<int, int>(0, 0),
        std::make_pair<int, int>(-1, 0),
        std::make_pair<int, int>(-1, -1),
      },
      {
        std::make_pair<int, int>(0, -1),
        std::make_pair<int, int>(0, 0),
        std::make_pair<int, int>(0, 1),
        std::make_pair<int, int>(-1, 1),
      },
    }};
    kicks = JLZSTKicks;
};

ZPiece::ZPiece(Playfield* p) : Tetromino(p)
{
    playfield = p;
    colour = Red;
    trueLocation = {std::make_pair<int, int>((WIDTH / 2) - 2, HEIGHT),
      std::make_pair<int, int>((WIDTH / 2) - 1, HEIGHT),
      std::make_pair<int, int>((WIDTH / 2) - 1, HEIGHT - 1),
      std::make_pair<int, int>((WIDTH / 2), HEIGHT - 1)};
    rotationBasicStates = {{
      {
        std::make_pair<int, int>(-1, 1),
        std::make_pair<int, int>(0, 1),
        std::make_pair<int, int>(0, 0),
        std::make_pair<int, int>(1, 0),
      },
      {
        std::make_pair<int, int>(1, 1),
        std::make_pair<int, int>(1, 0),
        std::make_pair<int, int>(0, 0),
        std::make_pair<int, int>(0, -1),
      },
      {
        std::make_pair<int, int>(1, -1),
        std::make_pair<int, int>(0, -1),
        std::make_pair<int, int>(0, 0),
        std::make_pair<int, int>(-1, 0),
      },
      {
        std::make_pair<int, int>(-1, -1),
        std::make_pair<int, int>(-1, 0),
        std::make_pair<int, int>(0, 0),
        std::make_pair<int, int>(0, 1),
      },
    }};
    kicks = JLZSTKicks;
};

SPiece::SPiece(Playfield* p) : Tetromino(p)
{
    playfield = p;
    colour = Green;
    trueLocation = {std::make_pair<int, int>((WIDTH / 2) - 2, HEIGHT - 1),
      std::make_pair<int, int>((WIDTH / 2) - 1, HEIGHT - 1),
      std::make_pair<int, int>((WIDTH / 2) - 1, HEIGHT),
      std::make_pair<int, int>((WIDTH / 2), HEIGHT)};
    rotationBasicStates = {{
      {
        std::make_pair<int, int>(-1, 0),
        std::make_pair<int, int>(0, 0),
        std::make_pair<int, int>(0, 1),
        std::make_pair<int, int>(1, 1),
      },
      {
        std::make_pair<int, int>(0, 1),
        std::make_pair<int, int>(0, 0),
        std::make_pair<int, int>(1, 0),
        std::make_pair<int, int>(1, -1),
      },
      {
        std::make_pair<int, int>(1, 0),
        std::make_pair<int, int>(0, 0),
        std::make_pair<int, int>(0, -1),
        std::make_pair<int, int>(-1, -1),
      },
      {
        std::make_pair<int, int>(0, -1),
        std::make_pair<int, int>(0, 0),
        std::make_pair<int, int>(-1, 0),
        std::make_pair<int, int>(-1, 1),
      },
    }};
    kicks = JLZSTKicks;
};

TPiece::TPiece(Playfield* p) : Tetromino(p)
{
    playfield = p;
    colour = Pink;
    trueLocation = {std::make_pair<int, int>((WIDTH / 2) - 2, HEIGHT - 1),
      std::make_pair<int, int>((WIDTH / 2) - 1, HEIGHT - 1),
      std::make_pair<int, int>((WIDTH / 2) - 1, HEIGHT),
      std::make_pair<int, int>((WIDTH / 2), HEIGHT - 1)};
    rotationBasicStates = {{
      {
        std::make_pair<int, int>(-1, 0),
        std::make_pair<int, int>(0, 0),
        std::make_pair<int, int>(0, 1),
        std::make_pair<int, int>(1, 0),
      },
      {
        std::make_pair<int, int>(0, 1),
        std::make_pair<int, int>(0, 0),
        std::make_pair<int, int>(1, 0),
        std::make_pair<int, int>(0, -1),
      },
      {
        std::make_pair<int, int>(1, 0),
        std::make_pair<int, int>(0, 0),
        std::make_pair<int, int>(0, -1),
        std::make_pair<int, int>(-1, 0),
      },
      {
        std::make_pair<int, int>(0, -1),
        std::make_pair<int, int>(0, 0),
        std::make_pair<int, int>(-1, 0),
        std::make_pair<int, int>(0, 1),
      },
    }};
    kicks = JLZSTKicks;
};
