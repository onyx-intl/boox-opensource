// -*- mode: c++; c-basic-offset: 4; -*-

#include "database.h"

#include <QApplication>
#include <QFile>
#include <QString>

#include "onyx/base/base.h"
#include "gtest/gtest.h"

namespace onyx {
namespace feed_reader {

class DatabaseTest: public ::testing::Test {
  public:
    void SetUp(){
        static char* name;
        static int FAKE_ARGC = 1;
        static char** FAKE_ARGV = &name;
        app_.reset(new QApplication(FAKE_ARGC, FAKE_ARGV));
        db_path_ = ":memory:";
    }

  protected:
    scoped_ptr<QApplication> app_;
    QString db_path_;
};

TEST_F(DatabaseTest, Initialization) {
    Database::init(db_path_);
    EXPECT_TRUE(Database::getShared().get());
}

}  // namespace onyx
}  // namespace feed_reader
