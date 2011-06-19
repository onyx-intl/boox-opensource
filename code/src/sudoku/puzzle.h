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

#ifndef  ONYX_SIMSU_PUZZLE_H
#define  ONYX_SIMSU_PUZZLE_H

#include <QList>
#include <QPoint>

namespace onyx
{
namespace simsu {

class Pattern;

class Puzzle
{
public:
        Puzzle();
        virtual ~Puzzle();

        void generate ( unsigned int seed, int symmetry );

        int given ( int x, int y ) const {
                x = qBound ( 0, x, 9 );
                y = qBound ( 0, y, 9 );
                return m_givens[x][y];
        }

        int value ( int x, int y ) const {
                x = qBound ( 0, x, 9 );
                y = qBound ( 0, y, 9 );
                return m_solution[x][y];
        }

private:
        void createSolution();
        void createGivens();
        virtual bool isUnique() = 0;

private:
        int m_solution[9][9];
        int m_givens[9][9];
        Pattern* m_pattern;
};


class PuzzleDancingLinks : public Puzzle
{
private:
        virtual bool isUnique();
};


class PuzzleSliceAndDice : public Puzzle
{
private:
        virtual bool isUnique();
};
}
}
#endif
