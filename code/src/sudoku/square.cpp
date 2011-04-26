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
#include <QResizeEvent>
#include <QStyle>
#include "square.h"
namespace onyx {
namespace simsu {

/*****************************************************************************/

Square::Square ( QWidget* parent )
        : QWidget ( parent ), m_child ( 0 ) {
    setMinimumSize (430, 430 );
//     setSizePolicy ( QSizePolicy::Expanding, QSizePolicy::Expanding);
}

/*****************************************************************************/

void Square::setChild ( QWidget* child ) {
    m_child = child;
    m_child->setParent ( this );
    resize ( size() );
}

/*****************************************************************************/

void Square::resizeEvent ( QResizeEvent* event ) {
    QWidget::resizeEvent ( event );
    if ( m_child ) {
        QRect region = contentsRect();
        int size = qMin ( region.width(), region.height() );
        QRect rect = QStyle::alignedRect ( Qt::LeftToRight, Qt::AlignCenter, QSize ( size, size ), region );
        m_child->setGeometry ( rect );
    }
}

/*****************************************************************************/
}
}
// kate: indent-mode cstyle; space-indent on; indent-width 4;
