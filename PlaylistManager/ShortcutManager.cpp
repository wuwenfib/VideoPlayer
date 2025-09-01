// ShortcutManager.cpp
#include "ShortcutManager.h"

ShortcutManager::ShortcutManager(QWidget *parent)
    : QObject(parent), m_parentWidget(parent)
{
    setupDefaultShortcuts();
}

void ShortcutManager::setupDefaultShortcuts()
{
    // 播放控制
    createShortcut(PlayerAction::PlayPause, QKeySequence(Qt::Key_Space));
    createShortcut(PlayerAction::Stop, QKeySequence(Qt::Key_S));
    createShortcut(PlayerAction::Previous, QKeySequence(Qt::CTRL | Qt::Key_Left));
    createShortcut(PlayerAction::Next, QKeySequence(Qt::CTRL | Qt::Key_Right));
    createShortcut(PlayerAction::SeekForward, QKeySequence(Qt::Key_Right));
    createShortcut(PlayerAction::SeekBackward, QKeySequence(Qt::Key_Left));

    // 音量控制
    createShortcut(PlayerAction::VolumeUp, QKeySequence(Qt::Key_Up));
    createShortcut(PlayerAction::VolumeDown, QKeySequence(Qt::Key_Down));
    createShortcut(PlayerAction::Mute, QKeySequence(Qt::Key_M));

    // 视图控制
    createShortcut(PlayerAction::FullScreen, QKeySequence(Qt::Key_F11));
    createShortcut(PlayerAction::ShowPlaylist, QKeySequence(Qt::Key_L));

    // 文件操作
    createShortcut(PlayerAction::OpenFile, QKeySequence::Open);
    createShortcut(PlayerAction::OpenFolder, QKeySequence(Qt::CTRL | Qt::Key_D));

    // 播放列表
    createShortcut(PlayerAction::AddToFavorites, QKeySequence(Qt::CTRL | Qt::Key_F));
    createShortcut(PlayerAction::RemoveFromPlaylist, QKeySequence(Qt::Key_Delete));
    createShortcut(PlayerAction::ClearPlaylist, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Delete));

    // 其他
    createShortcut(PlayerAction::ShowAbout, QKeySequence(Qt::Key_F1));
    createShortcut(PlayerAction::Quit, QKeySequence::Quit);
}

void ShortcutManager::createShortcut(PlayerAction action, const QKeySequence &keySequence)
{
    QShortcut *shortcut = new QShortcut(keySequence, m_parentWidget);
    connect(shortcut, &QShortcut::activated, this, &ShortcutManager::onShortcutActivated);
    m_shortcuts[action] = shortcut;
}

void ShortcutManager::setShortcutEnabled(PlayerAction action, bool enabled)
{
    if (m_shortcuts.contains(action)) {
        m_shortcuts[action]->setEnabled(enabled);
    }
}

QKeySequence ShortcutManager::getShortcut(PlayerAction action) const
{
    if (m_shortcuts.contains(action)) {
        return m_shortcuts[action]->key();
    }
    return QKeySequence();
}

void ShortcutManager::onShortcutActivated()
{
    QShortcut *shortcut = qobject_cast<QShortcut*>(sender());
    if (!shortcut) return;

    // 找到对应的动作
    for (auto it = m_shortcuts.begin(); it != m_shortcuts.end(); ++it) {
        if (it.value() == shortcut) {
            emit actionTriggered(it.key());
            break;
        }
    }
}
