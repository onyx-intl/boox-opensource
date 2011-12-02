#include <QApplication>

#include "onyx/ui/ui.h"
#include "onyx/ui/languages.h"
#include "onyx/ui/calendar.h"
#include "onyx/screen/screen_update_watcher.h"
#include "onyx/sys/sys.h"

using namespace ui;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    ui::loadTranslator (QLocale::system().name());

    Calendar calendar(0);
    calendar.showMaximized();
    sys::SysStatus::instance().setSystemBusy(false);
    return app.exec();
}
