
#include "rss_feed_dialog.h"
#include "onyx/screen/screen_proxy.h"
#include "onyx/sys/sys_conf.h"
#include "onyx/ui/menu.h"
#include "onyx/ui/ui_global.h"
#include "onyx/ui/ui_utils.h"
#include "onyx/screen/screen_update_watcher.h"

namespace rss_reader
{

const QString MY_MESSAGE_STYLE = "      \
QLabel                               \
{                                       \
    background: transparent;            \
    font: 22px ;                        \
    color: black;                       \
    padding: 0px;                       \
}";

RssFeedDialog::RssFeedDialog(const QString & str, QWidget *parent)
: MessageDialog(QMessageBox::Information,
                str,
                tr(""),
                QMessageBox::Yes|QMessageBox::No,
                parent)
, label_feed_title_(this)
, label_feed_url_(this)
, edit_feed_title_(this)
, edit_feed_url_(this)
, keyboard_(this)
, input_title_(true)
{
    createLayout();
    connect(button(QMessageBox::Yes),SIGNAL(clicked()),SLOT(onSave()));
    connect(button(QMessageBox::No),SIGNAL(clicked()),SLOT(onCancel()));
    button(QMessageBox::No)->setVisible(false);

    connect(&edit_feed_title_,SIGNAL(getFocus(OnyxLineEdit*)),SLOT(onClicked()));
    connect(&edit_feed_url_,SIGNAL(getFocus(OnyxLineEdit*)),SLOT(onClicked()));

    // keyboard.
    edit_feed_title_.installEventFilter(this);
    edit_feed_url_.installEventFilter(this);
    button(QMessageBox::Yes)->installEventFilter(this);
    button(QMessageBox::No)->installEventFilter(this);
    onyx::screen::watcher().addWatcher(&keyboard_);

    setFocusPolicy(Qt::NoFocus);
    connect(&keyboard_, SIGNAL(toLoseFocus()), button(QMessageBox::Yes), SLOT(setFocus()));
}

void RssFeedDialog::createLayout(void)
{
    updateTitleIcon(QPixmap());

    setFixedWidth(580);
    
    getInfoLabel().setVisible(false);

    getInfoLayout().setContentsMargins(0,0,0,0);
    h_layout_title_.setContentsMargins(0,0,10,0);
    h_layout_url_.setContentsMargins(0,0,10,0);

    button(QMessageBox::Yes)->setText(tr("OK"));
    button(QMessageBox::No)->setText(tr("Cancel"));
    button(QMessageBox::Yes)->adjustSize();
    button(QMessageBox::No)->adjustSize();
    
    label_feed_title_.setFixedWidth(60);
    label_feed_url_.setFixedWidth(60);

    h_layout_title_.addWidget(&label_feed_title_, 0);
    h_layout_title_.addWidget(&edit_feed_title_, 0);

    h_layout_url_.addWidget(&label_feed_url_, 0);
    h_layout_url_.addWidget(&edit_feed_url_, 0);

    QVBoxLayout *title_url_layout = new QVBoxLayout(this);
    title_url_layout->insertLayout(1, &h_layout_title_);
    title_url_layout->insertSpacing(2, 10);
    title_url_layout->insertLayout(3, &h_layout_url_);
    title_url_layout->insertSpacing(4, 20);
    getInfoLayout().insertLayout(2, title_url_layout);

    getContentLayout().addWidget(&keyboard_);

    getInfoLabel().setFixedWidth(width() - 10);

    label_feed_title_.setStyleSheet(MY_MESSAGE_STYLE);
    label_feed_url_.setStyleSheet(MY_MESSAGE_STYLE);
    getInfoLabel().setStyleSheet("QLabel{font:20px}");

    label_feed_title_.setText(tr("Title"));
    label_feed_url_.setText(tr("Url"));
}

RssFeedDialog::~RssFeedDialog(void)
{
}

QString RssFeedDialog::title() const
{
    return edit_feed_title_.text();
}

QString RssFeedDialog::url() const
{
    return edit_feed_url_.text();
}

void RssFeedDialog::setTitle(const QString & title)
{
    edit_feed_title_.setText(title);
}

void RssFeedDialog::setUrl(const QString & url)
{
    edit_feed_url_.setText(url);
}

void RssFeedDialog::onSave(void)
{
        done(QDialog::Accepted);
}

void RssFeedDialog::onCancel(void)
{
        done(QDialog::Rejected);
}

void RssFeedDialog::onClicked(void)
{
    if (sender() == &edit_feed_title_)
    {
        input_title_ = true;
    }
    else
    {
        input_title_ = false;
    }
    qDebug("onClicked %d",input_title_);
}

bool RssFeedDialog::eventFilter(QObject *obj, QEvent *event)
{
     QWidget * wnd = 0;
     if (event->type() == QEvent::KeyRelease)
     {
         QKeyEvent *key_event = static_cast<QKeyEvent *>(event);
         if (obj == button(QMessageBox::Yes))
         {
             if (key_event->key() == Qt::Key_Up ||key_event->key() == Qt::Key_Left)
             {
                 edit_feed_url_.setFocus();
                 return true;
             }
             else if (key_event->key() == Qt::Key_Down ||key_event->key() == Qt::Key_Right)
             {
                 // button(QMessageBox::No)->setFocus();
                 wnd = ui::moveFocus(this, key_event->key());
                 if (wnd)
                 {
                     wnd->setFocus();
                 }
                 return true;
             }
         }
         if (obj == button(QMessageBox::No))
         {
             if (key_event->key() == Qt::Key_Up ||key_event->key() == Qt::Key_Left)
             {
                 button(QMessageBox::Yes)->setFocus();
                 return true;
             }
             else if (key_event->key() == Qt::Key_Down ||key_event->key() == Qt::Key_Right)
             {
                 wnd = ui::moveFocus(this, key_event->key());
                 if (wnd)
                 {
                     wnd->setFocus();
                 }
                 return true;
             }
         }
         else if (obj == &edit_feed_title_)
         {
             if (key_event->key() == Qt::Key_Up || key_event->key() == Qt::Key_Left)
             {
                 button(QMessageBox::Yes)->setFocus();
                 return true;
             }
             if (key_event->key() == Qt::Key_Down || key_event->key() == Qt::Key_Right)
             {
                 edit_feed_url_.setFocus();
                 return true;
             }
         }
         else if (obj == &edit_feed_url_)
         {
             if (key_event->key() == Qt::Key_Up || key_event->key() == Qt::Key_Left)
             {
                 edit_feed_title_.setFocus();
                 return true;
             }
             else if (key_event->key() == Qt::Key_Down || key_event->key() == Qt::Key_Right)
             {
                 button(QMessageBox::Yes)->setFocus();
                 return true;
             }
         }
     }
     // standard event processing
     return QObject::eventFilter(obj, event);
}

void RssFeedDialog::keyPressEvent(QKeyEvent * ke)
{
    ke->accept();
    if (ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Return)
    {
        return;
    }
    else if (ke->key() == Qt::Key_Shift ||
             ke->key() == Qt::Key_CapsLock ||
             ke->key() == Qt::Key_F23 )
    {
        return;
    }

    // Disable the parent widget to update screen.
    onyx::screen::instance().enableUpdate(false);

    if (edit_feed_title_.hasFocus() ||
        edit_feed_url_.hasFocus() ||
        (ke->key() != Qt::Key_Down &&
         ke->key() != Qt::Key_Up &&
         ke->key() != Qt::Key_Left &&
         ke->key() != Qt::Key_Right))
    {
        QKeyEvent * key_event = new QKeyEvent(ke->type(), ke->key(), ke->modifiers(), ke->text());
        if (input_title_)
        {
            QApplication::postEvent(&edit_feed_title_, key_event);
        }
        else
        {
            QApplication::postEvent(&edit_feed_url_, key_event);
        }
        
    }

    while (QApplication::hasPendingEvents())
    {
        QApplication::processEvents();
    }

    onyx::screen::instance().enableUpdate(true);

    if (0)
    {
        onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GU);
    }
    else
    {
        onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::DW, true, onyx::screen::ScreenCommand::WAIT_ALL);
    }
}

void RssFeedDialog::keyReleaseEvent(QKeyEvent *ke)
{
    ke->accept();
    int key = ke->key();
    if (key == Qt::Key_Escape || key == ui::Device_Menu_Key)
    {
        done(QDialog::Rejected);
        return;
    }
}

void RssFeedDialog::mouseMoveEvent(QMouseEvent *me)
{
    me->accept();
}

void RssFeedDialog::mousePressEvent(QMouseEvent *me)
{
    me->accept();
}

void RssFeedDialog::mouseReleaseEvent(QMouseEvent *me)
{
    me->accept();
}

void RssFeedDialog::resizeEvent(QResizeEvent *e)
{
    OnyxDialog::resizeEvent(e);
    setFixedWidth(580);
}

int RssFeedDialog::popup()
{
    onyx::screen::instance().flush(0, onyx::screen::ScreenProxy::GU, false, onyx::screen::ScreenCommand::WAIT_ALL);
    int ret = exec();

    return ret;
}

}
