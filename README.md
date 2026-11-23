# Stellar Explorer

Gestor de archivos para Linux escrito en **C++/Qt6**, moderno, rápido y portable.

## Captura

- Panel lateral con ubicaciones, dispositivos y red
- Barra de ruta navegable (breadcrumbs)
- Pestañas múltiples
- Vista en lista, iconos e iconos grandes
- Búsqueda en tiempo real con filtro por nombre
- Temas claro / oscuro / sistema
- Ventana única o portable (todas las librerías incluidas)

## Características

| Función | Descripción |
|---|---|
| **Panel lateral** | Árbol con ubicaciones del sistema (home, escritorio, documentos, etc.), dispositivos montables y recursos de red (GVFS). La selección se sincroniza con la carpeta actual. |
| **Breadcrumbs** | Barra de ruta interactiva: cada segmento es cliqueable para navegar directamente. |
| **Pestañas** | Múltiples carpetas abiertas simultáneamente (Ctrl+T nueva, Ctrl+W cerrar). |
| **Navegación** | Atrás / Adelante con historial por pestaña (Alt+←/→). Arriba (Alt+↑). |
| **Operaciones con archivos** | Copiar (Ctrl+C), Cortar (Ctrl+X), Pegar (Ctrl+V) con barra de progreso. Renombrar, Eliminar (a Papelera), Crear carpeta. |
| **Búsqueda** | Filtro en tiempo real mientras escribes (200ms de retardo). Coincidencia por nombre, sin distinguir mayúsculas. |
| **Menú contextual** | Abrir, Abrir con..., Ejecutar en terminal, Mover a..., Propiedades. |
| **Papelera** | Ver contenido, restaurar archivos, eliminar definitivamente. |
| **Dispositivos** | Montar/desmontar discos usando udisksctl con autenticación por pkexec. |
| **Red (GVFS)** | Navegar recursos de red montados en `/run/user/*/gvfs`. |
| **Modos de vista** | Lista, Iconos (48px), Iconos grandes (96px). Slider de tamaño de iconos en la barra inferior. |
| **Temas** | Claro / Oscuro / Sistema (sigue el tema del escritorio). CSS personalizable (`themes.css`). |
| **Color de acento** | Selector de color en configuración. |
| **Persistencia** | Geometría de ventana, preferencias y última ruta guardados con QSettings. |
| **Portable** | Script `make-portable.sh` que empaqueta el binario con todas las librerías Qt6 y plugins necesarios. Sin instalación, funciona desde cualquier carpeta. |
| **Instalación** | Script `install.sh` que registra la app en el sistema: iconos, .desktop, MIME type `inode/directory` para "Abrir con...". |

## Requisitos

- **Compilación**: CMake ≥ 3.16, GCC/Clang con C++17, Qt6 (qt6-base-dev)
- **Ejecución**: Linux con X11 o Wayland, librerías Qt6 instaladas (o usar la versión portable)

```bash
sudo apt install cmake g++ qt6-base-dev
```

## Compilar y ejecutar

```bash
./build.sh              # compila y regenera el portable
cd build
./StellarExplorer      # ejecutar
./StellarExplorer /ruta # abrir carpeta específica
./StellarExplorer --help
```

## Versión portable

```bash
./make-portable.sh      # genera StellarExplorer-Portable/
cd StellarExplorer-Portable
./run.sh                # ejecutar sin dependencias del sistema
```

## Instalación en el sistema

```bash
sudo ./install.sh                        # instalar
sudo ./install.sh --uninstall            # desinstalar
xdg-mime default StellarExplorer.desktop inode/directory  # establecer como gestor predeterminado
```

## Estructura del proyecto

```
StellarExplorer/
├── CMakeLists.txt           # Build system (Qt6)
├── build.sh                 # Compilar y regenerar portable
├── make-portable.sh         # Empaquetar con librerías
├── install.sh               # Instalar en el sistema
├── resources.qrc            # Iconos embebidos en el binario
├── themes.css               # Estilos (claro/oscuro)
├── StellarExplorer.desktop  # Acceso directo
├── icons/                   # PNGs de iconos (embebidos vía .qrc)
└── src/
    ├── main.cpp             # Punto de entrada, tema, argumentos
    ├── mainwindow.h/.cpp    # Ventana principal con pestañas y toolbar
    ├── filetab.h/.cpp       # Widget de pestaña (árbol + vista + búsqueda)
    ├── breadcrumbs.h/.cpp   # Barra de ruta navegable
    ├── fileopworker.h/.cpp  # Hilo para copiar/mover con progreso
    ├── configdialog.h/.cpp  # Diálogo de configuración
    └── utils.h/.cpp         # Funciones auxiliares
```

## Licencia

Este proyecto está licenciado bajo la **GNU General Public License v3.0**.
Consulta el archivo [LICENSE](LICENSE) para más detalles.
