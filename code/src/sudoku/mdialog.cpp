#include "mdialog.h"
#include <QtGui/QKeyEvent>
#include <QtGui/QPainter>
#include <QtGui/QHBoxLayout>
#include "onyx/screen/screen_proxy.h"
#include "onyx/screen/screen_update_watcher.h"
#include "onyx/data/data_tags.h"

MDialog::MDialog(QWidget* parent)
    : QDialog(parent)
    , mark_buttons_(0)
    , set_buttons_(0)
    , enable_flush_flag_(true)
{
#ifndef BUILD_FOR_FB
    onyx::screen::instance().enableUpdate ( true );
    onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::DW);
#endif

    setBackgroundRole(QPalette::Light);
    setAutoFillBackground(true);
    setWindowFlags(Qt::WindowStaysOnTopHint|Qt::FramelessWindowHint);
    setFixedSize(320, 160);

    createLayout();
}

void MDialog::createKeysGroup(CatalogView &view, const int font_size)
{
    ODatas button_data;
    view.setSubItemType(ButtonView::type());
    view.setPreferItemSize(QSize(32, 32));

    for (int i = 1; i <= 9; ++i)
    {
        OData * dd = new OData;
        dd->insert(TAG_TITLE, QString::number(i));
        dd->insert(TAG_ID, i);
        dd->insert(TAG_FONT_SIZE, font_size);
        button_data.push_back(dd);
    }

    view.setData(button_data);
    view.setFixedGrid(3, 3);
    view.setSpacing(3);
    view.setFixedHeight(130);
    view.setFixedWidth(130);

    view.setSearchPolicy(CatalogView::NeighborFirst);
    connect(&view, SIGNAL(itemActivated(CatalogView*,ContentView*,int)),
            this, SLOT(onItemActivated(CatalogView *, ContentView *, int)));
}

void MDialog::enableScreenUpdate(bool flag)
{
    enable_flush_flag_ = flag;
}

void MDialog::onItemActivated(CatalogView *catalog, ContentView *item, int user_data)
{
    int key=item->data()->value(TAG_ID).toInt();
    if (catalog == &set_buttons_)
    {
        emit ActiveKey(key);
    }
    else
    {
        emit ActiveModeKey(key);
    }
    enable_flush_flag_ = false;
    hide();
}

void MDialog::createLayout()
{
    createKeysGroup(mark_buttons_, 20);
    createKeysGroup(set_buttons_, 32);

    set_buttons_.setFocus();
    set_buttons_.setFocusTo(0, 0);
    set_buttons_.setCheckedTo(0, 0);
    set_buttons_.setNeighbor(&mark_buttons_, CatalogView::RIGHT);
    set_buttons_.setNeighbor(&mark_buttons_, CatalogView::RECYCLE_LEFT);

    mark_buttons_.setNeighbor(&set_buttons_, CatalogView::LEFT);
    set_buttons_.setNeighbor(&mark_buttons_, CatalogView::RECYCLE_RIGHT);

    layout_ = new QHBoxLayout;
    layout_->setContentsMargins(5, 5, 5, 5);
    layout_->addWidget(&set_buttons_);
    layout_->addSpacing(10);
    layout_->addWidget(&mark_buttons_);

    this->setLayout(layout_);
}

bool MDialog::event(QEvent* e)
{
    bool ret = QDialog::event ( e );
#ifndef BUILD_FOR_FB
    if (e->type() == QEvent::UpdateRequest && enable_flush_flag_)
    {
        onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::DW, onyx::screen::ScreenCommand::WAIT_NONE);
    }
#endif
    return ret;
}

void MDialog::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape)
    {
        e->accept();
        return;
    }

    QDialog::keyPressEvent(e);
}

void MDialog::keyReleaseEvent(QKeyEvent *e)
{
    switch (e->key())
    {
    case Qt::Key_Right:
    case Qt::Key_Left:
    case Qt::Key_Up:
    case Qt::Key_Down:
        e->accept();
        break;
    case Qt::Key_Escape:
        {
            e->accept();
            hide();
        }
        break;
    default:
        break;
    }
}

void MDialog::paintEvent(QPaintEvent* e)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::HighQualityAntialiasing,true);
    painter.setRenderHint(QPainter::Antialiasing,true);
    painter.setRenderHint(QPainter::TextAntialiasing,true);
    painter.setBrush(QColor(0, 0, 0));
    painter.drawRoundedRect(rect(),5,5);
    painter.setBrush(QColor(255,255,255));
    painter.drawRoundedRect(rect().adjusted(6,6,rect().x()-6,rect().y()-6),5,5);
}
