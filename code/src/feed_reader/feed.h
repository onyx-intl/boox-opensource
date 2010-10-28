// -*- mode: c++; c-basic-offset: 4; -*-

#ifndef ONYX_FEED_READER_FEED_H__
#define ONYX_FEED_READER_FEED_H__

#include <QString>
#include <QUrl>

#include "onyx/base/base.h"
#include "onyx/base/macros.h"

class QSqlQuery;

namespace onyx {
namespace feed_reader {

class Article;

// This is a model class for RSS feeds.
class Feed {
  public:
    Feed();
    ~Feed();

    int unreadCount();

    int id() const {
        return id_;
    }

    const QString& title() const {
        return title_;
    }

    // Set the title, stripping any <...> tags.
    void set_title(const QString& title);

    const QUrl& site_url() const {
        return site_url_;
    }

    void set_site_url(const QUrl& url) {
        site_url_ = url;
    }

    void set_site_url(const QString& url) {
        site_url_ = QUrl(url);
    }
    const QUrl& feed_url() const {
        return feed_url_;
    }

    void set_feed_url(const QUrl& url) {
        feed_url_ = url;
    }

    void set_feed_url(const QString& url) {
        feed_url_ = QUrl(url);
    }

    const vector<shared_ptr<Article> >& articles();
    vector<shared_ptr<Article> >* mutable_articles();

    bool saveNew();
    bool update();
    bool saveArticles();
    static Feed* loadByUrl(const QString& url);
    static int count();
    static bool createTable();

    const bool to_delete() {
        return to_delete_;
    }
    void set_to_delete(const bool to_delete) {
        to_delete_ = to_delete;
    }
    // Load all feeds from the database. Return false if any error
    // occured. The contents of feeds will be cleared if the operation succeeds.
    // But if an error occured, the feeds argument is not modified.
    static bool all(vector<shared_ptr<Feed> >* feeds);
    // remove feed;
    bool remove();

  private:
    void initializeFromQuery(QSqlQuery* query);
    QString title_;
    QUrl site_url_;
    QUrl feed_url_;
    int id_;
    vector<shared_ptr<Article> > articles_;

    bool to_delete_;
    NO_COPY_AND_ASSIGN(Feed);
};

}  // namespace feed_reader
}  // namespace onyx

#endif  // ONYX_FEED_READER_FEED_H__
