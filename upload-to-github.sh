#!/bin/bash

# 🚀 Orion OS - Script d'Upload GitHub
# Script pour uploader Orion OS sur GitHub avec le logo intégré

set -e  # Arrêter en cas d'erreur

echo "🌟 Orion OS - Upload vers GitHub"
echo "=================================="

# Vérifier que nous sommes dans le bon répertoire
if [ ! -f "orion_logo.png" ]; then
    echo "❌ Erreur: orion_logo.png non trouvé dans le répertoire actuel"
    echo "   Assurez-vous d'être dans le répertoire racine d'Orion OS"
    exit 1
fi

if [ ! -f "README.md" ]; then
    echo "❌ Erreur: README.md non trouvé dans le répertoire actuel"
    exit 1
fi

echo "✅ Vérifications de base réussies"
echo ""

# Vérifier le statut git
echo "📋 Statut du repository Git:"
git status --short
echo ""

# Demander confirmation
read -p "🤔 Voulez-vous continuer avec l'upload vers GitHub? (y/N): " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "❌ Upload annulé par l'utilisateur"
    exit 0
fi

# Ajouter le remote GitHub
echo "🔗 Ajout du remote GitHub..."
if ! git remote get-url origin >/dev/null 2>&1; then
    git remote add origin https://github.com/iyotee/Orion.git
    echo "✅ Remote origin ajouté"
else
    echo "ℹ️  Remote origin existe déjà"
fi

# Renommer la branche en main
echo "🔄 Renommage de la branche master en main..."
git branch -M main
echo "✅ Branche renommée en main"

# Vérifier le remote
echo "🔍 Vérification du remote:"
git remote -v
echo ""

# Push vers GitHub
echo "📤 Push vers GitHub..."
echo "⚠️  ATTENTION: Cette opération va écraser le repository GitHub existant!"
read -p "🤔 Êtes-vous sûr de vouloir continuer? (y/N): " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "❌ Push annulé par l'utilisateur"
    exit 0
fi

echo "🚀 Push en cours..."
if git push -u origin main --force; then
    echo ""
    echo "🎉 SUCCÈS! Orion OS a été uploadé sur GitHub!"
    echo ""
    echo "📋 Vérifications à effectuer:"
    echo "   1. Visitez: https://github.com/iyotee/Orion"
    echo "   2. Vérifiez que le logo orion_logo.png est visible"
    echo "   3. Vérifiez que le README.md s'affiche correctement"
    echo "   4. Vérifiez que tous les fichiers sont présents"
    echo ""
    echo "🔧 Prochaines étapes recommandées:"
    echo "   - Configurer la protection de branche main"
    echo "   - Ajouter des topics au repository"
    echo "   - Configurer GitHub Actions"
    echo "   - Créer des issues pour les prochaines fonctionnalités"
    echo ""
    echo "🌟 Orion OS est maintenant visible sur GitHub avec le logo intégré!"
else
    echo ""
    echo "❌ ERREUR lors du push vers GitHub"
    echo "   Vérifiez vos permissions et votre connexion internet"
    echo "   Consultez GITHUB_SETUP_INSTRUCTIONS.md pour plus de détails"
    exit 1
fi
