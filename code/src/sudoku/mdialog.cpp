#include "mdialog.h"
#include "QKeyEvent"
#include "QGridLayout"
#include "QButtonGroup"
#include "QSettings"
#include "QDebug"
#include "onyx/screen/screen_proxy.h"

MDialog::MDialog(QWidget* parent): QDialog(parent) {
    onyx::screen::instance().enableUpdate ( true );
    onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::GU);
    setWindowModality(Qt::ApplicationModal);
    setWindowFlags(Qt::FramelessWindowHint);
    QGridLayout *layout = new QGridLayout(this);
    QButtonGroup *group = new QButtonGroup(this);
    QList<MToolButton*> list;
    for ( int i = 1; i < 10; ++i ) {
        MToolButton *key = new MToolButton(this);
        key->setText(QString::number(i));
        //key->resize(40,40);
        key->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
        //setFocusPolicy(Qt::NoFocus);
        list.append (key);
        group->addButton ( key, i );
        layout->addWidget ( key, (i - 1) / 3, (i - 1) % 3 );
        connect(key, SIGNAL(clicked(bool)),this, SLOT(accept()));
    }
    group->button ( qBound ( 1, QSettings().value ( "Key", 1 ).toInt(), 10 ) )->click();
    connect ( group, SIGNAL (buttonClicked(int)), this, SLOT ( setActiveKey ( int ) ) ); ///<will change keypad
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    list.at(0)->setFocus();
    setLayout(layout);
}

void MDialog::keyPressEvent(QKeyEvent* event) {
    switch (event->key()){
    case Qt::Key_Up:
        foreach(MToolButton *tmp_button, list)
        if (focusWidget() == tmp_button) {
            if (1<(list.indexOf(tmp_button)+6)%9<10) {
                list.at((list.indexOf(tmp_button)+6)%9)->setFocus();
            } else {
                list.at(0)->setFocus();
            }
            return;
        }
        break;
    case Qt::Key_Down:
        foreach(MToolButton *tmp_button, list)
        if (focusWidget() == tmp_button) {
            if (1<(list.indexOf(tmp_button)+3)%9<10) {
                list.at((list.indexOf(tmp_button)+3)%9)->setFocus();
            } else {
                list.at(0)->setFocus();
            }
            return;
        }
        break;
    case Qt::Key_Left:
        qDebug()<<__LINE__<<"Key_Left";
        foreach(MToolButton *tmp_button, list)
        if (focusWidget() == tmp_button) {
            if (1<(list.indexOf(tmp_button)+8)%9<10) {
                list.at((list.indexOf(tmp_button)+8)%9)->setFocus();
            } else {
                list.at(0)->setFocus();
            }
            return;
        }
        break;
    case Qt::Key_Right:
        foreach(MToolButton *tmp_button, list)
        if (focusWidget() == tmp_button) {
            if (1<(list.indexOf(tmp_button)+1)%9<10) {
                list.at((list.indexOf(tmp_button)+1)%9)->setFocus();
            } else {
                list.at(0)->setFocus();
            }
            return;
        }
          break;
    default:
        break;
    }
    update();
    QDialog::keyPressEvent(event);
}

void MDialog::mouseMoveEvent(QMouseEvent* event) {
    QDialog::mouseMoveEvent(event);
}


void MDialog::setActiveKey(int k) {
    emit ActiveKey(k);
}

bool MDialog::event(QEvent* e) {
    bool ret = QDialog::event ( e );
        if ( e->type() == QEvent::UpdateRequest )  {
        if ( onyx::screen::instance().isUpdateEnabled() ) {
            static int count = 0;

            if ( onyx::screen::instance().defaultWaveform() == onyx::screen::ScreenProxy::DW ) {
                qDebug ( "Explorer screen ScreenProxy::DW update %d", count++ );
                onyx::screen::instance().updateWidget ( this, onyx::screen::ScreenProxy::DW, true );
                onyx::screen::instance().setDefaultWaveform ( onyx::screen::ScreenProxy::GU );

            } else  {
                qDebug ( "Explorer screen full update %d", count++ );
                onyx::screen::instance().updateWidget ( this, onyx::screen::ScreenProxy::GU );
            }
        }
    }
    return ret;
}
