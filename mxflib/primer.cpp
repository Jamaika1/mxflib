/*! \file	primer.cpp
 *	\brief	Implementation of Primer class
 *
 *			The Primer class holds data about the mapping between local
 *          tags in a partition and the UL that gives access to the full
 *			definition
 *
 *	\version $Id$
 *
 */
/* 
 *  This software is provided 'as-is', without any express or implied warranty.
 *  In no event will the authors be held liable for any damages arising from
 *  the use of this software.
 *  
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 *  
 *   1. The origin of this software must not be misrepresented; you must
 *      not claim that you wrote the original software. If you use this
 *      software in a product, you must include an acknowledgment of the
 *      authorship in the product documentation.
 *  
 *   2. Altered source versions must be plainly marked as such, and must
 *      not be misrepresented as being the original software.
 *  
 *   3. This notice may not be removed or altered from any source
 *      distribution.
 */

#include "mxflib/mxflib.h"

using namespace mxflib;


//! Read the primer from a buffer
/*!	\return Number of bytes read
 */
UInt32 Primer::ReadValue(const UInt8 *Buffer, UInt32 Size)
{
	debug("Reading Primer\n");

	// Start off empty
	clear();

	if(Size < 8)
	{
		error("Primer too small, must be at least 8 bytes!\n");
		return 0;
	}

	// Read the vector header
	UInt32 ClaimedItems = GetU32(Buffer);
	UInt32 ClaimedItemSize = GetU32(&Buffer[4]);
	Buffer += 8;

	if(ClaimedItemSize < 18)
	{
		error("Malformed vector header in Primer - each entry is 18 bytes, size in vector header is %d\n", ClaimedItemSize);
		return 0;
	}

	// Each entry in the primer is ClaimedItemSize bytes
	UInt32 Items = (Size-8) / ClaimedItemSize;

	// Validate the size and only read whole items
	if((Items * ClaimedItemSize) != (Size-8))
	{
		error("Primer not an integer number of items!\n");
		Size = (Items * ClaimedItemSize) + 8;
	}

	if(Items != ClaimedItems)
	{
		error("Malformed vector header in Primer - number of entries is %d, vector header claims %d\n", Items, ClaimedItems);
		
		// Read the lesser of the two for safety
		if(Items > ClaimedItems) Items = ClaimedItems;
		Size = (Items * ClaimedItemSize) + 8;
	}

	// It is legal, but unlikely, to have some extra data in each primer entry - we treat this as padding and skip it
	int Padding = static_cast<int>(ClaimedItemSize - 18);

	// Read each item
	while(Items--)
	{
		Tag ThisTag = GetU16(Buffer);
		Buffer += 2;

		UL ThisUL(Buffer);
		Buffer += (16 + Padding);

		// Add this new entry to the primer
		insert(Primer::value_type(ThisTag, ThisUL));

		debug("  %s -> %s\n", Tag2String(ThisTag).c_str(), ThisUL.GetString().c_str());
	}

	// Return how many bytes we actually read
	return Size;
}


//! Determine the tag to use for a given UL - when no primer is availabe
Tag Primer::StaticLookup(ULPtr ItemUL, Tag TryTag /*=0*/)
{
	return MDOType::GetStaticPrimer()->Lookup(ItemUL, TryTag);
}


//! Determine the tag to use for a given UL
/*! If the UL has not yet been used the correct static or dynamic tag will 
 *	be determined and added to the primer
 *	\return The tag to use, or 0 if no more dynamic tags available
 */
Tag Primer::Lookup(ULPtr ItemUL, Tag TryTag /*=0*/)
{
	// If a tag has been suggested then try that
	if(TryTag != 0)
	{
		// Is it known by us?
		Primer::iterator it = find(TryTag);
		if(it != end())
		{
			// Only use it if the UL matches
			if(!memcmp((*it).second.GetValue(), ItemUL->GetValue(), 16)) return TryTag;
		}
		else
		{
			// It could be the right tag, but not yet in this primer
			// DRAGONS: Not implementer yet!!!
		}
	}

	// Do we have this UL already?
	std::map<UL, Tag>::iterator it = TagLookup.find(ItemUL);
	if(it != TagLookup.end())
	{
		return (*it).second;
	}

	// Try and find the type with this UL
	MDOTypePtr Type = MDOType::Find(ItemUL);
	if(Type)
	{
		if(Type->GetTag() != 0)
		{
			insert(Primer::value_type(Type->GetTag(), ItemUL));
			return Type->GetTag();
		}

		// No static tag supplied - fall through and use a dynamic tag
	}

	// Generate a dynamic tag
	// DRAGONS: Not very efficient
	while(NextDynamic >= 0x8000)
	{
		if(find(NextDynamic) == end())
		{
			Tag Ret = NextDynamic;
			NextDynamic--;
			insert(Primer::value_type(Ret, ItemUL));
			return Ret;
		}
		NextDynamic--;
	}

	//! Out of dynamic tags!
	error("Run out of dynamic tags!\n");
	return 0;
}


//! Write this primer to a memory buffer
/*! The primer will be <b>appended</b> to the DataChunk */
UInt32 Primer::WritePrimer(DataChunkPtr &Buffer)
{
	UInt32 Bytes;

	// Work out the primer value size first (to allow us to pre-allocate)
	UInt64 PrimerLen = UInt64(size()) * 18 + 8;

	// Re-size buffer to the probable final size
	Buffer->ResizeBuffer((UInt32)(Buffer->Size + 16 + 4 + PrimerLen));

	// Lookup the type to get the key - Static so only need to lookup once
	MDOTypePtr PrimerType = MDOType::Find(Primer_UL);
	mxflib_assert(PrimerType);

	Buffer->Append(PrimerType->GetKey());
	Bytes = static_cast<UInt32>(PrimerType->GetKey().Size);

	// Add the length
	DataChunkPtr BER = MakeBER(PrimerLen);
	Buffer->Append(*BER);
	Bytes += static_cast<UInt32>(BER->Size);

	// Add the vector header
	UInt8 Temp[4];
	PutU32(static_cast<UInt32>(size()), Temp);
	Buffer->Append(4, Temp);
	Bytes += 4;

	PutU32(18, Temp);
	Buffer->Append(4, Temp);
	Bytes += 4;

	// Write the primer data
	iterator it = begin();
	while(it != end())
	{
		PutU16((*it).first, Temp);
		Buffer->Append(2, Temp);
		Buffer->Append(16, (*it).second.GetValue());
		it++;
	}

	return Bytes;
}

