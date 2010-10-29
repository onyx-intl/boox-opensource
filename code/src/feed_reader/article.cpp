// -*- mode: c++; c-basic-offset: 4; -*-

#include "article.h"

#include <QDebug>
#include <QRegExp>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>

#include "onyx/base/base.h"

#include "database.h"
#include "feed.h"
#include "util.h"

namespace onyx {
namespace feed_reader {

// static
bool Article::createTableIfNeeded() {
    shared_ptr<Database> db(Database::getShared());
    QSqlQuery query;
    return query.exec("CREATE TABLE IF NOT EXISTS articles"
                      "(id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      " title, pubdate, url UNIQUE NOT NULL, text,"
                      " feed_id NOT NULL, is_read);");
}

// static
bool Article::loadByFeed(shared_ptr<Feed> feed,
                         vector<shared_ptr<Article> >* articles) {
    createTableIfNeeded();
    qDebug() << "Loading articles for feed: " << feed->feed_url();
    shared_ptr<Database> db(Database::getShared());
    QSqlQuery query("SELECT * FROM articles WHERE feed_id = :feed_id "
                    "ORDER BY id DESC");
    query.addBindValue(feed->id());
    if (!query.exec()) {
        ReportDatabaseError(query);
        return false;
    }
    qDebug() << query.size() << " articles loaded from database.";
    articles->clear();
    while (query.next()) {
        // FIXME: Only need feed_id
        shared_ptr<Article> article(new Article(feed));
        article->set_title(query.value(query.record().indexOf("title"))
                           .toString());
        article->set_pubdate(query.value(query.record().indexOf("pubdate"))
                           .toString());
        article->set_url(query.value(query.record().indexOf("url"))
                         .toString());
        article->set_text(query.value(query.record().indexOf("text"))
                          .toString());
        article->id_ = query.value(query.record().indexOf("id")).toInt();
        article->read_ =
                query.value(query.record().indexOf("is_read")).toBool();
        articles->push_back(article);
    }
    return true;
}

bool Article::existsInDbAndIsRead() {
    shared_ptr<Database> db(Database::getShared());
    QSqlQuery query("SELECT * FROM articles WHERE url = :url "
                    "ORDER BY id DESC");
    query.addBindValue(url_);
    if (!query.exec()) {
        ReportDatabaseError(query);
        return false;
    }
    if (query.next()) {
        return query.value(query.record().indexOf("is_read")).toBool();
    } else {
        return false;
    }
}

void Article::set_title(const QString& title) {
    title_ = title;
    title_.remove(QRegExp("<.*>"));
}

void Article::set_pubdate(const QString& pubdate)
{
    pubdate_ = pubdate;
}


bool Article::saveOrUpdate() {
    createTableIfNeeded();
    shared_ptr<Database> db(Database::getShared());
    QSqlQuery query;
    query.prepare("INSERT OR REPLACE INTO articles (title, pubdate, url, text, feed_id, "
                  "is_read)"
                  " VALUES (:title, :pubdate, :url, :text, :feed_id, :is_read)");
    query.addBindValue(title_);
    query.addBindValue(pubdate_);
    query.addBindValue(url_);
    query.addBindValue(text_);
    query.addBindValue(feed_->id());
    query.addBindValue(read_);
    if (!query.exec()) {
        ReportDatabaseError(query);
        return false;
    }
    return true;
}

}  // namespace feed_reader
}  // namespace onyx
