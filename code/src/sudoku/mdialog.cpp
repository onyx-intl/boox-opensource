#include "mdialog.h"
#include "QKeyEvent"
#include "QGridLayout"
#include "QButtonGroup"
#include "QSettings"
#include "QDebug"
#include "onyx/screen/screen_proxy.h"

MDialog::MDialog(QWidget* parent): QDialog(parent) {
    onyx::screen::instance().enableUpdate ( true );
    onyx::screen::instance().setDefaultWaveform(onyx::screen::ScreenProxy::GC);
    setWindowModality(Qt::ApplicationModal);
    setWindowFlags(Qt::FramelessWindowHint);
    QGridLayout *layout = new QGridLayout(this);
    QButtonGroup *group = new QButtonGroup(this);

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
    //TODO set current_button_ to selected one
    current_button_ = 0;
    list.at(0)->setFocus();
    setLayout(layout);
}

void MDialog::keyPressEvent(QKeyEvent* event) {
    current_button_ =  list.indexOf(static_cast<MToolButton*> (focusWidget()));
    switch (event->key()){
        case Qt::Key_Escape:
            close();
            break;
        case Qt::Key_Up:
            if (0<(current_button_+6)%9<9) {
                current_button_ = (current_button_+6)%9;
            }
            break;
        case Qt::Key_Down:
            if (0<(current_button_+3)%9<9) {
                current_button_ = (current_button_+3)%9;
            }
            break;
        case Qt::Key_Left:
            if (0<(current_button_+8)%9<9) {
                current_button_ = (current_button_+8)%9;
            }
            break;
        case Qt::Key_Right:
            if (0<(current_button_+1)%9<9) {
                current_button_ = (current_button_+1)%9;
            }
            break;
        default:
            break;
    }
    update();
}

void MDialog::mouseMoveEvent(QMouseEvent* event) {
    QDialog::mouseMoveEvent(event);
}


void MDialog::setActiveKey(int k) {
    emit ActiveKey(k);
}

bool MDialog::event(QEvent* e) {
    bool ret = QDialog::event ( e );
    //TODO just the buttons
    if (e->type() == QEvent::UpdateRequest)
    {
        if (list.size()<9) {
            onyx::screen::instance().updateWidget(this);
        } else {
            onyx::screen::instance().updateWidget(focusWidget());
        }
    }
    return ret;
}
