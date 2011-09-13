#include <QApplication>

#include "onyx/ui/clock_dialog.h"
#include "onyx/screen/screen_update_watcher.h"
#include "onyx/sys/sys.h"

using namespace ui;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    FullScreenClock clock(0);
    clock.showFullScreen();
    sys::SysStatus::instance().setSystemBusy(false);
    return app.exec();
}
