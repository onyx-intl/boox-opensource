// -*- mode: c++; c-basic-offset: 4; -*-

#ifndef ONYX_FEED_READER_ARTICLE_H__
#define ONYX_FEED_READER_ARTICLE_H__

#include <QMetaType>
#include <QString>
#include <QUrl>

#include "onyx/base/base.h"

#include "feed.h"

namespace onyx {
namespace feed_reader {

class Feed;

class Article {
  public:
    explicit Article(shared_ptr<Feed> feed) : feed_(feed), read_(false) {};
    ~Article() {};

    static bool createTableIfNeeded();
    static bool loadByFeed(shared_ptr<Feed> feed,
                           vector<shared_ptr<Article> >* articles);

    const QString& title() const {return title_; };
    const QString& url() const { return url_; }
    const QString& text() const { return text_; }
    const QString& pubdate() const {return pubdate_;}
    bool read() const { return read_; }

    // This article exists in the database and is marked as "is read".
    bool existsInDbAndIsRead();
    // Set the title, stripping any <...> tags.
    void set_title(const QString& title);

    void set_url(const QString& url) { url_ = url; }
    void set_text(const QString& text) { text_ = text; }
    void set_read(bool read) { read_ = read; }
    void set_pubdate(const QString& pubdate);
    bool saveOrUpdate();

  private:
    shared_ptr<Feed> feed_;
    QString title_;
    QString url_;
    QString text_;
    QString pubdate_;
    int id_;
    bool read_;
    NO_COPY_AND_ASSIGN(Article);
};

}  // namespace feed_reader
}  // namespace onyx

// Add QVariant support.
Q_DECLARE_METATYPE(shared_ptr<onyx::feed_reader::Article>)

#endif  // ONYX_FEED_READER_ARTICLE_H__
