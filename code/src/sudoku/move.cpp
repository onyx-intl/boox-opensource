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
#include "move.h"

#include "cell.h"
namespace onyx
{
namespace simsu {


/*****************************************************************************/

Move::Move ( Cell* cell, int id, int column, int row, bool note, int value )
                : m_cell ( cell ),
                m_id ( id )
{
        setText ( QString ( "%1%2%3%4" ).arg ( column ).arg ( row ).arg ( note ? "n" : "v" ).arg ( value ) );
}

/*****************************************************************************/

void Move::redo()
{
        m_cell->setState ( m_id + 1 );
}

/*****************************************************************************/

void Move::undo()
{
        m_cell->setState ( m_id );
}

/*****************************************************************************/
}
}
// kate: indent-mode cstyle; space-indent on; indent-width 8; 
