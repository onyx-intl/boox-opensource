#ifndef SIMSU_H
#define SIMSU_H

#include <QWidget>
#include "onyx/base/base.h"
#include "onyx/screen/screen_proxy.h"
#include "onyx/ui/ui.h"
#include "onyx/ui/message_dialog.h"

class QBoxLayout;
class QButtonGroup;
class QToolButton;
class QGridLayout;

using namespace ui;

namespace onyx {
namespace simsu {
class Board;
class Cell;
class Simsu : public QWidget {
        Q_OBJECT
    public:
        Simsu ( QWidget *parent = 0, Qt::WindowFlags f = 0 );

    protected:
        virtual void closeEvent ( QCloseEvent *event );
        virtual void wheelEvent ( QWheelEvent *event );
        virtual bool event ( QEvent *event );
        virtual void keyPressEvent (QKeyEvent * event);
    private slots:
        void newGame();
        void showDetails();
        void about();
        void activeKeyChanged ( int key );
        void notesModeChanged ( bool mode );
        void toggleMode( bool mode );
        void toggleWidescreen ( bool checked );
        void togglePopMode(bool pop);
        bool getPopMode(){return pop_;}
        void popUpdialog();
    private:
        Board *m_board;
        OnyxPushButton *new_button;///<to get focus
        OnyxPushButton *key_button;///<to get focus
        OnyxPushButton *mode_button;
        OnyxPushButton *dialog_button;
        OnyxPushButton *highlight_button;
        QButtonGroup *m_key_buttons;
        QButtonGroup *m_act_buttons;
        QGridLayout *m_keys_layout;
        QGridLayout *m_act_layout;
        QBoxLayout *m_mode_layout;
        QBoxLayout *m_layout;
        QBoxLayout *m_hlayout;
        QList<OnyxPushButton *> m_act_list_buttons;
        QList<OnyxPushButton *> m_keys_list_buttons;
        bool pop_;
};
}
}
#endif // SIMSU_H
