CC = clang++
CFLAGS = ${shell pkg-config --cflags --libs glew glfw3} -g
OUTPUT = bin/tetris
SOURCES = main.cpp tetrominos.cpp playfield.cpp generator.cpp
HEADERS = dimensions.hpp enums.hpp generator.hpp playfield.hpp tetrominos.hpp
OBJECTS = main.o tetrominos.o playfield.o generator.o

${OUTPUT} : ${SOURCES} ${HEADERS}
	[ ! -d "bin" ] && mkdir bin || continue
	${CC} ${CFLAGS} ${SOURCES} -o ${OUTPUT}

.PHONY : clean run

run : ${OUTPUT}
	./${OUTPUT}

clean :
	rm -f ${OUTPUT}
