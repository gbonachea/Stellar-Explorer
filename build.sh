#!/bin/bash
# Script para compilar StellarExplorer (C++/Qt6) en Linux

set -e

APP="StellarExplorer"
BUILD_DIR="build"

# --- Localizar Qt (preferentemente /opt/qt/<versión>/gcc_64) ---
QT_DIR="$(ls -d /opt/qt/*/gcc_64 2>/dev/null | sort -V | tail -1)"
if [ -z "$QT_DIR" ] || [ ! -d "$QT_DIR" ]; then
    echo "❌ No se encontró Qt en /opt/qt/*/gcc_64"
    echo "   Instala Qt 6.11 con el instalador oficial de Qt o define QT_DIR."
    exit 1
fi
export QT_DIR
export PATH="$QT_DIR/bin:$PATH"
export LD_LIBRARY_PATH="$QT_DIR/lib:${LD_LIBRARY_PATH}"

echo "🔧 Usando Qt: $QT_DIR ($("$QT_DIR/bin/qmake" -query QT_VERSION 2>/dev/null))"

echo "🔍 Verificando dependencias del sistema..."
for dep in cmake g++ make; do
    if command -v "$dep" &>/dev/null; then
        echo "✅ $dep ya está instalado"
    else
        echo "📦 Instalando $dep..."
        if command -v apt &>/dev/null; then
            sudo apt install -y "$dep"
        else
            echo "❌ Falta $dep. Instálalo manualmente."
            exit 1
        fi
    fi
done

# Crear directorio de build
mkdir -p "$BUILD_DIR"

cd "$BUILD_DIR"

echo "⚙️  Configurando con CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$QT_DIR"

echo "🔨 Compilando..."
cmake --build . -- -j"$(nproc)"

cd ..

echo ""
echo "✅ Compilación exitosa."
echo "▶️  Para ejecutar (usa el Qt de /opt/qt):"
echo "    LD_LIBRARY_PATH=\"$QT_DIR/lib\" ./$BUILD_DIR/$APP"

# Regenerar portable automáticamente
if [ -f "make-portable.sh" ]; then
    echo ""
    echo "📦 Regenerando versión portable..."
    bash make-portable.sh
fi
