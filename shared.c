enum opcodes
{
	OP_PUSH,
	OP_POP
};

enum sizes
{
	B1 = 1,
	B2 = 2,
	B4 = 4,
	B8 = 8
};

enum registers
{
	R1 = 1,
	R2,
	R3,
	R4
};

enum operand_types
{
	IMMEDIATE,
	REGISTER
};

enum addresing_modes
{
	LITERAL,
	ADDRESS
};

typedef struct operand
{
	unsigned char type;
	unsigned char mode;
	long long data;
} operand;

typedef struct instruction
{
	unsigned char opcode;
	unsigned char size;
	operand operands[2];
} instruction;

void operand_to_bytes(operand* oper, unsigned char* out)
{
	*(out++) = oper->type | oper->mode << 1;

	for (int shift = 0; shift <= 56; shift += 8)
	{
		*(out++) = (oper->data >> shift) & 0xff;
	}
}

void operand_from_bytes(unsigned char* in, operand* oper)
{
	oper->type = (*in & (1 << 0)) >> 0;
	oper->mode = (*in & (1 << 1)) >> 1;
	oper->data = *(long long *)(in + 1);
}

void operand_print(operand *oper)
{
	switch (oper->type)
	{
		case IMMEDIATE:
			printf("IMMEDIATE ");
			break;

		case REGISTER:
			printf("REGISTER ");
			break;

		default:
			printf("Unrecognized operand type\n");
			exit(1);
	}

	switch (oper->mode)
	{
		case LITERAL:
			printf("LITERAL ");
			break;

		case ADDRESS:
			printf("ADDRESS ");
			break;

		default:
			printf("Unrecognized operand mode\n");
			exit(2);
	}

	printf("%llu\n", oper->data);
}
