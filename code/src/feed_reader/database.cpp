// -*- mode: c++; c-basic-offset: 4; -*-

#include "database.h"

#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QString>

namespace onyx {
namespace feed_reader {

QString Database::path_;

void Database::init(const QString& path) {
    Database::path_ = path;
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(Database::path_);
}

shared_ptr<Database> Database::getShared() {
    return shared_ptr<Database>(new Database);
}

Database::Database()
        : db_(QSqlDatabase::database()) {
    if (!db_.isOpen()) {
        qDebug() << "Cannot open: " << Database::path_;
        QMessageBox::critical(
                0,
                QApplication::tr("Cannot open database"),
                QApplication::tr(
                        "Unable to create or open the feeds database.\n"
                        "Click Cancel to exit."),
                QMessageBox::Cancel);
    }
}

Database::~Database() {
    db_.close();
    db_ = QSqlDatabase();  // Decrease ref count.
}

}  // namespace feed_reader
}  // namespace onyx
