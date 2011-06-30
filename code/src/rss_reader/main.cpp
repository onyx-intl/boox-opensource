#include "rss_reader_view.h"
#include "onyx/ui/languages.h"
#include "onyx/sys/sys_status.h"
#include "onyx/screen/screen_update_watcher.h"

#include "rssFeedInfo.h"
#include "status.h"

using namespace rss_reader;

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setApplicationName("rss_reader");

    //-- Register signals
    qRegisterMetaType<CRSSFeedInfo>();
    qRegisterMetaType<CStatus>();
        
    Q_INIT_RESOURCE(rss_images);
    Q_INIT_RESOURCE(onyx_ui_images);
    Q_INIT_RESOURCE(wifi_images);
    
    ui::loadTranslator(QLocale::system().name());

    sys::SysStatus::instance().setSystemBusy(false);
    onyx::screen::instance().enableUpdate(true);

    RssReaderView view(0);
    view.show();
    onyx::screen::instance().flush(&view, onyx::screen::ScreenProxy::GC);

    return app.exec();
}
