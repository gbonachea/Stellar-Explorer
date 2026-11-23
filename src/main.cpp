#include <QApplication>
#include <QSettings>
#include <QFile>
#include <QDir>
#include <QIcon>
#include <QTextStream>
#include <QProcess>
#include "mainwindow.h"
#include "configdialog.h"

static const QString APP_NAME    = "Stellar Explorer";
static const QString ORG_NAME    = "StellarExplorer";
static const QString ORG_DOMAIN  = "stellarexplorer.local";

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(APP_NAME);
    app.setOrganizationName(ORG_NAME);
    app.setOrganizationDomain(ORG_DOMAIN);
    app.setWindowIcon(QIcon(":/icons/StellarExplorer.png"));

    // Forzar tema de iconos del sistema para que QFileSystemModel/QFileIconProvider
    // usen el tema correcto incluso en portables o si el platform theme GTK no carga.
    auto detectIconTheme = []() -> QString {
        auto tryGSettings = [](const char *schema) -> QString {
            QProcess p;
            p.start("gsettings", {"get", schema, "icon-theme"});
            if (p.waitForStarted() && p.waitForFinished(2000)) {
                QString out = QString::fromUtf8(p.readAllStandardOutput().trimmed());
                out.remove('"').remove("'");
                if (!out.isEmpty())
                    return out;
            }
            return QString();
        };
        QString t = tryGSettings("org.cinnamon.desktop.interface");
        if (!t.isEmpty()) return t;
        t = tryGSettings("org.gnome.desktop.interface");
        if (!t.isEmpty()) return t;
        {
            QFile f(QDir::homePath() + "/.config/gtk-3.0/settings.ini");
            if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&f);
                while (!in.atEnd()) {
                    QString line = in.readLine().trimmed();
                    if (line.startsWith("gtk-icon-theme-name")) {
                        line = line.mid(line.indexOf('=') + 1).trimmed();
                        line.remove('"').remove("'");
                        return line;
                    }
                }
            }
        }
        {
            QFile f(QDir::homePath() + "/.config/gtk-4.0/settings.ini");
            if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&f);
                while (!in.atEnd()) {
                    QString line = in.readLine().trimmed();
                    if (line.startsWith("gtk-icon-theme-name")) {
                        line = line.mid(line.indexOf('=') + 1).trimmed();
                        line.remove('"').remove("'");
                        return line;
                    }
                }
            }
        }
        if (!QIcon::themeName().isEmpty()) return QIcon::themeName();
        return "Adwaita";
    };

    {
        const QString t = detectIconTheme();
        if (!t.isEmpty()) {
            QIcon::setThemeName(t);
            QIcon::setFallbackThemeName("Adwaita");
        }
    }

    // Cargar themes.css
    QString cssPath = QDir(QCoreApplication::applicationDirPath()).filePath("themes.css");
    QFile cssFile(cssPath);
    if (cssFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        app.setStyleSheet(QString::fromUtf8(cssFile.readAll()));
        cssFile.close();
    }

    // Aplicar tema guardado en la configuración
    QSettings settings;
    QString theme = settings.value("theme", "Sistema").toString();

    ConfigDialog configHelper;   // solo para usar getDarkPalette / getLightPalette

    auto isDarkSystem = [&]() -> bool {
        return QApplication::palette().window().color().lightness() < 128;
    };

    if (theme == "Oscuro" || (theme == "Sistema" && isDarkSystem())) {
        app.setProperty("theme", "dark");
        app.setPalette(configHelper.getDarkPalette());
    } else {
        app.setProperty("theme", "light");
        app.setPalette(configHelper.getLightPalette());
    }

    // Manejar argumentos de línea de comandos
    QString startPath = QDir::homePath();
    for (int i = 1; i < argc; ++i) {
        QString arg = QString::fromLocal8Bit(argv[i]);
        if (arg == "--help" || arg == "-h") {
            printf("Uso: StellarExplorer [ruta]\nAbre el gestor de archivos Stellar Explorer.\n");
            return 0;
        }
        if (!arg.startsWith('-'))
            startPath = arg;
    }

    MainWindow w(startPath);
    w.show();

    return app.exec();
}
