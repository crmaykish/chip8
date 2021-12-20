CC=gcc

all: frontend_sdl

frontend_sdl: src/frontend_sdl.c src/chip8.c
	$(CC) -o chip8sdl src/frontend_sdl.c src/chip8.c -I ./include -lSDL2 -DDEBUG

clean:
	rm chip8sdl
	