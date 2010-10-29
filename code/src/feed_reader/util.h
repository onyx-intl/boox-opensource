#ifndef ONYX_FEED_READER_UTIL_H__
#define ONYX_FEED_READER_UTIL_H__

#include <QString>

class QSqlQuery;

namespace onyx {
namespace feed_reader {

void ReportDatabaseError(const QSqlQuery& query, const QString& msg = "");

}  // namespace feed_reader
}  // namespace onyx

#endif  // ONYX_FEED_READER_UTIL_H__
