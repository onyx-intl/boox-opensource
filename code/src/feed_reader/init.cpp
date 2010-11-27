// -*- mode: c++; c-basic-offset: 4; -*-

#include <QFile>
#include <QDir>
#include "init.h"

#include "onyx/sys/sys_status.h"

#include "database.h"
#include "main_widget.h"

namespace onyx {
namespace feed_reader {

MainWidget* init(const InitArgs& args) {
    using onyx::feed_reader::MainWidget;
    sys::SysStatus::instance().setSystemBusy(false);
    bool to_add_default =false;
    QFile file(args.db_path);
    if (! file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        to_add_default = true;
    }
    file.close();

    Database::init(args.db_path);
    // Add default rss
    QStringList feed_list;
    if (to_add_default) {
        //NOTE read $HOME/initial.rss, which is just a plain text
        file.setFileName(QDir::homePath() + "/initial.rss");
        if (! file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug("Fail open initial rss feed");
        }
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            feed_list << line;
        }
    }
    file.close();
    // We don't care the userfeed
    file.setFileName("/media//flash/userfeed.txt");
    if ( file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            feed_list << line;
        }
    }
    // We may add export functionality
    file.close();
    MainWidget* widget = new MainWidget;
    widget->fitToScreen();
    widget->show();
    if (!feed_list.isEmpty()) {
        widget->addFeedlist(feed_list);
    }
    return widget;
}

}  // namespace feed_reader
}  // namespace onyx
