// AdvancedVideoPlayer.cpp
#include "AdvancedVideoPlayer.h"
#include <QStandardPaths>
#include <QDir>
#include <QSizePolicy>

AdvancedVideoPlayer::AdvancedVideoPlayer(QWidget *parent)
    : QMainWindow(parent)
    , m_mediaPlayer(nullptr)
    , m_videoWidget(nullptr)
    , m_audioOutput(nullptr)
    , m_isFullScreen(false)
    , m_playlistVisible(true)
    , m_isMuted(false)
    , m_sliderPressed(false)
    , m_volume(50)
{
    setWindowTitle("Qt6高级视频播放器");
    setMinimumSize(1000, 700);
    resize(1200, 800);

    setupUI();
    setupMediaPlayer();
    setupMenus();
    setupStatusBar();
    setupConnections();
    applyStyles();

    // 创建定时器
    m_progressTimer = new QTimer(this);
    connect(m_progressTimer, &QTimer::timeout, this, &AdvancedVideoPlayer::updateProgress);
    m_progressTimer->start(100);

    m_fullscreenHideTimer = new QTimer(this);
    m_fullscreenHideTimer->setSingleShot(true);
    connect(m_fullscreenHideTimer, &QTimer::timeout, this, &AdvancedVideoPlayer::hideControlsInFullscreen);

    loadSettings();
    updateButtonStates();
}

AdvancedVideoPlayer::~AdvancedVideoPlayer()
{
    saveSettings();
}

void AdvancedVideoPlayer::setupUI()
{
    m_centralWidget = new QWidget();
    setCentralWidget(m_centralWidget);

    // 创建主分割器
    m_mainSplitter = new QSplitter(Qt::Horizontal);

    // 创建视频容器
    m_videoContainer = new QWidget();
    QVBoxLayout *videoLayout = new QVBoxLayout(m_videoContainer);
    videoLayout->setContentsMargins(0, 0, 0, 0);

    // 创建视频组件
    m_videoWidget = new QVideoWidget();
    m_videoWidget->setAspectRatioMode(Qt::KeepAspectRatio);
    m_videoWidget->setStyleSheet("background-color: black;");

    // 媒体信息标签
    m_mediaInfoLabel = new QLabel();
    m_mediaInfoLabel->setAlignment(Qt::AlignCenter);
    m_mediaInfoLabel->setStyleSheet("color: white; background: rgba(0,0,0,0.7); padding: 10px;");
    m_mediaInfoLabel->hide();

    videoLayout->addWidget(m_videoWidget);

    // 创建控制面板
    m_controlPanel = new QWidget();
    m_controlPanel->setMaximumHeight(120);
    setupControlPanel();

    videoLayout->addWidget(m_controlPanel);

    // 创建播放列表
    m_playlistWidget = new PlaylistWidget();
    m_playlistWidget->setMaximumWidth(300);

    // 添加到分割器
    m_mainSplitter->addWidget(m_videoContainer);
    m_mainSplitter->addWidget(m_playlistWidget);
    m_mainSplitter->setStretchFactor(0, 3);
    m_mainSplitter->setStretchFactor(1, 1);

    // 设置主布局
    QHBoxLayout *mainLayout = new QHBoxLayout(m_centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_mainSplitter);
}

void AdvancedVideoPlayer::setupControlPanel()
{
    QVBoxLayout *controlLayout = new QVBoxLayout(m_controlPanel);

    // 进度控制行
    QHBoxLayout *progressLayout = new QHBoxLayout();

    m_currentTimeLabel = new QLabel("00:00");
    m_currentTimeLabel->setMinimumWidth(50);

    m_positionSlider = new QSlider(Qt::Horizontal);
    m_positionSlider->setRange(0, 0);

    m_totalTimeLabel = new QLabel("00:00");
    m_totalTimeLabel->setMinimumWidth(50);

    progressLayout->addWidget(m_currentTimeLabel);
    progressLayout->addWidget(m_positionSlider, 1);
    progressLayout->addWidget(m_totalTimeLabel);

    // 控制按钮行
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    m_openButton = new QPushButton("打开");
    m_previousButton = new QPushButton("⏮");
    m_playButton = new QPushButton("▶");
    m_pauseButton = new QPushButton("⏸");
    m_stopButton = new QPushButton("⏹");
    m_nextButton = new QPushButton("⏭");

    // 设置按钮大小
    QSize buttonSize(40, 32);
    m_previousButton->setMaximumSize(buttonSize);
    m_playButton->setMaximumSize(buttonSize);
    m_pauseButton->setMaximumSize(buttonSize);
    m_stopButton->setMaximumSize(buttonSize);
    m_nextButton->setMaximumSize(buttonSize);

    buttonLayout->addWidget(m_openButton);
    buttonLayout->addSpacing(10);
    buttonLayout->addWidget(m_previousButton);
    buttonLayout->addWidget(m_playButton);
    buttonLayout->addWidget(m_pauseButton);
    buttonLayout->addWidget(m_stopButton);
    buttonLayout->addWidget(m_nextButton);
    buttonLayout->addStretch();

    // 音量和其他控制
    m_muteButton = new QPushButton("🔊");
    m_muteButton->setMaximumSize(30, 30);

    m_volumeSlider = new QSlider(Qt::Horizontal);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(m_volume);
    m_volumeSlider->setMaximumWidth(100);

    m_volumeLabel = new QLabel("50%");
    m_volumeLabel->setMinimumWidth(35);

    QLabel *speedLabel = new QLabel("速度:");
    m_speedComboBox = new QComboBox();
    m_speedComboBox->addItems(QStringList() << "0.5x" << "0.75x" << "1.0x" << "1.25x" << "1.5x" << "2.0x");
    m_speedComboBox->setCurrentText("1.0x");
    m_speedComboBox->setMaximumWidth(70);

    m_fullscreenButton = new QPushButton("全屏");
    m_playlistButton = new QPushButton("列表");

    buttonLayout->addWidget(m_muteButton);
    buttonLayout->addWidget(m_volumeSlider);
    buttonLayout->addWidget(m_volumeLabel);
    buttonLayout->addSpacing(10);
    buttonLayout->addWidget(speedLabel);
    buttonLayout->addWidget(m_speedComboBox);
    buttonLayout->addSpacing(10);
    buttonLayout->addWidget(m_fullscreenButton);
    buttonLayout->addWidget(m_playlistButton);

    // 缓冲进度条
    m_bufferProgress = new QProgressBar();
    m_bufferProgress->setMaximumHeight(3);
    m_bufferProgress->setTextVisible(false);
    m_bufferProgress->hide();

    controlLayout->addWidget(m_bufferProgress);
    controlLayout->addLayout(progressLayout);
    controlLayout->addLayout(buttonLayout);
}

void AdvancedVideoPlayer::setupMediaPlayer()
{
    m_mediaPlayer = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);

    m_audioOutput->setVolume(m_volume / 100.0);
    m_mediaPlayer->setAudioOutput(m_audioOutput);
    m_mediaPlayer->setVideoOutput(m_videoWidget);
}

void AdvancedVideoPlayer::setupMenus()
{
    // 文件菜单
    QMenu *fileMenu = menuBar()->addMenu("文件(&F)");

    fileMenu->addAction("打开文件(&O)", this, &AdvancedVideoPlayer::openFile, QKeySequence::Open);
    fileMenu->addAction("打开文件夹(&D)", this, &AdvancedVideoPlayer::openFolder, QKeySequence("Ctrl+D"));
    fileMenu->addSeparator();
    fileMenu->addAction("导入播放列表(&I)", this, &AdvancedVideoPlayer::importPlaylist);
    fileMenu->addAction("导出播放列表(&E)", this, &AdvancedVideoPlayer::exportPlaylist);
    fileMenu->addSeparator();
    fileMenu->addAction("退出(&X)", this, &QWidget::close, QKeySequence::Quit);

    // 播放菜单
    QMenu *playMenu = menuBar()->addMenu("播放(&P)");
    playMenu->addAction("播放/暂停(&P)", this, [this]() {
        if (m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
            pause();
        } else {
            play();
        }
    }, QKeySequence(Qt::Key_Space));
    playMenu->addAction("停止(&S)", this, &AdvancedVideoPlayer::stop, QKeySequence("S"));
    playMenu->addSeparator();
    playMenu->addAction("上一个(&R)", this, &AdvancedVideoPlayer::previous, QKeySequence("Ctrl+Left"));
    playMenu->addAction("下一个(&N)", this, &AdvancedVideoPlayer::next, QKeySequence("Ctrl+Right"));

    // 视图菜单
    QMenu *viewMenu = menuBar()->addMenu("视图(&V)");
    QAction *fullscreenAction = viewMenu->addAction("全屏(&F)", this, &AdvancedVideoPlayer::onFullScreenToggled, QKeySequence("F11"));
    fullscreenAction->setCheckable(true);

    QAction *playlistAction = viewMenu->addAction("显示播放列表(&L)", this, &AdvancedVideoPlayer::onPlaylistToggled, QKeySequence("L"));
    playlistAction->setCheckable(true);
    playlistAction->setChecked(m_playlistVisible);

    // 帮助菜单
    QMenu *helpMenu = menuBar()->addMenu("帮助(&H)");
    helpMenu->addAction("关于(&A)", this, &AdvancedVideoPlayer::showAbout);
}

void AdvancedVideoPlayer::setupStatusBar()
{
    statusBar()->showMessage("就绪");
}

void AdvancedVideoPlayer::setupConnections()
{
    // 媒体播放器连接
    connect(m_mediaPlayer, &QMediaPlayer::playbackStateChanged, this, &AdvancedVideoPlayer::onMediaStateChanged);
    connect(m_mediaPlayer, &QMediaPlayer::mediaStatusChanged, this, &AdvancedVideoPlayer::onMediaStatusChanged);
    connect(m_mediaPlayer, &QMediaPlayer::positionChanged, this, &AdvancedVideoPlayer::onPositionChanged);
    connect(m_mediaPlayer, &QMediaPlayer::durationChanged, this, &AdvancedVideoPlayer::onDurationChanged);
    connect(m_mediaPlayer, &QMediaPlayer::errorOccurred, this, &AdvancedVideoPlayer::onMediaError);

    // 控制按钮连接
    connect(m_openButton, &QPushButton::clicked, this, &AdvancedVideoPlayer::openFile);
    connect(m_playButton, &QPushButton::clicked, this, &AdvancedVideoPlayer::play);
    connect(m_pauseButton, &QPushButton::clicked, this, &AdvancedVideoPlayer::pause);
    connect(m_stopButton, &QPushButton::clicked, this, &AdvancedVideoPlayer::stop);
    connect(m_previousButton, &QPushButton::clicked, this, &AdvancedVideoPlayer::previous);
    connect(m_nextButton, &QPushButton::clicked, this, &AdvancedVideoPlayer::next);
    connect(m_fullscreenButton, &QPushButton::clicked, this, &AdvancedVideoPlayer::onFullScreenToggled);
    connect(m_playlistButton, &QPushButton::clicked, this, &AdvancedVideoPlayer::onPlaylistToggled);
    connect(m_muteButton, &QPushButton::clicked, this, &AdvancedVideoPlayer::onMuteToggled);

    // 滑块连接
    connect(m_positionSlider, &QSlider::sliderPressed, this, &AdvancedVideoPlayer::onPositionSliderPressed);
    connect(m_positionSlider, &QSlider::sliderReleased, this, &AdvancedVideoPlayer::onPositionSliderReleased);
    connect(m_positionSlider, &QSlider::sliderMoved, this, &AdvancedVideoPlayer::onPositionSliderMoved);
    connect(m_volumeSlider, &QSlider::valueChanged, this, &AdvancedVideoPlayer::onVolumeChanged);

    // 速度控制连接
    connect(m_speedComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AdvancedVideoPlayer::onSpeedChanged);

    // 播放列表连接
    connect(m_playlistWidget, &PlaylistWidget::mediaSelected, this, &AdvancedVideoPlayer::onMediaSelected);
    connect(m_playlistWidget, &PlaylistWidget::playModeChanged, this, &AdvancedVideoPlayer::onPlayModeChanged);
    connect(m_playlistWidget, &PlaylistWidget::playlistChanged, this, &AdvancedVideoPlayer::onPlaylistChanged);
    connect(m_playlistWidget, &PlaylistWidget::requestPlay, this, &AdvancedVideoPlayer::onPlayRequested);
    connect(m_playlistWidget, &PlaylistWidget::requestNext, this, &AdvancedVideoPlayer::onNextRequested);
    connect(m_playlistWidget, &PlaylistWidget::requestPrevious, this, &AdvancedVideoPlayer::onPreviousRequested);

    // 快捷键管理器
    m_shortcutManager = new ShortcutManager(this);
    connect(m_shortcutManager, &ShortcutManager::actionTriggered, this, &AdvancedVideoPlayer::onShortcutTriggered);
}

void AdvancedVideoPlayer::applyStyles()
{
    setStyleSheet(
        "QMainWindow {"
        "    background-color: #2b2b2b;"
        "    color: #ffffff;"
        "}"
        "QWidget {"
        "    background-color: #2b2b2b;"
        "    color: #ffffff;"
        "}"
        "QPushButton {"
        "    background-color: #404040;"
        "    border: 1px solid #606060;"
        "    border-radius: 5px;"
        "    padding: 5px 10px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #4a4a4a;"
        "    border-color: #707070;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #303030;"
        "}"
        "QSlider::groove:horizontal {"
        "    border: 1px solid #666666;"
        "    height: 8px;"
        "    background: #404040;"
        "    border-radius: 4px;"
        "}"
        "QSlider::handle:horizontal {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #0078d4, stop:1 #005a9e);"
        "    border: 1px solid #005a9e;"
        "    width: 18px;"
        "    margin: -5px 0;"
        "    border-radius: 9px;"
        "}"
        "QComboBox {"
        "    background-color: #404040;"
        "    border: 1px solid #606060;"
        "    border-radius: 3px;"
        "    padding: 3px;"
        "}"
        "QProgressBar {"
        "    border: none;"
        "    background-color: #404040;"
        "    border-radius: 1px;"
        "}"
        "QProgressBar::chunk {"
        "    background-color: #0078d4;"
        "    border-radius: 1px;"
        "}"
        );
}

// 播放控制实现
void AdvancedVideoPlayer::play()
{
    if (m_mediaPlayer->source().isEmpty()) {
        if (m_playlistWidget->getMediaCount() > 0) {
            m_playlistWidget->setCurrentIndex(0);
            return;
        } else {
            openFile();
            return;
        }
    }

    m_mediaPlayer->play();

    if (m_isFullScreen) {
        m_fullscreenHideTimer->start(3000); // 3秒后隐藏控制面板
    }
}

void AdvancedVideoPlayer::pause()
{
    m_mediaPlayer->pause();
    m_fullscreenHideTimer->stop();
}

void AdvancedVideoPlayer::stop()
{
    m_mediaPlayer->stop();
    m_fullscreenHideTimer->stop();
}

void AdvancedVideoPlayer::openFile()
{
    QString lastDir = QSettings().value("lastOpenDir",
                                        QStandardPaths::writableLocation(QStandardPaths::MoviesLocation)).toString();

    const QStringList formats = m_playlistWidget->getSupportedFormats();
    QStringList filterList;
    for (const QString &format : formats) {
        filterList << QString("*.%1").arg(format);
    }
    QString filter = QString("媒体文件 (%1);;所有文件 (*.*)").arg(filterList.join(" "));

    QStringList fileNames = QFileDialog::getOpenFileNames(this, "选择媒体文件", lastDir, filter);

    if (!fileNames.isEmpty()) {
        QSettings().setValue("lastOpenDir", QFileInfo(fileNames.first()).absolutePath());
        m_playlistWidget->addMediaList(fileNames);

        if (m_playlistWidget->currentIndex() == -1) {
            m_playlistWidget->setCurrentIndex(0);
        }

        showNotification(QString("已添加 %1 个文件").arg(fileNames.size()));
    }
}

void AdvancedVideoPlayer::openFolder()
{
    QString lastDir = QSettings().value("lastOpenDir",
                                        QStandardPaths::writableLocation(QStandardPaths::MoviesLocation)).toString();

    QString folderPath = QFileDialog::getExistingDirectory(this, "选择文件夹", lastDir);

    if (!folderPath.isEmpty()) {
        QSettings().setValue("lastOpenDir", folderPath);

        QDir directory(folderPath);
        QStringList formats = m_playlistWidget->getSupportedFormats();
        QStringList filters;
        for (const QString &format : formats) {
            filters << QString("*.%1").arg(format);
        }

        QFileInfoList files = directory.entryInfoList(filters, QDir::Files, QDir::Name);

        QStringList filePaths;
        for (const QFileInfo &fileInfo : files) {
            filePaths.append(fileInfo.absoluteFilePath());
        }

        if (!filePaths.isEmpty()) {
            m_playlistWidget->addMediaList(filePaths);
            if (m_playlistWidget->currentIndex() == -1) {
                m_playlistWidget->setCurrentIndex(0);
            }
            showNotification(QString("从文件夹添加了 %1 个文件").arg(filePaths.size()));
        } else {
            showNotification("文件夹中未找到支持的媒体文件");
        }
    }
}

void AdvancedVideoPlayer::previous()
{
    int prevIndex = m_playlistWidget->getPreviousIndex();
    if (prevIndex >= 0) {
        m_playlistWidget->setCurrentIndex(prevIndex);
    }
}

void AdvancedVideoPlayer::next()
{
    int nextIndex = m_playlistWidget->getNextIndex();
    if (nextIndex >= 0) {
        m_playlistWidget->setCurrentIndex(nextIndex);
    } else {
        // 如果没有下一个，停止播放
        stop();
    }
}

// 媒体事件处理
void AdvancedVideoPlayer::onMediaStateChanged(QMediaPlayer::PlaybackState state)
{
    updateButtonStates();

    switch (state) {
    case QMediaPlayer::PlayingState:
        statusBar()->showMessage("正在播放");
        break;
    case QMediaPlayer::PausedState:
        statusBar()->showMessage("暂停");
        break;
    case QMediaPlayer::StoppedState:
        statusBar()->showMessage("停止");
        break;
    }
}

void AdvancedVideoPlayer::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    switch (status) {
    case QMediaPlayer::LoadingMedia:
        statusBar()->showMessage("加载中...");
        m_bufferProgress->show();
        break;
    case QMediaPlayer::LoadedMedia:
        statusBar()->showMessage("媒体已加载");
        m_bufferProgress->hide();
        updateMediaInfo();
        break;
    case QMediaPlayer::BufferingMedia:
        m_bufferProgress->show();
        break;
    case QMediaPlayer::BufferedMedia:
        m_bufferProgress->hide();
        break;
    case QMediaPlayer::EndOfMedia:
        next(); // 自动播放下一个
        break;
    case QMediaPlayer::InvalidMedia:
        statusBar()->showMessage("无效媒体文件");
        m_bufferProgress->hide();
        break;
    default:
        break;
    }
}

void AdvancedVideoPlayer::onPositionChanged(qint64 position)
{
    if (!m_sliderPressed) {
        m_positionSlider->setValue(position);
    }
    updateTimeLabels(position, m_mediaPlayer->duration());
}

void AdvancedVideoPlayer::onDurationChanged(qint64 duration)
{
    m_positionSlider->setRange(0, duration);
    updateTimeLabels(m_mediaPlayer->position(), duration);
}

void AdvancedVideoPlayer::onMediaError(QMediaPlayer::Error error, const QString &errorString)
{
    QString message;
    switch (error) {
    case QMediaPlayer::ResourceError:
        message = "资源错误: " + errorString;
        break;
    case QMediaPlayer::FormatError:
        message = "格式错误: " + errorString;
        break;
    case QMediaPlayer::NetworkError:
        message = "网络错误: " + errorString;
        break;
    case QMediaPlayer::AccessDeniedError:
        message = "访问被拒绝: " + errorString;
        break;
    default:
        message = "播放错误: " + errorString;
        break;
    }

    QMessageBox::critical(this, "播放错误", message);
    statusBar()->showMessage("播放错误");
}

// UI事件处理
void AdvancedVideoPlayer::onVolumeChanged(int volume)
{
    m_volume = volume;
    m_audioOutput->setVolume(volume / 100.0);
    updateVolumeDisplay();

    if (volume > 0 && m_isMuted) {
        m_isMuted = false;
        m_muteButton->setText("🔊");
    }
}

void AdvancedVideoPlayer::onPositionSliderPressed()
{
    m_sliderPressed = true;
}

void AdvancedVideoPlayer::onPositionSliderReleased()
{
    m_sliderPressed = false;
    m_mediaPlayer->setPosition(m_positionSlider->value());
}

void AdvancedVideoPlayer::onPositionSliderMoved(int position)
{
    if (m_sliderPressed) {
        updateTimeLabels(position, m_mediaPlayer->duration());
    }
}

void AdvancedVideoPlayer::onSpeedChanged(int index)
{
    static const QList<qreal> speeds = {0.5, 0.75, 1.0, 1.25, 1.5, 2.0};
    if (index >= 0 && index < speeds.size()) {
        m_mediaPlayer->setPlaybackRate(speeds[index]);
    }
}

void AdvancedVideoPlayer::onMuteToggled()
{
    if (m_isMuted) {
        m_audioOutput->setMuted(false);
        m_volumeSlider->setValue(m_volume);
        m_muteButton->setText("🔊");
        m_isMuted = false;
    } else {
        m_audioOutput->setMuted(true);
        m_muteButton->setText("🔇");
        m_isMuted = true;
    }
}

void AdvancedVideoPlayer::onFullScreenToggled()
{
    if (m_isFullScreen) {
        showNormal();
        m_controlPanel->show();
        menuBar()->show();
        statusBar()->show();
        m_isFullScreen = false;
        m_fullscreenButton->setText("全屏");
        m_fullscreenHideTimer->stop();
    } else {
        showFullScreen();
        menuBar()->hide();
        statusBar()->hide();
        m_isFullScreen = true;
        m_fullscreenButton->setText("窗口");
        if (m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
            m_fullscreenHideTimer->start(3000);
        }
    }
}

void AdvancedVideoPlayer::onPlaylistToggled()
{
    m_playlistVisible = !m_playlistVisible;
    m_playlistWidget->setVisible(m_playlistVisible);
    m_playlistButton->setText(m_playlistVisible ? "隐藏" : "列表");
}

// 播放列表事件
void AdvancedVideoPlayer::onMediaSelected(int index)
{
    if (index >= 0) {
        MediaInfo info = m_playlistWidget->getMediaAt(index);
        QUrl mediaUrl = QUrl::fromLocalFile(info.filePath);
        m_mediaPlayer->setSource(mediaUrl);
        updateMediaInfo();
    } else {
        m_mediaPlayer->setSource(QUrl());
        updateMediaInfo();
    }
}

void AdvancedVideoPlayer::onPlayModeChanged(PlaylistWidget::PlayMode mode)
{
    static const QStringList modeNames = {"顺序播放", "列表循环", "随机播放", "单曲循环"};
    showNotification(QString("播放模式: %1").arg(modeNames[static_cast<int>(mode)]));
}

void AdvancedVideoPlayer::onPlaylistChanged()
{
    updateButtonStates();
}

void AdvancedVideoPlayer::onPlayRequested()
{
    play();
}

void AdvancedVideoPlayer::onNextRequested()
{
    next();
}

void AdvancedVideoPlayer::onPreviousRequested()
{
    previous();
}

// 快捷键事件
void AdvancedVideoPlayer::onShortcutTriggered(PlayerAction action)
{
    switch (action) {
    case PlayerAction::PlayPause:
        if (m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
            pause();
        } else {
            play();
        }
        break;
    case PlayerAction::Stop:
        stop();
        break;
    case PlayerAction::Previous:
        previous();
        break;
    case PlayerAction::Next:
        next();
        break;
    case PlayerAction::SeekForward:
    {
        qint64 pos = m_mediaPlayer->position();
        m_mediaPlayer->setPosition(qMin(pos + 10000, m_mediaPlayer->duration()));
        showNotification("快进 10 秒", 1000);
    }
    break;
    case PlayerAction::SeekBackward:
    {
        qint64 pos = m_mediaPlayer->position();
        m_mediaPlayer->setPosition(qMax(pos - 10000, 0LL));
        showNotification("快退 10 秒", 1000);
    }
    break;
    case PlayerAction::VolumeUp:
        m_volumeSlider->setValue(qMin(m_volumeSlider->value() + 10, 100));
        break;
    case PlayerAction::VolumeDown:
        m_volumeSlider->setValue(qMax(m_volumeSlider->value() - 10, 0));
        break;
    case PlayerAction::Mute:
        onMuteToggled();
        break;
    case PlayerAction::FullScreen:
        onFullScreenToggled();
        break;
    case PlayerAction::ShowPlaylist:
        onPlaylistToggled();
        break;
    case PlayerAction::OpenFile:
        openFile();
        break;
    case PlayerAction::OpenFolder:
        openFolder();
        break;
    case PlayerAction::AddToFavorites:
        // TODO: 实现收藏功能
        break;
    case PlayerAction::RemoveFromPlaylist:
        m_playlistWidget->removeCurrentItem();
        break;
    case PlayerAction::ClearPlaylist:
        m_playlistWidget->clearPlaylist();
        break;
    case PlayerAction::ShowAbout:
        showAbout();
        break;
    case PlayerAction::Quit:
        close();
        break;
    }
}

// 定时器事件
void AdvancedVideoPlayer::updateProgress()
{
    if (!m_sliderPressed && m_mediaPlayer->duration() > 0) {
        m_positionSlider->setValue(m_mediaPlayer->position());
    }
}

void AdvancedVideoPlayer::hideControlsInFullscreen()
{
    if (m_isFullScreen) {
        m_controlPanel->hide();
        setCursor(Qt::BlankCursor);
    }
}

// 菜单事件
void AdvancedVideoPlayer::showAbout()
{
    QMessageBox::about(this, "关于",
                       "<h2>Qt6高级视频播放器</h2>"
                       "<p>版本: 1.0</p>"
                       "<p>一个功能完整的视频播放器，支持多种格式和高级播放功能。</p>"
                       "<p><b>主要功能：</b></p>"
                       "<ul>"
                       "<li>多格式媒体播放支持</li>"
                       "<li>智能播放列表管理</li>"
                       "<li>多种播放模式</li>"
                       "<li>完整的快捷键支持</li>"
                       "<li>全屏播放</li>"
                       "<li>拖拽文件支持</li>"
                       "</ul>"
                       "<p>基于Qt6框架开发</p>");
}

void AdvancedVideoPlayer::exportPlaylist()
{
    QString fileName = QFileDialog::getSaveFileName(this, "导出播放列表", "",
                                                    "M3U播放列表 (*.m3u);;PLS播放列表 (*.pls)");

    if (!fileName.isEmpty()) {
        QString format = QFileInfo(fileName).suffix().toLower();
        m_playlistWidget->exportPlaylist(fileName, format);
        showNotification("播放列表已导出");
    }
}

void AdvancedVideoPlayer::importPlaylist()
{
    QString fileName = QFileDialog::getOpenFileName(this, "导入播放列表", "",
                                                    "播放列表文件 (*.m3u *.pls);;所有文件 (*.*)");

    if (!fileName.isEmpty()) {
        m_playlistWidget->importPlaylist(fileName);
        showNotification("播放列表已导入");
    }
}

// 事件处理
void AdvancedVideoPlayer::keyPressEvent(QKeyEvent *event)
{
    // 数字键快速跳转
    if (event->key() >= Qt::Key_0 && event->key() <= Qt::Key_9) {
        int percentage = (event->key() - Qt::Key_0) * 10;
        if (percentage == 0) percentage = 100;

        qint64 duration = m_mediaPlayer->duration();
        if (duration > 0) {
            qint64 position = (duration * percentage) / 100;
            m_mediaPlayer->setPosition(position);
            showNotification(QString("跳转到 %1%").arg(percentage), 1000);
        }
        return;
    }

    // ESC键退出全屏
    if (event->key() == Qt::Key_Escape && m_isFullScreen) {
        onFullScreenToggled();
        return;
    }

    QMainWindow::keyPressEvent(event);
}

void AdvancedVideoPlayer::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        onFullScreenToggled();
    }
    QMainWindow::mouseDoubleClickEvent(event);
}

void AdvancedVideoPlayer::closeEvent(QCloseEvent *event)
{
    saveSettings();
    QMainWindow::closeEvent(event);
}

// 辅助方法
void AdvancedVideoPlayer::updateButtonStates()
{
    QMediaPlayer::PlaybackState state = m_mediaPlayer->playbackState();

    m_playButton->setEnabled(state != QMediaPlayer::PlayingState);
    m_pauseButton->setEnabled(state == QMediaPlayer::PlayingState);
    m_stopButton->setEnabled(state != QMediaPlayer::StoppedState);

    bool hasPlaylist = m_playlistWidget->getMediaCount() > 0;
    int currentIndex = m_playlistWidget->currentIndex();

    m_previousButton->setEnabled(hasPlaylist && m_playlistWidget->getPreviousIndex() >= 0);
    m_nextButton->setEnabled(hasPlaylist && m_playlistWidget->getNextIndex() >= 0);
}

void AdvancedVideoPlayer::updateTimeLabels(qint64 current, qint64 total)
{
    m_currentTimeLabel->setText(formatTime(current));
    m_totalTimeLabel->setText(formatTime(total));
}

void AdvancedVideoPlayer::updateVolumeDisplay()
{
    m_volumeLabel->setText(QString("%1%").arg(m_volume));
}

void AdvancedVideoPlayer::updateMediaInfo()
{
    MediaInfo info = m_playlistWidget->getCurrentMedia();
    if (!info.title.isEmpty()) {
        setWindowTitle(QString("Qt6高级视频播放器 - %1").arg(info.displayName()));
        m_mediaInfoLabel->setText(QString("<b>%1</b><br>%2").arg(info.title).arg(info.artist));
        m_mediaInfoLabel->show();
    } else {
        setWindowTitle("Qt6高级视频播放器");
        m_mediaInfoLabel->hide();
    }
}

void AdvancedVideoPlayer::saveSettings()
{
    QSettings settings;
    settings.beginGroup("MainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("volume", m_volume);
    settings.setValue("playlistVisible", m_playlistVisible);
    settings.setValue("splitterState", m_mainSplitter->saveState());
    settings.endGroup();
}

void AdvancedVideoPlayer::loadSettings()
{
    QSettings settings;
    settings.beginGroup("MainWindow");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());

    m_volume = settings.value("volume", 50).toInt();
    m_volumeSlider->setValue(m_volume);
    m_audioOutput->setVolume(m_volume / 100.0);
    updateVolumeDisplay();

    m_playlistVisible = settings.value("playlistVisible", true).toBool();
    m_playlistWidget->setVisible(m_playlistVisible);
    m_playlistButton->setText(m_playlistVisible ? "隐藏" : "列表");

    m_mainSplitter->restoreState(settings.value("splitterState").toByteArray());
    settings.endGroup();
}

QString AdvancedVideoPlayer::formatTime(qint64 milliseconds) const
{
    qint64 seconds = milliseconds / 1000;
    qint64 minutes = seconds / 60;
    qint64 hours = minutes / 60;

    seconds %= 60;
    minutes %= 60;

    if (hours > 0) {
        return QString("%1:%2:%3")
        .arg(hours, 2, 10, QLatin1Char('0'))
            .arg(minutes, 2, 10, QLatin1Char('0'))
            .arg(seconds, 2, 10, QLatin1Char('0'));
    } else {
        return QString("%1:%2")
        .arg(minutes, 2, 10, QLatin1Char('0'))
            .arg(seconds, 2, 10, QLatin1Char('0'));
    }
}

void AdvancedVideoPlayer::showNotification(const QString &message, int duration)
{
    statusBar()->showMessage(message, duration);
}
#if 0
// main.cpp
#include <QApplication>
#include "AdvancedVideoPlayer.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("Qt6高级视频播放器");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("Qt6教程");

    AdvancedVideoPlayer player;
    player.show();

    return app.exec();
}
#endif
