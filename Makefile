#FLAGS=-pg
#FLAGS=-g

default: build

dirs:
	mkdir -p obj bin

obj/bstring.o: dirs
	gcc $(FLAGS) -c bstring.c -o obj/bstring.o

obj/bemu.o: dirs
	gcc $(FLAGS) -c bemu.c -o obj/bemu.o

obj/assembler.o: dirs
	gcc $(FLAGS) -c assembler.c -o obj/assembler.o

obj/emulator.o: dirs
	gcc $(FLAGS) -c emulator.c -o obj/emulator.o

bin/basm: obj/assembler.o obj/bemu.o obj/bstring.o
	gcc $(FLAGS) obj/assembler.o obj/bemu.o obj/bstring.o -o bin/basm

bin/bemu: obj/emulator.o obj/bemu.o
	gcc $(FLAGS) obj/emulator.o obj/bemu.o -o bin/bemu

build: bin/basm bin/bemu

clean:
	rm -r obj bin
