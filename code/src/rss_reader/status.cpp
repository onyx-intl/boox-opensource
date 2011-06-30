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

#include "status.h"

///<summary>Default c'tor</summary>
CStatus::CStatus() :
	Completion(0),
	Flags(sfNone),
	NewItems(0),
	ItemCount(0)
{
}

///<summary>C'tor</summary>
///<param name="status">The new status string to set.</param>
///<param name="completion">Percentage of completion for the download operation.</param>
///<param name="flags">Additional flags like sfCountUpdate or sfsfFinished.</param>
CStatus::CStatus(const QString status, int completion, EStateFlags flags/*=sfNone*/) :
	Status(status),
	Completion(completion),
	Flags(flags),
	NewItems(0),
	ItemCount(0)
{
}

///<summary>C'tor with item count</summary>
///<param name="status">The new status string to set.</param>
///<param name="completion">Percentage of completion for the download operation.</param>
///<param name="newItems">The number of new items for this feed.</param>
///<param name="itemCount">The number of items within the feed store.</param>
///<param name="flags">Additional flags like sfCountUpdate or sfsfFinished. sfCountUpdate is automatically set.</param>
CStatus::CStatus(const QString status, int completion, int newItems, int itemCount, EStateFlags flags/*=sfNone*/) :
	Status(status),
	Completion(completion),
	Flags((EStateFlags)(flags|sfCountUpdate)),
	NewItems(newItems),
	ItemCount(itemCount)
{
}

///<summary>Copy c'tor</summary>
///<param name="source">Reference to the source CStatus instance</param>
CStatus::CStatus(const CStatus &source) :
	Status(source.Status),
	Completion(source.Completion),
	Flags(source.Flags),
	NewItems(source.NewItems),
	ItemCount(source.ItemCount)
{
}

///<summary>Returns the status message that was transported by this message</summary>
///<returns>Returns a QString-Instance hat contains the status message.</returns>
QString CStatus::GetStatus() const 
{
  return Status;
}

///<summary>Returns the percentage of completion for the current task</summary>
///<returns>Returns an integer containing the percentage of completion for the current task.</returns>
int CStatus::GetCompletion() const 
{
  return Completion;
}

///<summary>Returns the over all number of items in this feed.</summary>
///<returns>Returns the over all number of items in this feed if sfCountUpdate is set. Zero otherwise.</returns>
int CStatus::GetItemCount() const 
{
	return ItemCount;
}

///<summary>Returns the number of new items in this feed.</summary>
///<returns>Returns the number of new items in this feed if sfCountUpdate is set. Zero otherwise.</returns>
int CStatus::GetNewItems() const 
{
	return NewItems;
}

///<summary>Retruns the state flags for this message</summary>
///<returns>State flags for this message as a combination of the values of the enum EStateFlags</returns>
CStatus::EStateFlags CStatus::GetFlags() const
{
	return Flags;
}
