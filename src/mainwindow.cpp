#include "mainwindow.h"
#include "configdialog.h"
#include "utils.h"

#include <QStyle>

#include <QToolBar>
#include <QStatusBar>
#include <QShortcut>
#include <QKeySequence>
#include <QSettings>
#include <QDir>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QWidget>
#include <QHBoxLayout>

MainWindow::MainWindow(const QString &startPath, QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Stellar Explorer");
    setWindowIcon(QIcon(":/icons/StellarExplorer.png"));

    // Restaurar geometría
    QSettings settings;
    QByteArray geometry = settings.value("window_geometry").toByteArray();
    if (!geometry.isEmpty())
        restoreGeometry(geometry);
    else
        resize(1100, 700);

    // Pestañas
    m_tabs = new QTabWidget(this);
    m_tabs->setTabsClosable(true);
    connect(m_tabs, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);
    setCentralWidget(m_tabs);

    // Barra de herramientas
    QToolBar *tb = addToolBar("Main");
    tb->setIconSize(QSize(20, 20));

    m_actBack    = tb->addAction(iconWithFallback(":/icons/atras.png",        "go-previous",        QStyle::SP_ArrowBack),     "Atrás");
    m_actUp      = tb->addAction(iconWithFallback(":/icons/arriba.png",       "go-up",              QStyle::SP_ArrowUp),       "Arriba");
    m_actForward = tb->addAction(iconWithFallback(":/icons/adelante.png",     "go-next",            QStyle::SP_ArrowForward),  "Adelante");
    tb->addSeparator();

    // Barra de dirección
    m_address = new QLineEdit(this);
    m_address->setPlaceholderText("Ruta… Enter para ir");
    connect(m_address, &QLineEdit::returnPressed, this, &MainWindow::goAddress);
    QWidget *addrw = new QWidget(this);
    QHBoxLayout *addrLayout = new QHBoxLayout(addrw);
    addrLayout->setContentsMargins(0, 0, 0, 0);
    addrLayout->addWidget(m_address);
    tb->addWidget(addrw);
    tb->addSeparator();

    m_actNewTab    = tb->addAction(iconWithFallback(":/icons/new_tab.png",    "tab-new",            QStyle::SP_FileIcon),            "Nueva pestaña");
    m_actNewFolder = tb->addAction(iconWithFallback(":/icons/new_folde.png",  "folder-new",         QStyle::SP_FileDialogNewFolder), "Carpeta nueva");
    m_actRename    = tb->addAction(iconWithFallback(":/icons/escribir.png",   "document-rename",    QStyle::SP_FileIcon),            "Renombrar");
    m_actDelete    = tb->addAction(iconWithFallback(":/icons/delete.png",     "edit-delete",        QStyle::SP_TrashIcon),           "Eliminar");
    tb->addSeparator();
    m_actCopy      = tb->addAction(iconWithFallback(":/icons/copiar.png",     "edit-copy",          QStyle::SP_FileIcon),            "Copiar");
    m_actCut       = tb->addAction(iconWithFallback(":/icons/cortar.png",     "edit-cut",           QStyle::SP_FileIcon),            "Cortar");
    m_actPaste     = tb->addAction(iconWithFallback(":/icons/pegar.png",      "edit-paste",         QStyle::SP_FileIcon),            "Pegar");
    tb->addSeparator();
    m_actRefresh   = tb->addAction(iconWithFallback(":/icons/refrescar.png",  "view-refresh",       QStyle::SP_BrowserReload),       "Refrescar");
    m_actConfig    = tb->addAction(iconWithFallback(":/icons/configuracion.png","preferences-system",QStyle::SP_FileDialogDetailedView), "Configuración");
    m_actConfig->setStatusTip("Abrir configuración");

    // Atajos de teclado
    new QShortcut(QKeySequence("Ctrl+T"), this, SLOT(newTabHere()));
    new QShortcut(QKeySequence("Ctrl+W"), this, [this](){ closeTab(m_tabs->currentIndex()); });
    new QShortcut(QKeySequence("Ctrl+C"), this, SLOT(onCopy()));
    new QShortcut(QKeySequence("Ctrl+X"), this, SLOT(onCut()));
    new QShortcut(QKeySequence("Ctrl+V"), this, SLOT(onPaste()));
    new QShortcut(QKeySequence("F5"),     this, SLOT(onRefresh()));
    new QShortcut(QKeySequence("Alt+Left"),  this, SLOT(onBack()));
    new QShortcut(QKeySequence("Alt+Right"), this, SLOT(onForward()));
    new QShortcut(QKeySequence("Alt+Up"),    this, SLOT(onUp()));
    new QShortcut(QKeySequence("Ctrl+Q"),    this, SLOT(close()));

    statusBar();

    // Conexiones barra de herramientas
    connect(m_actNewTab,    &QAction::triggered, this, &MainWindow::newTabHere);
    connect(m_actNewFolder, &QAction::triggered, this, &MainWindow::onNewFolder);
    connect(m_actRename,    &QAction::triggered, this, &MainWindow::onRename);
    connect(m_actDelete,    &QAction::triggered, this, &MainWindow::onDelete);
    connect(m_actCopy,      &QAction::triggered, this, &MainWindow::onCopy);
    connect(m_actCut,       &QAction::triggered, this, &MainWindow::onCut);
    connect(m_actPaste,     &QAction::triggered, this, &MainWindow::onPaste);
    connect(m_actRefresh,   &QAction::triggered, this, &MainWindow::onRefresh);
    connect(m_actUp,        &QAction::triggered, this, &MainWindow::onUp);
    connect(m_actBack,      &QAction::triggered, this, &MainWindow::onBack);
    connect(m_actForward,   &QAction::triggered, this, &MainWindow::onForward);
    connect(m_actConfig,    &QAction::triggered, this, &MainWindow::showConfig);

    // Pestaña inicial con la ruta recibida o home
    QString path = startPath.isEmpty() ? QDir::homePath() : startPath;
    newTab(path);

    connect(m_tabs, &QTabWidget::currentChanged,
            this, &MainWindow::updateNavigationButtons);
}

// -----------------------------------------------------------------------
FileTab *MainWindow::currentTab() const
{
    return qobject_cast<FileTab*>(m_tabs->currentWidget());
}

// -----------------------------------------------------------------------
// Pestañas
// -----------------------------------------------------------------------
void MainWindow::newTab(const QString &path)
{
    QString p = path.isEmpty() ? QDir::homePath() : path;
    FileTab *tab = new FileTab(p, this);
    connect(tab, &FileTab::statusMessage, this, [this](const QString &msg){ statusBar()->showMessage(msg); });
    connect(tab, &FileTab::requestOpenPath, this, &MainWindow::newTab);

    QString name = QDir(p).dirName();
    if (name.isEmpty()) name = p;
    int idx = m_tabs->addTab(tab, name);
    m_tabs->setCurrentIndex(idx);
    m_address->setText(tab->currentPath());
}

void MainWindow::newTabHere()
{
    if (FileTab *tab = currentTab())
        newTab(tab->currentPath());
}

void MainWindow::closeTab(int index)
{
    if (m_tabs->count() > 1)
        m_tabs->removeTab(index);
}

// -----------------------------------------------------------------------
// Acciones de la barra de herramientas
// -----------------------------------------------------------------------
void MainWindow::goAddress()
{
    QString text = m_address->text();
    QDir dir(QDir::cleanPath(text.startsWith('~')
        ? QDir::homePath() + text.mid(1) : text));

    FileTab *tab = currentTab();
    if (tab && dir.exists()) {
        tab->setDirectory(dir.absolutePath());
        QString name = dir.dirName().isEmpty() ? dir.absolutePath() : dir.dirName();
        m_tabs->setTabText(m_tabs->currentIndex(), name);
        updateNavigationButtons();
    } else {
        statusBar()->showMessage("Ruta inválida");
    }
}

void MainWindow::onNewFolder()  { if (auto t = currentTab()) t->newFolder(); }
void MainWindow::onRename()     { if (auto t = currentTab()) t->renameSelected(); }
void MainWindow::onDelete()     { if (auto t = currentTab()) t->deleteSelected(); }
void MainWindow::onCopy()       { if (auto t = currentTab()) t->copySelected(); }
void MainWindow::onCut()        { if (auto t = currentTab()) t->cutSelected(); }
void MainWindow::onPaste()      { if (auto t = currentTab()) t->pasteIntoCurrent(); }
void MainWindow::onRefresh()    { if (auto t = currentTab()) t->refresh(); }

void MainWindow::onUp()
{
    if (FileTab *tab = currentTab()) {
        tab->goUp();
        m_address->setText(tab->currentPath());
        QString name = QDir(tab->currentPath()).dirName();
        m_tabs->setTabText(m_tabs->currentIndex(), name.isEmpty() ? tab->currentPath() : name);
        updateNavigationButtons();
    }
}

void MainWindow::onBack()
{
    if (FileTab *tab = currentTab(); tab && tab->goBack()) {
        m_address->setText(tab->currentPath());
        QString name = QDir(tab->currentPath()).dirName();
        m_tabs->setTabText(m_tabs->currentIndex(), name.isEmpty() ? tab->currentPath() : name);
        updateNavigationButtons();
    }
}

void MainWindow::onForward()
{
    if (FileTab *tab = currentTab(); tab && tab->goForward()) {
        m_address->setText(tab->currentPath());
        QString name = QDir(tab->currentPath()).dirName();
        m_tabs->setTabText(m_tabs->currentIndex(), name.isEmpty() ? tab->currentPath() : name);
        updateNavigationButtons();
    }
}

void MainWindow::showConfig()
{
    ConfigDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        for (int i = 0; i < m_tabs->count(); ++i) {
            if (FileTab *tab = qobject_cast<FileTab*>(m_tabs->widget(i)))
                tab->applySettings();
        }
    }
}

void MainWindow::updateNavigationButtons()
{
    if (FileTab *tab = currentTab()) {
        // Acceder a historial mediante métodos públicos
        // Los botones se habilitan si hay historia hacia atrás/adelante.
        // FileTab expone esto indirectamente al intentar moverse.
        // Para simplificar, siempre los habilitamos y dejamos que el tab maneje
        // el caso sin historia (no hace nada silenciosamente).
        m_actBack->setEnabled(true);
        m_actForward->setEnabled(true);
    } else {
        m_actBack->setEnabled(false);
        m_actForward->setEnabled(false);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings;
    settings.setValue("window_geometry", saveGeometry());
    QMainWindow::closeEvent(event);
}
