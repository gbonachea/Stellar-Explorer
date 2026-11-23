#include "utils.h"
#include <QProcess>
#include <QStandardPaths>
#include <QApplication>
#include <QIcon>
#include <QStyle>

namespace {

// Comprueba que el icono exista realmente y que se pueda obtener un pixmap.
// QIcon(":/ruta.png") puede construirse aunque el PNG no se decodifique; por eso
// no basta con isNull(), hay que forzar la lectura del pixmap.
bool iconUsable(const QIcon &icon)
{
    if (icon.isNull())
        return false;

    const QList<QSize> sizes = icon.availableSizes();
    if (sizes.isEmpty())
        return !icon.pixmap(QSize(24, 24)).isNull();

    for (const QSize &s : sizes) {
        if (!icon.pixmap(s).isNull())
            return true;
    }
    return false;
}

} // namespace

QIcon iconWithFallback(const QString &resourcePath,
                       const QString &themeName,
                       QStyle::StandardPixmap fallback)
{
    // 1) Recurso embebido en el binario (independiente del tema del sistema).
    if (!resourcePath.isEmpty()) {
        QIcon icon(resourcePath);
        if (iconUsable(icon))
            return icon;
    }

    // 2) Icono del tema del sistema (puede faltar en otra distro).
    if (!themeName.isEmpty()) {
        QIcon icon = QIcon::fromTheme(themeName);
        if (iconUsable(icon))
            return icon;
    }

    // 3) Icono estándar del estilo: Qt lo dibuja siempre, sin depender de temas
    //    ni de códecs de imagen externos.
    if (QApplication *app = qobject_cast<QApplication *>(QCoreApplication::instance()))
        return app->style()->standardIcon(fallback);

    return QIcon();
}

QString humanSize(qint64 bytes)
{
    const QStringList units = {"B", "KB", "MB", "GB", "TB"};
    double num = static_cast<double>(bytes);
    for (const QString &unit : units) {
        if (num < 1024.0)
            return QString::number(num, 'f', 1) + " " + unit;
        num /= 1024.0;
    }
    return QString::number(num, 'f', 1) + " PB";
}

void openWithDefault(const QString &path)
{
    QProcess::startDetached("xdg-open", {path});
}

void runExecutable(const QString &path)
{
    QProcess::startDetached(path, {});
}

void openTerminalHere(const QString &directory)
{
    // Buscar el emulador de terminal disponible en el sistema
    QStringList candidates = {"x-terminal-emulator", "gnome-terminal", "konsole", "xterm"};
    for (const QString &term : candidates) {
        QString found = QStandardPaths::findExecutable(term);
        if (!found.isEmpty()) {
            QProcess *p = new QProcess();
            p->setWorkingDirectory(directory);
            p->startDetached(found, {});
            return;
        }
    }
}
