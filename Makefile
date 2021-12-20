CC=gcc

all: frontend_sdl frontend_console

frontend_sdl: src/frontend_sdl.c src/chip8.c
	$(CC) -o chip8sdl src/frontend_sdl.c src/chip8.c -I ./include -lSDL2 -DDEBUG

frontend_console: src/frontend_console.c src/chip8.c
	$(CC) -o chip8console src/frontend_console.c src/chip8.c -I ./include

clean:
	rm chip8sdl chip8console