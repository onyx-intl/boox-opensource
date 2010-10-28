#include "util.h"

#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>

namespace onyx {
namespace feed_reader {

void ReportDatabaseError(const QSqlQuery& query,
                         const QString& msg) {
    qDebug() << msg;
    qDebug() << "Error processing query: " << query.executedQuery();
    const QSqlError& error(query.lastError());
    qDebug() << error.databaseText();
    qDebug() << error.driverText();
    qDebug() << error.text();
}

}  // namespace feed_reader
}  // namespace onyx
