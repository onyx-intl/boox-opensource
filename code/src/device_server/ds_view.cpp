#include <QtGui/QtGui>
#include "ds_view.h"
#include "onyx/screen/screen_update_watcher.h"

#include "onyx/wireless/wifi_dialog.h"
#include "onyx/sys/sys_status.h"
#include "onyx/sys/platform.h"
#include "onyx/data/configuration.h"

static const int BUTTON_HEIGHT = 100;

DSView::DSView(QWidget *parent)
#ifndef Q_WS_QWS
    : QWidget(0, 0)
#else
    : QWidget(0, Qt::FramelessWindowHint)
#endif
    , layout_(this)
    , start_(tr("Start"), 0)
    , close_(tr("Close"), 0)
{
    createLayout();
    onyx::screen::watcher().addWatcher(this);
    onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GC);
}

DSView::~DSView()
{
}

void DSView::configNetwork()
{
    QString type = sys::SysStatus::instance().connectionType();
    if (type.contains("wifi", Qt::CaseInsensitive))
    {
        wifiDialog().popup(true);
    }
}

WifiDialog & DSView::wifiDialog()
{
    if (!conf_dialog_)
    {
        conf_dialog_.reset(new WifiDialog(0, SysStatus::instance()));
    }
    return *conf_dialog_;
}

void DSView::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
    case Qt::Key_Down:
    case Qt::Key_Up:
        break;
    default:
        QWidget::keyPressEvent(e);
        break;
    }
    e->accept();
}

void DSView::keyReleaseEvent(QKeyEvent *ke)
{
    switch (ke->key())
    {
    case ui::Device_Menu_Key:
        break;
    case Qt::Key_Left:
        break;
    case Qt::Key_Right:
        break;
    case Qt::Key_PageDown:
        break;
    case Qt::Key_Down:
        break;
    case Qt::Key_PageUp:
        break;
    case Qt::Key_Up:
        break;
    case Qt::Key_C:
        break;
    case Qt::Key_Escape:
    case Qt::Key_Home:
        onCloseClicked();
    default:
        QWidget::keyReleaseEvent(ke);
        break;
    }
    ke->ignore();
}

void DSView::closeEvent(QCloseEvent * event)
{
    QWidget::closeEvent(event);
}

/// Ignore the double click event.
void DSView::mouseDoubleClickEvent(QMouseEvent*me)
{
    me->accept();
}

bool DSView::eventFilter(QObject *obj, QEvent *e)
{
    qDebug("Select event:%d", e->type());
    if (e->type() == QEvent::MouseButtonRelease && obj->isWidgetType())
    {
        onyx::screen::instance().updateWidget(0, onyx::screen::ScreenProxy::GU);
    }
    return QObject::eventFilter(obj, e);
}

void DSView::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(rect(), Qt::white);
    QFont font = QApplication::font();
    font.setPointSize(34);
    painter.setFont(font);
    QFontMetrics fm(font);

    painter.drawText(QRect(0, BUTTON_HEIGHT, width(), height() - BUTTON_HEIGHT), Qt::AlignHCenter | Qt::AlignTop, currentState());

    int SPACING = 20;
    QImage image;
    image.load(":/images/connection.png");
    int x = (width() - image.width()) / 2;
    int y = height() - BUTTON_HEIGHT - image.height() - SPACING;
    painter.drawImage(QPoint(x, y), image);
}

void DSView::onStartClicked()
{
    configNetwork();
    vsftpd_.start();
    update();
    onyx::screen::watcher().enqueue(this, onyx::screen::ScreenProxy::GC);
}

void DSView::onCloseClicked()
{
    vsftpd_.stop();
    sys::SysStatus::instance().stopWpaSupplicant();
    qApp->exit();
}

void DSView::createLayout()
{
    layout_.addWidget(&start_, 0, Qt::AlignBottom);
    layout_.addWidget(&close_, 0, Qt::AlignBottom);

    start_.setFixedHeight(BUTTON_HEIGHT);
    close_.setFixedHeight(BUTTON_HEIGHT);

    connect(&start_, SIGNAL(clicked(bool)), this, SLOT(onStartClicked()));
    connect(&close_, SIGNAL(clicked(bool)), this, SLOT(onCloseClicked()));
}

bool DSView::start()
{
    onStartClicked();
    return true;
}

bool DSView::stop()
{
    onCloseClicked();
    return true;
}

bool DSView::exec(const QStringList & args)
{
    return true;
}

QString DSView::currentState()
{
    QString result(tr("Server is not running.\nClick Start to lunch server."));
    QList<QNetworkInterface> all = QNetworkInterface::allInterfaces();
    foreach(QNetworkInterface ni, all)
    {
        if (ni.flags().testFlag(QNetworkInterface::IsUp) && !ni.addressEntries().empty())
        {
#ifndef WIN32
            if (ni.name().contains("eth", Qt::CaseInsensitive) ||
                ni.name().contains("wlan", Qt::CaseInsensitive))
            {
#endif
                foreach(QNetworkAddressEntry entry, ni.addressEntries())
                {
                    result = tr("Server: ftp://%1");
                    result = result.arg(entry.ip().toString());
                    result += ("\n");
                    result += tr("You can transfer files to device now.");
                    return result;
                }
#ifndef WIN32
            }
#endif
        }
    }
    return result;
}

