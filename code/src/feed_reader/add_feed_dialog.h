// -*- mode: c++; c-basic-offset: 4; -*-

#ifndef ONYX_FEED_READER_ADD_FEED_DIALOG_H__
#define ONYX_FEED_READER_ADD_FEED_DIALOG_H__

#include <QDialog>
#include <QUrl>

namespace ui {
class KeyBoard;
class OnyxPushButton;
class OnyxLineEdit;
}

namespace onyx {
namespace feed_reader {

class AddFeedDialog : public QDialog
{
    Q_OBJECT;

  public:
    AddFeedDialog(QWidget *parent = 0);
    ~AddFeedDialog();

    const QUrl& url() { return url_; }

  public slots:
    void addClicked();

  private:
    friend class AcceptanceTest;

    ui::KeyBoard* key_board_;
    ui::OnyxPushButton *add_button_;
    ui::OnyxPushButton *cancel_button_;
    ui::OnyxLineEdit *url_edit_;
    QUrl url_;
};

}  // namespace feed_reader
}  // namespace onyx

#endif  // ONYX_FEED_READER_ADD_FEED_DIALOG_H__
