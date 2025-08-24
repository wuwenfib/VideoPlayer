// main.cpp
#include <QApplication>
#include "VideoPlayerMainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    VideoPlayerMainWindow window;
    window.show();

    return app.exec();
}
