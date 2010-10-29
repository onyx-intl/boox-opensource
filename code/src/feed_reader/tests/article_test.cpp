// -*- mode: c++; c-basic-offset: 4; -*-

#include "article.h"

#include <QApplication>
#include <QDateTime>
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

#include "database.h"
#include "feed.h"

namespace onyx {
namespace feed_reader {

static const QString TEST_DB_PATH(TEST_TMP_DIR "/testdb");

class ArticleTest: public ::testing::Test {
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

TEST_F(ArticleTest, createTableIfNeeded) {
    EXPECT_TRUE(Article::createTableIfNeeded());
}

TEST_F(ArticleTest, SaveNew) {
    shared_ptr<Feed> feed(new Feed);
    Article article(feed);
    article.set_url("http://someurl");
    article.set_title("test title");
    article.set_text("test text");
    EXPECT_TRUE(article.saveOrUpdate());
}

TEST_F(ArticleTest, Update) {
    shared_ptr<Feed> feed(new Feed);
    Article article(feed);
    article.set_url("http://someurl");
    article.set_title("test title");
    article.set_text("test text");
    article.set_pubdate("2009-11-05T19:57:00+00:00");
    EXPECT_TRUE(article.saveOrUpdate());
    article.set_title("blah");
    EXPECT_TRUE(article.saveOrUpdate());
    // TODO check no new article is created.
}

}  // namespace onyx
}  // namespace feed_reader
