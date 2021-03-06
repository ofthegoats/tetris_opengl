#include "generator.hpp"
#include "playfield.hpp"
#include "tetrominos.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <chrono>
#include <iostream>
#include <list>
#include <memory>

Tetromino makePiece(Piece, Playfield*);

void framebuffer_size_callback(GLFWwindow*, int, int);

void processInput(GLFWwindow*);

GLsizei windowWidth = 800;
GLsizei windowHeight = 1000;
std::size_t infoLogSize = 1024;

const char* vShaderSource = "#version 330 core\n"
                            "layout (location = 0) in vec2 aPos;\n"
                            "void main()\n"
                            "{\n"
                            "gl_Position = vec4(aPos, 0.0f, 1.0f);\n"
                            "}";

const char* fShaderSource = "#version 330 core\n"
                            "uniform vec3 colour;\n"
                            "out vec4 FragColor;\n"
                            "void main()\n"
                            "{\n"
                            "FragColor = vec4(colour, 1.0f);\n"
                            "}";

std::unique_ptr<Tetromino> activePiece;
std::unique_ptr<Tetromino> carryPiece;
std::list<std::unique_ptr<Tetromino>> upcoming;
bool swappable = true;
Playfield playfield;
RandomGenerator generator = RandomGenerator();

int main(int argc, char* argv[])
{
    // GLFW initialization, configuration and window creation
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    GLFWwindow* win = glfwCreateWindow(windowWidth, windowHeight, "Tetris", NULL, NULL);
    if (win == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(win);
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "glewInit failed" << std::endl;
        glfwTerminate();
        return -1;
    }
    glViewport(0, 0, windowWidth, windowHeight);
    glfwSetFramebufferSizeCallback(win, framebuffer_size_callback);

    // TODO shaders + textures
    int success;
    char infoLog[infoLogSize];
    unsigned int vShader;
    vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &vShaderSource, NULL);
    glCompileShader(vShader);
    glGetShaderiv(vShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vShader, infoLogSize, NULL, infoLog);
        std::cerr << "VERTEX SHADER COMPILATION FAILED" << std::endl << infoLog << std::endl;
        return -1;
    }
    unsigned int fShader;
    fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fShaderSource, NULL);
    glCompileShader(fShader);
    glGetShaderiv(fShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fShader, infoLogSize, NULL, infoLog);
        std::cerr << "FRAGMENT SHADER COMPILATION FAILED" << std::endl << infoLog << std::endl;
        return -1;
    }
    unsigned int shader;
    shader = glCreateProgram();
    glAttachShader(shader, vShader);
    glAttachShader(shader, fShader);
    glLinkProgram(shader);
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader, infoLogSize, NULL, infoLog);
        std::cerr << "SHADER LINKING FAILED" << std::endl << infoLog << std::endl;
        return -1;
    }

    // shaders are linked, delete to free resources
    glDeleteShader(vShader);
    glDeleteShader(fShader);

    // 4 corners of a square
    // NOTE these MUST be in the in the order, BL, TL, TR, BR
    double vertices[4 * 2];
    unsigned int squareIndices[] = {
      0, 1, 2,  // first triangle
      0, 3, 2  // second triangle
    };

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_DOUBLE, GL_FALSE, 2 * sizeof(double), (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(squareIndices), squareIndices, GL_DYNAMIC_DRAW);

    int colourLocation = glGetUniformLocation(shader, "colour");

    for (int i = 0; i < 4; i++)
        upcoming.push_back(
          std::make_unique<Tetromino>(makePiece(generator.getNextPiece(), &playfield)));

    activePiece = std::move(upcoming.front());
    upcoming.pop_front();
    upcoming.push_back(
      std::make_unique<Tetromino>(makePiece(generator.getNextPiece(), &playfield)));
    std::chrono::system_clock::time_point lastTimestamp = std::chrono::system_clock::now();
    std::chrono::system_clock::time_point currentTimestamp;
    std::chrono::duration<float> Dt;
    std::chrono::duration<float> levelTime = std::chrono::milliseconds(300);
    unsigned int score = 0;
    // within a second, this loop will repeat very many times, so don't worry too much
    // about being capable of infinite rotations or piece movement
    while (!playfield.isGameOver() && !glfwWindowShouldClose(win)) {
        // process inputs
        processInput(win);

        // manage the game
        currentTimestamp = std::chrono::system_clock::now();
        Dt = currentTimestamp - lastTimestamp;

        if (Dt >= levelTime) {
            lastTimestamp = currentTimestamp;
            activePiece->moveDownOrAdd();
            if (activePiece->isAdded()) {
                int inc = playfield.handleFullLines();
                activePiece = std::move(upcoming.front());
                upcoming.pop_front();
                upcoming.push_back(
                  std::make_unique<Tetromino>(makePiece(generator.getNextPiece(), &playfield)));
                swappable = true;
            }
        }

        // rendering
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);  // base background colour
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shader);

        auto grid = playfield.getGrid();
        // active piece is not in the grid by default
        for (auto coord : activePiece->getTrueLocation()) {
            if (coord.first < WIDTH && coord.second < HEIGHT)
                grid.at(coord.first).at(coord.second) = activePiece->getColour();
        }
        // colours for different squares
        for (int x = 0; x < WIDTH; x++) {
            for (int y = 0; y < HEIGHT; y++) {
                switch (grid.at(x).at(y)) {
                case Empty:  // different coloured columns
                    if (x % 2 == 0)
                        glUniform3f(colourLocation, 0.3f, 0.3f, 0.3f);
                    else
                        glUniform3f(colourLocation, 0.4f, 0.4f, 0.4f);
                    break;
                case Cyan: glUniform3f(colourLocation, 0.0f, 1.0f, 1.0f); break;
                case Blue: glUniform3f(colourLocation, 0.0f, 0.0f, 1.0f); break;
                case Orange: glUniform3f(colourLocation, 1.0f, 0.647f, 0.0f); break;
                case Yellow: glUniform3f(colourLocation, 1.0f, 1.0f, 0.0f); break;
                case Green: glUniform3f(colourLocation, 0.0f, 1.0f, 0.0f); break;
                case Pink: glUniform3f(colourLocation, 1.0f, 0.412f, 0.705f); break;
                case Red: glUniform3f(colourLocation, 1.0f, 0.0f, 0.0f); break;
                }
                // calculations to get corners in normalised coordinate system
                // Bottom Left
                vertices[0] = -1 + (double)(2 * x) / WIDTH;
                vertices[1] = -1 + (double)(2 * y) / HEIGHT;
                // Top Left
                vertices[2] = -1 + (double)(2 * x) / WIDTH;
                vertices[3] = -1 + (double)(2 * y + 2) / HEIGHT;
                // Top Right
                vertices[4] = -1 + (double)(2 * x + 2) / WIDTH;
                vertices[5] = -1 + (double)(2 * y + 2) / HEIGHT;
                // Bottom Right
                vertices[6] = -1 + (double)(2 * x + 2) / WIDTH;
                vertices[7] = -1 + (double)(2 * y) / HEIGHT;

                // draw in the current square
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
                glVertexAttribPointer(0, 2, GL_DOUBLE, GL_FALSE, 2 * sizeof(double), (void*)0);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
            }
        }

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    return 0;
}

Tetromino makePiece(Piece p, Playfield* play)
{
    switch (p) {
    case I: return IPiece(play);
    case J: return JPiece(play);
    case L: return LPiece(play);
    case O: return OPiece(play);
    case S: return SPiece(play);
    case T: return TPiece(play);
    case Z: return ZPiece(play);
    }
}

// change viewport on resize
void framebuffer_size_callback(GLFWwindow* win, int width, int height)
{
    glViewport(0, 0, width, height);
}

std::chrono::system_clock::time_point lastHarddrop;
std::chrono::system_clock::time_point lastSoftdrop;
std::chrono::duration<float> harddropTimeout = std::chrono::milliseconds(300);
std::chrono::duration<float> softdropTimeout = std::chrono::milliseconds(50);

void processInput(GLFWwindow* win)
{
    if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwGetKey(win, GLFW_KEY_Q) == GLFW_PRESS)
        glfwSetWindowShouldClose(win, true);
    if (glfwGetKey(win, GLFW_KEY_LEFT) == GLFW_PRESS || glfwGetKey(win, GLFW_KEY_H) == GLFW_PRESS)
        activePiece->moveHorizontal(-1);
    if (glfwGetKey(win, GLFW_KEY_RIGHT) == GLFW_PRESS || glfwGetKey(win, GLFW_KEY_L) == GLFW_PRESS)
        activePiece->moveHorizontal(1);
    if (glfwGetKey(win, GLFW_KEY_Z) == GLFW_PRESS || glfwGetKey(win, GLFW_KEY_UP) == GLFW_PRESS
        || glfwGetKey(win, GLFW_KEY_K))
        activePiece->rotate(Clockwise);
    if (glfwGetKey(win, GLFW_KEY_X) == GLFW_PRESS) activePiece->rotate(CounterClockwise);
    if (glfwGetKey(win, GLFW_KEY_DOWN) == GLFW_PRESS || glfwGetKey(win, GLFW_KEY_J) == GLFW_PRESS) {
        auto currentTime = std::chrono::system_clock::now();
        if (currentTime - lastSoftdrop >= softdropTimeout) {
            activePiece->moveDownOrAdd();
            lastSoftdrop = currentTime;
        }
    }
    if (glfwGetKey(win, GLFW_KEY_SPACE) == GLFW_PRESS) {
        auto currentTime = std::chrono::system_clock::now();
        if (currentTime - lastHarddrop >= harddropTimeout) {
            activePiece->harddrop();
            lastHarddrop = currentTime;
        }
    }
    if (glfwGetKey(win, GLFW_KEY_C) == GLFW_PRESS) {
        if (carryPiece && swappable) {
            std::swap(activePiece, carryPiece);
            activePiece->resetPosition();
        } else if (swappable) {
            carryPiece = std::move(activePiece);
            activePiece =
              std::make_unique<Tetromino>(makePiece(generator.getNextPiece(), &playfield));
            activePiece->resetPosition();
        }
        swappable = false;
    }
}
