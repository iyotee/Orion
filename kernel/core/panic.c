// Gestion des panics et arrêts d'urgence Orion
#include <orion/kernel.h>
#include <orion/types.h>

// État du panic
static bool panic_in_progress = false;

// Arrêt d'urgence du système  
void emergency_halt(void) {
    // Disable interrupts
    cli();
    
    // Arrêter tous les CPUs si SMP (TODO)
    
    // Boucle infinie
    while (1) {
        hlt();
    }
}

// Handler de panic principal (déjà défini dans entry.c, mais version complète ici)
void panic_full(const char* file, int line, const char* function, const char* fmt, ...) {
    // Éviter panic récursif
    if (panic_in_progress) {
        emergency_halt();
    }
    panic_in_progress = true;
    
    // Disable interrupts immédiatement
    cli();
    
    // Affichage du panic
    kprintf("\n");
    kprintf("################################\n");
    kprintf("###   ORION KERNEL PANIC     ###\n");
    kprintf("################################\n");
    kprintf("\n");
    
    kprintf("Location: %s:%d in %s()\n", file, line, function);
    kprintf("Message: %s\n", fmt); // Simplified - TODO: va_args
    kprintf("\n");
    
    // TODO: Afficher stack trace
    // TODO: Afficher registres CPU
    // TODO: Sauvegarder core dump
    
    kprintf("System state:\n");
    kprintf("  Interrupts: disabled\n");
    kprintf("  Panic in progress: yes\n");
    
    kprintf("\n");
    kprintf("System will halt.\n");
    kprintf("################################\n");
    
    // Arrêt d'urgence
    emergency_halt();
}

// Macro pour panic avec info de localisation
#define PANIC(fmt, ...) panic_full(__FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

// Assertions avec panic
void assert_failed(const char* expr, const char* file, int line, const char* function) {
    panic_full(file, line, function, "Assertion failed: %s", expr);
}

// Vérification de corruption mémoire basique
bool check_stack_canary(void) {
    // TODO: Implémenter vérification de stack canary
    return true;
}

// Dump des registres (stub)
void dump_registers(void) {
    kprintf("Register dump:\n");
    
    // TODO: Capturer et afficher tous les registres
    kprintf("  (register dump not implemented)\n");
}

// Dump de la pile (stub)
void dump_stack(void) {
    kprintf("Stack trace:\n");
    
    // TODO: Parcourir la pile et afficher les adresses de retour
    kprintf("  (stack trace not implemented)\n");
}

// Log d'audit pour événements de sécurité
void security_audit_log(const char* event, uint64_t pid, const char* details) {
    kprintf("[AUDIT] PID %llu: %s - %s\n", 
           (unsigned long long)pid, event, details);
    
    // TODO: Sauvegarder dans log persistant
}
