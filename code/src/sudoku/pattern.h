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

#ifndef  ONYX_SIMSU_PATTERN_H
#define  ONYX_SIMSU_PATTERN_H

#include <QCoreApplication>
#include <QHash>
#include <QList>
#include <QPoint>
#include <QString>
namespace onyx
{
namespace simsu {

class Pattern
{
        Q_DECLARE_TR_FUNCTIONS ( Pattern );

public:
        enum Symmetry {
                Rotational180,
                RotationalFull,
                Horizontal,
                Vertical,
                HorizontalVertical,
                Diagonal,
                AntiDiagonal,
                DiagonalAntiDiagonal,
                FullDihedral,
                Random,
                None
        };

        virtual ~Pattern() {
        }

        virtual int count() const = 0;
        virtual QList<QPoint> pattern ( const QPoint& cell ) const = 0;

        static QString name ( int symmetry ) {
                static QHash<int, QString> names;
                if ( names.isEmpty() ) {
                        names[Rotational180] = tr ( "180\260 Rotational" );
                        names[RotationalFull] = tr ( "Full Rotational" );
                        names[Horizontal] = tr ( "Horizontal" );
                        names[Vertical] = tr ( "Vertical" );
                        names[HorizontalVertical] = tr ( "Horizontal & Vertical" );
                        names[Diagonal] = tr ( "Diagonal" );
                        names[AntiDiagonal] = tr ( "Anti-Diagonal" );
                        names[DiagonalAntiDiagonal] = tr ( "Diagonal & Anti-Diagonal" );
                        names[FullDihedral] = tr ( "Full Dihedral" );
                        names[Random] = tr ( "Random" );
                        names[None] = tr ( "None" );
                }
                return names.value ( symmetry );
        }

        static QString icon ( int symmetry ) {
                static QHash<int, QString> icons;
                if ( icons.isEmpty() ) {
                        icons[Rotational180] = ":/rotational_180.png";
                        icons[RotationalFull] = ":/rotational_full.png";
                        icons[Horizontal] = ":/horizontal.png";
                        icons[Vertical] = ":/vertical.png";
                        icons[HorizontalVertical] = ":/horizontal_vertical.png";
                        icons[Diagonal] = ":/diagonal.png";
                        icons[AntiDiagonal] = ":/anti_diagonal.png";
                        icons[DiagonalAntiDiagonal] = ":/diagonal_anti_diagonal.png";
                        icons[FullDihedral] = ":/dihedral.png";
                        icons[Random] = ":/random.png";
                        icons[None] = ":/none.png";
                }
                return icons.value ( symmetry );
        }
};


class PatternFullDihedral : public Pattern
{
public:
        virtual int count() const {
                return 8;
        }

        virtual QList<QPoint> pattern ( const QPoint& cell ) const {
                return QList<QPoint>()
                       << cell
                       << QPoint ( cell.x(), 8 - cell.y() )
                       << QPoint ( 8 - cell.y(), cell.x() )
                       << QPoint ( cell.y(), cell.x() )
                       << QPoint ( 8 - cell.x(), 8-cell.y() )
                       << QPoint ( 8 - cell.x(), cell.y() )
                       << QPoint ( cell.y(), 8 - cell.x() )
                       << QPoint ( 8 - cell.y(), 8 - cell.x() );
        }
};


class PatternRotational180 : public Pattern
{
public:
        virtual int count() const {
                return 2;
        }

        virtual QList<QPoint> pattern ( const QPoint& cell ) const {
                return QList<QPoint>()
                       << cell
                       << QPoint ( 8 - cell.x(), 8 - cell.y() );
        }
};


class PatternRotationalFull : public Pattern
{
public:
        virtual int count() const {
                return 4;
        }

        virtual QList<QPoint> pattern ( const QPoint& cell ) const {
                return QList<QPoint>()
                       << cell
                       << QPoint ( 8 - cell.y(), cell.x() )
                       << QPoint ( 8 - cell.x(), 8 - cell.y() )
                       << QPoint ( cell.y(), 8 - cell.x() );
        }
};


class PatternHorizontal : public Pattern
{
public:
        virtual int count() const {
                return 2;
        }

        virtual QList<QPoint> pattern ( const QPoint& cell ) const {
                return QList<QPoint>()
                       << cell
                       << QPoint ( 8 - cell.x(), cell.y() );
        }
};


class PatternVertical : public Pattern
{
public:
        virtual int count() const {
                return 2;
        }

        virtual QList<QPoint> pattern ( const QPoint& cell ) const {
                return QList<QPoint>()
                       << cell
                       << QPoint ( cell.x(), 8 - cell.y() );
        }
};


class PatternHorizontalVertical : public Pattern
{
public:
        virtual int count() const {
                return 4;
        }

        virtual QList<QPoint> pattern ( const QPoint& cell ) const {
                return QList<QPoint>()
                       << cell
                       << QPoint ( 8 - cell.x(), cell.y() )
                       << QPoint ( cell.x(), 8 - cell.y() )
                       << QPoint ( 8 - cell.x(), 8- cell.y() );
        }
};


class PatternDiagonal : public Pattern
{
public:
        virtual int count() const {
                return 2;
        }

        virtual QList<QPoint> pattern ( const QPoint& cell ) const {
                return QList<QPoint>()
                       << cell
                       << QPoint ( cell.y(), cell.x() );
        }
};


class PatternAntiDiagonal : public Pattern
{
public:
        virtual int count() const {
                return 2;
        }

        virtual QList<QPoint> pattern ( const QPoint& cell ) const {
                return QList<QPoint>()
                       << cell
                       << QPoint ( 8 - cell.y(), 8 - cell.x() );
        }
};


class PatternDiagonalAntiDiagonal : public Pattern
{
public:
        virtual int count() const {
                return 4;
        }

        virtual QList<QPoint> pattern ( const QPoint& cell ) const {
                return QList<QPoint>()
                       << cell
                       << QPoint ( cell.y(), cell.x() )
                       << QPoint ( 8 - cell.y(), 8 - cell.x() )
                       << QPoint ( 8 - cell.x(), 8 - cell.y() );
        }
};


class PatternNone : public Pattern
{
public:
        virtual int count() const {
                return 1;
        }

        virtual QList<QPoint> pattern ( const QPoint& cell ) const {
                return QList<QPoint>()
                       << cell;
        }
};
}
}
#endif
// kate: indent-mode cstyle; space-indent on; indent-width 8; 
