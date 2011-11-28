#include <QApplication>
#include "gomoku_widget.h"
#include "main_widget.h"

#include <QLocale>
#include <QTranslator>
#include "onyx/base/base.h"

#include "onyx/screen/screen_proxy.h"
#include "onyx/screen/screen_update_watcher.h"
#include "onyx/sys/sys_status.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setApplicationName("gomoku");
    QTranslator translator;
    translator.load(":/gomoku_" + QLocale::system().name());
    app.installTranslator(&translator);

    sys::SysStatus::instance().setSystemBusy(false);
    MainWidget widget;

    widget.showFullScreen();
    onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GC);
    return app.exec();
}
