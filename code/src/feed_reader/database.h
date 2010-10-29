// -*- mode: c++; c-basic-offset: 4; -*-

#ifndef ONYX_FEED_READER_DATABASE_H__
#define ONYX_FEED_READER_DATABASE_H__

#include <QSqlDatabase>
#include <QString>

#include "onyx/base/base.h"

namespace onyx {
namespace feed_reader {

class Database {
  public:
    // Set path to the database file.
    static void init(const QString& path);
    // Open a shared connection, close it when the last instance goes
    // out of scope. Call this before running a query.
    static shared_ptr<Database> getShared();

    ~Database();
  private:
    static QString path_;

    Database();
    QSqlDatabase db_;
    NO_COPY_AND_ASSIGN(Database);
};

}  // namespace feed_reader
}  // namespace onyx

#endif  // ONYX_FEED_READER_DATABASE_H__
