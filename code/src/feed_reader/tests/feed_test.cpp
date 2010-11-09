// -*- mode: c++; c-basic-offset: 4; -*-

#include "feed.h"

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>

#include "onyx/base/base.h"
#include "onyx/base/qt_support.h"
#include "gtest/gtest.h"

#include "article.h"
#include "database.h"

namespace onyx {
namespace feed_reader {

static const QString TEST_DB_PATH(TEST_TMP_DIR "/testdb");

class FeedTest: public ::testing::Test {
  public:
    void SetUp(){
        static char* name;
        static int FAKE_ARGC = 1;
        static char** FAKE_ARGV = &name;
        removeDatabaseFile();
        app_.reset(new QApplication(FAKE_ARGC, FAKE_ARGV));
        Database::init(TEST_DB_PATH);
    }

    void TearDown() {
        removeDatabaseFile();
    }

  protected:
    scoped_ptr<QApplication> app_;
    QString db_path_;

  private:
    void removeDatabaseFile() {
        if (QFile::exists(TEST_DB_PATH)) {
            ASSERT_TRUE(QFile::remove(TEST_DB_PATH));
        }
    }
};

TEST_F(FeedTest, Save) {
    Feed feed;
    feed.set_title("Blah");
    feed.set_site_url("http://google.com");
    feed.set_feed_url("http://baidu.com");
    EXPECT_TRUE(feed.saveNew());
}

TEST_F(FeedTest, Unread) {
    Feed feed;
    feed.set_title("Blah");
    feed.set_site_url("http://google.com");
    feed.set_feed_url("http://baidu.com");
    ASSERT_TRUE(feed.saveNew());
    EXPECT_EQ(0, feed.unreadCount());
}

TEST_F(FeedTest, Count) {
    ASSERT_TRUE(Feed::createTable());
    EXPECT_EQ(0, Feed::count());
    Feed feed;
    feed.set_title("Blah");
    feed.set_site_url("http://google.com");
    feed.set_feed_url("http://baidu.com");
    EXPECT_TRUE(feed.saveNew());
}

TEST_F(FeedTest, LoadAll) {
    vector<shared_ptr<Feed> > feeds;
    Feed feed1;
    feed1.set_title("Blah");
    feed1.set_site_url("http://google.com");
    feed1.set_feed_url("http://baidu.com");
    EXPECT_TRUE(feed1.saveNew());
    EXPECT_TRUE(Feed::all(&feeds));
    EXPECT_EQ(static_cast<size_t>(1), feeds.size());
    Feed feed2;
    feed2.set_title("another");
    feed2.set_site_url("http://gooe.com");
    feed2.set_feed_url("http://badu.com");
    EXPECT_TRUE(feed2.saveNew());
    EXPECT_TRUE(Feed::all(&feeds));
    EXPECT_EQ(static_cast<size_t>(2), feeds.size());
}

TEST_F(FeedTest, SaveAndLoad) {
    Feed feed;
    const char SITE_URL[] = "http://google.com";
    const char FEED_URL[] = "http://baidu.com";
    feed.set_title("Blah");
    feed.set_site_url(SITE_URL);
    feed.set_feed_url(FEED_URL);
    ASSERT_TRUE(feed.saveNew());
    scoped_ptr<Feed> loaded_feed(Feed::loadByUrl(FEED_URL));
    ASSERT_TRUE(loaded_feed.get());
    EXPECT_EQ("Blah", feed.title());
    EXPECT_EQ(SITE_URL, feed.site_url().toString());
    EXPECT_EQ(FEED_URL, feed.feed_url().toString());
}

TEST_F(FeedTest, FeedsHaveDistinctIds) {
    Feed feed1;
    feed1.set_feed_url("http://asd.com");
    ASSERT_TRUE(feed1.saveNew());
    Feed feed2;
    feed2.set_feed_url("http://qwer.com");
    ASSERT_TRUE(feed2.saveNew());
    scoped_ptr<Feed> loaded_feed1(Feed::loadByUrl("http://asd.com"));
    ASSERT_TRUE(loaded_feed1.get());
    scoped_ptr<Feed> loaded_feed2(Feed::loadByUrl("http://qwer.com"));
    ASSERT_TRUE(loaded_feed2.get());
    EXPECT_NE(loaded_feed1->id(), loaded_feed2->id());
    EXPECT_EQ(feed1.id(), loaded_feed1->id());
    EXPECT_EQ(feed2.id(), loaded_feed2->id());
}

TEST_F(FeedTest, UpdateAndLoad) {
    Feed feed;
    const char SITE_URL[] = "http://google.com";
    const char FEED_URL[] = "http://baidu.com";
    feed.set_title("Blah");
    feed.set_site_url(SITE_URL);
    feed.set_feed_url(FEED_URL);
    ASSERT_TRUE(feed.saveNew());
    shared_ptr<Feed> loaded_feed(Feed::loadByUrl(FEED_URL));
    ASSERT_TRUE(loaded_feed.get());
    loaded_feed->set_title("Updated Title");
    const char UPDATED_SITE_URL[] = "http://google.com/updated";
    loaded_feed->set_site_url(UPDATED_SITE_URL);
    EXPECT_TRUE(loaded_feed->update());
    loaded_feed.reset(Feed::loadByUrl(FEED_URL));
    ASSERT_TRUE(loaded_feed.get());
    EXPECT_EQ("Updated Title", loaded_feed->title());
    EXPECT_EQ(UPDATED_SITE_URL, loaded_feed->site_url().toString());
    EXPECT_EQ(FEED_URL, loaded_feed->feed_url().toString());
}

TEST_F(FeedTest, UnreadCount) {
    {
        Feed feed;
        feed.set_title("Blah");
        feed.set_site_url("http://google.com");
        feed.set_feed_url("http://baidu.com");
        ASSERT_TRUE(feed.saveNew());
    }
    {
        shared_ptr<Feed> feed(Feed::loadByUrl("http://baidu.com"));
        Article article(feed);
        article.set_url("http://someurl");
        article.set_title("test title");
        article.set_text("test text");
        ASSERT_TRUE(article.saveOrUpdate());
    }
    {
        shared_ptr<Feed> feed(Feed::loadByUrl("http://baidu.com"));
        vector<shared_ptr<Article> > articles;
        Article::loadByFeed(feed, &articles);
        ASSERT_EQ(1, articles.size());
        ASSERT_FALSE(articles[0]->read());
        EXPECT_EQ(1, feed->unreadCount());
    }
}

}  // namespace onyx
}  // namespace feed_reader
