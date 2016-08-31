#FLAGS=-pg
#FLAGS=-g

default: build

dirs:
	mkdir -p obj bin

obj/bstring.o: dirs
	gcc $(FLAGS) -c bstring.c -o obj/bstring.o

obj/shared.o: dirs
	gcc $(FLAGS) -c shared.c -o obj/shared.o

obj/assembler.o: dirs
	gcc $(FLAGS) -c assembler.c -o obj/assembler.o

obj/disassembler.o: dirs
	gcc $(FLAGS) -c disassembler.c -o obj/disassembler.o

obj/emulator.o: dirs
	gcc $(FLAGS) -c emulator.c -o obj/emulator.o

obj/bemu.o: dirs
	gcc $(FLAGS) -c bemu.c -o obj/bemu.o

obj/bdbg.o: dirs
	gcc $(FLAGS) -c bdbg.c -o obj/bdbg.o

obj/basm.o: dirs
	gcc $(FLAGS) -c basm.c -o obj/basm.o

bin/basm: obj/assembler.o obj/shared.o obj/bstring.o obj/basm.o
	gcc $(FLAGS) obj/basm.o obj/assembler.o obj/shared.o obj/bstring.o \
		-o bin/basm

bin/bemu: obj/bemu.o obj/emulator.o obj/shared.o
	gcc $(FLAGS) obj/bemu.o obj/emulator.o obj/shared.o -o bin/bemu

bin/bdbg: obj/bdbg.o obj/emulator.o obj/shared.o obj/disassembler.o
	gcc $(FLAGS) obj/bdbg.o obj/emulator.o obj/shared.o obj/disassembler.o \
		obj/assembler.o obj/bstring.o -o bin/bdbg

build: bin/basm bin/bemu bin/bdbg

clean:
	rm -r obj bin
