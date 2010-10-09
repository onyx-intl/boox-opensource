/*
 * Copyright (C) 2007-2009 Geometer Plus <contact@geometerplus.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "W32Container.h"

using namespace std;

W32VBorderBox::W32VBorderBox() : mySpacing(0) {
}

void W32VBorderBox::setTopElement(W32WidgetPtr widget) {
	myTopElement = widget;
}

void W32VBorderBox::setCenterElement(W32WidgetPtr widget) {
	myCenterElement = widget;
}

void W32VBorderBox::setBottomElement(W32WidgetPtr widget) {
	myBottomElement = widget;
}

void W32VBorderBox::setSpacing(int spacing) {
	mySpacing = spacing;
}

void W32VBorderBox::allocate(WORD *&p, short &id) const {
	if (myTopElement) {
		myTopElement->allocate(p, id);
	}
	if (myCenterElement) {
		myCenterElement->allocate(p, id);
	}
	if (myBottomElement) {
		myBottomElement->allocate(p, id);
	}
}

int W32VBorderBox::allocationSize() const {
	int size = 0;
	if (myTopElement) {
		size += myTopElement->allocationSize();
	}
	if (myCenterElement) {
		size += myCenterElement->allocationSize();
	}
	if (myBottomElement) {
		size += myBottomElement->allocationSize();
	}
	return size;
}

void W32VBorderBox::setVisible(bool visible) {
	if (myTopElement) {
		myTopElement->setVisible(visible);
	}
	if (myCenterElement) {
		myCenterElement->setVisible(visible);
	}
	if (myBottomElement) {
		myBottomElement->setVisible(visible);
	}
}

bool W32VBorderBox::isVisible() const {
	if (myTopElement && myTopElement->isVisible()) {
		return true;
	}
	if (myCenterElement && myCenterElement->isVisible()) {
		return true;
	}
	if (myBottomElement && myBottomElement->isVisible()) {
		return true;
	}
	return false;
}

int W32VBorderBox::controlNumber() const {
	int counter = 0;
	if (myTopElement) {
		counter += myTopElement->controlNumber();
	}
	if (myCenterElement) {
		counter += myCenterElement->controlNumber();
	}
	if (myBottomElement) {
		counter += myBottomElement->controlNumber();
	}
	return counter;
}

W32Widget::Size W32VBorderBox::minimumSize() const {
	Size size;
	int counter = 0;

	if (myTopElement && myTopElement->isVisible()) {
		++counter;
		Size elementSize = myTopElement->minimumSize();
		size.Height += elementSize.Height;
		size.Width = max(size.Width, elementSize.Width);
	}
	if (myCenterElement && myCenterElement->isVisible()) {
		++counter;
		Size elementSize = myCenterElement->minimumSize();
		size.Height += elementSize.Height;
		size.Width = max(size.Width, elementSize.Width);
	}
	if (myBottomElement && myBottomElement->isVisible()) {
		++counter;
		Size elementSize = myBottomElement->minimumSize();
		size.Height += elementSize.Height;
		size.Width = max(size.Width, elementSize.Width);
	}

	if (counter > 0) {
		size.Height += (counter - 1) * mySpacing;
	}

	size.Width += leftMargin() + rightMargin();
	size.Height += topMargin() + bottomMargin();
	return size;
}

void W32VBorderBox::setPosition(int x, int y, Size size) {
	const short elementX = x + leftMargin();
	const short elementWidth = size.Width - leftMargin() - rightMargin();

	short centerElementX = elementX;
	short centerElementY = y + topMargin();
	short centerElementHeight = size.Height - topMargin() - bottomMargin();

	if (myTopElement && myTopElement->isVisible()) {
		Size topElementSize = myTopElement->minimumSize();
		myTopElement->setPosition(elementX, centerElementY, Size(elementWidth, topElementSize.Height));
		centerElementY += topElementSize.Height + mySpacing;
		centerElementHeight -= topElementSize.Height + mySpacing;
	}
	if (myBottomElement && myBottomElement->isVisible()) {
		Size bottomElementSize = myBottomElement->minimumSize();
		myBottomElement->setPosition(elementX, y + size.Height - bottomMargin() - bottomElementSize.Height, Size(elementWidth, bottomElementSize.Height));
		centerElementHeight -= bottomElementSize.Height + mySpacing;
	}
	if (myCenterElement && myCenterElement->isVisible()) {
		myCenterElement->setPosition(centerElementX, centerElementY, Size(elementWidth, centerElementHeight));
	}
}

void W32VBorderBox::init(HWND parent, W32ControlCollection *collection) {
	if (myTopElement) {
		myTopElement->init(parent, collection);
	}
	if (myCenterElement) {
		myCenterElement->init(parent, collection);
	}
	if (myBottomElement) {
		myBottomElement->init(parent, collection);
	}
}
