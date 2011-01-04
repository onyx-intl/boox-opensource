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

#ifndef  ONYX_SIMSU_BOARD_H
#define  ONYX_SIMSU_BOARD_H

#include "frame.h"
class QLabel;
class QUndoStack;

namespace onyx {
namespace simsu {

class Cell;
class Puzzle;
class Board : public Frame {
    Q_OBJECT
public:
    Board ( QWidget* parent = 0 );
    ~Board();

    void newPuzzle ( int seed = 0, int symmetry = -1, int algorithm = -1, bool load = false );

    int activeKey() const {
        return m_active_key;
    }

    bool autoSwitch() const {
        return m_auto_switch;
    }

    bool highlightActive() const {
        return m_highlight_active;
    }

    bool notesMode() const {
        return m_notes_mode;
    }

    bool isFinished() const {
        return m_finished;
    }

    void checkFinished();

    void moveFocus ( int column, int row, int deltax, int deltay );

    Cell* cell ( int column, int row ) const {
        column = qBound ( 0, column, 9 );
        row = qBound ( 0, row, 9 );
        return m_cells[column][row];
    }

    QUndoStack* moves() {
        return m_moves;
    }

    int getRow(){
        return m_row;}
    int getColumn(){
        return m_column;}
signals:
    void activeKeyChanged ( int key );
    void notesModeChanged ( bool mode );

public slots:
    void showWrong ( bool show = true );
    void setActiveKey ( int key );
    void setAutoSwitch ( bool auto_switch );
    void setHighlightActive ( bool highlight );
    void setMode ( int mode );
protected:
    bool event(QEvent *e) {
        return Frame::event ( e );
    }
private:
    Cell* m_cells[9][9];
    Puzzle* m_puzzle;
    int m_active_key;
    bool m_auto_switch;
    bool m_highlight_active;
    bool m_notes_mode;
    bool m_finished;
    QLabel* m_message;
    QUndoStack* m_moves;
    int m_row;
    int m_column;
};
}
}
#endif
// kate: indent-mode cstyle; space-indent on; indent-width 4; 
