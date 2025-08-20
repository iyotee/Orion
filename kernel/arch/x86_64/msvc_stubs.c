// Stubs assembleur pour MSVC - sera remplacé par de l'assembleur pur plus tard
#include <orion/types.h>

#ifdef _MSC_VER

// Stubs pour les fonctions assembleur x86_64 
void cli(void) {
    // Stub MSVC - désactive interruptions
    // TODO: Implémenter avec __asm ou assembleur externe
}

void hlt(void) {
    // Stub MSVC - halt CPU
    // TODO: Implémenter avec __asm ou assembleur externe
    while (1) { /* busy wait */ }
}

uint64_t read_cr3(void) {
    // Stub MSVC - lit registre CR3
    // TODO: Implémenter avec __asm ou assembleur externe
    return 0x1000; // Valeur bidon pour compilation
}

// Stubs pour I/O ports
unsigned char inb(unsigned short port) {
    (void)port;
    return 0; // Stub
}

void outb(unsigned short port, unsigned char data) {
    (void)port;
    (void)data;
    // Stub
}

#endif // _MSC_VER
