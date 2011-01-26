/*#include "dialog.h"

#include "dialog.h"
#include "pattern.h"
#include "onyx/ui/onyx_dialog.h"
using namespace ui;
namespace onyx {
namespace simsu {

Dialog::Dialog ( QWidget* parent ) : QDialog ( parent ) {
    //setWindowTitle ( tr ( "New Game" ) );
    // setWindowFlags(Qt::FramelessWindowHint);
    QStackedWidget* preview = new QStackedWidget ( this );
    QComboBox* symmetry_box = new QComboBox ( this );

    for ( int i = Pattern::Rotational180; i <= Pattern::None; ++i ) {
        symmetry_box->addItem ( Pattern::name ( i ), i );
        QLabel* image = new QLabel ( preview );
        image->setPixmap ( Pattern::icon ( i ) );
        preview->addWidget ( image );
    }
    preview->setCurrentIndex ( 0 );
    connect ( symmetry_box, SIGNAL ( currentIndexChanged ( int ) ), preview, SLOT ( setCurrentIndex ( int ) ) );
    symmetry_box->setCurrentIndex ( symmetry_box->findData ( settings.value ( "Symmetry", Pattern::Rotational180 ).toInt() ) );
    QComboBox* algorithm_box = new QComboBox ( this );
    algorithm_box->addItem ( tr ( "Dancing Links" ), 0 );
    algorithm_box->addItem ( tr ( "Slice and Dice" ), 1 );
    algorithm_box->setCurrentIndex ( algorithm_box->findData ( settings.value ( "Algorithm" ).toInt() ) );
    QSpinBox* seed_box = new QSpinBox ( this );
    seed_box->setRange ( 0, 2147483647 );
    seed_box->setSpecialValueText ( tr ( "Random" ) );
    QDialogButtonBox* buttons = new QDialogButtonBox ( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this );
    connect ( buttons, SIGNAL ( accepted() ), this, SLOT ( accept() ) );
    connect ( buttons, SIGNAL ( rejected() ), this, SLOT ( reject() ) );
    QFormLayout* contents_layout = new QFormLayout;
    contents_layout->addRow ( QString(), preview );
    contents_layout->addRow ( tr ( "Symmetry:" ), symmetry_box );
    contents_layout->addRow ( tr ( "Algorithm:" ), algorithm_box );
    contents_layout->addRow ( tr ( "Seed:" ), seed_box );
    QVBoxLayout* layout = new QVBoxLayout ( this );
    layout->addLayout ( contents_layout );
    layout->addSpacing ( 18 );
    layout->addWidget ( buttons );
}

void Dialog::accept() {
    QDialog::accept();
    int seed=settings.value("Seed").toInt(),
             symmetry=settings.value("Symmetry").toInt(),
             algorithm=settings.value("Algorithm").toInt();
    emit setGameSignal ( seed, symmetry, algorithm );
}

bool Dialog::event ( QEvent* event ) {
    bool ret= QDialog::event ( event );
    if (event->type() == QEvent::UpdateRequest)
    {
        qDebug() << "Dialog::event";
        onyx::screen::instance().updateWidget(this);
    }
    return ret;
}

void Dialog::mouseMoveEvent ( QMouseEvent* event )
{

}


}
}*/
// kate: indent-mode cstyle; space-indent on; indent-width 4;
