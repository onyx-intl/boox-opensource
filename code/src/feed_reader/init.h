// -*- mode: c++; c-basic-offset: 4; -*-

#ifndef ONYX_FEED_READER_INIT_H__
#define ONYX_FEED_READER_INIT_H__

#include <QString>

namespace onyx {
namespace feed_reader {

class MainWidget;

struct InitArgs {
    QString db_path;
};

MainWidget* init(const InitArgs& args);

}  // namespace feed_reader
}  // namespace onyx

#endif  // ONYX_FEED_READER_INIT_H__
