// -*- mode: c++; c-basic-offset: 4; -*-

#include "main_widget.h"

#include <QtGui>

#include "onyx/screen/screen_proxy.h"
#include "onyx/ui/status_bar.h"

#include "add_feed_dialog.h"
#include "article_list_model.h"
#include "article_list_page.h"
#include "article_page.h"
#include "feed.h"
#include "feed_fetcher.h"
#include "feed_list_model.h"
#include "feeds_page.h"
#include "rss_feed_parser.h"
#include "singleton.h"
#include "widget_updater.h"
#include "onyx/ui/system_actions.h"
#include "onyx/ui/menu.h"
#include "onyx/sys/sys_status.h"
namespace onyx {
namespace feed_reader {
namespace {
static const QString STYLE ="\
QTabBar::tab\
        {\
        background: white;  \
        font-size: 24px;    \
        border: 2px solid gray;\
        border-top-left-radius: 4px;    \
        border-top-right-radius: 4px;   \
        padding: 2px;       \
        border-bottom-color: black; \
        min-height: 32px;   \
        min-width: 32px     \
        }\
QTabBar::tab:hover \
        {\
        background: white;  \
        font-size: 24px;    \
        border: 2px solid gray;\
        border-top-left-radius: 4px;    \
        border-top-right-radius: 4px;   \
        padding: 2px;       \
        border-bottom-color: gray; \
        border-top-color: black;    \
        min-height: 32px;   \
        min-width: 32px     \
        }\
QTabBar::tab:selected\
           {\
               border-color: black;  \
               border-bottom-color: white;  \
               top: 6px;\
           }\
QTabBar::tab:first:selected\
           {\
            margin-left: 0; \
           }\
QTabBar::tab:last:selected\
           {\
            margin-right: 0;    \
           }\
QTabBar::tab:only-one {\
            margin: 0;  \
          }\
QWidget                                       \
           {                                  \
            background: white;                \
            font-size: 24px;                  \
            border-style: solid;              \
            padding: 0px;                     \
           }\
QWidget:focus                                 \
           {                                  \
            background: white;                \
            font-size: 24px;                  \
            border-style: solid;              \
            padding: 0px;                     \
           }\
QWidget:disabled                              \
           {                                  \
            background: white;                \
            font-size: 24px;                  \
            border-style: solid;              \
            padding: 0px;                     \
           }\
QTabWidget                                    \
           {                                  \
            border-width: 1px;\
            border-color: black;    \
            background: white;                \
            font-size: 24px;                  \
            border-style: solid;              \
            padding: 0px;                     \
            min-height: 32px;                 \
           }\
QTabWidget:tab-bar                            \
           {                                  \
             alignment: center;                \
           }\
QTabWidget::pane                               \
           {                                  \
               border-top: 1px solid black;    \
           }";


class TabBar : public QTabBar {
    public:
        TabBar(QWidget *parent = 0);
    protected:
         void mouseMoveEvent(QMouseEvent *event) {
         //do nothing here
         }
};

TabBar::TabBar(QWidget *parent)
        : QTabBar(parent) {
     setFocusPolicy(Qt::TabFocus);
     //setStyleSheet(STYLE);
        }


class  TabWidget : public  QTabWidget{
    public:
        TabWidget(QWidget* parent = 0);
    protected:
        void mouseMoveEvent(QMouseEvent *event) {
            //do nothing here
        }
};

TabWidget::TabWidget(QWidget* parent)
        : QTabWidget(parent) {
        clear();
        setTabBar(new TabBar(this));
        setStyleSheet(STYLE);
        }
}

MainWidget::MainWidget(QWidget* parent)
        : QWidget(parent, Qt::FramelessWindowHint),
          feed_list_model_(new FeedListModel(
                                   this, new FeedFetcher(new RssFeedParser))),
          article_list_model_(new ArticleListModel(this)),
          // Just parent all widgets to this for now. They will be
          // reparented by the layout anyway.
          tab_widget_(new TabWidget(this)),
          feeds_page_(new FeedsPage(feed_list_model_, this)),
          article_list_page_(new ArticleListPage(article_list_model_, this)),
          article_page_(new ArticlePage(this)) {
    using onyx::screen::ScreenProxy;
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);
    QLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(1, 0, 1, 0);
    layout->addWidget(tab_widget_);
    tab_widget_->setAutoFillBackground(true);
    tab_widget_->setBackgroundRole(QPalette::Base);
    tab_widget_->addTab(feeds_page_, tr("Feeds"));
    tab_widget_->addTab(article_list_page_, tr("Items"));
    tab_widget_->addTab(article_page_, tr("Article"));
    ui::StatusBar* status_bar = new ui::StatusBar(
            this, ui::MENU| ui::MESSAGE|ui::BATTERY|ui::CONNECTION|ui::CLOCK|
            ui::SCREEN_REFRESH);
    layout->addWidget(status_bar);
    setLayout(layout);

    connect(feeds_page_, SIGNAL(feedActivated(int)),
            this, SLOT(displayItemListForUrl(int)));
    connect(article_list_page_, SIGNAL(articleActivated(shared_ptr<Article>)),
            this, SLOT(displayArticle(shared_ptr<Article>)));
    connect(tab_widget_, SIGNAL(currentChanged(int)),
            this, SLOT(updateTab(int)));
    connect(status_bar, SIGNAL(menuClicked()), this, SLOT(showContextMenu()));
    // refresh feeds in 3 seconds at start
    feed_list_model_->loadFromDatabase();
    WidgetUpdater& updater(Singleton<WidgetUpdater>::instance());
    updater.addWidget(this, ScreenProxy::GU);
    updater.addWidget(tab_widget_, ScreenProxy::GU);
    updater.addWidget(feeds_page_, ScreenProxy::GU);
    updater.addWidget(article_list_page_, ScreenProxy::GU);
    updater.addWidget(article_page_, ScreenProxy::GU);
}

MainWidget::~MainWidget() {}

void MainWidget::updateTab(int index) {
    qDebug() << "Updating tab " << index;
    QWidget* w = tab_widget_->widget(index);
    Singleton<WidgetUpdater>::instance().postDelayedUpdate(w);
    Singleton<WidgetUpdater>::instance().postDelayedUpdate(tab_widget_);
}

void MainWidget::fitToScreen() {
    // SCREEN_WIDTH and SCREEN_HEIGHT are set to proper values by the
    // build environment.
    #ifdef BUILD_FOR_ARM
        #define SCREEN_WIDTH -1
        #define SCREEN_HEIGHT -1
    #else
        #define SCREEN_WIDTH 600
        #define SCREEN_HEIGHT 800
    #endif
    int width = SCREEN_WIDTH;
    int height = SCREEN_HEIGHT;
    if (width <= 0 || height <= 0) {
        QRect screen_rect = QApplication::desktop()->screenGeometry();
        width = screen_rect.width();
        height = screen_rect.height();
    }
    resize(width, height);
}

void MainWidget::displayItemListForUrl(int id) {
    tab_widget_->setCurrentIndex(1);
    article_list_model_->switchToFeed(feed_list_model_->getFeed(id));
}

void MainWidget::displayArticle(shared_ptr<Article> article) {
    tab_widget_->setCurrentIndex(2);
    article_page_->displayArticle(article);
}


void MainWidget::keyPressEvent (QKeyEvent* e) {
    if (e->key() == Qt::Key_Escape)
        qApp->quit();
    return;
}

void MainWidget::showContextMenu() {
    using namespace ui;
    PopupMenu menu(this);
    SystemActions sy;
    sy.generateActions();
    menu.setSystemAction(&sy);
    if (menu.popup() != QDialog::Accepted) {
      QApplication::processEvents();
      return;;
    }
    QAction * group = menu.selectedCategory();
    if( group == sy.category()) {
	SystemAction sy_tmp = sy.selected();
	switch (sy_tmp) {
	  case RETURN_TO_LIBRARY: {
	      qApp->quit();
	      break;
	  }
	  case ROTATE_SCREEN: {
	      onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::INVALID);
	      sys::SysStatus::instance().rotateScreen();
	      break;
	  }
	  default:
	    break;
	}
    }
}

}  // namespace feed_reader
}  // namespace onyx
