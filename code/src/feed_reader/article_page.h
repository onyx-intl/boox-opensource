// -*- mode: c++; c-basic-offset: 4; -*-

#ifndef ONYX_FEED_READER_ARTICLE_PAGE_H__
#define ONYX_FEED_READER_ARTICLE_PAGE_H__

#include <QWidget>

#include "onyx/base/base.h"

#include "article.h"

class QWebView;

namespace onyx {
namespace feed_reader {

class ArticlePage : public QWidget {
    Q_OBJECT;
  public:
    explicit ArticlePage(QWidget* parent = NULL);
    virtual ~ArticlePage();

    void displayArticle(shared_ptr<Article> article);

  private:
    QWebView* web_view_;

    NO_COPY_AND_ASSIGN(ArticlePage);
};

}  // namespace feed_reader
}  // namespace onyx

#endif  // ONYX_FEED_READER_ARTICLE_PAGE_H__
