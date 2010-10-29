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

#ifndef  ONYX_SIMSU_MOVE_H
#define  ONYX_SIMSU_MOVE_H

#include <QUndoCommand>
namespace onyx
{
namespace simsu {
class Cell;

class Move : public QUndoCommand
{
public:
        Move ( Cell* cell, int id, int column, int row, bool note, int value );

        virtual void redo();
        virtual void undo();

private:
        Cell* m_cell;
        int m_id;
};
}
}
#endif
// kate: indent-mode cstyle; space-indent on; indent-width 8; 
