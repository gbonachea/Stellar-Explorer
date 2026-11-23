#!/bin/bash
# Crea una versión portable de Stellar Explorer con todas las librerías incluidas
# Uso: bash make-portable.sh

set -e

APP="StellarExplorer"
PORTABLE_DIR="${APP}-Portable"
BUILD_DIR="build"

echo "=========================================="
echo " Creando versión portable de $APP"
echo "=========================================="

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
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_PREFIX_PATH="$QT_DIR"
cmake --build . -- -j"$(nproc)"
cd ..

# --- Limpiar y crear estructura portable ---
rm -rf "$PORTABLE_DIR"
mkdir -p "$PORTABLE_DIR/lib"
mkdir -p "$PORTABLE_DIR/plugins/platforms"
mkdir -p "$PORTABLE_DIR/plugins/imageformats"
mkdir -p "$PORTABLE_DIR/plugins/iconengines"
mkdir -p "$PORTABLE_DIR/plugins/styles"
mkdir -p "$PORTABLE_DIR/plugins/platformthemes"

echo "📦 Copiando binario..."
cp "$BUILD_DIR/$APP" "$PORTABLE_DIR/"

echo "📦 Copiando themes.css..."
cp themes.css "$PORTABLE_DIR/"

# --- Resolver y copiar librerías ---
echo "🔍 Resolviendo dependencias de librerías..."

resolve_libs() {
    local BIN="$1"
    local DEST="$2"
    local SEEN="$3"
    local DEPS

    DEPS=$(ldd "$BIN" 2>/dev/null | awk '/=> \//{print $3}' | grep -v '^$' || true)

    for lib in $DEPS; do
        local name
        name=$(basename "$lib")
        if [ -f "$lib" ] && ! grep -q "^$name$" "$SEEN"; then
            echo "$name" >> "$SEEN"
            cp -L "$lib" "$DEST/" 2>/dev/null || true
            resolve_libs "$lib" "$DEST" "$SEEN"
        fi
    done
}

# Collect initial deps from the binary
SEEN_FILE=$(mktemp)
trap "rm -f $SEEN_FILE" EXIT

resolve_libs "$PORTABLE_DIR/$APP" "$PORTABLE_DIR/lib" "$SEEN_FILE"

# --- Copiar plugins de Qt desde la instalación de /opt/qt ---
QT_PLUGIN_DIR="$QT_DIR/plugins"

copy_plugins() {
    local subdir="$1"
    [ -d "$QT_PLUGIN_DIR/$subdir" ] || return 0
    for plugin in "$QT_PLUGIN_DIR/$subdir"/*.so; do
        [ -f "$plugin" ] || continue
        cp "$plugin" "$PORTABLE_DIR/plugins/$subdir/"
        resolve_libs "$plugin" "$PORTABLE_DIR/lib" "$SEEN_FILE"
    done
}

if [ -d "$QT_PLUGIN_DIR" ]; then
    # Plataformas: xcb (X11) y wayland, más minimales de respaldo
    copy_plugins platforms
    # Formatos de imagen (PNG es integrado, pero se copian todos los disponibles)
    copy_plugins imageformats
    # Motores de iconos (SVG, XDG)
    copy_plugins iconengines
    # Temas de plataforma (integración GTK)
    copy_plugins platformthemes
    # Estilos
    copy_plugins styles
fi

# --- Eliminar librerías redundantes o problemáticas ---
rm -f "$PORTABLE_DIR/lib/libc.so.6" 2>/dev/null || true
rm -f "$PORTABLE_DIR/lib/libm.so.6" 2>/dev/null || true
rm -f "$PORTABLE_DIR/lib/ld-linux-x86-64.so.2" 2>/dev/null || true
rm -f "$PORTABLE_DIR/lib/libdl.so.2" 2>/dev/null || true
rm -f "$PORTABLE_DIR/lib/libpthread.so.0" 2>/dev/null || true
rm -f "$PORTABLE_DIR/lib/librt.so.1" 2>/dev/null || true
rm -f "$PORTABLE_DIR/lib/libresolv.so.2" 2>/dev/null || true
rm -f "$PORTABLE_DIR/lib/libutil.so.1" 2>/dev/null || true
rm -f "$PORTABLE_DIR/lib/libnss*" 2>/dev/null || true

# --- Reducir tamaño con strip ---
if command -v strip &>/dev/null; then
    echo "✂️  Reduciendo tamaño de librerías (strip)..."
    find "$PORTABLE_DIR/lib" -name "*.so*" -type f -exec strip --strip-unneeded {} \; 2>/dev/null || true
    find "$PORTABLE_DIR/plugins" -name "*.so" -type f -exec strip --strip-unneeded {} \; 2>/dev/null || true
    strip --strip-all "$PORTABLE_DIR/$APP" 2>/dev/null || true
fi

# --- Ajustar RPATH para que funcione aunque el sistema tenga LD_LIBRARY_PATH ---
# Se usa RPATH (no RUNPATH) con patchelf --force-rpath: RPATH tiene prioridad
# sobre LD_LIBRARY_PATH, así el binario encuentra SUS librerías ($ORIGIN/lib)
# aunque el sistema exporte LD_LIBRARY_PATH (p. ej. /usr/lib/x86_64-linux-gnu).
if command -v patchelf &>/dev/null; then
    echo "🔧 Ajustando RPATH (\$ORIGIN) para ejecución directa..."
    patchelf --force-rpath --set-rpath '$ORIGIN/lib' "$PORTABLE_DIR/$APP" 2>/dev/null || true
    find "$PORTABLE_DIR/lib" -name '*.so*' -type f \
        -exec patchelf --force-rpath --set-rpath '$ORIGIN' {} \; 2>/dev/null || true
    find "$PORTABLE_DIR/plugins" -name '*.so' -type f \
        -exec patchelf --force-rpath --set-rpath '$ORIGIN/../lib' {} \; 2>/dev/null || true
else
    echo "⚠️  patchelf no instalado: usa ./run.sh para lanzar la app."
fi

# --- Crear qt.conf para que Qt use las librerías y plugins del portable ---
echo "📝 Creando qt.conf..."
cat > "$PORTABLE_DIR/qt.conf" << 'QTCONF'
[Paths]
Prefix = .
Plugins = plugins
Libraries = lib
QTCONF

# --- Crear script de ejecución ---
echo "📝 Creando run.sh..."
cat > "$PORTABLE_DIR/run.sh" << 'RUNEOF'
#!/bin/bash
# Lanzador portable para Stellar Explorer
DIR="$(cd "$(dirname "$0")" && pwd)"
export LD_LIBRARY_PATH="$DIR/lib:$LD_LIBRARY_PATH"
export QT_PLUGIN_PATH="$DIR/plugins"
export QT_QPA_PLATFORM_PLUGIN_PATH="$DIR/plugins/platforms"
exec "$DIR/StellarExplorer" "$@"
RUNEOF
chmod +x "$PORTABLE_DIR/run.sh"

# --- Crear .desktop para la versión portable ---
echo "📝 Creando StellarExplorer.desktop portable..."
cat > "$PORTABLE_DIR/StellarExplorer.desktop" << DESKEOF
[Desktop Entry]
Name=Stellar Explorer (Portable)
Exec=$(pwd)/StellarExplorer-Portable/run.sh %U
Icon=$(pwd)/StellarExplorer-Portable/stellarexplorer.png
Terminal=false
Type=Application
Categories=Qt;Utility;FileManager;
MimeType=inode/directory;
DESKEOF

# --- Copiar icono PNG ---
if [ -f "icons/StellarExplorer.png" ]; then
    cp icons/StellarExplorer.png "$PORTABLE_DIR/stellarexplorer.png"
fi

# --- Calcular tamaño total ---
TOTAL_SIZE=$(du -sh "$PORTABLE_DIR" | cut -f1)

echo ""
echo "=========================================="
echo "✅ Portable creado: $PORTABLE_DIR/"
echo "   Qt: $("$QT_DIR/bin/qmake" -query QT_VERSION 2>/dev/null)"
echo "   Tamaño total: $TOTAL_SIZE"
echo ""
echo "▶️  Para ejecutar:"
echo "    cd $PORTABLE_DIR && ./run.sh [ruta]"
echo ""
echo "📦 Para distribuir comprime la carpeta:"
echo "    tar -czf ${PORTABLE_DIR}.tar.gz $PORTABLE_DIR/"
echo "=========================================="
