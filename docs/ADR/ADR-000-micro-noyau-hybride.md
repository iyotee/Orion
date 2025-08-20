# ADR-000: Architecture Micro-Noyau Hybride avec Capabilities

## Statut
Accepté

## Contexte
Orion vise à surpasser Linux sur la latence, l'isolation et la robustesse tout en restant pragmatique. Le choix architectural fondamental détermine les performances et la sécurité du système.

## Options Considérées

### Option 1: Noyau Monolithique (style Linux)
**Avantages:**
- Performance maximale (pas de changements de contexte pour les services)
- Simplicité d'implémentation initiale
- Compatibilité plus facile avec l'existant

**Inconvénients:**
- Surface d'attaque énorme (>300 syscalls dans Linux)
- Isolation faible entre composants
- Difficile à maintenir et déboguer
- Crash d'un pilote = crash système

### Option 2: Micro-Noyau Pur (style L4/seL4)
**Avantages:**
- Isolation maximale
- TCB minimal (~10k LOC)
- Vérification formelle possible
- Robustesse exceptionnelle

**Inconvénients:**
- Overhead IPC important pour services fréquents
- Complexité de développement élevée
- Performance dégradée pour certains cas d'usage

### Option 3: Micro-Noyau Hybride (CHOIX RETENU)
**Avantages:**
- TCB réduit (~30k LOC) mais services critiques en noyau
- Isolation forte pour les pilotes (userland)
- Performance acceptable via optimisations IPC
- Pragmatique pour adoption

**Inconvénients:**
- Plus complexe qu'un monolithe
- Nécessite des optimisations IPC avancées

## Décision
**Micro-noyau hybride avec modèle de capabilities**

### TCB (Trusted Computing Base) en C
- Planificateur (scheduler)
- Gestion mémoire (VMM/PMM) 
- IPC noyau optimisé
- Gestion des capabilities
- Primitive de synchronisation
- Syscall gate
- Gestion exceptions/interrupts
- Bootstrap UEFI/ACPI

### Userland Privilégié
- Gestionnaire de pilotes (Rust)
- Serveurs FS (OrionFS)
- Pile réseau TCP/IP
- Serveur de compatibilité POSIX
- Daemon de sécurité

### Pilotes Userland (Rust)
- Tous les pilotes par défaut en userland
- Isolation via processus + IOMMU
- Communication IPC + event-driven
- Restart automatique en cas de crash

## Modèle de Sécurité: Capabilities

Remplace le modèle traditionnel UID/GID par des capabilities non-forgeables :

```c
typedef uint64_t or_cap_t;

// Chaque objet a des droits spécifiques
#define CAP_READ    (1 << 0)
#define CAP_WRITE   (1 << 1) 
#define CAP_EXEC    (1 << 2)
#define CAP_GRANT   (1 << 3)

int sys_cap_grant(or_cap_t obj, or_cap_t target, uint64_t rights);
int sys_cap_revoke(or_cap_t obj, or_cap_t target, uint64_t rights);
```

## Conséquences

### Positives
- Surface d'attaque réduite (<60 syscalls vs >300 Linux)
- Isolation forte des pilotes
- Redémarrage des composants en cas de crash
- Vérification plus facile du TCB
- Performance mesurable et optimisable

### Négatives  
- Complexité IPC à optimiser
- Courbe d'apprentissage pour développeurs
- Plus d'effort initial qu'un monolithe

## KPIs de Validation
- Latence syscall < 150ns (vs 200-300ns Linux)
- Context switch < 250ns 
- IPC 64B < 300ns, >8M msg/s
- TCB < 30k LOC
- 0 crash système après fuzz 24h

## Date
2024-01-XX

## Auteur
Orion Team
