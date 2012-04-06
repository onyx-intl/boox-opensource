#include "rssFeedInfo.h"
#include "rss_view.h"
#include "onyx/data/data_tags.h"
#include <QDateTime>

static const int MARGIN  = 8;


using namespace ui;
namespace rss_reader
{

extern QString TAG_POINTER ;
extern QString TAG_ROW;

RssFactory::RssFactory()
{
}

RssFactory::~RssFactory()
{
}

ContentView * RssFactory::createView(QWidget *parent, const QString &type)
{
   ContentView * target = 0;
   if (type == "RssView")
   {
       target = new RssView(parent);
   }
   else if (type == "ButtonView")
   {
       target = new ButtonView(parent);
   }
   else if (type == "EditView")
   {
       target = new EditView(parent);
   }
    
    return target;
}

const QString RssView::TAG_NEW_ITEMS = "NEW_ITEMS";
const QString RssView::TAG_ALL_ITEMS = "ALL_ITEMS";
const QString RssView::TAG_UPDATE_TIME = "UPDATE_TIME";

RssView::RssView(QWidget *parent)
: ContentView(parent)
, h_layout_(this)
{
    createLayout();
}

RssView::~RssView()
{
}

const QString RssView::type()
{
    return "RssView";
}

void RssView::createLayout()
{
    h_layout_.setContentsMargins(20, 10, 20, 10);

    h_layout_.addLayout(&v_layout_left_, 200);
    h_layout_.addWidget(&label_updating_, Qt::AlignRight);
    h_layout_.addStretch(0);
    h_layout_.addWidget(&label_update_, Qt::AlignRight);
    h_layout_.addLayout(&v_layout_right_, 0);

    v_layout_left_.addWidget(&label_title_, 0, Qt::AlignLeft);
    v_layout_left_.addWidget(&label_url_, 0, Qt::AlignLeft);

    v_layout_right_.addWidget(&label_items_, 0);
    v_layout_right_.addWidget(&label_time_, 0);

    QFont font(label_title_.font());
    font.setPointSize(25);
    label_title_.setFont(font);
    font.setPointSize(18);
    label_url_.setFont(font);
    font.setBold(true);
    label_items_.setFont(font);
    font.setPointSize(16);
    label_time_.setFont(font);

    label_updating_.setPixmap(QPixmap(":/rss_images/updating.png"));
    label_update_.setPixmap(QPixmap(":/rss_images/update.png"));

    label_items_.setAlignment(Qt::AlignRight);
    label_time_.setAlignment(Qt::AlignRight);

    // label_items_.setFixedWidth(90);
    // label_time_.setFixedWidth(90);
}

void RssView::updateView()
{
    update();
}

void RssView::mouseReleaseEvent(QMouseEvent * event)
{
    if (label_update_.isVisible() && label_update_.geometry().contains(event->pos()))
    {
        event->accept();
        label_update_.setVisible(false);
        label_updating_.setVisible(true);

        update();
        emit activate(UPDATE);
    }
    else
    {
        ContentView::mouseReleaseEvent(event);
    }
}

void RssView::keyReleaseEvent(QKeyEvent *e)
{
    ContentView::keyReleaseEvent(e);
}

void RssView::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), bkColor());

    if (data())
    {
        if (isChecked())
        {
             painter.fillRect(rect().adjusted(penWidth(), penWidth(),
                     -penWidth() - 1, -penWidth() - 1), Qt::gray);
        }

        if (hasFocus())
        {
            QPen pen;
            pen.setWidth(penWidth());
            painter.setPen(pen);
            painter.drawRoundedRect(rect().adjusted(penWidth(), penWidth(), -penWidth() , -penWidth()), 5, 5);
        }
        else
        {
            QPen pen;
            pen.setWidth(1);
            painter.setPen(pen);
            painter.drawRoundedRect(rect().adjusted(penWidth(), penWidth(), -penWidth() , -penWidth()), 5, 5);
        }

/*
        if (isPressed() || isChecked())
        {
            painter.setPen(Qt::black);

            QPainterPath path;
            QRect rc(rect().x() + 10, rect().y() + 10, 20, rect().height() - 20);
            QBrush brush(Qt::black);

            path.moveTo(rc.topLeft());
            path.lineTo(rc.bottomLeft());
            path.lineTo(rc.right(), rc.y() + rc.height() / 2);
            path.lineTo(rc.topLeft());
            painter.fillPath(path, brush);
        }
*/
        drawTitle(painter, rect());
    }
    else
    {
        label_updating_.setVisible(false);
        label_update_.setVisible(false);
    }
}

void RssView::drawTitle(QPainter & painter, QRect rect)
{
    label_title_.setText(data()->value(TAG_TITLE).toString());
    label_url_.setText(data()->value(TAG_URL).toString());

    QString str = QString("%1/%2").arg(data()->value(TAG_NEW_ITEMS).toInt()).arg(data()->value(TAG_ALL_ITEMS).toInt());

    label_items_.setText(str);

    QDateTime time = data()->value(TAG_UPDATE_TIME).toDateTime();
    if (time.isNull())
    {
        str = tr("Updating");
        label_update_.setVisible(false);
        label_updating_.setVisible(true);
    }
    else
    {
        label_updating_.setVisible(false);
        label_update_.setVisible(true);

        if (time.date() == QDate::currentDate ())
        {
            str = time.toString("hh:mm"); 
        }
        else
        {
            str = time.toString("dd.M.yyyy"); 
        }
    }
    label_time_.setText(str);
}


ButtonView::ButtonView(QWidget *parent)
: ContentView(parent)
{
}

ButtonView::~ButtonView()
{
}

const QString ButtonView::type()
{
    return "ButtonView";
}

void ButtonView::updateView()
{
    update();
}


void ButtonView::keyPressEvent(QKeyEvent *e)
{
    ContentView::keyPressEvent(e);
}

void ButtonView::keyReleaseEvent(QKeyEvent *e)
{
    ContentView::keyReleaseEvent(e);
}

void ButtonView::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);
    // painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), bkColor());

    if (data())
    {
        if (hasFocus())
        {
            QPen pen;
            pen.setWidth(penWidth());
            painter.setPen(pen);
            painter.drawRoundedRect(rect().adjusted(penWidth(), penWidth(), -penWidth() , -penWidth()), 5, 5);
        }
        else
        {
            QPen pen;
            pen.setWidth(2);
            pen.setColor(Qt::black);
            painter.setPen(pen);
            painter.drawRect(rect());
        }

    }

    drawTitle(painter, rect());
}

void ButtonView::drawTitle(QPainter & painter, QRect rect)
{
    int alignment = Qt::AlignCenter;
    if (data()->contains(TAG_ALIGN))
    {
        bool ok;
        int val = data()->value(TAG_ALIGN).toInt(&ok);
        if (ok)
        {
            alignment = val;
        }
    }
    ContentView::drawTitle(painter, rect, alignment);
}

static const QString STYLE = "                          \
 QCheckBox::indicator:unchecked {                       \
     image: url(:/images/check_box_normal.png);         \
 }                                                      \
 QCheckBox::indicator:unchecked:hover {                 \
     image: url(:/images/check_box_normal.png);         \
 }                                                      \
 QCheckBox::indicator:unchecked:pressed {               \
     image: url(:/images/check_box_normal.png);         \
 }                                                      \
 QCheckBox::indicator:checked {                         \
     image: url(:/images/check_box_checked.png);        \
 }                                                      \
 QCheckBox::indicator:checked:hover {                   \
     image: url(:/images/check_box_checked.png);        \
 }                                                      \
 QCheckBox::indicator:checked:pressed {                 \
     image: url(:/images/check_box_checked.png);        \
 }                                                      \
";

static const QString SPECIAL_STYLE = "                          \
 QCheckBox::indicator:unchecked {                       \
     image: url(:/images/check_box_normal.png);         \
 }                                                      \
 QCheckBox::indicator:unchecked:hover {                 \
     image: url(:/images/check_box_normal.png);         \
 }                                                      \
 QCheckBox::indicator:unchecked:pressed {               \
     image: url(:/images/check_box_normal.png);         \
 }                                                      \
 QCheckBox::indicator:checked {                         \
     image: url(:/rss_images/check_box_speical.png);        \
 }                                                      \
 QCheckBox::indicator:checked:hover {                   \
     image: url(:/rss_images/check_box_speical.png);        \
 }                                                      \
 QCheckBox::indicator:checked:pressed {                 \
     image: url(:/rss_images/check_box_speical.png);        \
 }                                                      \
";

EditView::EditView(QWidget *parent)
: ContentView(parent)
, h_layout_(this)
{
    createLayout();
}

EditView::~EditView()
{
}

const QString EditView::type()
{
    return "EditView";
}

void EditView::createLayout()
{
    h_layout_.setContentsMargins(10, 10, 10, 10);

    checkbox_.setStyleSheet(STYLE);

    label_edit_.setPixmap(QPixmap(":/rss_images/edit.png"));
    label_remove_.setPixmap(QPixmap(":/rss_images/remove.png"));

    h_layout_.addWidget(&checkbox_, 0);
    h_layout_.addWidget(&label_title_, 0);
    label_title_.setFixedWidth(320);

    h_layout_.addStretch(0);
    h_layout_.addWidget(&label_edit_, 0);
    h_layout_.addWidget(&label_remove_, 0);

    QFont font(label_title_.font());
    font.setPointSize(18);
    label_title_.setFont(font);

    checkbox_.setCheckable(true);
    connect(&checkbox_, SIGNAL(clicked(bool)), this, SLOT(onCheckBoxClicked(bool)));
}

void EditView::onCheckBoxClicked(bool state)
{
    CRSSFeedInfo* feedInfo = (CRSSFeedInfo *) data()->value(TAG_POINTER).toInt();
    feedInfo->Selected = state;
    data()->insert(TAG_CHECKED, state);
    setFocus();
    update();

    emit activate(CHECKBOX);
}

void EditView::mouseReleaseEvent(QMouseEvent * event)
{
    if (label_edit_.isVisible() && label_edit_.geometry().contains(event->pos()))
    {
        event->accept();
        onEdit();
    }
    else if (label_remove_.isVisible() && label_remove_.geometry().contains(event->pos()))
    {
        event->accept();
        onRemove();
    }
    else
    {
        ContentView::mouseReleaseEvent(event);
    }
}

void EditView::onRemove()
{
    emit activate(REMOVE);
}

void EditView::onEdit()
{
    emit activate(EDIT);
}

void EditView::updateView()
{
    //first is commercial feed
    //if (data() && data()->value(TAG_ROW).toInt() == 0)
    //{
    //    checkbox_.setStyleSheet(SPECIAL_STYLE);
    //    checkbox_.setEnabled(false);
    //
    //    checkbox_.setVisible(true);
    //    label_edit_.setVisible(false);
    //    label_remove_.setVisible(false);
    //}

    update();
}

void EditView::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Return)
    {
        e->accept();
        checkbox_.click();
        // onCheckBoxClicked(!checkbox_.isChecked());
    }
    else
    {
        ContentView::keyReleaseEvent(e);
    }
}

void EditView::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), bkColor());

    if (data())
    {
        if (data()->value(TAG_ROW).toInt())
        {
            checkbox_.setVisible(true);
            label_edit_.setVisible(true);
            label_remove_.setVisible(true);
        }
        drawTitle(painter, rect());

        if (checkbox_.isChecked())
        {
            QPen pen;
            pen.setWidth(penWidth());
            painter.setPen(pen);
            painter.fillRect(rect().adjusted(penWidth(), penWidth(),
                        -penWidth() - 1, -penWidth() - 1), Qt::gray);
        }

        if (hasFocus())
        {

            QPen pen;
            pen.setWidth(penWidth());
            painter.setPen(pen);
            painter.drawRoundedRect(rect().adjusted(penWidth(), penWidth(), -penWidth(), -penWidth()), 5, 5);
        }
    }
    else
    {
        checkbox_.setVisible(false);
        label_edit_.setVisible(false);
        label_remove_.setVisible(false);
    }
}

void EditView::drawTitle(QPainter & painter, QRect rect)
{
    label_title_.setText(data()->value(TAG_TITLE).toString());
}

void EditView::showEvent ( QShowEvent * event )
{
    if (isVisible() && data())
    {
        checkbox_.setChecked(data()->value(TAG_CHECKED).toBool());
    }
}

}
