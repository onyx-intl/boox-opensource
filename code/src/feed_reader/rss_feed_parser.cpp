// -*- mode: c++; c-basic-offset: 4; -*-

#include "rss_feed_parser.h"

#include <QDebug>
#include <QXmlStreamReader>

#include "article.h"
#include "feed.h"



namespace onyx {
namespace feed_reader {

RssFeedParser::RssFeedParser()
        : FeedParser(),
          feed_(),
          xml_reader_(),
          current_article_(),
          current_text_(),
          pudate_(""),
          in_content_(false){
    taglist_.clear();
    taglist_ << "a" << "b" << "big" << "em" << "i"  << "div" << "strong" << "span"
        << "li" << "ul" << "ol" <<"p" << "blockquote" << "code" << "u"
        << "table" << "tbody" << "small" << "tt" << "tr" << "td" << "th"
        << "h1" << "h2" << "h3" << "h4" << "h5" << "h6" << "center"
        << "dd" << "dt" << "dl" << "font" << "form" << "menu" << "option";
              //"br", "img", hr 
}

RssFeedParser::~RssFeedParser() {
}

void RssFeedParser::startNewFeedInternal(shared_ptr<Feed> feed) {
    feed_ = feed;
    current_text_.clear();
    pudate_.clear();
    xml_reader_.clear();
    current_article_.reset();
    qDebug() << "Starting new feed:" << feed->feed_url();
}

bool RssFeedParser::appendInternal(const QByteArray& data) {
    xml_reader_.addData(data);
    return parseMore();
}

bool RssFeedParser::hasErrorInternal() const {
    return xml_reader_.hasError() &&
            xml_reader_.error() !=
            QXmlStreamReader::PrematureEndOfDocumentError;
}

QString RssFeedParser::errorStringInternal() const {
    return xml_reader_.errorString();
}

bool RssFeedParser::finishedInternal() const {
    return xml_reader_.atEnd() && !xml_reader_.hasError();
}

const shared_ptr<Feed> RssFeedParser::feedInternal() const {
    return feed_;
}

namespace {
bool is_new_article(const Article& article,
                    const vector<shared_ptr<Article> >& articles) {
    for (size_t i = 0; i < articles.size(); ++i) {
        if (article.url() == articles[i]->url()) {
            return false;
        }
    }
    return true;
}
}

// TODO can we probe the header and check out of which type the feed is, rss 2.0, rdf, or atom
/** give a varible to store type like QString feed_type_
    do switch in handleStartElement with feed_type_ specified
    and then as well as handleEndElement;
        Comparison of the RSS and Atom tags
                  Atom  RSS 2.0           RSS 1.0
Channel
   Container      feed     channel        channel
   Title          title    title          title
   URL            link     link           link
   Summary        subtitle description    description
   Date           updated  lastBuildDate  dc:date
   Logo           icon     image          image
   Author         author   managingEditor  -
Item
   Container      entry    item           item
   Title          title    title          title
   URL            link     link           link
   Description    summary  description    description
   Date           updated  pubDate        dc:date
   Image          logo     -              image
Image
   Container      Logo     image          image
   Title                   title          title
   Article URL             link           link
   URL image file          url            url
*/
void RssFeedParser::handleStartElement() {
    shared_ptr<QString> name(new QString(xml_reader_.name().toString()));
    //Check if *name matches the tags that nested in the tags content or
    //sumary in atom in which there is not surround them with CDATA tag.
    if (in_content_) {
        if (taglist_.indexOf(*name)) {
            current_text_ += (QString("<")+ *name + QString(">"));
            return;
        } else if (*name == "br" || *name == "hr" ||*name == "img") {
            current_text_ += (QString("<")+ xml_reader_.text().toString()+QString("/>"));
            return;
        }
        return;
    }
    current_text_.clear();
    if (*name == "content" || *name == "summary" ) {
        if (in_content_) qDebug() << "Error occurs";
       // qDebug() << "Content or Summary: " << xml_reader_.readElementText();
        in_content_ = true;
       // xml_reader_.skipCurrentElement();
        return;
    }
    if (*name == "item" || *name == "entry") {
        tag_stack_.push(name);
        current_article_.reset(new Article(feed_));
    }
}

void RssFeedParser::handleEndElement() {
    if (tag_stack_.size() &&
        xml_reader_.name() == *(tag_stack_.top())) {
        tag_stack_.pop();
    }
    if (xml_reader_.name() == "title") {
        if (tag_stack_.size() && (*(tag_stack_.top()) == "channel"
            || *(tag_stack_.top()) == "feed") && feed_->title().isEmpty()) {
            feed_->set_title(current_text_);
        }
        if (tag_stack_.size() && (*(tag_stack_.top()) == "item"
            || *(tag_stack_.top()) == "entry") && current_article_.get() &&
            current_article_->title().isEmpty()) {
            current_article_->set_title(current_text_);
        }
    } else if (xml_reader_.name() == "link" ||
               xml_reader_.name() == "id") {
        if (tag_stack_.size() && *(tag_stack_.top()) == "channel" &&
            feed_->site_url().isEmpty()) {
            feed_->set_site_url(current_text_);
            qDebug() << "site url " << current_text_;
        }
        if (tag_stack_.size() && (*(tag_stack_.top()) == "item"
            || *(tag_stack_.top()) == "entry") && current_article_.get() &&
            current_article_->url().isEmpty()) {
            current_article_->set_url(current_text_);
        }
    } else if (xml_reader_.name() == "description"
           ) {
        if (tag_stack_.size() && (*(tag_stack_.top()) == "item"
            || *(tag_stack_.top()) == "entry") && current_article_.get() &&
            current_article_->text().isEmpty()) {
            current_article_->set_text(current_text_);
        }
    } else if (xml_reader_.name() == "pubDate"
            || xml_reader_.name() == "updated"
            || xml_reader_.name() == "date") {
        if (tag_stack_.size() &&
            (*(tag_stack_.top()) == "item" || *(tag_stack_.top()) == "entry") &&
            current_article_.get() &&
            current_article_->pubdate().isEmpty()) {
            current_article_->set_pubdate(current_text_);
        }
    } else if ((xml_reader_.name() == "content" ||
                xml_reader_.name() == "summary" ||
                xml_reader_.namespaceUri() ==
               "http://purl.org/rss/1.0/modules/content/")// &&
               /** !current_article_->title().isEmpty()*/) {
        in_content_ = false;
        if (tag_stack_.size() &&
            (*(tag_stack_.top()) == "item" || *(tag_stack_.top()) == "entry")
            && current_article_.get()) {
            current_article_->set_text(current_text_ );
        qDebug()<< "current_text_" << current_text_;
        }
    } else if (taglist_.indexOf(xml_reader_.name().toString())
        && in_content_ /** ensure inside the tag "content" or "summary"*/ ) {
        // well we accpet them :-/, perhaps we should check for each one
        current_text_ += (QString("</") + xml_reader_.name().toString() + QString(">"));
       // current_text_  = current_text_ +xml_reader_.tokenString();
    } else if ((xml_reader_.name() == "item" || xml_reader_.name() == "entry")
        && current_article_.get()) {
        qDebug() << "One article parsed.";
        feed_->mutable_articles()->push_back(current_article_);
        if (is_new_article(*current_article_, feed_->articles())) {
            new_articles_.push_back(current_article_);
        }
        current_article_.reset();
    } else {
        qDebug() << "Ignoring tag: " << xml_reader_.name();
    }
}

bool RssFeedParser::parseMore() {
    DCHECK(feed_.get());
    while (!xml_reader_.atEnd()) {
        xml_reader_.readNext();
        if (xml_reader_.isStartElement()) {
            handleStartElement();
        } else if (xml_reader_.isEndElement()) {
            handleEndElement();
        } else if (xml_reader_.isCharacters() && !xml_reader_.isWhitespace()) {
            current_text_ += xml_reader_.text().toString();
        }
    }
    if (xml_reader_.hasError() &&
        xml_reader_.error() != QXmlStreamReader::PrematureEndOfDocumentError) {
        qWarning() << "XML ERROR:"
                   << xml_reader_.errorString();
        return false;
    } else {
        return true;
    }
}

void RssFeedParser::finalizeInternal() {
    feed_->mutable_articles()->insert(feed_->mutable_articles()->begin(),
                                      new_articles_.begin(),
                                      new_articles_.end());
}

}  // namespace feed_reader
}  // namespace onyx
