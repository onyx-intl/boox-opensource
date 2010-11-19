// -*- mode: c++; c-basic-offset: 4; -*-

#ifndef ONYX_FEED_READER_RSS_FEED_PARSER_H__
#define ONYX_FEED_READER_RSS_FEED_PARSER_H__

#include <stack>
#include <QXmlStreamReader>
#include <QStringList>
#include "feed_parser.h"

namespace onyx {
namespace feed_reader {

class Feed;

class RssFeedParser : public FeedParser {
  public:
    RssFeedParser();
    virtual ~RssFeedParser();

  private:
    virtual void startNewFeedInternal(shared_ptr<Feed> feed);
    virtual bool appendInternal(const QByteArray& data);
    virtual bool hasErrorInternal() const;
    virtual QString errorStringInternal() const;
    virtual bool finishedInternal() const;
    virtual const shared_ptr<Feed> feedInternal() const;
    virtual void finalizeInternal();
    void handleStartElement();
    void handleEndElement();

    bool parseMore();
    shared_ptr<Feed> feed_;
    QXmlStreamReader xml_reader_;
    // TODO(hjiang): Does QString do ref-counting? perhaps shared_ptr
    // is not needed.
    std::stack<shared_ptr<QString> > tag_stack_;
    shared_ptr<Article> current_article_;
    QString current_text_;  // Text content of the current element.
    QString pudate_;
    vector<shared_ptr<Article> > new_articles_;
    NO_COPY_AND_ASSIGN(RssFeedParser);
};

}  // namespace feed_reader
}  // namespace onyx

#endif  // ONYX_FEED_READER_RSS_FEED_PARSER_H__
