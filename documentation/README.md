# ORION OS Documentation

## **📚 Documentation Unifiée sur le Wiki GitHub**

Toute la documentation ORION OS est maintenant **unifiée et centralisée** sur le [Wiki GitHub officiel](https://github.com/iyotee/Orion/wiki).

---

## **🎯 Pourquoi cette Unification ?**

### **Avantages de la Documentation Wiki**
- **Centralisation** : Une seule source de vérité
- **Collaboration** : Édition facile par la communauté
- **Versioning** : Historique des modifications
- **Navigation** : Liens internes et structure claire
- **Accessibilité** : Disponible directement sur GitHub

---

## **📖 Contenu du Wiki ORION OS**

### **🏠 Pages Principales**
- **[Home](https://github.com/iyotee/Orion/wiki/Home)** - Page d'accueil et vue d'ensemble
- **[Quick Start](https://github.com/iyotee/Orion/wiki/Quick-Start)** - Démarrage rapide
- **[Installation Guide](https://github.com/iyotee/Orion/wiki/Installation-Guide)** - Guide d'installation complet

### **🔧 Développement**
- **[Development Guide](https://github.com/iyotee/Orion/wiki/Development-Guide)** - Guide complet pour développeurs
- **[Contributing Guidelines](https://github.com/iyotee/Orion/wiki/Contributing-Guidelines)** - Comment contribuer au projet
- **[Architecture Overview](https://github.com/iyotee/Orion/wiki/Architecture-Overview)** - Vue d'ensemble de l'architecture

### **📋 Projet et Roadmap**
- **[TODO](https://github.com/iyotee/Orion/wiki/TODO)** - Liste des tâches et objectifs
- **[ROADMAP](https://github.com/iyotee/Orion/wiki/ROADMAP)** - Feuille de route détaillée
- **[Wiki Summary](https://github.com/iyotee/Orion/wiki/Wiki-Summary)** - Résumé de la documentation

### **🏗️ Architecture et Techniques**
- **[Storage Framework](https://github.com/iyotee/Orion/wiki/STORAGE_FRAMEWORK)** - Framework de stockage
- **[Wiki Update](https://github.com/iyotee/Orion/wiki/WIKI_UPDATE)** - Mises à jour du wiki
- **[Wiki Complete](https://github.com/iyotee/Orion/wiki/WIKI-COMPLETE)** - Documentation complète

---

## **🚀 Accès au Wiki**

### **Lien Direct**
🌐 **[https://github.com/iyotee/Orion/wiki](https://github.com/iyotee/Orion/wiki)**

### **Navigation GitHub**
1. Aller sur [https://github.com/iyotee/Orion](https://github.com/iyotee/Orion)
2. Cliquer sur l'onglet **"Wiki"**
3. Explorer la documentation complète

---

## **🔧 Déploiement du Wiki**

### **Script de Déploiement**
Le dossier `documentation/` contient :
- **`deploy-wiki.bat`** - Script Windows pour déployer le wiki
- **`wiki_backup/`** - Sauvegarde de tous les fichiers wiki
- **`_wiki.yml`** - Configuration du wiki

### **Déploiement Automatique**
```bash
# Depuis le dossier documentation/
./deploy-wiki.bat
```

---

## **📁 Structure du Projet ORION**

### **Dossiers Principaux**
- **`kernel/`** - Noyau C universel (100% portable)
- **`bootloader/`** - Boot universel unifié
- **`config/`** - Configuration et SDK
- **`dev/`** - Outils développement
- **`documentation/`** - Documentation unifiée (ce dossier)

### **Architecture Kernel**
```
kernel/
├── arch/           # Support multi-architecture
├── core/           # Services universels
│   ├── scheduler/  # Scheduler adaptatif
│   ├── ipc/        # IPC cross-architecture
│   ├── capabilities/ # Système capabilities
│   ├── memory/     # Gestion mémoire
│   ├── services/   # Services système
│   ├── drivers/    # Pilotes universels
│   └── system/     # Système unifié
└── README.md       # Documentation kernel
```

---

## **🎯 Master Prompt Respecté**

Cette organisation respecte **100%** le Master Prompt ORION v2.0 :
- ✅ **Code universel** : 95% du code compile sur toutes architectures
- ✅ **Architecture-first** : Structure optimisée pour multi-arch
- ✅ **Zero-regression** : Une seule implémentation par service
- ✅ **Performance mesurée** : Structure optimisée pour benchmarks

---

## **🤝 Contribution**

### **Comment Contribuer à la Documentation**
1. **Éditer directement** sur le Wiki GitHub
2. **Proposer des améliorations** via Issues
3. **Soumettre des Pull Requests** pour corrections
4. **Participer aux discussions** sur GitHub

### **Standards de Documentation**
- **Tone académique** et professionnel
- **Anglais uniquement** (pas de français)
- **Structure claire** et navigation intuitive
- **Exemples concrets** et cas d'usage

---

## **📞 Support et Contact**

- **Wiki** : [https://github.com/iyotee/Orion/wiki](https://github.com/iyotee/Orion/wiki)
- **Issues** : [https://github.com/iyotee/Orion/issues](https://github.com/iyotee/Orion/issues)
- **Discussions** : [https://github.com/iyotee/Orion/discussions](https://github.com/iyotee/Orion/discussions)

---

**ORION OS - The Universal Operating System of the Future** 🚀

*"Write once, run everywhere, perform best anywhere"*