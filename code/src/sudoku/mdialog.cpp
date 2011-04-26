#include "mdialog.h"
#include <QtGui/QKeyEvent>
#include <QtGui/QPainter>
#include <QtGui/QGridLayout>
#include <QtGui/QWidget>
#include <QtGui/QButtonGroup>
#include <QtCore/QSettings>
#include <QtCore/QDebug>
#include <QtGui/QBoxLayout>
#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include "onyx/screen/screen_proxy.h"

MDialog::MDialog(QWidget* parent):QDialog(parent) {
#ifndef BUILD_FOR_FB
    onyx::screen::instance().enableUpdate ( true );
    onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::DW);
#endif
    setBackgroundRole(QPalette::Light);
    setAutoFillBackground(true);
    setWindowFlags(Qt::WindowStaysOnTopHint|Qt::FramelessWindowHint);
    layout_key = new QGridLayout(this);
    QButtonGroup *group_key = new QButtonGroup(this);
    layout_key->setColumnMinimumWidth(3,32);
    list_key.clear();
    for ( int i = 1; i < 10; ++i ) {
        MToolButton *key = new MToolButton(this);
        key->setText(QString::number(i));
        key->setFont(QFont("sans",28,QFont::Black));
        list_key.insert (i-1, key);
        group_key->addButton ( key, i );
        layout_key->addWidget ( key, (i - 1) / 3, (i - 1) % 3 );
        connect(key, SIGNAL(clicked(bool)),this, SLOT(accept()));
        MToolButton *mode = new MToolButton(this);
        mode->setText(QString::number(i));
        mode->setFont(QFont("mono",24,QFont::DemiBold));
        list_key.insert(8+i, mode);
        group_key->addButton ( mode, 9 + i );
        layout_key->addWidget ( mode, (i - 1) / 3, (i - 1) % 3+4 );
        connect(mode, SIGNAL(clicked(bool)),this, SLOT(accept()));
    }
    group_key->button ( qBound ( 1, QSettings().value ( "Key", 1 ).toInt(),18 ) )->click();
    connect ( group_key, SIGNAL (buttonClicked(int)), this, SLOT ( setActiveKey ( int ) ) ); ///<will change keypad
    setLayout(layout_key);
    list_key.at(0)->setFocus();
}

void MDialog::keyPressEvent(QKeyEvent* event) {
//     update();
    int current_button_ =  list_key.indexOf(static_cast<MToolButton*> (focusWidget()));
    switch (event->key()) {
        case Qt::Key_Right:
            if ((current_button_ - 11) % 3 == 0 ) {
                current_button_ -= 11;
                list_key.at(current_button_)->setFocus();
            }
            break;
        case  Qt::Key_Left:
            if ((current_button_ == 0 ) || (current_button_ == 3 ) || (current_button_ == 6 )) {
                current_button_ += 11;
                list_key.at(current_button_)->setFocus();
            }
            break;
        case Qt::Key_Escape:
            close();
        default:
            break;
    }
}

void MDialog::mouseMoveEvent(QMouseEvent* event) {
}


void MDialog::setActiveKey(int k) {
    if (k<10) {
        emit ActiveKey(k);
    } else {
        setActiveModeKey(1+k%10);
    }
}

void MDialog::setActiveModeKey(int k)
{
    emit ActiveModeKey(k);
}

bool MDialog::event(QEvent* e) {
    bool ret = QDialog::event ( e );
#ifndef BUILD_FOR_FB
    if (e->type() == QEvent::UpdateRequest)
    {
        onyx::screen::instance().updateWidget(this, onyx::screen::ScreenProxy::GU, onyx::screen::ScreenCommand::WAIT_NONE);
    }
#endif
    return ret;
}

void MDialog::paintEvent(QPaintEvent* e)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::HighQualityAntialiasing,true);
    painter.setRenderHint(QPainter::Antialiasing,true);
    painter.setRenderHint(QPainter::TextAntialiasing,true);
    painter.setBrush(QColor(96,96,96));
    painter.drawRoundedRect(rect(),5,5);
    painter.setBrush(QColor(255,255,255));
    painter.drawRoundedRect(rect().adjusted(6,6,rect().x()-6,rect().y()-6),5,5);
}

