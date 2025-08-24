// VideoPlayerMainWindow.h
#ifndef VIDEOPLAYERMAINWINDOW_H
#define VIDEOPLAYERMAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QListWidget>
#include <QSplitter>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QProgressBar>
#include <QGroupBox>
#include <QFrame>
#include <QSpinBox>

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

class VideoPlayerMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    VideoPlayerMainWindow(QWidget *parent = nullptr);
    ~VideoPlayerMainWindow();

private slots:
    void onOpenFile();
    void onPlay();
    void onPause();
    void onStop();
    void onPrevious();
    void onNext();
    void onVolumeChanged(int value);
    void onPositionChanged(int value);
    void onSpeedChanged(int value);

private:
    // UI组件
    QWidget *centralWidget;
    QSplitter *mainSplitter;

    // 视频显示区域
    QFrame *videoFrame;
    QLabel *videoLabel;

    // 控制面板
    QWidget *controlPanel;
    QGroupBox *playControlGroup;
    QGroupBox *volumeControlGroup;
    QGroupBox *progressGroup;
    QGroupBox *speedControlGroup;

    // 播放控制按钮
    QPushButton *openFileButton;
    QPushButton *playButton;
    QPushButton *pauseButton;
    QPushButton *stopButton;
    QPushButton *previousButton;
    QPushButton *nextButton;

    // 音量控制
    QSlider *volumeSlider;
    QLabel *volumeLabel;
    QPushButton *muteButton;

    // 进度控制
    QSlider *positionSlider;
    QLabel *currentTimeLabel;
    QLabel *totalTimeLabel;

    // 播放速度控制
    QSlider *speedSlider;
    QLabel *speedLabel;

    // 播放列表
    QListWidget *playlistWidget;

    // 菜单和工具栏
    // QMenuBar *menuBar;
    QToolBar *toolBar;
    // QStatusBar *statusBar;

    // 状态栏组件
    QLabel *statusLabel;
    QProgressBar *loadingProgress;

    // 私有方法
    void setupUI();
    void setupMenus();
    void setupToolBar();
    void setupStatusBar();
    void setupCentralWidget();
    void setupControlPanel();
    void setupPlaylist();
    void connectSignals();
    void updateButtonStates();
};

#endif // VIDEOPLAYERMAINWINDOW_H
