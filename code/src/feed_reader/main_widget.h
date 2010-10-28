// -*- mode: c++; c-basic-offset: 4; -*-

#ifndef ONYX_FEED_READER_H__
#define ONYX_FEED_READER_H__

#include <QtGui>

#include "onyx/base/base.h"
#include "article.h"
#include "feed.h"
#include "main_widget.h"

namespace onyx {
namespace feed_reader {

class AddFeedDialog;
class ArticleListModel;
class ArticleListPage;
class ArticlePage;
class FeedListModel;
class FeedsPage;

// The main widget of the application. It has a TabWidget with two
// tabs: the feed list tab and the article tab.
class MainWidget : public QWidget
{
    Q_OBJECT;
  public:
    MainWidget(QWidget* parent = 0);
    virtual ~MainWidget();

    void fitToScreen();
    protected:
        void keyPressEvent(QKeyEvent *e);
  private slots:
    void displayItemListForUrl(int id);
    void displayArticle(shared_ptr<Article> article);
    void updateTab(int index);

  private:
    friend class AcceptanceTest;

    // TODO(hjiang): refactor using the pImpl idiom.
    FeedListModel* feed_list_model_;
    ArticleListModel* article_list_model_;
    QTabWidget* tab_widget_;
    FeedsPage* feeds_page_;
    ArticleListPage* article_list_page_;
    ArticlePage* article_page_;

    NO_COPY_AND_ASSIGN(MainWidget);
};


}  // namespace feed_reader
}  // namespace onyx

#endif  // ONYX_FEED_READER_H__
