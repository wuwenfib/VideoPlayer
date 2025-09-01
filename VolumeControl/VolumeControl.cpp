// VolumeControl.cpp
#include "VolumeControl.h"
#include <QDebug>
#include <QDateTime>
VolumeControl::VolumeControl(QWidget *parent)
    : QWidget(parent), previousVolume(50), isMuted(false)
{
    setupUI();
    connectSignals();
}

void VolumeControl::setupUI()
{
    setWindowTitle("音量控制示例 - Qt6信号与槽");
    setFixedSize(400, 200);

    // 创建控件
    volumeSlider = new QSlider(Qt::Horizontal);
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(50);
    volumeSlider->setTickPosition(QSlider::TicksBelow);
    volumeSlider->setTickInterval(10);

    volumeLabel = new QLabel("音量: 50%");
    volumeLabel->setAlignment(Qt::AlignCenter);
    volumeLabel->setStyleSheet("font-size: 16px; font-weight: bold;");

    volumeProgress = new QProgressBar();
    volumeProgress->setRange(0, 100);
    volumeProgress->setValue(50);

    muteButton = new QPushButton("静音");
    resetButton = new QPushButton("重置");
    maxButton = new QPushButton("最大");

    // 创建布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 音量标签
    mainLayout->addWidget(volumeLabel);

    // 滑块布局
    QHBoxLayout *sliderLayout = new QHBoxLayout();
    sliderLayout->addWidget(new QLabel("0"));
    sliderLayout->addWidget(volumeSlider);
    sliderLayout->addWidget(new QLabel("100"));
    mainLayout->addLayout(sliderLayout);
    // 新增：音量增减按钮
    volumeUpButton = new QPushButton("+5%");
    volumeUpButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; }");
    volumeUpButton->setToolTip("增加音量5%");

    volumeDownButton = new QPushButton("-5%");
    volumeDownButton->setStyleSheet("QPushButton { background-color: #f44336; color: white; font-weight: bold; }");
    volumeDownButton->setToolTip("减少音量5%");

    // 音量增减按钮布局（新增）
    QHBoxLayout *volumeAdjustLayout = new QHBoxLayout();
    volumeAdjustLayout->addStretch();  // 左侧弹性空间
    volumeAdjustLayout->addWidget(volumeDownButton);
    volumeAdjustLayout->addWidget(volumeUpButton);
    volumeAdjustLayout->addStretch();  // 右侧弹性空间

    mainLayout->addLayout(volumeAdjustLayout);


    // 进度条
    mainLayout->addWidget(volumeProgress);

    // 按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(muteButton);
    buttonLayout->addWidget(resetButton);
    buttonLayout->addWidget(maxButton);

    mainLayout->addLayout(buttonLayout);
}

void VolumeControl::connectSignals()
{
    // 连接信号与槽的几种方式：

    // 1. 新式connect语法（推荐，类型安全）
    connect(volumeSlider, &QSlider::valueChanged,
            this, &VolumeControl::onVolumeChanged);

    // 2. Lambda表达式（适合简单操作）
    connect(volumeSlider, &QSlider::valueChanged, [this](int value){
        volumeProgress->setValue(value);
    });

    // 3. 连接按钮信号
    connect(muteButton, &QPushButton::clicked,
            this, &VolumeControl::onMuteClicked);

    connect(resetButton, &QPushButton::clicked,
            this, &VolumeControl::onResetClicked);
    connect(maxButton, &QPushButton::clicked,
            this, &VolumeControl::onMaxClicked);

    // 新增：连接音量增减按钮
    connect(volumeUpButton, &QPushButton::clicked,
            this, &VolumeControl::onVolumeUpClicked);

    connect(volumeDownButton, &QPushButton::clicked,
            this, &VolumeControl::onVolumeDownClicked);

    // 4. 直接连接两个控件（不需要中间处理）
    // connect(volumeSlider, &QSlider::valueChanged,
    //         volumeProgress, &QProgressBar::setValue);
}

// 新增：音量增加按钮槽函数
void VolumeControl::onVolumeUpClicked()
{
    adjustVolume(5);
    logVolumeChange(volumeSlider->value(), "按钮增加");
}

// 新增：音量减少按钮槽函数
void VolumeControl::onVolumeDownClicked()
{
    adjustVolume(-5);
    logVolumeChange(volumeSlider->value(), "按钮减少");
}

// 新增：音量变化日志输出方法
void VolumeControl::logVolumeChange(int value, const QString& source)
{
    // 获取当前时间戳
    QDateTime currentTime = QDateTime::currentDateTime();
    QString timeString = currentTime.toString("yyyy-MM-dd hh:mm:ss.zzz");

    // 输出详细的日志信息
    qDebug() << "=== 音量变化日志 ===";
    qDebug() << "时间:" << timeString;
    qDebug() << "音量:" << value << "%";
    qDebug() << "操作:" << source;
    qDebug() << "状态:" << (isMuted ? "静音" : "正常");
    qDebug() << "==================";

    // 也可以输出到标准输出（控制台）
    printf("[%s] 音量变化: %d%% (操作: %s)\n",
           timeString.toLocal8Bit().data(),
           value,
           source.toLocal8Bit().data());
}

// 新增：音量调节方法
void VolumeControl::adjustVolume(int delta)
{
    int currentVolume = volumeSlider->value();
    int newVolume = currentVolume + delta;

    // 确保音量在0-100范围内
    newVolume = qMax(0, qMin(100, newVolume));

    // 如果音量有变化才设置
    if (newVolume != currentVolume) {
        volumeSlider->setValue(newVolume);

        // 如果之前是静音状态且现在音量大于0，取消静音
        if (isMuted && newVolume > 0) {
            isMuted = false;
            muteButton->setText("静音");
        }
    }
}

void VolumeControl::onVolumeChanged(int value)
{
    updateVolumeDisplay(value);

    // 如果之前是静音状态，现在取消静音
    if (isMuted && value > 0) {
        isMuted = false;
        muteButton->setText("静音");
    }

    qDebug() << "音量变化:" << value << "%";
}

void VolumeControl::onMuteClicked()
{
    if (isMuted) {
        // 取消静音，恢复之前的音量
        volumeSlider->setValue(previousVolume);
        muteButton->setText("静音");
        isMuted = false;
        qDebug() << "取消静音，恢复音量:" << previousVolume << "%";
    } else {
        // 静音，保存当前音量
        previousVolume = volumeSlider->value();
        volumeSlider->setValue(0);
        muteButton->setText("取消静音");
        isMuted = true;
        qDebug() << "静音，保存音量:" << previousVolume << "%";
    }
}

void VolumeControl::onMaxClicked(){
    volumeSlider->setValue(100);
    isMuted = false;
    muteButton->setText("最大");
    qDebug() << "音量重置为50%";
}

void VolumeControl::onResetClicked()
{
    volumeSlider->setValue(50);
    isMuted = false;
    muteButton->setText("静音");
    qDebug() << "音量重置为50%";
}

void VolumeControl::updateVolumeDisplay(int value)
{
    volumeLabel->setText(QString("音量: %1%").arg(value));

    // 根据音量值改变标签颜色
    QString color;
    if (value == 0) {
        color = "red";
    } else if (value < 30) {
        color = "orange";
    } else if (value < 70) {
        color = "blue";
    } else {
        color = "green";
    }

    volumeLabel->setStyleSheet(QString("font-size: 16px; font-weight: bold; color: %1;").arg(color));
}
