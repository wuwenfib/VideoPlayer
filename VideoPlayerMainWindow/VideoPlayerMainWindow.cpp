// VideoPlayerMainWindow.cpp
#include "VideoPlayerMainWindow.h"
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

VideoPlayerMainWindow::VideoPlayerMainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    connectSignals();
    updateButtonStates();
}

VideoPlayerMainWindow::~VideoPlayerMainWindow()
{
}

void VideoPlayerMainWindow::setupUI()
{
    setWindowTitle("Qt6视频播放器 - 界面设计版本");
    setMinimumSize(1000, 700);
    resize(1200, 800);

    // 设置窗口图标
    // setWindowIcon(QIcon(":/icons/player.png"));

    setupMenus();
    setupToolBar();
    setupStatusBar();
    setupCentralWidget();
}

void VideoPlayerMainWindow::setupMenus()
{
    // 文件菜单
    QMenu *fileMenu = menuBar()->addMenu("文件(&F)");
    QAction *openAction = fileMenu->addAction("打开文件(&O)");
    openAction->setShortcut(QKeySequence::Open);
    fileMenu->addSeparator();
    QAction *exitAction = fileMenu->addAction("退出(&X)");
    exitAction->setShortcut(QKeySequence::Quit);

    // 播放菜单
    QMenu *playMenu = menuBar()->addMenu("播放(&P)");
    playMenu->addAction("播放/暂停(&P)");
    playMenu->addAction("停止(&S)");
    playMenu->addSeparator();
    playMenu->addAction("上一个(&R)");
    playMenu->addAction("下一个(&N)");

    // 视图菜单
    QMenu *viewMenu = menuBar()->addMenu("视图(&V)");
    viewMenu->addAction("全屏(&F)");
    viewMenu->addAction("显示播放列表(&L)");

    // 帮助菜单
    QMenu *helpMenu = menuBar()->addMenu("帮助(&H)");
    helpMenu->addAction("关于(&A)");
}

void VideoPlayerMainWindow::setupToolBar()
{
    toolBar = addToolBar("主工具栏");
    toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    // 添加工具栏按钮
    QAction *openAction = toolBar->addAction("打开");
    // openAction->setIcon(QIcon(":/icons/open.png"));

    toolBar->addSeparator();

    QAction *playAction = toolBar->addAction("播放");
    QAction *pauseAction = toolBar->addAction("暂停");
    QAction *stopAction = toolBar->addAction("停止");

    toolBar->addSeparator();

    // 音量控制工具栏
    toolBar->addWidget(new QLabel("音量:"));
    QSlider *toolbarVolumeSlider = new QSlider(Qt::Horizontal);
    toolbarVolumeSlider->setMaximumWidth(100);
    toolbarVolumeSlider->setRange(0, 100);
    toolbarVolumeSlider->setValue(50);
    toolBar->addWidget(toolbarVolumeSlider);
}

void VideoPlayerMainWindow::setupStatusBar()
{
    statusLabel = new QLabel("就绪");
    statusBar()->addWidget(statusLabel);

    // 添加进度条到状态栏
    loadingProgress = new QProgressBar();
    loadingProgress->setVisible(false);
    loadingProgress->setMaximumWidth(200);
    statusBar()->addPermanentWidget(loadingProgress);

    // 添加时间信息
    QLabel *timeLabel = new QLabel("00:00 / 00:00");
    statusBar()->addPermanentWidget(timeLabel);
}

void VideoPlayerMainWindow::setupCentralWidget()
{
    centralWidget = new QWidget();
    setCentralWidget(centralWidget);

    // 创建主分割器
    mainSplitter = new QSplitter(Qt::Horizontal);

    // 设置中央布局
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->addWidget(mainSplitter);

    // 设置视频显示区域和控制面板
    QWidget *leftWidget = new QWidget();
    QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);

    // 创建视频显示区域
    videoFrame = new QFrame();
    videoFrame->setFrameStyle(QFrame::StyledPanel);
    videoFrame->setStyleSheet("QFrame { background-color: black; }");
    videoFrame->setMinimumSize(640, 480);

    QVBoxLayout *videoLayout = new QVBoxLayout(videoFrame);
    videoLabel = new QLabel("视频显示区域");
    videoLabel->setAlignment(Qt::AlignCenter);
    videoLabel->setStyleSheet("QLabel { color: white; font-size: 18px; }");
    videoLayout->addWidget(videoLabel);

    leftLayout->addWidget(videoFrame);

    // 设置控制面板
    setupControlPanel();
    leftLayout->addWidget(controlPanel);

    // 设置播放列表
    setupPlaylist();

    // 添加到分割器
    mainSplitter->addWidget(leftWidget);
    mainSplitter->addWidget(playlistWidget);

    // 设置分割器比例
    mainSplitter->setStretchFactor(0, 3);  // 左侧占3/4
    mainSplitter->setStretchFactor(1, 1);  // 右侧占1/4
}

void VideoPlayerMainWindow::setupControlPanel()
{
    controlPanel = new QWidget();
    controlPanel->setMaximumHeight(150);

    QHBoxLayout *controlLayout = new QHBoxLayout(controlPanel);

    // 播放控制组
    playControlGroup = new QGroupBox("播放控制");
    QHBoxLayout *playLayout = new QHBoxLayout(playControlGroup);

    openFileButton = new QPushButton("打开文件");
    previousButton = new QPushButton("上一个");
    playButton = new QPushButton("播放");
    pauseButton = new QPushButton("暂停");
    stopButton = new QPushButton("停止");
    nextButton = new QPushButton("下一个");

    playLayout->addWidget(openFileButton);
    playLayout->addWidget(previousButton);
    playLayout->addWidget(playButton);
    playLayout->addWidget(pauseButton);
    playLayout->addWidget(stopButton);
    playLayout->addWidget(nextButton);

    // 音量控制组
    volumeControlGroup = new QGroupBox("音量控制");
    QHBoxLayout *volumeLayout = new QHBoxLayout(volumeControlGroup);

    muteButton = new QPushButton("静音");
    volumeSlider = new QSlider(Qt::Horizontal);
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(50);
    volumeLabel = new QLabel("50%");

    volumeLayout->addWidget(muteButton);
    volumeLayout->addWidget(volumeSlider);
    volumeLayout->addWidget(volumeLabel);

    // 播放速度控制组
    speedControlGroup = new QGroupBox("播放速度");
    QHBoxLayout *speedLayout = new QHBoxLayout(speedControlGroup);

    speedSlider = new QSlider(Qt::Horizontal);
    speedSlider->setRange(25, 300);  // 0.25x to 3.0x
    speedSlider->setValue(100);      // 1.0x normal speed
    speedLabel = new QLabel("1.0x");

    speedLayout->addWidget(new QLabel("0.25x"));
    speedLayout->addWidget(speedSlider);
    speedLayout->addWidget(new QLabel("3.0x"));
    speedLayout->addWidget(speedLabel);

    // 进度控制组
    progressGroup = new QGroupBox("播放进度");
    QVBoxLayout *progressLayout = new QVBoxLayout(progressGroup);

    positionSlider = new QSlider(Qt::Horizontal);
    positionSlider->setRange(0, 100);

    QHBoxLayout *timeLayout = new QHBoxLayout();
    currentTimeLabel = new QLabel("00:00");
    totalTimeLabel = new QLabel("00:00");
    timeLayout->addWidget(currentTimeLabel);
    timeLayout->addStretch();
    timeLayout->addWidget(totalTimeLabel);

    progressLayout->addWidget(positionSlider);
    progressLayout->addLayout(timeLayout);

    // 添加到控制布局
    controlLayout->addWidget(playControlGroup);
    controlLayout->addWidget(volumeControlGroup);
    controlLayout->addWidget(speedControlGroup);
    controlLayout->addWidget(progressGroup, 1);  // 进度条占更多空间
}

void VideoPlayerMainWindow::setupPlaylist()
{
    playlistWidget = new QListWidget();
    playlistWidget->setMaximumWidth(250);

    // 添加示例播放列表项目
    playlistWidget->addItem("示例视频1.mp4");
    playlistWidget->addItem("示例视频2.avi");
    playlistWidget->addItem("示例视频3.mkv");
    playlistWidget->addItem("示例视频4.mov");

    // 设置播放列表样式
    playlistWidget->setAlternatingRowColors(true);
}

void VideoPlayerMainWindow::connectSignals()
{
    // 连接播放控制按钮
    connect(openFileButton, &QPushButton::clicked, this, &VideoPlayerMainWindow::onOpenFile);
    connect(playButton, &QPushButton::clicked, this, &VideoPlayerMainWindow::onPlay);
    connect(pauseButton, &QPushButton::clicked, this, &VideoPlayerMainWindow::onPause);
    connect(stopButton, &QPushButton::clicked, this, &VideoPlayerMainWindow::onStop);
    connect(previousButton, &QPushButton::clicked, this, &VideoPlayerMainWindow::onPrevious);
    connect(nextButton, &QPushButton::clicked, this, &VideoPlayerMainWindow::onNext);

    // 连接滑块控件
    connect(volumeSlider, &QSlider::valueChanged, this, &VideoPlayerMainWindow::onVolumeChanged);
    connect(positionSlider, &QSlider::valueChanged, this, &VideoPlayerMainWindow::onPositionChanged);
    connect(speedSlider, &QSlider::valueChanged, this, &VideoPlayerMainWindow::onSpeedChanged);

    // 连接播放列表
    connect(playlistWidget, &QListWidget::itemDoubleClicked, [this](QListWidgetItem *item){
        statusLabel->setText(QString("准备播放: %1").arg(item->text()));
    });
}

void VideoPlayerMainWindow::updateButtonStates()
{
    // 初始状态：播放按钮可用，暂停和停止按钮不可用
    playButton->setEnabled(true);
    pauseButton->setEnabled(false);
    stopButton->setEnabled(false);
}

// 槽函数实现
void VideoPlayerMainWindow::onOpenFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "打开视频文件", "",
                                                    "视频文件 (*.mp4 *.avi *.mkv *.mov *.wmv *.flv);;所有文件 (*.*)");

    if (!fileName.isEmpty()) {
        statusLabel->setText(QString("已选择文件: %1").arg(QFileInfo(fileName).fileName()));
        videoLabel->setText(QString("准备播放:\n%1").arg(QFileInfo(fileName).fileName()));

        // 添加到播放列表
        playlistWidget->addItem(QFileInfo(fileName).fileName());
    }
}

void VideoPlayerMainWindow::onPlay()
{
    statusLabel->setText("正在播放...");
    playButton->setEnabled(false);
    pauseButton->setEnabled(true);
    stopButton->setEnabled(true);

    qDebug() << "播放按钮被点击";
}

void VideoPlayerMainWindow::onPause()
{
    statusLabel->setText("暂停");
    playButton->setEnabled(true);
    pauseButton->setEnabled(false);

    qDebug() << "暂停按钮被点击";
}

void VideoPlayerMainWindow::onStop()
{
    statusLabel->setText("停止");
    playButton->setEnabled(true);
    pauseButton->setEnabled(false);
    stopButton->setEnabled(false);
    positionSlider->setValue(0);
    currentTimeLabel->setText("00:00");

    qDebug() << "停止按钮被点击";
}

void VideoPlayerMainWindow::onPrevious()
{
    int currentRow = playlistWidget->currentRow();
    if (currentRow > 0) {
        playlistWidget->setCurrentRow(currentRow - 1);
        statusLabel->setText("切换到上一个");
    }
    qDebug() << "上一个按钮被点击";
}

void VideoPlayerMainWindow::onNext()
{
    int currentRow = playlistWidget->currentRow();
    if (currentRow < playlistWidget->count() - 1) {
        playlistWidget->setCurrentRow(currentRow + 1);
        statusLabel->setText("切换到下一个");
    }
    qDebug() << "下一个按钮被点击";
}

void VideoPlayerMainWindow::onVolumeChanged(int value)
{
    volumeLabel->setText(QString("%1%").arg(value));
    qDebug() << "音量调节到:" << value << "%";
}

void VideoPlayerMainWindow::onPositionChanged(int value)
{
    // 模拟时间更新
    int totalSeconds = 180;  // 假设总时长3分钟
    int currentSeconds = (totalSeconds * value) / 100;

    int currentMin = currentSeconds / 60;
    int currentSec = currentSeconds % 60;

    currentTimeLabel->setText(QString("%1:%2")
                                  .arg(currentMin, 2, 10, QLatin1Char('0'))
                                  .arg(currentSec, 2, 10, QLatin1Char('0')));

    qDebug() << "播放位置:" << value << "%";
}

void VideoPlayerMainWindow::onSpeedChanged(int value)
{
    double speed = value / 100.0;
    speedLabel->setText(QString("%1x").arg(speed, 0, 'f', 2));
    qDebug() << "播放速度:" << speed << "x";
}
