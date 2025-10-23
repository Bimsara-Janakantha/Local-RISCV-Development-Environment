/*
    Title: Trigger an ecall Manually
    Auther: Janakantha S.M.B.G.
    Last Update: 23 Oct 2025   
    
    Note: This program should write a message to the console.
*/

#include <stdio.h>

// Make a ecall
void my_write(int fd, const char* buff, size_t count){
    register long a0 asm("a0") = fd;
    register long a1 asm("a1") = (long)buff;
    register long a2 asm("a2") = count;

    // Syscall for write
    register long a7 asm("a7") = 64; 

    asm volatile("ecall" : : "r"(a0), "r"(a1), "r"(a2), "r"(a7) : "memory");
}

int main(){
    printf("About to trigger ecall...\n");
    
    // Call to ecall
    const char msg[] = "Hello from raw ecall!\n";
    
    // fd = 1 means stdout
    my_write(1, msg, sizeof(msg) - 1);

    // End of the program
    printf("That's it RISC-V.\n");
    return 0;
}