// -*- mode: c++; c-basic-offset: 4; -*-

#ifndef ONYX_FEED_READER_FEED_LIST_MODEL_H__
#define ONYX_FEED_READER_FEED_LIST_MODEL_H__

#include <QAbstractTableModel>
#include <QObject>
#include <QTimer>

#include "onyx/base/base.h"
#include "feed.h"

namespace onyx {
namespace feed_reader {

class Feed;
class FeedFetcher;

class FeedListModel : public QAbstractTableModel {
    Q_OBJECT;

  public:
    enum FeedDataRole {
        FeedIdentifierRole = Qt::UserRole
    };

    // Takes ownership of FeedFetcher.
    FeedListModel(QObject* parent, FeedFetcher*);
    virtual ~FeedListModel();
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(
            const QModelIndex& parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &Value, int role);
    void loadFromDatabase();
    shared_ptr<Feed> getFeed(int id);

    Qt::ItemFlags flags(const QModelIndex &index) const;
  public slots:
    void addFeed(shared_ptr<Feed> feed);
    void addFeed(const QUrl& url);
    void refreshAllFeeds();
    void updateFeed(shared_ptr<Feed> feed);
    void deleteFeeds();
    void insertFeedToDelete(shared_ptr<Feed>  feed);
    void removeFeedToDelete(shared_ptr<Feed>  feed);

  private:
    vector<shared_ptr<Feed> > feeds_;
    scoped_ptr<FeedFetcher> feed_fetcher_;
    QTimer timer_;
};

}  // namespace feed_reader
}  // namespace onyx

#endif  // ONYX_FEED_READER_FEED_LIST_MODEL_H__
