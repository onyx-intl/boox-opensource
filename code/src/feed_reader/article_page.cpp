// -*- mode: c++; c-basic-offset: 4; -*-

#include "article_page.h"

#include <QDebug>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QWebView>

#include "onyx/screen/screen_proxy.h"

#include "article.h"
#include "singleton.h"
#include "widget_updater.h"

namespace onyx {
namespace feed_reader {

ArticlePage::ArticlePage(QWidget* parent)
        : QWidget(parent),
          web_view_(new QWebView(this)) {
    QSizePolicy size_policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    web_view_->setSizePolicy(size_policy);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(web_view_);
    setLayout(layout);

    using onyx::screen::ScreenProxy;
    WidgetUpdater& updater(Singleton<WidgetUpdater>::instance());
    updater.addWidget(web_view_, ScreenProxy::GU);
}

ArticlePage::~ArticlePage() {
}

void ArticlePage::displayArticle(shared_ptr<Article> article) {
    web_view_->setHtml(article->text());
}

}  // namespace feed_reader
}  // namespace onyx
