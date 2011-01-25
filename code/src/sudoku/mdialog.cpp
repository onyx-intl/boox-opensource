#include "mdialog.h"
#include <QtGui/QKeyEvent>
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
    onyx::screen::instance().enableUpdate ( true );
    onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::DW);
    setBackgroundRole(QPalette::Light);
    setAutoFillBackground(true);
    setWindowFlags(Qt::WindowStaysOnTopHint|Qt::FramelessWindowHint);
    layout_key = new QGridLayout(this);
    QButtonGroup *group_key = new QButtonGroup(this);
    layout_key->setColumnMinimumWidth(3,40);
    list_key.clear();
    for ( int i = 1; i < 10; ++i ) {
        MToolButton *key = new MToolButton(this);
        key->setText(QString::number(i));
        list_key.insert (i-1, key);
        group_key->addButton ( key, i );
        layout_key->addWidget ( key, (i - 1) / 3, (i - 1) % 3 );
        connect(key, SIGNAL(clicked(bool)),this, SLOT(accept()));
        MToolButton *mode = new MToolButton(this);
        mode->setText(QString::number(i));
        list_key.insert(8+i, mode);
        group_key->addButton ( mode, 9 + i );
        layout_key->addWidget ( mode, (i - 1) / 3, (i - 1) % 3+4 );
        connect(mode, SIGNAL(clicked(bool)),this, SLOT(accept()));
    }
    group_key->button ( qBound ( 1, QSettings().value ( "Key", 1 ).toInt(),18 ) )->click();
    connect ( group_key, SIGNAL (buttonClicked(int)), this, SLOT ( setActiveKey ( int ) ) ); ///<will change keypad
    setStyleSheet("QLabel{ font: 32px;}");
    QLabel *val = new QLabel(this);
    val->setText(tr("<center><b>Value</b></center>"));
    val->setTextFormat(Qt::RichText);
    layout_key->addWidget(val,4,0,1,3);
    QLabel *note = new QLabel(this);;
    note->setText(tr("<center><b>Note</b></center>"));
    note->setTextFormat(Qt::RichText);
    layout_key->addWidget(note,4,4,1,3);

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
    //QDialog::keyPressEvent(event);
}

void MDialog::mouseMoveEvent(QMouseEvent* event) {
    QDialog::mouseMoveEvent(event);
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
    //TODO just the buttons
    if (e->type() == QEvent::UpdateRequest)
    {
        onyx::screen::instance().updateWidget(this);
    }
    return ret;
}

void MDialog::paintEvent(QPaintEvent* e)
{

    int h = height();
    int w = width();
    QRect rc_t(0,0,w,6);
    QRect rc_l(0,0,6,h);
    QRect rc_r(w-10,0,10,h);
    QRect rc_b(0,h-12,w,12);
    QBrush brush(Qt::gray, Qt::SolidPattern);
    QPainter painter(this);
    painter.fillRect(rc_t, brush);
    painter.fillRect(rc_l, brush);
    painter.fillRect(rc_r, brush);
    painter.fillRect(rc_b, brush);
    QDialog::paintEvent(e);

}

