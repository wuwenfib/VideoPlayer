
// AdvancedVideoPlayer.h
#ifndef ADVANCEDVIDEOPLAYER_H
#define ADVANCEDVIDEOPLAYER_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QAudioOutput>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QComboBox>
#include <QProgressBar>
#include <QTimer>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenuBar>
#include <QStatusBar>
#include <QApplication>

#include "PlaylistWidget.h"
#include "ShortcutManager.h"

class AdvancedVideoPlayer : public QMainWindow
{
    Q_OBJECT

public:
    AdvancedVideoPlayer(QWidget *parent = nullptr);
    ~AdvancedVideoPlayer();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void setupControlPanel();
private slots:
    // 播放控制
    void play();
    void pause();
    void stop();
    void openFile();
    void openFolder();
    void previous();
    void next();

    // 媒体事件
    void onMediaStateChanged(QMediaPlayer::PlaybackState state);
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onPositionChanged(qint64 position);
    void onDurationChanged(qint64 duration);
    void onMediaError(QMediaPlayer::Error error, const QString &errorString);

    // UI事件
    void onVolumeChanged(int volume);
    void onPositionSliderPressed();
    void onPositionSliderReleased();
    void onPositionSliderMoved(int position);
    void onSpeedChanged(int index);
    void onMuteToggled();
    void onFullScreenToggled();
    void onPlaylistToggled();

    // 播放列表事件
    void onMediaSelected(int index);
    void onPlayModeChanged(PlaylistWidget::PlayMode mode);
    void onPlaylistChanged();
    void onPlayRequested();
    void onNextRequested();
    void onPreviousRequested();

    // 快捷键事件
    void onShortcutTriggered(PlayerAction action);

    // 定时器事件
    void updateProgress();
    void hideControlsInFullscreen();

    // 菜单事件
    void showAbout();
    void exportPlaylist();
    void importPlaylist();

private:
    // 核心组件
    QMediaPlayer *m_mediaPlayer;
    QVideoWidget *m_videoWidget;
    QAudioOutput *m_audioOutput;
    PlaylistWidget *m_playlistWidget;
    ShortcutManager *m_shortcutManager;
    // UI组件
    QWidget *m_centralWidget;
    QSplitter *m_mainSplitter;
    QWidget *m_videoContainer;
    QWidget *m_controlPanel;

    // 控制组件
    QPushButton *m_playButton;
    QPushButton *m_pauseButton;
    QPushButton *m_stopButton;
    QPushButton *m_previousButton;
    QPushButton *m_nextButton;
    QPushButton *m_openButton;
    QPushButton *m_fullscreenButton;
    QPushButton *m_playlistButton;
    QPushButton *m_muteButton;

    QSlider *m_positionSlider;
    QSlider *m_volumeSlider;
    QComboBox *m_speedComboBox;

    QLabel *m_currentTimeLabel;
    QLabel *m_totalTimeLabel;
    QLabel *m_volumeLabel;
    QLabel *m_mediaInfoLabel;

    QProgressBar *m_bufferProgress;

    // 状态变量
    bool m_isFullScreen;
    bool m_playlistVisible;
    bool m_isMuted;
    bool m_sliderPressed;
    int m_volume;

    // 定时器
    QTimer *m_progressTimer;
    QTimer *m_fullscreenHideTimer;

    // 私有方法
    void setupUI();
    void setupMediaPlayer();
    void setupMenus();
    void setupStatusBar();
    void setupConnections();
    void applyStyles();

    void updateButtonStates();
    void updateTimeLabels(qint64 current, qint64 total);
    void updateVolumeDisplay();
    void updateMediaInfo();

    void saveSettings();
    void loadSettings();

    QString formatTime(qint64 milliseconds) const;
    void showNotification(const QString &message, int duration = 2000);
};

#endif // ADVANCEDVIDEOPLAYER_H
