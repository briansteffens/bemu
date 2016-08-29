#!/usr/bin/bash

gcc -o basm assembler.c shared.c bstring.c
gcc -o bemu emulator.c shared.c
