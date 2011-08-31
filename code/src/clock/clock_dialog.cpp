#include "clock_dialog.h"
#include "onyx/screen/screen_proxy.h"
#include <QTimer>

const QString LABEL_STYLE = "           \
QLabel                                  \
{                                       \
     padding: 0px;                      \
     background: transparent;           \
     font: 48px ;                       \
     color: black;                      \
 }";


const QString DATE_FORMAT  = "hh:mm";


ClockDialog::ClockDialog(const QDateTime & start, QWidget *parent)
    : OnyxDialog(parent)
    , start_(start)
    , layout_(&content_widget_)
    , time_layout_(0)
    , reading_layout_(0)
    , time_number_(this)
    , year_label_(this)
    , separator_(0)
    , reading_label_(this)
{
    createLayout();
}

ClockDialog::~ClockDialog(void)
{
}

void ClockDialog::createLayout()
{
    content_widget_.setBackgroundRole(QPalette::Button);
    updateTitle(QApplication::tr("Clock"));
    updateTitleIcon(QPixmap(":/images/clock.png"));
    layout_.setSizeConstraint(QLayout::SetFixedSize);
    layout_.setContentsMargins(20, 20, 20, 20);
    time_layout_.setContentsMargins(0, 0, 0, 0);
    reading_layout_.setContentsMargins(0, 0, 0, 0);

    // time
    time_layout_.addWidget(&time_number_);
    time_number_.setStyleSheet(LABEL_STYLE);

    // spacing.
    time_layout_.addSpacing(100);

    // year
    year_label_.setWordWrap(true);
    year_label_.setTextFormat(Qt::RichText);
    time_layout_.addWidget(&year_label_, 100);
    layout_.addLayout(&time_layout_);

    // separator
    separator_.setFocusPolicy(Qt::NoFocus);
    separator_.setFixedHeight(1);
    separator_.setFrameShape(QFrame::HLine);
    separator_.setAutoFillBackground(true);
    separator_.setBackgroundRole(QPalette::Light);
    layout_.addWidget(&separator_);

    // label
    reading_label_.setWordWrap(true);
    reading_layout_.addWidget(&reading_label_);
    layout_.addLayout(&reading_layout_);
}

int ClockDialog::exec()
{
    shadows_.show(true);
    updateText();
    show();
    onyx::screen::instance().flush();
    onyx::screen::instance().updateWidgetRegion(
        0,
        outbounding(parentWidget()),
        onyx::screen::ScreenProxy::GC,
        false,
        onyx::screen::ScreenCommand::WAIT_ALL);
    int ret = QDialog::exec();
    onyx::screen::instance().updateWidget(0, onyx::screen::ScreenProxy::GU);
    return ret;
}

void ClockDialog::updateText()
{
    QDateTime current(QDateTime::currentDateTime());
    QString string = current.toString(DATE_FORMAT);
    time_number_.setText(string);

    QString y("%1<br>%2-%3");
    y = y.arg(current.date().year()).arg(current.date().month()).arg(current.date().day());
    year_label_.setText(y);

    int seconds = start_.secsTo(QDateTime::currentDateTime());

    if (seconds < 60)
    {
        QString str(tr("Reading time:<br> Less than one minute."));
        reading_label_.setText(str);
    }
    else if (seconds < 3600)
    {
        QString str(tr("Reading time:<br> %1 minutes."));
        str = str.arg(seconds / 60);
        reading_label_.setText(str);
    }
    else
    {
        QString str(tr("Reading time:<br> %1 hours %2 minutes."));
        str = str.arg(seconds / 3600).arg((seconds % 3600) / 60);
        reading_label_.setText(str);
    }
}

///Full screen clock
FullScreenClock::FullScreenClock(QWidget *parent)
: QDialog(parent)
, need_GC_(false)
{  
    setFocusPolicy(Qt::StrongFocus);
    setFocus();
    setModal(true);
    connect(&sys::SysStatus::instance(), SIGNAL(hardwareTimerTimeout()), this, SLOT(updateFSClock()));
    sys::SysStatus::instance().startSingleShotHardwareTimer(60 - QTime::currentTime().second());

    //update clock per minute in connecting USB
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateFSClock()));
    timer->start(60000);
}

FullScreenClock::~FullScreenClock(void)
{
    sys::SysStatus::instance().setDefaultHardwareTimerInterval();
}

void FullScreenClock::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(rect(), Qt::white);
    drawTime(&painter);
    drawDate(&painter);
}

void FullScreenClock::drawTime(QPainter* painter)
{
    int total_width = this->rect().width();
    int total_height = this->rect().height();
    QFont time_font;
    time_font.setBold(true);
    time_font.setPixelSize(qMin(total_width, total_height)/4);
    painter->setFont(time_font);
    painter->drawText(0, 0, total_width, total_height*4/5, Qt::AlignCenter, QTime::currentTime().toString("hh:mm"));
    if(QTime::currentTime().minute() % 10 == 0)
    {
        need_GC_ = true;
    }
}

void FullScreenClock::updateFSClock()
{
    sys::SysStatus::instance().startSingleShotHardwareTimer(60);
    repaint();
}

void FullScreenClock::drawDate(QPainter* painter)
{
    int total_width = this->rect().width();
    int total_height = this->rect().height();
    QFont time_font;
    time_font.setBold(true);
    time_font.setPixelSize(qMin(total_width, total_height)/20);
    painter->setFont(time_font);
    painter->drawText(0, total_height*4/5, total_width, total_height/5, Qt::AlignCenter, QDate::currentDate().toString(Qt::TextDate));
}

int FullScreenClock::exec()
{
    onyx::screen::instance().enableUpdate(false);
    showFullScreen();
    QApplication::processEvents();
    onyx::screen::instance().enableUpdate(true);
    onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GC, true, onyx::screen::ScreenCommand::WAIT_ALL);
    return QDialog::exec();
}

void FullScreenClock::keyPressEvent(QKeyEvent *ke)
{
    ke->accept();
}

void FullScreenClock::keyReleaseEvent(QKeyEvent *ke)
{
    // Check the current selected type.
    ke->accept();
    switch (ke->key())
    {
        case Qt::Key_Left:
        case Qt::Key_PageUp:
        case Qt::Key_Right:
        case Qt::Key_PageDown:
        case Qt::Key_Down:
        case Qt::Key_Up:
            break;
        case Qt::Key_Return:
            onReturn();
            break;
        case Qt::Key_Escape:
        case Qt::Key_Home:
            onCloseClicked();
            break;
    }
}

void FullScreenClock::mousePressEvent(QMouseEvent *event)
{
}

void FullScreenClock::mouseReleaseEvent(QMouseEvent *event)
{
    onCloseClicked();
}


bool FullScreenClock::event(QEvent *e)
{
    int ret = QDialog::event(e);
    if (e->type() == QEvent::UpdateRequest)
    {
        if (need_GC_)
        {
            onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GC, true, onyx::screen::ScreenCommand::WAIT_ALL);
            need_GC_ = false;
        }
        else
        {
            onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GU, true, onyx::screen::ScreenCommand::WAIT_ALL);
        }
        e->accept();
        return true;
    }
    return ret;
}



void FullScreenClock::onReturn()
{
    onOkClicked(true);
}

void FullScreenClock::onOkClicked(bool)
{
    accept();
}

void FullScreenClock::onCloseClicked()
{
    reject();
}
