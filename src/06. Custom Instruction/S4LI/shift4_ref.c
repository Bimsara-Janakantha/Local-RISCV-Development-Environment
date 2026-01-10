// shift4_ref.c
#include <stdio.h>


int main() {
    volatile int input = 0x00000001;
    volatile int result = 0;

    printf("Input before shift: 0x%08X\n", input);
    
    // Left shift by 4 using standard instruction (slli)
    result = input << 4;
    
    printf("Result after left shift by 4: 0x%08X\n", result);

    return 0;
}

/*
We use volatile to prevent the compiler from optimizing away memory writes—useful if you later inspect memory/register state in Spike.
*/