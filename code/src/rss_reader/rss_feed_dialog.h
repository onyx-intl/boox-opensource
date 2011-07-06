#ifndef RSS_FEED_DIALOG_H_H
#define RSS_FEED_DIALOG_H_H

#include "onyx/ui/message_dialog.h"
#include "onyx/ui/text_browser.h"
#include "onyx/ui/line_edit.h"
#include "onyx/ui/onyx_keyboard.h"
#include "onyx/ui/keyboard_navigator.h"
#include "onyx/ui/ui.h"

using namespace ui;
namespace rss_reader
{

class RssFeedDialog : public MessageDialog
{
    Q_OBJECT

public:
    explicit RssFeedDialog(const QString & str, QWidget *parent = 0);
    ~RssFeedDialog(void);

public:
    int popup();

    void createLayout(void);
    QString title() const;
    QString url() const;

    void setTitle(const QString & title);
    void setUrl(const QString & url);

private Q_SLOTS:
    void onSave(void);
    void onCancel(void);
    void onClicked(void);
    bool eventFilter(QObject *obj, QEvent *event);

    void mouseMoveEvent(QMouseEvent *me);
    void mousePressEvent(QMouseEvent *me);
    void mouseReleaseEvent(QMouseEvent *me);
    void keyReleaseEvent(QKeyEvent *);
    void keyPressEvent(QKeyEvent * ke);
    void resizeEvent(QResizeEvent *e);
    

private:
    QHBoxLayout h_layout_title_;
    QHBoxLayout h_layout_url_;

    OnyxLabel label_feed_title_;    
    OnyxLabel label_feed_url_;    

    OnyxLineEdit  edit_feed_title_;
    OnyxLineEdit  edit_feed_url_;

    OnyxKeyboard  keyboard_;     ///< Keyboard.

    bool   input_title_;
};

};  // namespace rss_reader


#endif  // RSS_FEED_DIALOG_H_H
