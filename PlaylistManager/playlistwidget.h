// PlaylistWidget.h
#ifndef PLAYLISTWIDGET_H
#define PLAYLISTWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QMenu>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QFileInfo>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

// 媒体项结构
struct MediaInfo {
    QString filePath;
    QString title;
    QString artist;
    QString album;
    qint64 duration;
    QDateTime addTime;
    int playCount;
    bool isFavorite;

    MediaInfo() : duration(0), playCount(0), isFavorite(false) {
        addTime = QDateTime::currentDateTime();
    }

    QString displayName() const {
        if (!title.isEmpty() && !artist.isEmpty()) {
            return QString("%1 - %2").arg(artist, title);
        }
        return title.isEmpty() ? QFileInfo(filePath).baseName() : title;
    }

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["filePath"] = filePath;
        obj["title"] = title;
        obj["artist"] = artist;
        obj["album"] = album;
        obj["duration"] = duration;
        obj["addTime"] = addTime.toString(Qt::ISODate);
        obj["playCount"] = playCount;
        obj["isFavorite"] = isFavorite;
        return obj;
    }

    static MediaInfo fromJson(const QJsonObject &obj) {
        MediaInfo info;
        info.filePath = obj["filePath"].toString();
        info.title = obj["title"].toString();
        info.artist = obj["artist"].toString();
        info.album = obj["album"].toString();
        info.duration = obj["duration"].toVariant().toLongLong();
        info.addTime = QDateTime::fromString(obj["addTime"].toString(), Qt::ISODate);
        info.playCount = obj["playCount"].toInt();
        info.isFavorite = obj["isFavorite"].toBool();
        return info;
    }
};

class PlaylistWidget : public QWidget
{
    Q_OBJECT

public:
    enum PlayMode {
        Sequential = 0,  // 顺序播放
        Loop,           // 列表循环
        Random,         // 随机播放
        RepeatOne       // 单曲循环
    };

    explicit PlaylistWidget(QWidget *parent = nullptr);
    ~PlaylistWidget();
    QStringList getSupportedFormats() const;
    // 播放列表操作
    void addMedia(const QString &filePath);
    void addMediaList(const QStringList &filePaths);
    void removeCurrentItem();
    void clearPlaylist();
    void moveItem(int from, int to);

    // 播放控制
    int currentIndex() const { return m_currentIndex; }
    void setCurrentIndex(int index);
    MediaInfo getCurrentMedia() const;
    MediaInfo getMediaAt(int index) const;
    int getMediaCount() const { return m_mediaList.size(); }

    // 播放模式
    PlayMode getPlayMode() const { return m_playMode; }
    void setPlayMode(PlayMode mode);
    int getNextIndex() const;
    int getPreviousIndex() const;

    // 搜索和过滤
    void searchMedia(const QString &keyword);
    void showFavoritesOnly(bool favOnly);

    // 数据持久化
    void savePlaylist();
    void loadPlaylist();
    void exportPlaylist(const QString &fileName, const QString &format = "m3u");
    void importPlaylist(const QString &fileName);

signals:
    void mediaSelected(int index);
    void playModeChanged(PlayMode mode);
    void playlistChanged();
    void requestPlay();
    void requestNext();
    void requestPrevious();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void onItemDoubleClicked(QListWidgetItem *item);
    void onPlayModeButtonClicked();
    void onSearchTextChanged(const QString &text);
    void onAddFilesClicked();
    void onRemoveItemClicked();
    void onClearPlaylistClicked();
    void showContextMenu(const QPoint &pos);
    void toggleFavorite();
    void showItemProperties();

private:
    // UI组件
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_controlLayout;
    QHBoxLayout *m_buttonLayout;

    QLineEdit *m_searchEdit;
    QPushButton *m_playModeButton;
    QLabel *m_playModeLabel;
    QLabel *m_countLabel;

    QPushButton *m_addFilesButton;
    QPushButton *m_removeButton;
    QPushButton *m_clearButton;

    QListWidget *m_listWidget;
    QMenu *m_contextMenu;

    // 数据成员
    QList<MediaInfo> m_mediaList;
    QList<MediaInfo> m_originalList;  // 搜索前的原始列表
    int m_currentIndex;
    PlayMode m_playMode;
    QList<int> m_randomOrder;  // 随机播放顺序
    int m_randomIndex;
    bool m_showingFavorites;

    // 支持的格式
    QStringList m_supportedFormats;

    // 私有方法
    void setupUI();
    void setupConnections();
    void updateUI();
    void updatePlayModeDisplay();
    void updateItemDisplay(int index);
    void generateRandomOrder();

    bool isMediaFile(const QString &filePath) const;
    void extractMediaInfo(MediaInfo &info);
    QString formatDuration(qint64 duration) const;
};

#endif // PLAYLISTWIDGET_H
