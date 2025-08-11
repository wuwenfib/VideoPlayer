#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 应用信息设置
    app.setApplicationName("VideoPlayer");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("YourCompany");

    // 设置应用图标
    app.setWindowIcon(QIcon(":/Weixin Image_20250811153509.png"));

    // 创建主窗口
    MainWindow window;
    window.show();

    return app.exec();
}
