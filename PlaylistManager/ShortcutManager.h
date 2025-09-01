
// ShortcutManager.h
#ifndef SHORTCUTMANAGER_H
#define SHORTCUTMANAGER_H

#include <QObject>
#include <QWidget>
#include <QShortcut>
#include <QKeySequence>
#include <QHash>
#include <QSettings>

enum class PlayerAction {
    // 播放控制
    PlayPause,
    Stop,
    Previous,
    Next,
    SeekForward,
    SeekBackward,

    // 音量控制
    VolumeUp,
    VolumeDown,
    Mute,

    // 视图控制
    FullScreen,
    ShowPlaylist,

    // 文件操作
    OpenFile,
    OpenFolder,

    // 播放列表
    AddToFavorites,
    RemoveFromPlaylist,
    ClearPlaylist,

    // 其他
    ShowAbout,
    Quit
};

class ShortcutManager : public QObject
{
    Q_OBJECT

public:
    explicit ShortcutManager(QWidget *parent = nullptr);

    void setupDefaultShortcuts();
    void setShortcutEnabled(PlayerAction action, bool enabled);
    QKeySequence getShortcut(PlayerAction action) const;

signals:
    void actionTriggered(PlayerAction action);

private slots:
    void onShortcutActivated();

private:
    QWidget *m_parentWidget;
    QHash<PlayerAction, QShortcut*> m_shortcuts;

    void createShortcut(PlayerAction action, const QKeySequence &keySequence);
};

#endif // SHORTCUTMANAGER_H
