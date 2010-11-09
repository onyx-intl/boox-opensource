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

#ifndef ONYX_SIMSU_SQUARE_H
#define ONYX_SIMSU_SQUARE_H

#include <QWidget>
#include "onyx/base/base.h"
#include "onyx/screen/screen_proxy.h"
#include "onyx/ui/ui.h"
namespace onyx
{
namespace simsu {
class Square : public QWidget
{
public:
        Square ( QWidget* parent = 0 );

        void setChild ( QWidget* child );

protected:
        virtual void resizeEvent ( QResizeEvent* event );

private:
        QWidget* m_child;
};
}
}
#endif
// kate: indent-mode cstyle; space-indent on; indent-width 8; 
