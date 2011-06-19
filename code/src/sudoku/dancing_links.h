/***********************************************************************
 *
 * Copyright (C) 2008-2009 Graeme Gott <graeme@gottcode.org>
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

#ifndef  ONYX_SIMSU_DANCING_LINKS_H
#define ONYX_SIMSU_DANCING_LINKS_H

#include <QLinkedList>
#include <QVector>

namespace onyx
{
namespace simsu {
namespace DLX {

struct HeaderNode;
struct Node {
        Node()
                        : left ( 0 ), right ( 0 ), up ( 0 ), down ( 0 ), column ( 0 ) {
        }

        Node* left;
        Node* right;
        Node* up;
        Node* down;
        HeaderNode* column;
};

struct HeaderNode : public Node {
        HeaderNode()
                        : size ( 0 ), id ( 0 ) {
        }

        quint32 size;
        quint32 id;
};

class Matrix
{
        class Callback
        {
        public:
                virtual ~Callback() {
                }

                virtual void operator() ( const QVector<Node*>&, quint32 ) {
                }
        };

        class GlobalCallback : public Callback
        {
        public:
                typedef void ( *function ) ( const QVector<Node*>&, quint32 );
                GlobalCallback ( function f )
                                : m_function ( f ) {
                }

                virtual void operator() ( const QVector<Node*>& rows, quint32 count ) {
                        ( *m_function ) ( rows, count );
                }

        private:
                function m_function;
        };

        template <typename T>
        class MemberCallback : public Callback
        {
        public:
                typedef void ( T::*function ) ( const QVector<Node*>& rows, quint32 count );
                MemberCallback ( T* object, function f )
                                : m_object ( object ), m_function ( f ) {
                }

                virtual void operator() ( const QVector<Node*>& rows, quint32 count ) {
                        ( *m_object.*m_function ) ( rows, count );
                }

        private:
                T* m_object;
                function m_function;
        };

public:
        Matrix ( quint32 max_columns );
        ~Matrix();

        void addRow();
        void addElement ( quint32 column );

        quint32 search ( quint32 max_solutions = 0xFFFFFFFF ) {
                m_max_solutions = max_solutions;
                Callback solution;
                m_solution = &solution;
                solve ( 0 );
                return m_solutions;
        }

        quint32 search ( void ( *function ) ( const QVector<Node*>& rows, quint32 count ), quint32 max_solutions = 0xFFFFFFFF ) {
                m_max_solutions = max_solutions;
                GlobalCallback solution ( function );
                m_solution = &solution;
                solve ( 0 );
                return m_solutions;
        }

        template <typename T>
        quint32 search ( T* object, void ( T::*function ) ( const QVector<Node*>& rows, quint32 count ), quint32 max_solutions = 0xFFFFFFFF ) {
                m_max_solutions = max_solutions;
                MemberCallback<T> solution ( object, function );
                m_solution = &solution;
                solve ( 0 );
                return m_solutions;
        }

private:
        void solve ( quint32 k );
        void cover ( HeaderNode* column );
        void uncover ( HeaderNode* column );

        quint32 m_max_columns;

        HeaderNode* m_header;
        QVector<HeaderNode> m_columns;
        QLinkedList<HeaderNode> m_rows;
        QLinkedList<Node> m_nodes;
        QVector<Node*> m_output;

        Callback* m_solution;
        quint32 m_solutions;
        quint32 m_max_solutions;
};
}
}
}

#endif
// kate: indent-mode cstyle; space-indent on; indent-width 8; 
