# Orion - Vue d'ensemble

## Qu'est-ce qu'Orion ?

Orion est un noyau d'OS moderne conçu pour surpasser Linux sur des aspects critiques : **latence**, **isolation** et **robustesse**, tout en restant pragmatique pour une adoption réelle.

## Philosophie

Là où Linux privilégie l'universalité, Orion privilégie la **prévisibilité**, la **mesure** et l'**isolation**. C'est complémentaire, pas antagoniste.

## Différences clés vs Linux/BSD

### Architecture

| Aspect | Linux | BSD | Orion |
|--------|-------|-----|-------|
| **Type** | Monolithique | Monolithique | Micro-noyau hybride |
| **TCB** | ~30M LOC | ~15M LOC | **<30k LOC** |
| **Syscalls** | >300 | >250 | **<60** |
| **Pilotes** | Noyau (C) | Noyau (C) | **Userland (Rust)** |
| **Sécurité** | UID/GID + LSM | UID/GID + MAC | **Capabilities pures** |

### Performance ciblée

| Métrique | Linux LTS | Orion Cible |
|----------|-----------|-------------|
| **Syscall null** | 200-300ns | **<150ns** (médiane) |
| **Context switch** | 300-500ns | **<250ns** (même core) |
| **IPC 64B** | N/A | **<300ns**, >8M msg/s |
| **Surface d'attaque** | Énorme | **<60 syscalls** |

### Robustesse

- **Pilotes isolés** : Crash pilote ≠ crash système
- **Memory safety** : Rust + C défensif avec invariants vérifiés
- **Capabilities** : Pas de privilèges globaux, droits fins et révocables
- **Fuzzing continu** : 24h sans panic requis pour release

## Pourquoi un nouveau noyau ?

### 1. **Latence prévisible**
Linux optimise le débit, Orion optimise la latence. Critical pour :
- Trading haute fréquence
- Systèmes temps réel mou  
- Interactive computing

### 2. **Isolation moderne**
- Pilotes en processus isolés par défaut
- IOMMU mandatory pour DMA
- Capabilities au lieu de UID/GID
- Sandbox bytecode pour filtrage

### 3. **Sécurité by design**
- TCB minimal (<30k LOC vs 30M Linux)
- W^X partout, KASLR, SMEP/SMAP/UMIP
- Pas de RWX pages, guard pages, stack canaries
- Crypto : DRBG + entropy multi-sources

### 4. **Mesurabilité**
Chaque optimisation est mesurée, pas devinée :
- Benchmarks automatisés vs Linux LTS
- KPIs objectifs avec régression detection
- Profiling intégré (orion-trace)

## Cas d'usage cibles

### Primaires
- **HPC/Scientific computing** : Latence IPC prévisible
- **Trading systems** : Syscalls <150ns garantis
- **Embedded haute performance** : TCB minimal
- **Development/Research** : Architecture clean pour expériments

### Secondaires  
- **Desktop léger** : Via couche POSIX + containers
- **IoT sécurisé** : Capabilities + drivers isolés
- **Virtualisation** : Host lean et sécurisé

## Roadmap vs Alternatives

### Court terme (6 mois)
- Boot UEFI + console ✓
- Syscalls de base + IPC ✓  
- Pilotes VirtIO (développement)
- Couche POSIX basique (BusyBox)

### Moyen terme (1 an)
- Pilotes réels (NVMe, e1000, AHCI)
- FS performant (OrionFS)
- Réseau complet (TCP/IP stack)
- Sandbox + policies de sécurité

### Long terme (2+ ans)
- SMP optimisé + NUMA awareness
- Conteneurs légers intégrés
- GUI moderne (Wayland-like)
- Écosystème développeur complet

## Comparaison détaillée

### vs Linux
**Avantages Orion :**
- Latence syscall 2-3x meilleure
- Surface d'attaque 10x plus petite
- Pilotes isolés (robustesse)
- Architecture plus clean

**Avantages Linux :**
- Écosystème énorme
- Hardware support universel
- Compatibilité applications
- Équipe de milliers de devs

### vs seL4/QNX
**Avantages Orion :**
- Plus pragmatique (hybrid vs pur microkernel)
- Performance acceptable (pas de overhead L4)
- Rust drivers (memory safety moderne)
- Open source + development friendly

**Avantages seL4/QNX :**
- Proof formelle (seL4)
- Maturité industrielle (QNX)
- Certification temps réel

### vs Redox/Fuchsia
**Avantages Orion :**
- Focus performance mesurable
- Architecture hybrid (pragmatique)
- C+Rust optimal (vs Rust pur)
- KPIs objectifs vs Linux

**Avantages Redox/Fuchsia :**
- Plus de funding/équipe (Fuchsia)
- Rust pur plus safe (Redox)
- Écosystème en développement

## Adoption strategy

### Phase 1: Niche high-value
- Labs de recherche (latence prévisible)
- Trading firms (syscalls rapides)
- Embedded critique (TCB minimal)

### Phase 2: Compatibilité
- POSIX complète + containers Linux
- Desktop basic (navigateur + dev tools)
- Cloud native (micro-VMs)

### Phase 3: Mainstream
- Hardware OEM partnerships
- Distribution complète
- Écosystème développeur

## Métriques de succès

### Techniques
- [ ] Syscall <150ns médiane (vs ~250ns Linux)
- [ ] Context switch <250ns (vs ~400ns Linux)
- [ ] IPC >8M msg/s avec latence <300ns
- [ ] 0 crash sur fuzz 24h
- [ ] Débit FS ≥85% ext4 même disque

### Adoption
- [ ] 10 organisations utilisent en production
- [ ] 100 contributeurs actifs
- [ ] 1000 stars GitHub
- [ ] Documentation complète + tutoriels

## Contribution

Orion est **open source** (Apache-2.0) et accueille les contributions :

- **C kernel** : Architecture, mémoire, IPC, syscalls
- **Rust drivers** : Stockage, réseau, USB, GPU  
- **Userland** : FS servers, pile réseau, outils
- **Testing** : Fuzzing, benchmarks, intégration
- **Docs** : Guides, specs, tutorials

Voir `CONTRIBUTING.md` pour démarrer.

---

*Orion ose réduire le TCB, imposer les capabilities, isoler les pilotes par défaut, et mesurer objectivement la latence et l'IPC.*
