// -*- mode: c++; c-basic-offset: 4; -*-

#include <queue>

#include <QDebug>
#include <QHttp>
#include <QXmlStreamReader>

#include "onyx/base/base.h"
#include "onyx/base/shared_ptr.h"
#include "feed.h"
#include "feed_fetcher.h"
#include "feed_parser.h"

namespace onyx {
namespace feed_reader {

struct FeedFetcher::Impl {
    shared_ptr<Feed> current_feed_;
    std::queue<shared_ptr<Feed> > pending_feeds_;
    QXmlStreamReader xml_;
    int connection_id_;
    scoped_ptr<FeedParser> parser_;
    scoped_ptr<QHttp> http_;
};

FeedFetcher::FeedFetcher(FeedParser* parser) : impl_(new Impl) {
    impl_->parser_.reset(parser);
    bytes_.clear();
    init();
}

void FeedFetcher::init() {
    impl_->http_.reset(new QHttp);
    connect(impl_->http_.get(), SIGNAL(readyRead(const QHttpResponseHeader &)),
            this, SLOT(readData(const QHttpResponseHeader &)));

    connect(impl_->http_.get(), SIGNAL(requestFinished(int, bool)),
            this, SLOT(finishFetch(int, bool)));
}

FeedFetcher::~FeedFetcher() {}

void FeedFetcher::startFetch() {
    // Do nothing if the queue is empty, or we haven't done with the
    // current feed.
    if (impl_->pending_feeds_.empty() || impl_->current_feed_.get()) {
        return;
    }

    // When the program is being closed, QHttp may emit a
    // requestFinished signal that triggers this function.
    if (!impl_->parser_.get()) {
        return;
    }

    impl_->xml_.clear();
    impl_->connection_id_ = -1;
    impl_->current_feed_ = impl_->pending_feeds_.front();
    assert(impl_->current_feed_.get());
    impl_->parser_->startNewFeed(impl_->current_feed_);
    const QUrl& url(impl_->current_feed_->feed_url());
    int port = url.port() == -1 ? 80 : url.port();
    init();
    CHECK(impl_->http_.get());
    qDebug() << "Fetching '" << url.path() << "' from host " << url.host()
             << " and port " << port;
    impl_->http_->setHost(url.host(), port);
    impl_->connection_id_ = impl_->http_->get(url.path());
}

void FeedFetcher::scheduleFetch(shared_ptr<Feed> feed) {
    CHECK(feed.get());
    qDebug() << "Feed added to queue: " << feed->feed_url();
    // TODO(hjiang): Check if the feed is already in the queue.
    impl_->pending_feeds_.push(feed);
    startFetch();
}

void FeedFetcher::readData(const QHttpResponseHeader& response_header) {
    if(!impl_->current_feed_.get())
        return;
    if (response_header.statusCode() == 200) {
        //TODO in order to insert CDATA to invalid XML atom,
        //we need capture data of full page
        bytes_.append(impl_->http_->readAll());
        qDebug() << bytes_.size() << " bytes received.";
        // insert CDATA FLAG if not exists
    } else if ((response_header.statusCode() >300 || response_header.statusCode() <300) && response_header.hasKey("location")) {
        qDebug()<<response_header.statusCode();
        QUrl location = QUrl(response_header.value("location"));
        impl_->http_.get()->setHost(location.host(),80);
        impl_->http_.get()->get(location.path());
    } else {
        qDebug() << "Received non-200 response code: "
                 << response_header.statusCode();
    }
}

QByteArray FeedFetcher::xmlAtomValidator(const QByteArray& d)
{
    QString xml_plain_text = QString::fromLocal8Bit(d);
    if (xml_plain_text.indexOf("CDATA") ==-1) {
    // If this is not well formed xml
        static QRegExp rx_summary_head = QRegExp("<summary([^<]*)>");
        static QRegExp rx_summary_end = QRegExp("(\\s*)</summary>");
        static QRegExp rx_content_head = QRegExp("<content([^<]*)>");
        static QRegExp rx_content_end = QRegExp("(\\s*)</content>");
        xml_plain_text.replace(rx_summary_head, "<description><![CDATA[");
        xml_plain_text.replace(rx_summary_end, "]]></description>");
        xml_plain_text.replace(rx_content_head, "<description><![CDATA[");
        xml_plain_text.replace(rx_content_end, "]]></description>");
        //TODO change '<link xxx/>' to be '<link xxx> </link>'
        static QRegExp rx_link = QRegExp("<link([^<]*)/>");
        xml_plain_text.replace(rx_link, "<link \\1></link>");
    }
    return xml_plain_text.toLocal8Bit();
}

void FeedFetcher::finishFetch(int connection_id, bool error) {
    // process data here, yeah, need a better way
    QByteArray bytes = xmlAtomValidator(bytes_);
    if (!impl_->parser_->append(bytes)) {
        qDebug() << "Error parsing feed: "
                    << impl_->parser_->errorString();
    }
    bytes_.clear();
    qDebug() << "Connection finished: " << connection_id;
    if (connection_id == impl_->connection_id_) {
        // Stop and clear all pending fetch ops if this is true
        // (network connection problem).
        bool has_error = false;
        if (error) {
            qDebug() << "Received error during HTTP fetch: "
                     << impl_->http_->errorString();
            QHttp::Error e = impl_->http_->error();
            if (e == QHttp::HostNotFound || e == QHttp::UnknownError) {
                has_error = true;
            }
        } else {
            if (impl_->parser_->hasError()) {
                qDebug() << "Error parsing feed: "
                         << impl_->parser_->errorString();
            } else if (impl_->parser_->finished()) {
                impl_->parser_->finalize();
                emit feedUpdated(impl_->parser_->feed());
            } else {
                qDebug() << "Feed ended prematurely!";
            }
        }
        impl_->pending_feeds_.pop();
        impl_->current_feed_.reset();
        if (has_error) {
            emit networkError();
            return;
        }
        startFetch();
    }
}

}  // namespace feed_reader
}  // namespace onyx
