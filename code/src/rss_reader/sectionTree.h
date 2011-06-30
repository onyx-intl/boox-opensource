/*	NewsFlash
		Copyright 2010 Daniel Goﬂ (Flash Systems)

		This file is part of NewsFlash

    NewsFlash is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    NewsFlash is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with NewsFlash.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <QList>

///<summary>Represents a node within the section tree.</summary>
class CSectionNode
{
//-- Public properties
public:
	int Start;
	int End;
	int ContStart;
	int ContEnd;
	int Weight;
	QString Tag;
	QString Content;
	bool Kept;
	
//-- Private members
private:
	CSectionNode* Parent;
	QList<CSectionNode*> Children;
	
//-- c'tor and d'tor
public:
  CSectionNode(CSectionNode* parent=NULL);
	~CSectionNode();
	
//-- Protected methods calld by other instances
protected:
	void AddChild(CSectionNode* child);

//-- Public methods
public:	
	CSectionNode* GetParent();
	QListIterator<CSectionNode*> GetChildren();
	int ChildCount();
	void Dump(QTextStream* log, int indent=0);
};
