# Branding — Stellar Explorer

## Nombre del proyecto

**Stellar Explorer** — escrito siempre con inicial mayúscula en cada palabra.

| Correcto | Incorrecto |
|---|---|
| Stellar Explorer | stellar explorer |
| Stellar Explorer | Stellar explorer |
| Stellar Explorer | StellarExplorer (solo en identificadores de código) |

## Logotipo e icono

El icono oficial de la aplicación es un PNG cuadrado de 256×256 píxeles ubicado en `icons/icon.png`.  
Se utiliza como:

- Icono de la ventana
- Icono del ejecutable
- Icono del acceso directo `.desktop`
- Imagen en la pestaña "Acerca de..."

### Tamaños recomendados

| Contexto | Tamaño |
|---|---|
| Barra de título | 16×16, 22×22, 24×24 |
| Notificador | 22×24, 32×32 |
| Acceso directo | 48×48, 64×64, 96×96 |
| Acerca de | 64×64 |
| Instalación sistema | 256×256 |

## Paleta de colores

| Rol | Color Claro | Color Oscuro |
|---|---|---|
| Fondo ventana | `#ffffff` | `#1e1e1e` |
| Fondo toolbar | `#f5f5f5` | `#333333` |
| Texto | `#000000` | `#ffffff` |
| Acento | `#0078D7` | `#0078D7` |

## Tipografía

La aplicación no incluye tipografías personalizadas. Usa la fuente predeterminada del sistema (`QApplication::font()`).

## Tono y estilo

- Interfaz en español (los nombres de las acciones y menús están en español)
- Nombres de carpetas en el árbol lateral adaptados al locale del sistema (`QStandardPaths`)
- Ventana única o con pestañas
- Diseño minimalista con barra de navegación superior

## Uso del nombre

El nombre "Stellar Explorer" no debe usarse para proyectos no oficiales sin autorización.  
Para distribuciones modificadas, usa un nombre diferente o añade un sufijo como "Stellar Explorer (modificado por...)".
