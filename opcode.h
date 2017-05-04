#pragma once

#define JMPREL_8 0xEB // relative jump short 8bits

#define JMPREL_16_32 0xE9

#define JNZREL_8 0x75 // relative jump short if != 0 8bits

#define JZEREL_16_32 0x0F84 // relative jump short if 0

#define JNZREL_16_32 0x0F85 // relative jump short 16/ 32

#define JBEREL_16_32 0x0F86 // relative jump short if < or ==  

#define NOP 0x90
