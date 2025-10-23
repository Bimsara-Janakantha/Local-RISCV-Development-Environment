/*
    Title: Trigger an ecall Manually
    Auther: Janakantha S.M.B.G.
    Last Update: 23 Oct 2025   
    
    Note: This program should exit without printiing line "Never reaches here\n"
*/

#include <stdio.h>

// Make a ecall
void my_exit(int code){
    register long a7 asm("a7") = 93;  // Linux/RISC-V exit syscall number
    register long a0 asm("a0") = code;
    asm volatile ("ecall" : : "r"(a0), "r"(a7) : "memory");
}

int main(){
    printf("About to trigger ecall...\n");
    
    // Call to ecall
    my_exit(0);

    // Never reaches here
    printf("Never reaches here\n");
    return 0;
}