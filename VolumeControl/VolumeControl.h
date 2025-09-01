// VolumeControl.h
#ifndef VOLUMECONTROL_H
#define VOLUMECONTROL_H

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QProgressBar>

class VolumeControl : public QWidget
{
    Q_OBJECT  // 这个宏是使用信号槽机制必需的

public:
    explicit VolumeControl(QWidget *parent = nullptr);

private slots:
    // 槽函数：响应滑块值变化
    void onVolumeChanged(int value);
    // 槽函数：响应静音按钮
    void onMuteClicked();
    // 槽函数：响应重置按钮
    void onResetClicked();

    void onMaxClicked();
    // 新增：音量增减按钮槽函数
    void onVolumeUpClicked();
    void onVolumeDownClicked();
private:
    // UI组件
    QSlider *volumeSlider;
    QLabel *volumeLabel;
    QPushButton *muteButton;
    QPushButton *resetButton;
    QPushButton *maxButton;
    QProgressBar *volumeProgress;
    QPushButton *volumeUpButton;      // 新增：+5%按钮
    QPushButton *volumeDownButton;    // 新增：-5%按钮


    // 私有变量
    int previousVolume;
    bool isMuted;
    void adjustVolume(int delta);
    // 私有方法
    void setupUI();
    void connectSignals();
    void updateVolumeDisplay(int value);
    void logVolumeChange(int value, const QString& source);
};

#endif // VOLUMECONTROL_H
