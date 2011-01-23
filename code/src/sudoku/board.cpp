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

#include "board.h"

#include "cell.h"
#include "pattern.h"
#include "puzzle.h"

#include <QApplication>
#include <QFrame>
#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPainter>
#include <QSettings>
#include <QUndoStack>

/*****************************************************************************/
namespace onyx
{
namespace simsu {
Board::Board ( QWidget* parent )
                : Frame ( parent ), m_puzzle ( 0 ), m_active_key ( 1 ), m_auto_switch ( true ), m_highlight_active ( false ), m_notes_mode ( false ), m_finished ( false ), m_column(0),  m_row(0)
{
        setBackgroundRole ( QPalette::Mid );

        m_moves = new QUndoStack ( this );

        QGridLayout* layout = new QGridLayout ( this );
        layout->setMargin ( 3 );
        layout->setSpacing ( 0 );

        // Create cells
        for ( int i = 0; i < 9; ++i ) {
                int col = ( i % 3 ) * 3;
                int max_col = col + 3;
                int row = ( i / 3 ) * 3;
                int max_row = row + 3;

                QGridLayout* box = new QGridLayout;
                box->setMargin ( 2 );
                box->setSpacing ( 1 );
                layout->addLayout ( box, row / 3, col / 3 );

                for ( int r = row; r < max_row; ++r ) {
                        for ( int c = col; c < max_col; ++c ) {
                                Cell* cell = new Cell ( c, r, this, this );
                                box->addWidget ( cell, r - row, c - col );
                                m_cells[c][r] = cell;
                        }
                }
        }

        // Create success message
        m_message = new QLabel ( this );
        QFontMetrics metrics ( QFont ( "Sans", 24 ) );
        int width = metrics.width ( tr ( "Success" ) );
        int height = metrics.height();
        QPixmap success ( QSize ( width + height, height * 2 ) );
        success.fill ( QColor ( 0, 0, 0, 0 ) );
        {
                QPainter painter ( &success );

                painter.setPen ( Qt::NoPen );
                painter.setBrush ( QColor ( 0, 0, 0, 255 ) );
                painter.setRenderHint ( QPainter::Antialiasing, true );
                painter.drawRoundedRect ( 0, 0, width + height, height * 2, 10, 10 );

                painter.setFont ( QFont ( "Sans", 32 ) );
                painter.setPen ( Qt::white );
                painter.setRenderHint ( QPainter::TextAntialiasing, true );
                painter.drawText ( height / 2, height / 2 + metrics.ascent(), tr ( "Success" ) );
        }
        m_message->setPixmap ( success );
        m_message->hide();
        layout->addWidget ( m_message, 0, 0, 3, 3, Qt::AlignCenter );

        // Load current puzzle
        QSettings settings;
        if ( settings.value ( "Current/Version", 0 ).toInt() != 3 ) {
                settings.remove ( "Current" );
        }
        int seed = settings.value ( "Current/Seed", 0 ).toInt();
        int symmetry = settings.value ( "Current/Symmetry", -1 ).toInt();
        int algorithm = settings.value ( "Current/Algorithm", -1 ).toInt();
        if ( seed > 0 ) {
                QStringList moves = settings.value ( "Current/Moves" ).toStringList();
                newPuzzle ( seed, symmetry, algorithm, true );

                // Load moves
                foreach ( const QString& move, moves ) {
                        if ( move.length() == 4 ) {
                                m_notes_mode = ( move[2] == 'n' );
                                Cell* c = cell ( move[0].digitValue(), move[1].digitValue() );
                                int key = move[3].digitValue();
                                QKeyEvent event ( QEvent::KeyPress, 0x30 + key, Qt::NoModifier );
                                QApplication::sendEvent ( c, &event );
                        }
                }
                m_notes_mode = false;

                // Select current cell
                if ( settings.contains ( "Current/Active" ) ) {
                        QStringList cell = settings.value ( "Current/Active" ).toString().split ( 'x' );
                        if ( cell.count() == 2 ) {
                                m_cells[cell[0].toInt() ][cell[1].toInt() ]->setFocus();
                        }
                }
        } else {
                newPuzzle();
        }
}

/*****************************************************************************/

Board::~Board()
{
        if ( !m_finished ) {
                QStringList moves;
                int count = m_moves->index();
                for ( int i = 0; i < count; ++i ) {
                        moves += m_moves->text ( i );
                }
                QSettings().setValue ( "Current/Moves", moves );
        } else {
                QSettings().remove ( "Current" );
        }
        delete m_puzzle;
}

/*****************************************************************************/

void Board::newPuzzle ( int seed, int symmetry, int algorithm, bool load )
{
        QSettings settings;

        if ( seed <= 0 ) {
#ifndef _WINDOWS
                srand ( time ( 0 ) );
#endif
                seed = rand();
        }

        if ( symmetry == -1 ) {
                symmetry = settings.value ( "Symmetry", Pattern::Rotational180 ).toInt();
        }
        if ( symmetry == Pattern::Random ) {
                symmetry = rand() % Pattern::Random;
        }

        if ( algorithm == -1 ) {
                algorithm = settings.value ( "Algorithm", 0 ).toInt();
        }

        if ( !load ) {
                settings.remove ( "Current" );
                settings.setValue ( "Current/Version", 3 );
                settings.setValue ( "Current/Seed", seed );
                settings.setValue ( "Current/Symmetry", symmetry );
                settings.setValue ( "Current/Algorithm", algorithm );
        }

        showWrong ( false );
        m_finished = false;
        m_message->hide();
        m_moves->clear();

        delete m_puzzle;
        switch ( algorithm ) {
        case 1:
                m_puzzle = new PuzzleSliceAndDice;
                break;
        case 0:
        default:
                m_puzzle = new PuzzleDancingLinks;
                break;
        }
        m_puzzle->generate ( seed, symmetry );
        for ( int r = 0; r < 9; ++r ) {
                for ( int c = 0; c < 9; ++c ) {
                        m_cells[c][r]->setPuzzle ( m_puzzle );
                }
        }
}

/*****************************************************************************/

void Board::checkFinished()
{
        m_finished = true;
        for ( int r = 0; r < 9; ++r ) {
                for ( int c = 0; c < 9; ++c ) {
                        m_finished = m_finished && m_cells[c][r]->isCorrect();
                }
        }

        if ( m_finished ) {
                for ( int r = 0; r < 9; ++r ) {
                        for ( int c = 0; c < 9; ++c ) {
                                m_cells[c][r]->clearFocus();
                                m_cells[c][r]->setFocusPolicy ( Qt::NoFocus );
                        }
                }
                m_message->show();
                update();
        }
}

/*****************************************************************************/

void Board::showWrong ( bool show )
{
        for ( int r = 0; r < 9; ++r ) {
                for ( int c = 0; c < 9; ++c ) {
                        m_cells[c][r]->showWrong ( show );
                }
        }
}

/*****************************************************************************/

void Board::moveFocus ( int column, int row, int xdelta, int ydelta )
{
        xdelta = qBound ( -1, xdelta, 2 );
        ydelta = qBound ( -1, ydelta, 2 );
        Q_ASSERT ( xdelta != ydelta );

        if ( column + xdelta < 0 ) {
                column = 9;
        } else if ( column + xdelta > 8 ) {
                column = -1;
        }
        column += xdelta;

        if ( row + ydelta < 0 ) {
                row = 9;
        } else if ( row + ydelta > 8 ) {
                row = -1;
        }
        row += ydelta;

        m_cells[column][row]->setFocus();

        m_column=column;
        m_row=row;
}

/*****************************************************************************/

void Board::setActiveKey ( int key )
{
        m_active_key = qBound ( 1, key, 10 );
        QSettings().setValue ( "Key", m_active_key );
        update();
        emit activeKeyChanged ( m_active_key );
}

/*****************************************************************************/

void Board::setAutoSwitch ( bool auto_switch )
{
        m_auto_switch = auto_switch;
        QSettings().setValue ( "AutoSwitch", m_auto_switch );
}

/*****************************************************************************/

void Board::setHighlightActive ( bool highlight )
{
        m_highlight_active = highlight;
        QSettings().setValue ( "Highlight", m_highlight_active );
        update();
}

/*****************************************************************************/

void Board::setMode ( int mode )
{
        m_notes_mode = mode;
        QSettings().setValue ( "Mode", ( m_notes_mode ? "Pencil" : "Pen" ) );
        emit notesModeChanged ( mode );
}

/*****************************************************************************/
}
}
// kate: indent-mode cstyle; space-indent on; indent-width 8;
