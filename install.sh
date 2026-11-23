#!/bin/bash
# Script para instalar Stellar Explorer e integrarlo con el sistema
# Ejecutar: sudo bash install.sh
# Para desinstalar: sudo bash install.sh --uninstall

set -e

APP="StellarExplorer"
PREFIX="/usr/local"
BINDIR="$PREFIX/bin"
DESKTOP_DIR="$PREFIX/share/applications"
ICON_DIR="$PREFIX/share/icons/hicolor"
CSS_DIR="$PREFIX/share/$APP"

echo "========================================"
echo " Instalador de Stellar Explorer"
echo "========================================"

# --- Desinstalar ---
if [ "$1" == "--uninstall" ]; then
    echo "🗑️  Desinstalando $APP..."
    rm -f "$BINDIR/$APP"
    rm -f "$DESKTOP_DIR/$APP.desktop"
    rm -f "$ICON_DIR/256x256/apps/$APP.png"
    rm -rf "$CSS_DIR"
    for sz in 16 22 24 32 48 64 96 128; do
        rm -f "$ICON_DIR/${sz}x${sz}/apps/$APP.png"
    done
    update-desktop-database "$DESKTOP_DIR" 2>/dev/null || true
    gtk-update-icon-cache -f -t "$PREFIX/share/icons/hicolor" 2>/dev/null || true
    xdg-mime uninstall "$DESKTOP_DIR/$APP.desktop" 2>/dev/null || true
    echo "✅ Desinstalación completada."
    exit 0
fi

# --- Localizar Qt (preferentemente /opt/qt/<versión>/gcc_64) ---
QT_DIR="$(ls -d /opt/qt/*/gcc_64 2>/dev/null | sort -V | tail -1)"
if [ -z "$QT_DIR" ] || [ ! -d "$QT_DIR" ]; then
    echo "❌ No se encontró Qt en /opt/qt/*/gcc_64"
    exit 1
fi
export QT_DIR
export PATH="$QT_DIR/bin:$PATH"
export LD_LIBRARY_PATH="$QT_DIR/lib:${LD_LIBRARY_PATH}"
echo "🔧 Usando Qt: $QT_DIR ($("$QT_DIR/bin/qmake" -query QT_VERSION 2>/dev/null))"

# --- Compilar ---
echo "🔨 Compilando..."
cd "$(dirname "$0")"
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PREFIX" -DCMAKE_PREFIX_PATH="$QT_DIR"
cmake --build . -- -j"$(nproc)"
cd ..

echo "📦 Instalando archivos..."

# --- Crear directorios ---
mkdir -p "$BINDIR"
mkdir -p "$DESKTOP_DIR"
mkdir -p "$CSS_DIR"
for sz in 16 22 24 32 48 64 96 128 256; do
    mkdir -p "$ICON_DIR/${sz}x${sz}/apps"
done

# --- Copiar binario ---
cp build/$APP "$BINDIR/$APP"
chmod 755 "$BINDIR/$APP"

# --- Copiar CSS ---
cp themes.css "$CSS_DIR/"

# --- Generar iconos escalados desde el PNG base ---
if command -v convert &>/dev/null; then
    echo "🖼️  Generando iconos en múltiples tamaños..."
    for sz in 16 22 24 32 48 64 96 128; do
        convert icons/StellarExplorer.png -resize ${sz}x${sz} "$ICON_DIR/${sz}x${sz}/apps/$APP.png"
    done
    cp icons/StellarExplorer.png "$ICON_DIR/256x256/apps/$APP.png"
else
    echo "⚠️  ImageMagick no instalado. Solo se copia el icono 256x256."
    cp icons/StellarExplorer.png "$ICON_DIR/256x256/apps/$APP.png"
fi

# --- Copiar .desktop con la ruta correcta ---
sed "s|Exec=StellarExplorer|Exec=$BINDIR/$APP|g" StellarExplorer.desktop > "$DESKTOP_DIR/$APP.desktop"
chmod 644 "$DESKTOP_DIR/$APP.desktop"

# --- Actualizar bases de datos del sistema ---
echo "🔄 Actualizando bases de datos del sistema..."
update-desktop-database "$DESKTOP_DIR" 2>/dev/null || true
gtk-update-icon-cache -f -t "$PREFIX/share/icons/hicolor" 2>/dev/null || true

# --- Registrar MIME type ---
echo "🔗 Registrando como gestor de directorios..."
xdg-mime install --novendor "$DESKTOP_DIR/$APP.desktop" 2>/dev/null || true

echo ""
echo "========================================"
echo "✅ Instalación completada."
echo ""
echo "Para abrir una carpeta con Stellar Explorer:"
echo "  1. Haz clic derecho en una carpeta → 'Abrir con...' → Stellar Explorer"
echo ""
echo "Para establecerlo como predeterminado:"
echo "  xdg-mime default $APP.desktop inode/directory"
echo "  (o desde Configuración del sistema → Aplicaciones predeterminadas)"
echo "========================================"
