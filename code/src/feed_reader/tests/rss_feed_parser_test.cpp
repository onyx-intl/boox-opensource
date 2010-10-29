// -*- mode: c++; c-basic-offset: 4; -*-

#include "rss_feed_parser.h"

#include <iostream>
#include <QFile>
#include <QDebug>

#include "onyx/base/base.h"
#include "onyx/base/qt_support.h"
#include "gtest/gtest.h"

#include "article.h"
#include "feed.h"

namespace onyx {
namespace feed_reader {

namespace {
const int CHUNK_SIZE = 100;
}

class FeedParserTest: public ::testing::Test {
  protected:
    void SetUp() {
        feed_.reset(new Feed);
    }

    RssFeedParser parser_;
    shared_ptr<Feed> feed_;
};

TEST_F(FeedParserTest, ParseSlashdot) {
    QFile rss(TEST_DATA_DIR "/slashdot.rss");
    ASSERT_TRUE(rss.exists());
    rss.open(QIODevice::ReadOnly);
    parser_.startNewFeed(feed_);
    while (rss.bytesAvailable() > 0) {
        parser_.append(rss.read(CHUNK_SIZE));
    }
    EXPECT_TRUE(parser_.finished());
    EXPECT_EQ("Slashdot", parser_.feed()->title());
    EXPECT_EQ("http://slashdot.org/",
              parser_.feed()->site_url().toString());
}

TEST_F(FeedParserTest, ParseSlashdotArticles) {
    QFile rss(TEST_DATA_DIR "/slashdot.rss");
    ASSERT_TRUE(rss.exists());
    rss.open(QIODevice::ReadOnly);
    parser_.startNewFeed(feed_);
    while (rss.bytesAvailable() > 0) {
        parser_.append(rss.read(CHUNK_SIZE));
    }
    EXPECT_TRUE(parser_.finished());
    EXPECT_EQ(static_cast<size_t>(15), parser_.feed()->articles().size());
}

TEST_F(FeedParserTest, ParseWordPress) {
    QFile rss(TEST_DATA_DIR "/wp.rss");
    ASSERT_TRUE(rss.exists());
    rss.open(QIODevice::ReadOnly);
    parser_.startNewFeed(feed_);
    while (rss.bytesAvailable() > 0) {
        parser_.append(rss.read(CHUNK_SIZE));
    }
    EXPECT_TRUE(parser_.finished());
    EXPECT_EQ("A Humble Programmer", parser_.feed()->title());
    EXPECT_EQ("http://hjiang.net",
              parser_.feed()->site_url().toString());
}

TEST_F(FeedParserTest, ParseWordPressArticles) {
    QFile rss(TEST_DATA_DIR "/wp.rss");
    ASSERT_TRUE(rss.exists());
    rss.open(QIODevice::ReadOnly);
    parser_.startNewFeed(feed_);
    while (rss.bytesAvailable() > 0) {
        parser_.append(rss.read(CHUNK_SIZE));
    }
    EXPECT_TRUE(parser_.finished());
    EXPECT_EQ(static_cast<size_t>(10), parser_.feed()->articles().size());
}

TEST_F(FeedParserTest, ParseWordPressPubdate) {
    QFile rss(TEST_DATA_DIR "/slashdot.rss");
    ASSERT_TRUE(rss.exists());
    rss.open(QIODevice::ReadOnly);
    parser_.startNewFeed(feed_);
    while (rss.bytesAvailable() > 0) {
        parser_.append(rss.read(CHUNK_SIZE));
    }
    EXPECT_TRUE(parser_.finished());
    EXPECT_EQ("2009-11-05T23:24:00+00:00", parser_.feed()->articles().at(0)->pubdate());
}
}  // namespace feed_reader
}  // namespace onyx
