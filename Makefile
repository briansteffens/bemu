all: build

build:
	mkdir -p obj bin
	gcc -c -o obj/bstring.o bstring.c
	gcc -c -o obj/bemu.o bemu.c
	gcc -c -o obj/assembler.o assembler.c
	gcc -c -o obj/emulator.o emulator.c
	gcc -o bin/basm obj/assembler.o obj/bemu.o obj/bstring.o
	gcc -o bin/bemu obj/emulator.o obj/bemu.o
