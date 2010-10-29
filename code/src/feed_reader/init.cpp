// -*- mode: c++; c-basic-offset: 4; -*-

#include "init.h"

#include "onyx/sys/sys_status.h"

#include "database.h"
#include "main_widget.h"

namespace onyx {
namespace feed_reader {

MainWidget* init(const InitArgs& args) {
    using onyx::feed_reader::MainWidget;
    sys::SysStatus::instance().setSystemBusy(false);
    Database::init(args.db_path);
    MainWidget* widget = new MainWidget;
    widget->fitToScreen();
    widget->show();
    return widget;
}

}  // namespace feed_reader
}  // namespace onyx
