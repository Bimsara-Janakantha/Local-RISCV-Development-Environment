/*
    Title: Trigger a custom ecall from M-mode
    Auther: Janakantha S.M.B.G.
    Last Update: 25 Oct 2025   
*/

void trigger_ecall() {
    asm volatile ("ecall");
}

void _exit(int code) {
    while(1); // simple halt
}

int main() {
    // Trigger an ecall from M-mode
    trigger_ecall();

    // If handler works, we return here
    _exit(0);
    return 0;
}