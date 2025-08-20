# ADR-001: Pilotes Rust en Userland par Défaut

## Statut
Accepté

## Contexte
Les pilotes représentent 70% des bugs dans Linux et sont une source majeure de vulnérabilités. Orion doit réduire drastiquement les crashes système tout en maintenant les performances.

## Options Considérées

### Option 1: Pilotes C en Noyau (style Linux)
**Avantages:**
- Performance maximale (accès direct mémoire/registres)
- Pas d'overhead IPC
- Modèle familier aux développeurs

**Inconvénients:**
- Crash pilote = crash système
- Aucune isolation mémoire
- Difficile à déboguer
- Exploits pilotes → compromission noyau

### Option 2: Pilotes C en Userland 
**Avantages:**
- Isolation processus
- Redémarrage possible
- Débogage standard userland

**Inconvénients:**
- Toujours vulnérable aux bugs C (buffer overflow, use-after-free)
- Pas de garanties mémoire
- Overhead IPC

### Option 3: Pilotes Rust Userland (CHOIX RETENU)
**Avantages:**
- Memory safety garantie
- Isolation processus forte
- Redémarrage transparent
- Pas de segfaults
- Écosystème moderne

**Inconvénients:**
- Courbe d'apprentissage Rust
- Overhead IPC à optimiser
- Quelques unsafe{} nécessaires pour MMIO

## Décision
**Pilotes Rust en userland avec framework orion-driver**

### Architecture Pilote

```rust
// Trait commun pour tous les pilotes
pub trait OrionDriver {
    fn probe(dev: &DeviceInfo) -> Result<bool, DriverError>;
    fn init(dev: Device) -> Result<Self, DriverError> where Self: Sized;
    fn handle_irq(&self) -> Result<(), DriverError>;
    fn shutdown(&mut self);
}

// Example pilote VirtIO
pub struct VirtIOBlockDriver {
    device: Device,
    queues: Vec<VirtQueue>,
}

impl OrionDriver for VirtIOBlockDriver {
    fn probe(dev: &DeviceInfo) -> Result<bool, DriverError> {
        Ok(dev.vendor_id == 0x1AF4 && dev.device_id == 0x1001)
    }
    
    // Implementation...
}
```

### Communication Noyau ↔ Pilote

1. **IPC Optimisé**: Messages typés via orion-ipc
2. **Shared Memory**: Zero-copy pour gros transferts  
3. **Event-driven**: Notifications interruptions via eventfd
4. **Capabilities**: Accès contrôlé aux ressources

### Gestion MMIO Sécurisée

```rust
// Abstraction sûre pour accès registres
pub struct MmioRegion {
    base: NonNull<u8>,
    size: usize,
}

impl MmioRegion {
    // Seule fonction unsafe, vérifiée
    pub unsafe fn new(phys_addr: u64, size: usize) -> Result<Self, MmioError> {
        // Validation + mapping via syscall
    }
    
    // Accès sûrs
    pub fn read_u32(&self, offset: usize) -> u32 { /* */ }
    pub fn write_u32(&self, offset: usize, value: u32) { /* */ }
}
```

### Isolation et Robustesse

**Processus séparés**: Chaque pilote = processus isolé
- Budget mémoire/CPU via capabilities
- Crash pilote n'affecte pas le système
- Redémarrage automatique supervisé

**IOMMU**: Isolation DMA quand disponible
- Pilote ne peut accéder qu'à ses buffers autorisés
- Protection contre DMA malveillant

### Pilotes Cibles v1.0

1. **VirtIO** (block, net, gpu) - Base pour développement/test
2. **NVMe** - Performance stockage moderne  
3. **E1000/IGB** - Réseau réel
4. **AHCI/SATA** - Compatibilité stockage
5. **USB XHCI** - Périphériques
6. **Framebuffer** - Display basique

### Exceptions: Stubs Noyau

Rares cas où un stub C minimal en noyau est acceptable :
- Timer hardware critique (scheduler)
- Console série (debug early boot)
- Interrupts controller de base

**Contraintes stubs:**
- < 200 LOC par stub
- Code defensive programming
- Tests exhaustifs
- Review obligatoire 2 personnes

## Conséquences

### Positives
- Élimination quasi-totale des crashes pilotes
- Memory safety garantie (sauf unsafe vérifiés)
- Débogage pilotes simplifié
- Hot-reload des pilotes
- Écosystème Rust (async, testing, etc.)

### Négatives
- Overhead IPC (optimisable via zero-copy)
- Courbe apprentissage Rust
- Quelques unsafe{} pour MMIO (minimal, vérifié)

## KPIs de Validation
- 0 crash système dû aux pilotes pendant fuzz 24h
- Redémarrage pilote < 100ms
- Throughput I/O ≥ 85% des pilotes C équivalents
- < 5% lignes unsafe dans les pilotes

## Implémentation

### Phase 1: Framework orion-driver
```rust
// Crate lib/orion-driver
- traits::OrionDriver
- mmio::MmioRegion  
- ipc::DriverIpc
- device::DeviceInfo
```

### Phase 2: Pilotes de base
- VirtIO block/net (développement)
- Console série stub (boot)

### Phase 3: Pilotes production
- NVMe, E1000, AHCI
- Intégration IOMMU

## Date
2024-01-XX

## Auteur  
Orion Team
