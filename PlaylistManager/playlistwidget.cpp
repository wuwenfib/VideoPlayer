// PlaylistWidget.cpp
#include "PlaylistWidget.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QTextStream>
#include <QHeaderView>
#include <algorithm>
#include <random>

PlaylistWidget::PlaylistWidget(QWidget *parent)
    : QWidget(parent)
    , m_currentIndex(-1)
    , m_playMode(Sequential)
    , m_randomIndex(-1)
    , m_showingFavorites(false)
{
    m_supportedFormats = getSupportedFormats();
    setupUI();
    setupConnections();
    setAcceptDrops(true);

    // 加载保存的播放列表
    loadPlaylist();
}

PlaylistWidget::~PlaylistWidget()
{
    savePlaylist();
}

void PlaylistWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(5);
    m_mainLayout->setContentsMargins(5, 5, 5, 5);

    // 控制区域
    m_controlLayout = new QHBoxLayout();

    m_searchEdit = new QLineEdit();
    m_searchEdit->setPlaceholderText("搜索媒体文件...");
    m_searchEdit->setClearButtonEnabled(true);

    m_playModeButton = new QPushButton("▶");
    m_playModeButton->setMaximumSize(30, 30);
    m_playModeButton->setToolTip("播放模式");

    m_playModeLabel = new QLabel("顺序播放");
    m_playModeLabel->setStyleSheet("color: #666;");

    m_controlLayout->addWidget(m_searchEdit);
    m_controlLayout->addWidget(m_playModeButton);
    m_controlLayout->addWidget(m_playModeLabel);

    // 按钮区域
    m_buttonLayout = new QHBoxLayout();

    m_addFilesButton = new QPushButton("添加文件");
    m_removeButton = new QPushButton("移除");
    m_clearButton = new QPushButton("清空");

    m_countLabel = new QLabel("0 个文件");

    m_buttonLayout->addWidget(m_addFilesButton);
    m_buttonLayout->addWidget(m_removeButton);
    m_buttonLayout->addWidget(m_clearButton);
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_countLabel);

    // 列表组件
    m_listWidget = new QListWidget();
    m_listWidget->setAlternatingRowColors(true);
    m_listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    m_listWidget->setDragDropMode(QAbstractItemView::InternalMove);

    // 创建右键菜单
    m_contextMenu = new QMenu(this);
    m_contextMenu->addAction("播放", this, [this]() {
        if (m_listWidget->currentRow() >= 0) {
            setCurrentIndex(m_listWidget->currentRow());
            emit requestPlay();
        }
    });
    m_contextMenu->addSeparator();
    m_contextMenu->addAction("添加到收藏", this, &PlaylistWidget::toggleFavorite);
    m_contextMenu->addAction("移除", this, &PlaylistWidget::removeCurrentItem);
    m_contextMenu->addSeparator();
    m_contextMenu->addAction("属性", this, &PlaylistWidget::showItemProperties);

    // 布局组装
    m_mainLayout->addLayout(m_controlLayout);
    m_mainLayout->addLayout(m_buttonLayout);
    m_mainLayout->addWidget(m_listWidget, 1);

    updateUI();
}

void PlaylistWidget::setupConnections()
{
    // 列表事件
    connect(m_listWidget, &QListWidget::itemDoubleClicked,
            this, &PlaylistWidget::onItemDoubleClicked);
    connect(m_listWidget, &QListWidget::customContextMenuRequested,
            this, &PlaylistWidget::showContextMenu);

    // 控制按钮
    connect(m_searchEdit, &QLineEdit::textChanged,
            this, &PlaylistWidget::onSearchTextChanged);
    connect(m_playModeButton, &QPushButton::clicked,
            this, &PlaylistWidget::onPlayModeButtonClicked);

    // 操作按钮
    connect(m_addFilesButton, &QPushButton::clicked,
            this, &PlaylistWidget::onAddFilesClicked);
    connect(m_removeButton, &QPushButton::clicked,
            this, &PlaylistWidget::onRemoveItemClicked);
    connect(m_clearButton, &QPushButton::clicked,
            this, &PlaylistWidget::onClearPlaylistClicked);
}

void PlaylistWidget::addMedia(const QString &filePath)
{
    if (!QFileInfo::exists(filePath) || !isMediaFile(filePath)) {
        return;
    }

    // 检查是否已存在
    for (const MediaInfo &info : m_mediaList) {
        if (info.filePath == filePath) {
            return;
        }
    }

    MediaInfo mediaInfo;
    mediaInfo.filePath = filePath;
    mediaInfo.title = QFileInfo(filePath).baseName();

    // 异步提取媒体信息
    extractMediaInfo(mediaInfo);

    m_mediaList.append(mediaInfo);

    // 更新UI
    QListWidgetItem *item = new QListWidgetItem(mediaInfo.displayName());
    item->setToolTip(filePath);
    if (mediaInfo.isFavorite) {
        item->setIcon(QIcon("⭐"));
    }
    m_listWidget->addItem(item);

    // 如果是第一个文件，设置为当前项
    if (m_mediaList.size() == 1) {
        setCurrentIndex(0);
    }

    updateUI();
    emit playlistChanged();
}

void PlaylistWidget::addMediaList(const QStringList &filePaths)
{
    for (const QString &path : filePaths) {
        addMedia(path);
    }

    // 生成新的随机顺序
    if (m_playMode == Random) {
        generateRandomOrder();
    }
}

void PlaylistWidget::removeCurrentItem()
{
    int currentRow = m_listWidget->currentRow();
    if (currentRow < 0 || currentRow >= m_mediaList.size()) {
        return;
    }

    m_mediaList.removeAt(currentRow);
    delete m_listWidget->takeItem(currentRow);

    // 调整当前索引
    if (m_currentIndex == currentRow) {
        if (m_mediaList.isEmpty()) {
            m_currentIndex = -1;
        } else if (m_currentIndex >= m_mediaList.size()) {
            m_currentIndex = m_mediaList.size() - 1;
        }
        emit mediaSelected(m_currentIndex);
    } else if (m_currentIndex > currentRow) {
        m_currentIndex--;
    }

    updateUI();
    emit playlistChanged();
}

void PlaylistWidget::clearPlaylist()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认清空", "确定要清空播放列表吗？",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        m_mediaList.clear();
        m_listWidget->clear();
        m_currentIndex = -1;
        m_randomOrder.clear();
        m_randomIndex = -1;

        updateUI();
        emit playlistChanged();
        emit mediaSelected(-1);
    }
}

void PlaylistWidget::setCurrentIndex(int index)
{
    if (index < -1 || index >= m_mediaList.size()) {
        return;
    }

    // 清除之前的高亮
    if (m_currentIndex >= 0 && m_currentIndex < m_listWidget->count()) {
        QListWidgetItem *oldItem = m_listWidget->item(m_currentIndex);
        if (oldItem) {
            QFont font = oldItem->font();
            font.setBold(false);
            oldItem->setFont(font);
            oldItem->setBackground(QBrush());
        }
    }

    m_currentIndex = index;

    // 设置新的高亮
    if (m_currentIndex >= 0) {
        QListWidgetItem *newItem = m_listWidget->item(m_currentIndex);
        if (newItem) {
            QFont font = newItem->font();
            font.setBold(true);
            newItem->setFont(font);
            newItem->setBackground(QColor(100, 149, 237, 50));
            m_listWidget->setCurrentItem(newItem);
            m_listWidget->scrollToItem(newItem);
        }

        // 更新播放统计
        m_mediaList[m_currentIndex].playCount++;
    }

    emit mediaSelected(m_currentIndex);
}

void PlaylistWidget::setPlayMode(PlayMode mode)
{
    if (m_playMode != mode) {
        m_playMode = mode;
        updatePlayModeDisplay();

        if (mode == Random) {
            generateRandomOrder();
        }

        emit playModeChanged(mode);
    }
}

int PlaylistWidget::getNextIndex() const
{
    if (m_mediaList.isEmpty()) {
        return -1;
    }

    switch (m_playMode) {
        case Sequential:
            return (m_currentIndex + 1 < m_mediaList.size()) ? m_currentIndex + 1 : -1;

        case Loop:
            return (m_currentIndex + 1) % m_mediaList.size();

        case Random:
            if (m_randomOrder.isEmpty()) {
                return -1;
            }
            int nextRandomIndex = (m_randomIndex + 1) % m_randomOrder.size();
            return m_randomOrder[nextRandomIndex];

        // case RepeatOne:
        //     return m_currentIndex;

        // default:
        //     break;
    }

    return -1;
}

int PlaylistWidget::getPreviousIndex() const
{
    if (m_mediaList.isEmpty()) {
        return -1;
    }

    switch (m_playMode) {
    case Sequential:
        return (m_currentIndex > 0) ? m_currentIndex - 1 : -1;

    case Loop:
        return (m_currentIndex - 1 + m_mediaList.size()) % m_mediaList.size();

    case Random:
        if (m_randomOrder.isEmpty()) {
            return -1;
        }
        int prevRandomIndex = (m_randomIndex - 1 + m_randomOrder.size()) % m_randomOrder.size();
        return m_randomOrder[prevRandomIndex];

    // case RepeatOne:
    //     return m_currentIndex;
    // default:
    //     break;
    }


    return -1;
}

void PlaylistWidget::searchMedia(const QString &keyword)
{
    if (keyword.isEmpty()) {
        // 恢复原始列表
        if (!m_originalList.isEmpty()) {
            m_mediaList = m_originalList;
            m_originalList.clear();
        }
    } else {
        // 保存原始列表
        if (m_originalList.isEmpty()) {
            m_originalList = m_mediaList;
        }

        // 过滤列表
        QList<MediaInfo> filteredList;
        for (const MediaInfo &info : m_originalList) {
            if (info.displayName().contains(keyword, Qt::CaseInsensitive) ||
                info.filePath.contains(keyword, Qt::CaseInsensitive)) {
                filteredList.append(info);
            }
        }
        m_mediaList = filteredList;
    }

    // 重建列表显示
    m_listWidget->clear();
    for (int i = 0; i < m_mediaList.size(); ++i) {
        const MediaInfo &info = m_mediaList[i];
        QListWidgetItem *item = new QListWidgetItem(info.displayName());
        item->setToolTip(info.filePath);
        if (info.isFavorite) {
            item->setIcon(QIcon("⭐"));
        }
        m_listWidget->addItem(item);
    }

    updateUI();
}

MediaInfo PlaylistWidget::getCurrentMedia() const
{
    if (m_currentIndex >= 0 && m_currentIndex < m_mediaList.size()) {
        return m_mediaList[m_currentIndex];
    }
    return MediaInfo();
}

MediaInfo PlaylistWidget::getMediaAt(int index) const
{
    if (index >= 0 && index < m_mediaList.size()) {
        return m_mediaList[index];
    }
    return MediaInfo();
}

// 事件处理
void PlaylistWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void PlaylistWidget::dropEvent(QDropEvent *event)
{
    QStringList filePaths;
    foreach (const QUrl &url, event->mimeData()->urls()) {
        if (url.isLocalFile()) {
            QString path = url.toLocalFile();
            QFileInfo fileInfo(path);

            if (fileInfo.isFile()) {
                if (isMediaFile(path)) {
                    filePaths.append(path);
                }
            } else if (fileInfo.isDir()) {
                // 扫描目录中的媒体文件
                QDir dir(path);
                QStringList filters;
                for (const QString &format : m_supportedFormats) {
                    filters << QString("*.%1").arg(format);
                }

                QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Name);
                for (const QFileInfo &file : files) {
                    filePaths.append(file.absoluteFilePath());
                }
            }
        }
    }

    if (!filePaths.isEmpty()) {
        addMediaList(filePaths);
        event->acceptProposedAction();
    }
}

// 槽函数实现
void PlaylistWidget::onItemDoubleClicked(QListWidgetItem *item)
{
    int index = m_listWidget->row(item);
    setCurrentIndex(index);
    emit requestPlay();
}

void PlaylistWidget::onPlayModeButtonClicked()
{
    PlayMode newMode = static_cast<PlayMode>((m_playMode + 1) % 4);
    setPlayMode(newMode);
}

void PlaylistWidget::onSearchTextChanged(const QString &text)
{
    searchMedia(text);
}

void PlaylistWidget::onAddFilesClicked()
{
    QString lastDir = QSettings().value("lastOpenDir",
                                        QStandardPaths::writableLocation(QStandardPaths::MusicLocation)).toString();

    QStringList filters;
    for (const QString &format : m_supportedFormats) {
        filters << QString("*.%1").arg(format);
    }
    QString filter = QString("媒体文件 (%1);;所有文件 (*.*)").arg(filters.join(" "));

    QStringList fileNames = QFileDialog::getOpenFileNames(this, "选择媒体文件", lastDir, filter);

    if (!fileNames.isEmpty()) {
        QSettings().setValue("lastOpenDir", QFileInfo(fileNames.first()).absolutePath());
        addMediaList(fileNames);
    }
}

void PlaylistWidget::onRemoveItemClicked()
{
    removeCurrentItem();
}

void PlaylistWidget::onClearPlaylistClicked()
{
    clearPlaylist();
}

void PlaylistWidget::showContextMenu(const QPoint &pos)
{
    if (m_listWidget->itemAt(pos)) {
        m_contextMenu->exec(m_listWidget->mapToGlobal(pos));
    }
}

void PlaylistWidget::toggleFavorite()
{
    int currentRow = m_listWidget->currentRow();
    if (currentRow >= 0 && currentRow < m_mediaList.size()) {
        m_mediaList[currentRow].isFavorite = !m_mediaList[currentRow].isFavorite;

        QListWidgetItem *item = m_listWidget->item(currentRow);
        if (m_mediaList[currentRow].isFavorite) {
            item->setIcon(QIcon("⭐"));
        } else {
            item->setIcon(QIcon());
        }
    }
}

void PlaylistWidget::showItemProperties()
{
    int currentRow = m_listWidget->currentRow();
    if (currentRow >= 0 && currentRow < m_mediaList.size()) {
        const MediaInfo &info = m_mediaList[currentRow];

        QString message = QString(
                              "文件路径: %1\n"
                              "标题: %2\n"
                              "艺术家: %3\n"
                              "专辑: %4\n"
                              "时长: %5\n"
                              "添加时间: %6\n"
                              "播放次数: %7"
                              ).arg(info.filePath)
                              .arg(info.title.isEmpty() ? "未知" : info.title)
                              .arg(info.artist.isEmpty() ? "未知" : info.artist)
                              .arg(info.album.isEmpty() ? "未知" : info.album)
                              .arg(formatDuration(info.duration))
                              .arg(info.addTime.toString("yyyy-MM-dd hh:mm:ss"))
                              .arg(info.playCount);

        QMessageBox::information(this, "媒体文件属性", message);
    }
}

// 辅助方法
void PlaylistWidget::updateUI()
{
    m_countLabel->setText(QString("%1 个文件").arg(m_mediaList.size()));

    m_removeButton->setEnabled(m_listWidget->currentRow() >= 0);
    m_clearButton->setEnabled(!m_mediaList.isEmpty());

    updatePlayModeDisplay();
}

void PlaylistWidget::updatePlayModeDisplay()
{
    static const QString modeIcons[] = {"▶", "🔄", "🔀", "🔂"};
    static const QString modeNames[] = {"顺序播放", "列表循环", "随机播放", "单曲循环"};

    m_playModeButton->setText(modeIcons[m_playMode]);
    m_playModeLabel->setText(modeNames[m_playMode]);

    QString tooltip = QString("当前播放模式: %1\n点击切换到下一种模式").arg(modeNames[m_playMode]);
    m_playModeButton->setToolTip(tooltip);
}

void PlaylistWidget::generateRandomOrder()
{
    m_randomOrder.clear();
    for (int i = 0; i < m_mediaList.size(); ++i) {
        m_randomOrder.append(i);
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(m_randomOrder.begin(), m_randomOrder.end(), g);

    // 找到当前播放项在随机序列中的位置
    m_randomIndex = m_randomOrder.indexOf(m_currentIndex);
    if (m_randomIndex == -1) {
        m_randomIndex = 0;
    }
}

QStringList PlaylistWidget::getSupportedFormats() const
{
    return QStringList() << "mp4" << "avi" << "mkv" << "mov" << "wmv"
                         << "flv" << "webm" << "m4v" << "3gp" << "ogv"
                         << "mp3" << "wav" << "flac" << "ogg" << "aac"
                         << "wma" << "m4a";
}

bool PlaylistWidget::isMediaFile(const QString &filePath) const
{
    QString extension = QFileInfo(filePath).suffix().toLower();
    return m_supportedFormats.contains(extension);
}

void PlaylistWidget::extractMediaInfo(MediaInfo &info)
{
    // 这里可以使用更复杂的媒体信息提取逻辑
    // 简化实现：从文件名尝试解析信息
    QString baseName = QFileInfo(info.filePath).baseName();

    // 尝试解析 "艺术家 - 标题" 格式
    if (baseName.contains(" - ")) {
        QStringList parts = baseName.split(" - ");
        if (parts.size() >= 2) {
            info.artist = parts[0].trimmed();
            info.title = parts[1].trimmed();
        }
    }

    if (info.title.isEmpty()) {
        info.title = baseName;
    }
}

QString PlaylistWidget::formatDuration(qint64 duration) const
{
    if (duration <= 0) return "未知";

    qint64 seconds = duration / 1000;
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

void PlaylistWidget::savePlaylist()
{
    QSettings settings;
    settings.beginGroup("Playlist");

    QJsonArray jsonArray;
    for (const MediaInfo &info : m_mediaList) {
        jsonArray.append(info.toJson());
    }

    QJsonDocument doc(jsonArray);
    settings.setValue("mediaList", doc.toJson(QJsonDocument::Compact));
    settings.setValue("currentIndex", m_currentIndex);
    settings.setValue("playMode", static_cast<int>(m_playMode));

    settings.endGroup();
}

void PlaylistWidget::loadPlaylist()
{
    QSettings settings;
    settings.beginGroup("Playlist");

    QByteArray data = settings.value("mediaList").toByteArray();
    if (!data.isEmpty()) {
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonArray jsonArray = doc.array();

        m_mediaList.clear();
        m_listWidget->clear();

        for (const QJsonValue &value : jsonArray) {
            MediaInfo info = MediaInfo::fromJson(value.toObject());
            // 检查文件是否仍然存在
            if (QFileInfo::exists(info.filePath)) {
                m_mediaList.append(info);

                QListWidgetItem *item = new QListWidgetItem(info.displayName());
                item->setToolTip(info.filePath);
                if (info.isFavorite) {
                    item->setIcon(QIcon("⭐"));
                }
                m_listWidget->addItem(item);
            }
        }

        m_currentIndex = settings.value("currentIndex", -1).toInt();
        if (m_currentIndex >= m_mediaList.size()) {
            m_currentIndex = -1;
        }

        PlayMode mode = static_cast<PlayMode>(settings.value("playMode", 0).toInt());
        setPlayMode(mode);

        if (m_currentIndex >= 0) {
            setCurrentIndex(m_currentIndex);
        }
    }

    settings.endGroup();
    updateUI();
}

void PlaylistWidget::exportPlaylist(const QString &fileName, const QString &format)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "导出失败", "无法创建文件：" + fileName);
        return;
    }

    QTextStream out(&file);

    if (format.toLower() == "m3u") {
        out << "#EXTM3U" << Qt::endl;
        for (const MediaInfo &info : m_mediaList) {
            out << QString("#EXTINF:%1,%2 - %3")
            .arg(info.duration / 1000)
                    .arg(info.artist.isEmpty() ? "Unknown" : info.artist)
                    .arg(info.title.isEmpty() ? QFileInfo(info.filePath).baseName() : info.title)
                << Qt::endl;
            out << info.filePath << Qt::endl;
        }
    } else if (format.toLower() == "pls") {
        out << "[playlist]" << Qt::endl;
        for (int i = 0; i < m_mediaList.size(); ++i) {
            const MediaInfo &info = m_mediaList[i];
            out << QString("File%1=%2").arg(i + 1).arg(info.filePath) << Qt::endl;
            out << QString("Title%1=%2").arg(i + 1).arg(info.displayName()) << Qt::endl;
            out << QString("Length%1=%2").arg(i + 1).arg(info.duration / 1000) << Qt::endl;
        }
        out << QString("NumberOfEntries=%1").arg(m_mediaList.size()) << Qt::endl;
        out << "Version=2" << Qt::endl;
    }

    file.close();
}

void PlaylistWidget::importPlaylist(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "导入失败", "无法打开文件：" + fileName);
        return;
    }

    QTextStream in(&file);
    QStringList filesToAdd;

    QFileInfo fileInfo(fileName);
    QString extension = fileInfo.suffix().toLower();

    if (extension == "m3u" || extension == "m3u8") {
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (!line.startsWith("#") && !line.isEmpty()) {
                // 处理相对路径
                if (QFileInfo(line).isRelative()) {
                    line = fileInfo.absoluteDir().absoluteFilePath(line);
                }
                if (QFileInfo::exists(line) && isMediaFile(line)) {
                    filesToAdd.append(line);
                }
            }
        }
    } else if (extension == "pls") {
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.startsWith("File")) {
                QString filePath = line.split("=", Qt::SkipEmptyParts)[1];
                if (QFileInfo(filePath).isRelative()) {
                    filePath = fileInfo.absoluteDir().absoluteFilePath(filePath);
                }
                if (QFileInfo::exists(filePath) && isMediaFile(filePath)) {
                    filesToAdd.append(filePath);
                }
            }
        }
    }

    file.close();

    if (!filesToAdd.isEmpty()) {
        addMediaList(filesToAdd);
        QMessageBox::information(this, "导入成功", QString("成功导入 %1 个文件").arg(filesToAdd.size()));
    } else {
        QMessageBox::warning(this, "导入失败", "未找到有效的媒体文件");
    }
}

