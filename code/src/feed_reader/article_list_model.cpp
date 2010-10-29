// -*- mode: c++; c-basic-offset: 4; -*-

#include "article_list_model.h"

#include <QDebug>
#include <QFont>
#include <QIcon>

#include "article.h"

namespace onyx {
namespace feed_reader {

ArticleListModel::ArticleListModel(QObject* parent)
        : QAbstractTableModel(parent) {
}

ArticleListModel::~ArticleListModel() {
}

int ArticleListModel::rowCount(const QModelIndex &parent) const {
    return articles_.size();
}

QVariant ArticleListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    if (index.row() >= static_cast<int>(articles_.size()))
        return QVariant();

    shared_ptr<Article> article = articles_.at(index.row());

    if (index.column() == 1) {
        if (role == Qt::DisplayRole) {
            QString title = article->title();
            title.truncate(32);
            title += QString("...");
            return title;
        } else if (role == ArticleIdentifierRole) {
            QString url = article->url();
            url.truncate(32);
            url += QString("...");
            return url;
        } else if (role == ArticleDisplayRole) {
            return QVariant::fromValue(article);
        } else if (role == Qt::FontRole) {
            if (article->read()) {
                return QVariant::fromValue(QFont("Mono", 14));
            } else {
                return QVariant::fromValue(QFont("Mono", 14, QFont::Bold));
            }
        }
    }

    if (index.column() == 0) {
        if (article->read()) {
            switch (role) {
                case Qt::DisplayRole: {
                    QString pubdate =  article->pubdate();
                    pubdate.truncate(25);
                    return pubdate;
                }
                case Qt::FontRole:
                    return QVariant::fromValue(QFont("Mono", 8));
                case Qt::BackgroundColorRole:
                    return QColor(255, 255, 255);
                case  Qt::ForegroundRole:
                    return QColor(0, 0, 0);
                case Qt::CheckStateRole:
                    return Qt::Checked;
            }
        } else {
            switch (role) {
                case Qt::DisplayRole: {
                    QString pubdate =  article->pubdate();
                    pubdate.truncate(25);
                    return pubdate;
                }
                case Qt::FontRole:
                    return QVariant::fromValue(QFont("Mono", 8, QFont::Bold));
                case Qt::BackgroundColorRole:
                    return QColor(0, 0, 0);
                case  Qt::ForegroundRole:
                    return QColor(255, 255, 255);
                case Qt::CheckStateRole:
                    return Qt::Unchecked;
            }
        }
    }
    return QVariant();
}


bool ArticleListModel::setData(const QModelIndex& index, const QVariant& Value, int role)
{
    if (index.column() == 0/* && role == Qt::CheckStateRole*/) {
        if (Value == Qt::Checked) {
            articles_.at(index.row())->set_read(true);
        } else {
            articles_.at(index.row())->set_read(false);
        }
        // modify database
        articles_.at(index.row())->saveOrUpdate();
        return true;
    }
    return false;
}

void ArticleListModel::switchToFeed(shared_ptr<Feed> feed) {
    Article::loadByFeed(feed, &articles_);
    reset();
}

Qt::ItemFlags ArticleListModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) {
        return 0;
    }
    if (index.column()==0) return  Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
    return Qt::ItemIsEnabled;// | Qt::ItemIsSelectable;
}


}  // namespace feed_reader
}  // namespace onyx
