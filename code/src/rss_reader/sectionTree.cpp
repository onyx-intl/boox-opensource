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

#include <QDebug>

#include "sectionTree.h"

///<summary>C'tor</summary>
///<param name="parent">Pointer to the parent item of this tree node</param>
CSectionNode::CSectionNode(CSectionNode* parent/*=NULL*/) :
	Parent(parent),
	Children(),	
	Start(0),
	End(0),
	ContStart(0),
	ContEnd(0),
	Weight(0),
	Kept(false)
{
	if (parent!=NULL)
		parent->AddChild(this);
}

///<summary>D'tor</summary>
///<remarks>Deletes alls child items. The child items delete their children and so on...</remarks>
CSectionNode::~CSectionNode()
{
	while (!Children.isEmpty())
		delete Children.takeFirst();		
}

///<summary>Adds a new child node to this node</summary>
///<param name="child">Pointer to the new child node.</param>
///<remarks>This node will handle the lifetime of the child node. The child node will be freed when the parent node is deleted.</remarks>
void CSectionNode::AddChild(CSectionNode* child)
{
	Children+=child;
}

///<summary>Returns the iterator for iterating over all rssSections</summary>
///<returns>Returns an QListIterator<SSection> instance</returns>
QListIterator<CSectionNode*> CSectionNode::GetChildren()
{
	return QListIterator<CSectionNode*>(Children);
}

///<summary>Returns the number of child items</summary>
///<returns>The number of child items stored in this list</returns>
int CSectionNode::ChildCount()
{
	return Children.count();
}

///<summary>Returns the parent node of this node</summary>
///<returns>Returns the parent node of this node or NULL if it is the root node.</summary>
CSectionNode* CSectionNode::GetParent()
{
	return Parent;
}

///<summary>Creates a debug dump of this node</summary>
///<param name="log">Pointer to a QTextStream-Instance receiving the log output</param>
///<param name="indent">Indent for this item</param>
void CSectionNode::Dump(QTextStream* log, int indent/*=0*/)
{
	QString indentString(indent*2, ' ');
	QString contentToDump=Content;
	
	(*log) << indentString << "- Tag " << Tag << " (" << this << ")\r\n";
	(*log) << indentString << "  Range: " << Start << " - " << End << "\r\n";
	(*log) << indentString << "  Content range: " << ContStart << " - " << ContEnd << "\r\n";
	(*log) << indentString << "  Weight: " << Weight << " - Kept: " << Kept << "\r\n";
	contentToDump.replace("\n", "\n  " + indentString);
	(*log) << indentString << "  " << contentToDump << "\r\n";
	
	//-- Dump all subitems
	QListIterator<CSectionNode*> childrenI=GetChildren();
	while (childrenI.hasNext())
		childrenI.next()->Dump(log, indent+1);
}
