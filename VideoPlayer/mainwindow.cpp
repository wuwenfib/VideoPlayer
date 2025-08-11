#include "mainwindow.h"
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_statusLabel(nullptr)
{
    setupUI();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();

    // 设置窗口属性
    setWindowTitle("Video Player - Ready");
    setMinimumSize(800, 600);
    resize(1200, 800);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    // 创建中央组件
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // 临时添加欢迎标签
    QLabel *welcomeLabel = new QLabel("🎬 Welcome to Video Player!", this);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setStyleSheet(
        "QLabel { "
        "   font-size: 24px; "
        "   font-weight: bold; "
        "   color: #2c3e50; "
        "   padding: 50px; "
        "}"
        );

    mainLayout->addWidget(welcomeLabel);
}

void MainWindow::setupMenuBar()
{
    // 文件菜单
    QMenu *fileMenu = menuBar()->addMenu("&File");
    fileMenu->addAction("&Open Video...", [this]() {
        m_statusLabel->setText("Open Video clicked");
    });
    fileMenu->addSeparator();
    fileMenu->addAction("&Exit", this, &QWidget::close);

    // 播放菜单
    QMenu *playMenu = menuBar()->addMenu("&Playback");
    playMenu->addAction("&Play/Pause");
    playMenu->addAction("&Stop");

    // 工具菜单
    QMenu *toolsMenu = menuBar()->addMenu("&Tools");
    toolsMenu->addAction("&Video Transcoder...");

    // 帮助菜单
    QMenu *helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction("&About");
}

void MainWindow::setupToolBar()
{
    QToolBar *mainToolBar = addToolBar("Main");
    mainToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    // 添加工具栏按钮（暂时使用文本，后续会替换为图标）
    mainToolBar->addAction("Open", [this]() {
        m_statusLabel->setText("Open clicked");
    });
    mainToolBar->addSeparator();
    mainToolBar->addAction("Play", [this]() {
        m_statusLabel->setText("Play clicked");
    });
    mainToolBar->addAction("Pause", [this]() {
        m_statusLabel->setText("Pause clicked");
    });
    mainToolBar->addAction("Stop", [this]() {
        m_statusLabel->setText("Stop clicked");
    });
}

void MainWindow::setupStatusBar()
{
    m_statusLabel = new QLabel("Ready", this);
    statusBar()->addWidget(m_statusLabel);

    // 添加右侧状态信息
    QLabel *versionLabel = new QLabel("v1.0.0", this);
    statusBar()->addPermanentWidget(versionLabel);
}
