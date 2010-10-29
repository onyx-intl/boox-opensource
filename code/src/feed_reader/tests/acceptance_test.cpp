// -*- mode: c++; c-basic-offset: 4; -*-

#include <QTest>

#include "init.h"

#include "onyx/base/base.h"
#include "onyx/base/qt_support.h"
#include "gtest/gtest.h"

#include "add_feed_dialog.h"
#include "feed_list_model.h"
#include "feeds_page.h"
#include "main_widget.h"

namespace onyx {
namespace feed_reader {

static const QString TEST_DB_PATH(TEST_TMP_DIR "/testdb");

class AcceptanceTest: public ::testing::Test {
  protected:
    void SetUp() {
        static char* name;
        static int FAKE_ARGC = 1;
        static char** FAKE_ARGV = &name;
        q_app_.reset(new QApplication(FAKE_ARGC, FAKE_ARGV));
        InitArgs args;
        args.db_path = TEST_DB_PATH;
        main_widget_.reset(init(args));
    }

    void TearDown() {
        if (QFile::exists(TEST_DB_PATH)) {
            ASSERT_TRUE(QFile::remove(TEST_DB_PATH));
        }
    }

    void addFeed(const QString& url) {
        QTest::mouseClick(main_widget_->feeds_page_->add_feed_button_,
                          Qt::LeftButton);
        QTest::keyClicks(main_widget_->feeds_page_->add_feed_dialog_->url_edit_,
                         url);
        QTest::mouseClick(
                main_widget_->feeds_page_->add_feed_dialog_->add_button_,
                Qt::LeftButton);
    }

    QVariant nthFeedListModelItem(int row, int role) {
        return main_widget_->feed_list_model_->data(
                main_widget_->feed_list_model_->index(row),
                role);
    }

    int feedListModelRowCount() {
        return main_widget_->feed_list_model_->rowCount();
    }

    // The objects should be ordered to ensure proper tear down.
    scoped_ptr<QApplication> q_app_;
    scoped_ptr<MainWidget> main_widget_;
};

TEST_F(AcceptanceTest, CleanClose) {
    EXPECT_TRUE(main_widget_->close());
}

TEST_F(AcceptanceTest, AddWordEngadgetRSS) {
    const char URL[] = "http://www.engadget.com/rss.xml";
    const char FEED_TITLE[] = "Engadget";
    addFeed(URL);
    int delay_ms = 500;
    EXPECT_EQ(1, feedListModelRowCount());
    // Wait for the title of the feed to be updated, but only wait for
    // 10 sec at most.
    QString title;
    do {
        delay_ms *= 2;
        QTest::qWait(delay_ms);
        title = nthFeedListModelItem(0, Qt::DisplayRole).toString();
    } while (FEED_TITLE != title &&
             delay_ms <= 10000);
    EXPECT_EQ(FEED_TITLE, title);
}

}  // namespace feed_reader
}  // namespace onyx
