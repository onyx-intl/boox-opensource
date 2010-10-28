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
#include "puzzle.h"

#include "dancing_links.h"
#include "pattern.h"

#include <algorithm>

namespace onyx
{
namespace simsu {

/*****************************************************************************/

Puzzle::Puzzle()
                : m_pattern ( 0 )
{
}

/*****************************************************************************/

Puzzle::~Puzzle()
{
        delete m_pattern;
}

/*****************************************************************************/

void Puzzle::generate ( unsigned int seed, int symmetry )
{
        srand ( seed );

        delete m_pattern;
        switch ( symmetry ) {
        case Pattern::FullDihedral:
                m_pattern = new PatternFullDihedral;
                break;
        case Pattern::Rotational180:
                m_pattern = new PatternRotational180;
                break;
        case Pattern::RotationalFull:
                m_pattern = new PatternRotationalFull;
                break;
        case Pattern::Horizontal:
                m_pattern = new PatternHorizontal;
                break;
        case Pattern::Vertical:
                m_pattern = new PatternVertical;
                break;
        case Pattern::HorizontalVertical:
                m_pattern = new PatternHorizontalVertical;
                break;
        case Pattern::Diagonal:
                m_pattern = new PatternDiagonal;
                break;
        case Pattern::AntiDiagonal:
                m_pattern = new PatternAntiDiagonal;
                break;
        case Pattern::DiagonalAntiDiagonal:
                m_pattern = new PatternDiagonalAntiDiagonal;
                break;
        case Pattern::None:
        default:
                m_pattern = new PatternNone;
                break;
        }

        int givens = 0;
        do {
                createSolution();
                createGivens();

                givens = 81;
                for ( int r = 0; r < 9; ++r ) {
                        for ( int c = 0; c < 9; ++c ) {
                                givens -= ( m_givens[c][r] == 0 );
                        }
                }
        } while ( givens > 30 );
}

/*****************************************************************************/

void Puzzle::createSolution()
{
        // Create list of initial values
        QList<int> initial;
        for ( int i = 1; i < 10; ++i ) {
                initial.append ( i );
        }

        QList< QList<int> > cells;
        for ( int r = 0; r < 9; ++r ) {
                for ( int c = 0; c < 9; ++c ) {
                        m_solution[c][r] = 0;
                        cells.append ( initial );
                }
        }

        // Fill solution grid
        for ( int i = 0; i < 81; ++i ) {
                int r = i / 9;
                int c = i - ( r * 9 );
                int box_row = ( r / 3 ) * 3;
                int box_col = ( c / 3 ) * 3;

                m_solution[c][r] = 0;
                QList<int>& cell = cells[i];
                std::random_shuffle ( cell.begin(), cell.end() );

                forever {
                        // Backtrack if there are no possiblities
                        if ( cell.isEmpty() ) {
                                cell = initial;
                                i -= 2;
                                break;
                        }

                        // Fetch value
                        int value = cell.takeLast();

                        // Check for conflicts
                        bool conflicts = false;
                        for ( int j = 0; j < 9; ++j ) {
                                conflicts |= ( m_solution[j][r] == value );
                                conflicts |= ( m_solution[c][j] == value );
                        }
                        for ( int row = box_row; row < box_row + 3; ++row ) {
                                for ( int col = box_col; col < box_col + 3; ++col ) {
                                        conflicts |= ( m_solution[col][row] == value );
                                }
                        }
                        if ( !conflicts ) {
                                m_solution[c][r] = value;
                                break;
                        }
                }
        }
}

/*****************************************************************************/

void Puzzle::createGivens()
{
        // Initialize givens
        QList<QPoint> cells;
        for ( int r = 0; r < 9; ++r ) {
                for ( int c = 0; c < 9; ++c ) {
                        m_givens[c][r] = m_solution[c][r];
                        cells.append ( QPoint ( c, r ) );
                }
        }
        std::random_shuffle ( cells.begin(), cells.end() );

        // Remove as many givens as possible
        QVector<int> values ( m_pattern->count() );
        int count = values.count();
        QList<QPoint> positions;
        foreach ( const QPoint& cell, cells ) {
                positions = m_pattern->pattern ( cell );
                for ( int i = 0; i < count; ++i ) {
                        QPoint pos = positions.at ( i );
                        values[i] = m_givens[pos.x() ][pos.y() ];
                }
                if ( !values.contains ( 0 ) ) {
                        for ( int i = 0; i < count; ++i ) {
                                const QPoint& pos = positions.at ( i );
                                m_givens[pos.x() ][pos.y() ] = 0;
                        }
                        if ( !isUnique() ) {
                                for ( int i = 0; i < count; ++i ) {
                                        const QPoint& pos = positions.at ( i );
                                        m_givens[pos.x() ][pos.y() ] = values.at ( i );
                                }
                        }
                }
        }
}

/*****************************************************************************/

bool PuzzleDancingLinks::isUnique()
{
        QList<int> initial;
        for ( int i = 0; i < 9; ++i ) {
                initial.append ( i );
        }

        DLX::Matrix matrix ( 324 );
        for ( int r = 0; r < 9; ++r ) {
                for ( int c = 0; c < 9; ++c ) {
                        int g = given ( c, r );
                        QList<int> values = ( g == 0 ) ? initial : ( QList<int>() << g - 1 );
                        foreach ( int value, values ) {
                                matrix.addRow();
                                matrix.addElement ( r * 9 + c );
                                matrix.addElement ( r * 9 + value + 81 );
                                matrix.addElement ( c * 9 + value + 162 );
                                matrix.addElement ( ( 3 * ( r / 3 ) + ( c / 3 ) ) * 9 + value + 243 );
                        }
                }
        }

        return ( matrix.search ( 2 ) == 1 );
}

/*****************************************************************************/

bool PuzzleSliceAndDice::isUnique()
{
        // Create list of initial values
        QList<int> initial;
        for ( int i = 1; i < 10; ++i ) {
                initial.append ( i );
        }

        // Fill possible values into all cells
        QList<int> cells[9][9];
        for ( int r = 0; r < 9; ++r ) {
                for ( int c = 0; c < 9; ++c ) {
                        cells[c][r] = initial;
                }
        }

        // Remove givens
        for ( int r = 0; r < 9; ++r ) {
                for ( int c = 0; c < 9; ++c ) {
                        int g = given ( c, r );
                        if ( g != 0 ) {
                                cells[c][r] = QList<int>() << g;
                        }
                }
        }

        bool done = false;
        bool solvable = false;
        while ( !done ) {
                done = true;
                solvable = false;

                for ( int r = 0; r < 9; ++r ) {
                        for ( int c = 0; c < 9; ++c ) {
                                QList<int>& cell = cells[c][r];
                                int count = cell.count();
                                if ( count > 1 ) {
                                        std::random_shuffle ( cell.begin(), cell.end() );
                                        done = false;
                                } else if ( count == 1 ) {
                                        int value = cell.first();

                                        // Remove all instances of value in column
                                        for ( int row = 0; row < 9; ++row ) {
                                                cells[c][row].removeOne ( value );
                                        }

                                        // Remove all instances of value in row
                                        for ( int col = 0; col < 9; ++col ) {
                                                cells[col][r].removeOne ( value );
                                        }

                                        // Remove all instances of value in box
                                        int box_row = ( r / 3 ) * 3;
                                        int box_col = ( c / 3 ) * 3;
                                        for ( int row = box_row; row < box_row + 3; ++row ) {
                                                for ( int col = box_col; col < box_col + 3; ++col ) {
                                                        cells[col][row].removeOne ( value );
                                                }
                                        }

                                        solvable = true;
                                }
                        }
                }

                if ( !solvable ) {
                        return false;
                }
        }
        return true;
}

/*****************************************************************************/
}
}
// kate: indent-mode cstyle; space-indent on; indent-width 8; 
