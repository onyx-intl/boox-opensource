// -*- mode: c++; c-basic-offset: 4; -*-

#ifndef ONYX_FEED_READER_FEED_PARSER_H__
#define ONYX_FEED_READER_FEED_PARSER_H__

#include <QString>

#include "onyx/base/base.h"
#include "feed.h"

class QByteArray;

namespace onyx {
namespace feed_reader {

// TODO: expose a method to reset the feed.
class FeedParser {
  public:
    FeedParser() {}
    virtual ~FeedParser() {};
    void startNewFeed(shared_ptr<Feed> feed /* outputv */) {
        startNewFeedInternal(feed);
    };
    bool append(const QByteArray& data) { return appendInternal(data); }
    bool hasError() const { return hasErrorInternal(); }
    QString errorString() const { return errorStringInternal(); }
    bool finished() const { return finishedInternal(); }
    const shared_ptr<Feed> feed() const { return feedInternal(); }
    // Do final processing of a feed. hasError() must be false before
    // calling this function.
    void finalize() {
        DCHECK(!hasError());
        finalizeInternal();
    }

  private:
    virtual void startNewFeedInternal(shared_ptr<Feed> feed) = 0;
    virtual bool appendInternal(const QByteArray& data) = 0;
    virtual bool hasErrorInternal() const = 0;
    virtual QString errorStringInternal() const = 0;
    virtual bool finishedInternal() const = 0;
    virtual const shared_ptr<Feed> feedInternal() const = 0;
    virtual void finalizeInternal() = 0;

    NO_COPY_AND_ASSIGN(FeedParser);
};

}  // namespace feed_reader
}  // namespace onyx

#endif  // ONYX_FEED_READER_FEED_PARSER_H__
