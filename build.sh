#!/usr/bin/bash

gcc -o basm assembler.c shared.c
gcc -o bvm vm.c shared.c
