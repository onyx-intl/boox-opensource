/***********************************************************************
 *
 * Copyright (C) 2009 Graeme Gott <graeme@gottcode.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ***********************************************************************/

#ifndef  ONYX_SIMSU_CELL_H
#define  ONYX_SIMSU_CELL_H

#include "frame.h"


namespace onyx
{
namespace simsu {
class Board;
class Puzzle;

class Cell : public Frame
{
        Q_OBJECT
public:
        Cell ( int column, int row, Board* board, QWidget* parent = 0 );

        bool isCorrect() const;
        void setHintVisible ( bool visible );
        void setPuzzle ( Puzzle* puzzle );
        void setState ( int state );
        void showWrong ( bool show );
        const bool given() {return m_given;}
        void setSelected(bool val) {
            selected = val;
        }

protected:
        virtual void focusInEvent ( QFocusEvent* event );
        virtual void focusOutEvent ( QFocusEvent* event );
        virtual void keyPressEvent ( QKeyEvent* event );
        virtual void mouseMoveEvent ( QMouseEvent* event );
        virtual void mousePressEvent ( QMouseEvent* event );
        virtual void paintEvent ( QPaintEvent* event );
        virtual void resizeEvent ( QResizeEvent* event );
private:
        struct State {
                int value;
                bool notes[9];
        };
        QList<State> m_states;
        int m_current_state;
public slots:
        void updateValue();
private:
        void checkConflict ( Cell* cell );
        void updateFont();
private:
        int m_column;
        int m_row;
        QList<Cell*> m_conflicts;
        bool m_wrong;
        bool m_given;
        bool selected;
        Board* m_board;
        Puzzle* m_puzzle;
};
}
}
#endif
