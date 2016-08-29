default: build

dirs:
	mkdir -p obj bin

obj/bstring.o: dirs
	gcc -c bstring.c -o obj/bstring.o

obj/bemu.o: dirs
	gcc -c bemu.c -o obj/bemu.o

obj/assembler.o: dirs
	gcc -c assembler.c -o obj/assembler.o

obj/emulator.o: dirs
	gcc -c emulator.c -o obj/emulator.o

bin/basm: obj/assembler.o obj/bemu.o obj/bstring.o
	gcc obj/assembler.o obj/bemu.o obj/bstring.o -o bin/basm

bin/bemu: obj/emulator.o obj/bemu.o
	gcc obj/emulator.o obj/bemu.o -o bin/bemu

build: bin/basm bin/bemu

clean:
	rm -r obj bin
