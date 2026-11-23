#pragma once
#include <QString>
#include <QDir>
#include <QIcon>
#include <QStyle>

// Convierte bytes a representación legible (B, KB, MB, GB...)
QString humanSize(qint64 bytes);

// Abre un archivo/directorio con la aplicación predeterminada del sistema (xdg-open)
void openWithDefault(const QString &path);

// Ejecuta un archivo ejecutable
void runExecutable(const QString &path);

// Abre una terminal en el directorio dado
void openTerminalHere(const QString &directory);

// Devuelve un icono garantizado, intentando en orden:
//   1. El recurso embebido en el binario (resourcePath, p.ej. ":/icons/copiar.png")
//   2. El icono del tema del sistema (themeName, p.ej. "edit-copy")
//   3. Un icono estándar del estilo (fallback), que Qt siempre puede dibujar.
// Sirve para que la interfaz no se quede sin iconos en distros donde falte el
// tema del sistema o no se puedan decodificar los PNG en tiempo de ejecución.
QIcon iconWithFallback(const QString &resourcePath,
                       const QString &themeName = QString(),
                       QStyle::StandardPixmap fallback = QStyle::SP_FileIcon);
