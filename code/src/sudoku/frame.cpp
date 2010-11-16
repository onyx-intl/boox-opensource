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

    QPainter painter ( this );
    painter.setRenderHint ( QPainter::Antialiasing, true );

    painter.setPen ( Qt::black);
    painter.setBrush (Qt::NoBrush);
    painter.drawRoundedRect ( QRectF ( 0.5, 0.5, width() - 1, height() - 1 ), 3, 3);

    if ( m_highlight ) {
        QRadialGradient gradient(25, 25, 100, 100, 100);
        gradient.setColorAt(0, QColor::fromRgbF(0, 0, 0, 1));
        gradient.setColorAt(1, QColor::fromRgbF(1, 1, 1, 1));
        QBrush brush(gradient);
        painter.setBrush ( brush );
        painter.setPen ( Qt::black);
        painter.drawRoundedRect ( QRectF ( 0, 0, width(), height()), 3, 3);
    }

    if ( m_highlight_border ) {
        QRadialGradient gradient(25, 25, 100, 100, 100);
        gradient.setColorAt(1, QColor::fromRgbF(0, 0, 0, 1));
        gradient.setColorAt(0, QColor::fromRgbF(1, 1, 1, 1));
        QBrush brush(gradient);
        painter.setBrush ( brush );
        painter.setPen ( Qt::black);
        painter.setBrush ( Qt::NoBrush );
        painter.drawRoundedRect ( QRectF ( 1.5, 1.5, width() - 3, height() - 3 ), 3, 3 );
    }
}

}
}
// kate: indent-mode cstyle; space-indent on; indent-width 4; 
