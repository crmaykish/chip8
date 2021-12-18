CC=gcc

all: emulator

emulator: src/main.c src/chip8.c
	$(CC) -o chip8emu src/main.c src/chip8.c -I ./include -lSDL2 -DDEBUG

clean:
	rm chip8emu