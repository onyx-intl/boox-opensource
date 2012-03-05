#include <QtGui>

#include "onyx/sys/sys_status.h"
#include "ds_application.h"



int main(int argc, char * argv[])
{
    DSApplication app(argc, argv);
    DSApplicationAdaptor adaptor(&app);

    Q_INIT_RESOURCE(wifi_images);
    Q_INIT_RESOURCE(res);

    sys::SysStatus::instance().setSystemBusy(false);
    sys::SysStatus::instance().enableIdle(false);
    adaptor.start();
    int ret = app.exec();
    sys::SysStatus::instance().enableIdle(true);
    return ret;
}
