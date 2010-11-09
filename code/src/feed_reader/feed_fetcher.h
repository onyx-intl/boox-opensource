// -*- mode: c++; c-basic-offset: 4; -*-

#ifndef ONYX_FEED_READER_FEED_FETCHER
#define ONYX_FEED_READER_FEED_FETCHER

#include <QObject>

#include "onyx/base/base.h"
#include "onyx/base/shared_ptr.h"
#include "feed.h"

class QHttpResponseHeader;

namespace onyx {
namespace feed_reader {

class FeedParser;

class FeedFetcher : public QObject {
    Q_OBJECT;
  public:
    explicit FeedFetcher(FeedParser* parser);
    virtual ~FeedFetcher();

  public slots:
    virtual void scheduleFetch(shared_ptr<Feed> feed);

  signals:
    void feedUpdated(shared_ptr<Feed>);
    void networkError();

  private slots:
    void readData(const QHttpResponseHeader& response_header);
    void finishFetch(int id, bool error);

  private:
    void startFetch();
    void parseXml();
    QByteArray xmlAtomValidator(const QByteArray&);
    // Reinitialize the state of the object. Called before starting to
    // fetch a new feed.
    void init();

    struct Impl;
    scoped_ptr<Impl> impl_;
    QByteArray bytes_;

    NO_COPY_AND_ASSIGN(FeedFetcher);
};

}  // namespace feed_reader
}  // namespace onyx

#endif  // ONYX_FEED_READER_FEED_FETCHER
