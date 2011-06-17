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

    void newPuzzle ( qint32 seed = 0, qint32 symmetry = -1, qint32 algorithm = -1, bool load = false );

    qint32 activeKey() const {
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

    void moveFocus ( qint32 column, qint32 row, qint32 deltax, qint32 deltay );

    Cell* cell ( qint32 column, qint32 row ) const {
        column = qBound ( 0, column, 9 );
        row = qBound ( 0, row, 9 );
        return m_cells[column][row];
    }

    QUndoStack* moves() {
        return m_moves;
    }

    qint32 getRow() {
        return m_row;
    }
    qint32 getColumn() {
        return m_column;
    }
signals:
    void activeKeyChanged ( qint32 key );
    void notesModeChanged ( bool mode );
    void toShowBoard();
    void win();
public slots:
    void showWrong ( bool show = true );
    void setActiveKey ( qint32 key );
    void setActiveModeKey ( qint32 key );
    void setAutoSwitch ( bool auto_switch );
    void setHighlightActive ( bool highlight );
    void setMode ( qint32 mode );
    void showBoard()
    {
        emit toShowBoard();
    }
protected:
    bool event(QEvent *e) {
        return Frame::event ( e );
    }
private:
    Cell* m_cells[9][9];
    Puzzle* m_puzzle;
    qint32 m_active_key;
    bool m_auto_switch;
    bool m_highlight_active;
    bool m_notes_mode;
    bool m_finished;
//     QLabel* m_message;
    QUndoStack* m_moves;
    qint32 m_row;
    qint32 m_column;
};
}
}
#endif
// kate: indent-mode cstyle; space-indent on; indent-width 4;
