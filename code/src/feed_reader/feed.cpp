// -*- mode: c++; c-basic-offset: 4; -*-

#include "feed.h"

#include <QDebug>
#include <QRegExp>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>

#include "onyx/base/base.h"

#include "article.h"
#include "database.h"
#include "util.h"

namespace onyx {
namespace feed_reader {

// static
bool Feed::createTable() {
    shared_ptr<Database> db(Database::getShared());
    QSqlQuery query;
    return query.exec("CREATE TABLE IF NOT EXISTS feeds"
                      "(id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      " title, site_url, feed_url UNIQUE NOT NULL);");
}

// static
bool Feed::all(vector<shared_ptr<Feed> >* feeds) {
    createTable();
    shared_ptr<Database> db(Database::getShared());
    QSqlQuery query;
    query.prepare("SELECT * FROM feeds;");
    if (!query.exec()) {
        ReportDatabaseError(query, "Error retrieving all feeds");
        return false;
    }
    feeds->clear();

    while (query.next()) {
        shared_ptr<Feed> feed(new Feed);
        feed->initializeFromQuery(&query);
        feeds->push_back(feed);
        qDebug() << "Number of feeds: " << feeds->size();
    }
    return true;
}

Feed::Feed() : title_(), site_url_(), feed_url_(), id_(0), articles_(), to_delete_(0) {}

Feed::~Feed() {}

void Feed::set_title(const QString& title) {
    title_ = title;
    title_.remove(QRegExp("<.*>"));
}

const vector<shared_ptr<Article> >& Feed::articles() {
    return articles_;
}

vector<shared_ptr<Article> >* Feed::mutable_articles() {
    return &articles_;
}

bool Feed::saveArticles() {
    for (int i = articles_.size()-1; i >= 0; --i) {
        shared_ptr<Article> article(articles_[i]);
        // TODO: Perhaps we should skip saving articles that already
        // exists?
        if (article->existsInDbAndIsRead()) {
            article->set_read(true);
        }
        if (!article->saveOrUpdate()) {
            qDebug() << "Failed to save article.";
        }
    }
    return true;
}

bool Feed::saveNew() {
    createTable();
    shared_ptr<Database> db(Database::getShared());
    QSqlQuery query;
    if (!query.prepare("INSERT INTO feeds (title, site_url, feed_url) "
                       "VALUES (:title, :site_url, :feed_url)")) {
        ReportDatabaseError(query, "Error preparing to save new feeds");
        return false;
    }
    query.addBindValue(title_);
    query.addBindValue(site_url_.toString());
    query.addBindValue(feed_url_.toString());
    if (!query.exec()) {
        ReportDatabaseError(query, "Error saving new feeds");
        return false;
    }
    query.prepare("SELECT id FROM feeds WHERE feed_url=:feed_url");
    query.addBindValue(feed_url_.toString());
    if (!query.exec()) {
        ReportDatabaseError(query, "Error refreshing feed after saving");
        return false;
    }
    if (query.next()) {
        id_ = query.value(query.record().indexOf("id")).toInt();
    } else {
        return false;
    }
    return true;
}

bool Feed::update() {
    shared_ptr<Database> db(Database::getShared());
    {
        QSqlQuery query;
        if (!query.prepare("UPDATE feeds SET "
                           "title=:title, site_url=:site_url "
                           "WHERE feed_url=:feed_url")) {
            ReportDatabaseError(query, "Error preparing to update feed.");
            return false;
        }
        query.addBindValue(title_);
        query.addBindValue(site_url_.toString());
        query.addBindValue(feed_url_.toString());
        if (!query.exec()) {
            ReportDatabaseError(query, "Error updating feed.");
            return false;
        }
        return true;
    }
}

int Feed::unreadCount() {
    Article::createTableIfNeeded();
    shared_ptr<Database> db(Database::getShared());
    {
        QSqlQuery query;
        if(!query.prepare("select count(id) from articles "
                             "where feed_id=:feed_id and "
                          "is_read='false'")) {
            ReportDatabaseError(query, "Error preparing statement when getting"
                                " unread count");
            return 0;
        }
        query.addBindValue(id_);
        if (!query.exec()) {
            ReportDatabaseError(query, "Error getting unread count");
            return 0;
        }
        if (query.next()) {
            return query.record().field(0).value().toInt();
        } else {
            qDebug() << "BUG: query.next() returned false";
            return 0;
        }
    }
}

Feed* Feed::loadByUrl(const QString& url) {
    shared_ptr<Database> db(Database::getShared());
    QSqlQuery query;
    query.prepare("SELECT * FROM feeds WHERE feed_url=:feed_url");
    query.addBindValue(url);
    if (!query.exec()) {
        ReportDatabaseError(query, "Error loading feed by url.");
        return NULL;
    }
    if (query.next()) {
        scoped_ptr<Feed> feed(new Feed);
        feed->initializeFromQuery(&query);
        return feed.release();
    } else {
        qDebug() << "BUG: query.next() returned false";
        return NULL;
    }
}

void Feed::initializeFromQuery(QSqlQuery* query) {
    id_ = query->value(query->record().indexOf("id"))
            .toInt();
    set_title(query->value(query->record().indexOf("title"))
              .toString());
    set_site_url(query->value(query->record().indexOf("site_url"))
                 .toString());
    set_feed_url(query->value(query->record().indexOf("feed_url"))
                 .toString());
}

//static
int Feed::count() {
    shared_ptr<Database> db(Database::getShared());
    QSqlQuery query;
    if (!query.exec("SELECT count(id) FROM feeds;")) {
        ReportDatabaseError(query, "Error getting feed count.");
        return 0;
    }
    if (query.next()) {
        return query.record().field(0).value().toInt();
    } else {
        qDebug() << "BUG: no result returned.";
        return 0;
    }
}

bool Feed::remove() {
    shared_ptr<Database> db(Database::getShared());
    QSqlQuery query;

    //delete articles
    if (!query.prepare("DELETE FROM articles WHERE feed_id=:feed_id")) {
        ReportDatabaseError(query, "Error preparing to delete feeds");
        return false;
    }

    query.addBindValue(id_);

    if (!query.exec()) {
        ReportDatabaseError(query, "Error deleting articles");
        return false;
    }

    if (!query.prepare("DELETE FROM feeds WHERE id=:id")) {
        ReportDatabaseError(query, "Error preparing to delete feeds");
        return false;
    }

    query.addBindValue(id_);

    if (!query.exec()) {
        ReportDatabaseError(query, "Error deleting  feeds");
        return false;
    }

    return true;
}
}  // namespace feed_reader
}  // namespace onyx
