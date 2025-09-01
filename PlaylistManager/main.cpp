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
