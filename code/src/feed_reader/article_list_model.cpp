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

    if (index.column() == 0) {
        if (role == Qt::DisplayRole) {
            QString title = article->title();
            title.trimmed();
            title += QString("...");
            title.truncate(30);
            title += QString("...");
            title.truncate(30);
            return title;
        } else if (role == ArticleIdentifierRole) {
            QString url = article->url();
            url.trimmed();
            url += QString("...");
            url.truncate(30);
            url += QString("...");
            url.truncate(30);
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
    } else if (index.column() == 1) {
        if (role == Qt::EditRole){
            if(article->read()) {
                return true;
            } else {
                return false;
            }
        } else if (role == Qt::CheckStateRole) {
            if(article->read()) {
                return Qt::Checked;
            } else {
                return Qt::Unchecked;
            }
        }
    }
    return QVariant();
}

void ArticleListModel::switchToFeed(shared_ptr<Feed> feed) {
    Article::loadByFeed(feed, &articles_);
    reset();
}

bool ArticleListModel::setData(const QModelIndex& index, const QVariant& Value, int role)
{
    if (index.column() == 1 && role == Qt::CheckStateRole) {
        if (Value == Qt::Checked) {
            articles_.at(index.row())->set_read(true);
        } else {
            articles_.at(index.row())->set_read(false);
        }
            // modify database
        //articles_.at(index.row())->saveOrUpdate();
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags ArticleListModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) {
        return 0;
    }
    if (index.column() == 1) return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
    return Qt::ItemIsEditable;
}

}  // namespace feed_reader
}  // namespace onyx
