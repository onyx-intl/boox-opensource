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

#include "frame.h"

#include <QPainter>
namespace onyx {
namespace simsu {
/*****************************************************************************/

Frame::Frame ( QWidget* parent )
        : QWidget ( parent ), m_highlight ( false ), m_highlight_border ( false ) {
}

/*****************************************************************************/

void Frame::paintEvent ( QPaintEvent* event ) {
    QWidget::paintEvent ( event );
/*
    static QRadialGradient gradient_hb(25, 25, 50, 50, 50);
    gradient_hb.setColorAt(0, QColor::fromRgbF(0.7, 0.7, 0.7, 1));
    gradient_hb.setColorAt(1, QColor::fromRgbF(0.3, 0.3, 0.3, 1));
    static QRadialGradient gradient_h(25, 25, 50, 50, 50);
    gradient_h.setColorAt(1, QColor::fromRgbF(0.6, 0.6, 0.6, 1));
    gradient_h.setColorAt(0, QColor::fromRgbF(0.2, 0.2, 0.2, 1));
*/
    QPainter painter ( this );
    painter.setRenderHint ( QPainter::Antialiasing, true );
    painter.setPen (QPen(Qt::black, 5));
    painter.setBrush (Qt::NoBrush);
    painter.drawRoundedRect ( QRectF ( 0.5, 0.5, width() - 1, height() - 1 ), 5, 5);

    if ( m_highlight ) {
        QBrush brush(Qt::black);
        painter.setBrush ( brush );
        painter.drawRoundedRect ( QRectF ( 0, 0, width() , height() ), 5, 5);
    }

    if ( m_highlight_border ) {
        QBrush brush(Qt::white);
        painter.setBrush ( brush );
        painter.drawRoundedRect ( QRectF ( 5, 5, width() - 10, height() - 10 ), 5, 5 );
    }
}

}
}
// kate: indent-mode cstyle; space-indent on; indent-width 4; 
