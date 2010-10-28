// -*- mode: c++; c-basic-offset: 4; -*-

#include <QApplication>
#include <QDir>
#include <QEvent>

#include "onyx/base/base.h"

#include "init.h"
#include "main_widget.h"
#include "widget_updater.h"

#if defined(QT_ARCH_ARM) && !defined(BUILD_FOR_ARM)
#error "Building for x86 but QT_ARCH_ARM is set!"
#endif

#if !defined(QT_ARCH_ARM) && defined(BUILD_FOR_ARM)
#error "Building for ARM but QT_ARCH_ARM is not set!"
#endif

int main(int argc, char *argv[])
{
    using namespace onyx::feed_reader;
    DELAYED_UPDATE_EVENT_TYPE = static_cast<QEvent::Type>(
            QEvent::registerEventType(QEvent::User + 1));
    QApplication app(argc, argv);
    QApplication::setFont(QFont("Sans Serif", 20));
    InitArgs args;
    // for development
    args.db_path = "/tmp/.onyx_feed_reader.db";
#if defined(BUILD_FOR_ARM)
    args.db_path = "/media/flash/.onyx_feed_reader.db";
#endif
    scoped_ptr<MainWidget> mw(init(args));
    return app.exec();
}
