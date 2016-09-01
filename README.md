bemu
====

This is either an imaginary machine emulator or a bytecode interpreter (or
both). Basically it's a super simplified, made-up machine code / bytecode with
an assembly language, assembler, emulator/interpreter, and debugger.

# Download and compile

Standard stuff:

```bash
git clone https://github.com/briansteffens/bemu
cd bemu
make
```

# Example program

Here's a program which sums all integers from 1 to 100 and prints it out:

```asm
start:
    mov r0 0
    mov r1 1

loop_start:
    cmp r1 100
    jg loop_end

    add r0 r1

    inc r1
    jmp loop_start

loop_end:
    print r0
    exit
```

# Assembling it

The above can be assembled into bytecode like so:

```bash
bin/basm examples/sum.basm
```

This produces the file "b.out".

# Running it

The file can be run like this:

```bash
bin/bemu b.out
```

You should see the following output:

```
5050
```

# Debugging

The binary file can be decoded with the debugger:

```bash
bin/bdbg b.out
```

There aren't any breakpoints or anything yet, you basically just hit enter to
run each instruction. It shows the registers at each instruction and lights up
changed registers in red to make them stand out.

For debugging purposes, you can run basm code directly in the debugger without
moving the instruction pointer. At the debugger prompt, a dollar symbol
assembles and executes whatever instruction comes after it:

```
> $ print r1
```

This will print the value in the register `r1`.

# The basm language

It's pretty x64-inspired, but register names have some differences and there
are no commas between operands.

## Registers

| Name  | Description                                                         |
|:-----:|---------------------------------------------------------------------|
| r0    | General-purpose                                                     |
| r1    | General-purpose                                                     |
| r2    | General-purpose                                                     |
| r3    | General-purpose                                                     |
| r4    | General-purpose                                                     |
| r5    | General-purpose                                                     |
| rip   | Instruction pointer; points to address of next instruction          |
| rsp   | Stack pointer; points to top of the stack                           |
| rflag | Set by compare instructions; used by conditional jumps              |
| rmem  | Start of "free" memory area after the code                          |

## Operand/addressing modes

| Example          | Description                                              |
|------------------|----------------------------------------------------------|
| `r3`             | The value of `r3`                                        |
| `[r3]`           | Memory `r3` is pointing to                               |
| `[r3+64]`        | Memory 64 bytes after the address `r3` is pointing to    |
| `[r3+rmem]`      | Memory `rmem` bytes after the address in `r3`            |
| `[r3*2]`         | Memory at double the address in `r3`                     |
| `[r3*8+rmem+32]` | Memory at `r3` * 8 + the value in `rmem` + 32            |

There are up to four components in an address operand:

- **Base register**: This can be any register. This is the only required
  component.
- **Multiplier**: Any number from 0-255, multiplied by the base register.
- **Second register**: Any register, added or subtracted from the product of
  the base register and multiplier depending on the sign preceding it.
- **Offset**: A signed 32-bit integer offset, added or subtracted from the
  previous components depending on the sign preceding it.

## Memory layout

Here's how memory is laid out for the running program:

| Address | Description                                                       |
|:-------:|-------------------------------------------------------------------|
| 0       | The assembled bytecode is at the top of the memory region         |
| `rmem`  | The start of the "free" memory section for program use            |
| `rsp`   | The top of the stack. Starts at the very bottom (highest address) |

So the program's code sits at the top. The `rmem` register provides the first
(8-byte aligned) address after the code section in memory. This is the start
of where the program can save arbitrary data (kind of like the heap). The stack
starts at the very bottom and grows up. No protections exist to keep the free
memory area and the stack from running into each other.

## Instructions

There aren't many instructions at the moment. Here are some/most of them:

### Basic instructions

Copy the second element in the stack to `r0`:

```asm
mov r0 [rsp+8]
```

Jump to the label `some_label:`:

```asm
jmp some_label
```

Print the current value of `r2` to the console:

```asm
print r2
```

End the program:

```asm
exit
```

### Stack instructions

Push something onto the stack:

```asm
push r2
```

Pop something off the stack into a register or address:

```asm
pop [rmem+8]
```

### Functions

Push the current `rip` value onto the stack and then jump to the given label:

```asm
call some_function_label
```

Pop an instruction pointer off the stack and jump to it:

```asm
ret
```

### Math instructions

Add the values in `r0` and `r1`, leaving the result in `r0`:

```asm
add r0 r1
```

Subtract the value in `r3` from the value in `r2`, leaving the result in `r2`:

```asm
sub r2 r3
```

Multiply the first value in the "free" memory space by `r4`, storing the result
in `r4`:

```asm
mul r4 [rmem]
```

Divide the second value in the "free" memory space by `r2`, storing the result
in the free memory space:

```asm
div [rmem+8] r2
```

Calculate the modulus (remainder) of `r3` divided by `r4`, storing the result
in `r3`:

```asm
mod r3 r4
```

Increment the value in `r0`:

```asm
inc r0
```

Decrement the value in `r1`:

```asm
dec r1
```

### Conditional branching

Compare the value in `r0` to the value in `r2`, setting the `rflag` register.
This must be done before a conditional jump.

```asm
cmp r0 r2
```

Jump to the `was_equal:` label if the values previously compared were equal:

```asm
je was_equal
```

Jump to `not_equal:` if they weren't equal:

```asm
jne not_equal
```

Jump to `less_than:` if the second operand to the previous *cmp* instruction
was less than the first operand:

```asm
jl less_than
```

Jump to `greater_than:` if the second operand was greater than the first:

```asm
jg greater_than
```

Jump to `less_or_equal:` if the second operand was less than or equal to the
first:

```asm
jle less_or_equal
```

Jump to `greater_or_equal:` if the second operand was greater than or equal to
the first:

```asm
jge greater_or_equal
```
