/*! \file	klvobject.cpp
 *	\brief	Implementation of classes that define basic KLV objects
 *
 *			Class KLVObject holds info about a KLV object
 *
 *	\version $Id$
 *
 */
/*
 *	Copyright (c) 2003, Matt Beard
 *	Portions Copyright (c) 2003, Metaglue Corporation
 *
 *	This software is provided 'as-is', without any express or implied warranty.
 *	In no event will the authors be held liable for any damages arising from
 *	the use of this software.
 *
 *	Permission is granted to anyone to use this software for any purpose,
 *	including commercial applications, and to alter it and redistribute it
 *	freely, subject to the following restrictions:
 *
 *	  1. The origin of this software must not be misrepresented; you must
 *	     not claim that you wrote the original software. If you use this
 *	     software in a product, an acknowledgment in the product
 *	     documentation would be appreciated but is not required.
 *	
 *	  2. Altered source versions must be plainly marked as such, and must
 *	     not be misrepresented as being the original software.
 *	
 *	  3. This notice may not be removed or altered from any source
 *	     distribution.
 */

#include <mxflib/mxflib.h>

using namespace mxflib;

//! Build a new KLVObject
KLVObject::KLVObject(ULPtr ObjectUL)
{
	TheUL = ObjectUL;
	
	Init();
}


//! Initialise newly built KLVObject
void KLVObject::Init(void)
{
	ValueLength = 0;
	DataBase = 0;

//	ObjectName = "";
}


//! Get text that describes where this item came from
std::string KLVObject::GetSource(void) 
{ 
	if(Source.File) return Source.File->Name; else return "memory buffer"; 
}


//! Base verion: Read the key and length fot this KLVObject from the current source
/*! \return The number of bytes read (i.e. KLSize)
 *
 *  DRAGONS: This base function may be called from derived class objects to get base behaviour.
 *           It is therefore vital that the function does not call any "virtual" KLVObject
 *           functions, directly or indirectly.
 */
Int32 KLVObject::Base_ReadKL(void)
{
//@'@printf("@0x%08x:Base_ReadKL()\n", (int)Source.Offset);
	if(!Source.File)
	{
		error("KLVObject::Base_ReadKL() called with no SourceFile defined\n");
		return 0;
	}

	// Read the key
	Source.File->Seek(Source.Offset);
	TheUL = Source.File->ReadKey();

	// Abort now if now valid key
	if(!TheUL) return 0;

	// Read the length
	ValueLength = Dest.OuterLength = Source.OuterLength = Source.File->ReadBER();

	// Work out the size of the key and length
	Source.KLSize = Source.File->Tell() - Source.Offset;
	
	// Initially set the destination KLSize target to match the source
	Dest.KLSize = Source.KLSize;

	return Source.KLSize;
}



//! Base verion: Read data from a specified position in the KLV value field into the DataChunk
/*! \param Offset Offset from the start of the KLV value from which to start reading
 *  \param Size Number of bytes to read, if <=0 all available bytes will be read (which could be billions!)
 *  \return The number of bytes read
 *
 *  DRAGONS: This base function may be called from derived class objects to get base behaviour.
 *           It is therefore vital that the function does not call any "virtual" KLVObject
 *           functions, directly or indirectly.
 */
Length KLVObject::Base_ReadDataFrom(Position Offset, Length Size /*=-1*/)
{
//@'@printf("@0x%08x:Base_ReadDataFrom(%d, %d)\n", (int)(Source.Offset + Source.KLSize + Offset), (int)Offset, (int)Size);
	// Delagate to ReadHandler if defined
	if(ReadHandler) return ReadHandler->ReadData(this, Offset, Size);

	if(Source.Offset < 0)
	{
		error("Call to KLVObject::Base_ReadDataFrom() with no read handler defined and DataBase undefined\n");
		return 0;
	}

	if(!Source.File)
	{
		error("Call to KLVObject::Base_ReadDataFrom() with no read handler defined and source file not set\n");
		return 0;
	}

	// Initially plan to read all the bytes available
	Length BytesToRead = Source.OuterLength - Offset;

	// Limit to specified size if >= 0 and if < available
	if( (Size >= 0) && (Size < BytesToRead)) BytesToRead = Size;

	// Don't do anything if nothing to read
	if(BytesToRead <= 0) 
	{
		Data.Resize(0);
		return 0;
	}

	// Seek to the start of the requested data
	Source.File->Seek(Source.Offset + Source.KLSize + Offset);

	// Resize the chunk
	// Discarding old data first (by setting Size to 0) prevents old data being 
	// copied needlessly if the buffer is reallocated to increase its size
	Data.Size = 0;
	Data.Resize(BytesToRead);

	// Read into the buffer (only as big as the buffer is!)
	Length Bytes = (Length)Source.File->Read(Data.Data, Data.Size);

	// Resize the buffer if something odd happened (such as an early end-of-file)
	if(Bytes != BytesToRead) Data.Resize(Bytes);

	return Bytes;
}


//! Base verion: Write the key and length of the current DataChunk to the destination file
/*! The key and length will be written to the source file as set by SetSource.
 *  If LenSize is zero the length will be formatted to match KLSize (if possible!)
 *
 *  DRAGONS: This base function may be called from derived class objects to get base behaviour.
 *           It is therefore vital that the function does not call any "virtual" KLVObject
 *           functions, directly or indirectly.
 */
Int32 KLVObject::Base_WriteKL(Int32 LenSize /*=0*/, Length NewLength /*=-1*/)
{
//@'@printf("Base_WriteKL(%d, %d[%d])\n", LenSize, (int)NewLength, (int)Dest.OuterLength);

	if(!Dest.File)
	{
		error("Call to KLVObject::Base_WriteKL() with destination file not set\n");
		return 0;
	}

	if(Dest.Offset < 0)
	{
		error("Call to KLVObject::Base_WriteKL() with destination file location undefined\n");
		return 0;
	}

	// Seek to the start of the KLV space
	Dest.File->Seek(Dest.Offset);

	// Write the key
	Int32 Bytes = (Int32)Dest.File->Write(TheUL->GetValue(), TheUL->Size());

	if(LenSize == 0) 
	{
		Bytes = Dest.KLSize - Bytes;		// Work out how many bytes we should use for the length
		if(Bytes > 0) LenSize = Bytes;
	}

	// Decide what length to write (Use Dest.OuterLength unless something else is supplied)
	if(NewLength < 0) NewLength = Dest.OuterLength;

	// Write the length
	Dest.File->WriteBER(NewLength, LenSize);

	// Work out the new KLSize
	Dest.KLSize = Dest.File->Tell() - Dest.Offset;

	// Return the number of bytes we wrote
	return Dest.KLSize;
}


//! Base verion: Write data from a given buffer to a given location in the destination file
/*! \param Buffer Pointer to data to be written
 *  \param Offset The offset within the KLV value field of the first byte to write
 *  \param Size The number of bytes to write
 *  \return The number of bytes written
 *
 *  DRAGONS: This base function may be called from derived class objects to get base behaviour.
 *           It is therefore vital that the function does not call any "virtual" KLVObject
 *           functions, directly or indirectly.
 */
Length KLVObject::Base_WriteDataTo(const Uint8 *Buffer, Position Offset, Length Size)
{
//@'@printf("Base_WriteDataTo(0x%08x, %d, %d)\n", (int)Buffer, (int)Offset, (int)Size);

	// Don't write zero bytes
	if(Size == 0) return 0;

	if(!Dest.File)
	{
		error("Call to KLVObject::Base_WriteDataTo() with destination file not set\n");
		return 0;
	}

	if(Dest.Offset < 0)
	{
		error("Call to KLVObject::Base_WriteDataTo() with destination file location undefined\n");
		return 0;
	}

	if(Dest.KLSize < 0)
	{
		error("Call to KLVObject::Base_WriteDataTo() before call to KLVObject::Base_WriteKL()\n");
		return 0;
	}

	// Seek to the start of the requested data
	Dest.File->Seek(Dest.Offset + Dest.KLSize + Offset);

//@'@printf("@0x%06x:Base_WriteDataTo(Buffer, 0x%06x, 0x%06x)", (int)Dest.File->Tell(), (int)Offset, (int)Size);
	// Write from the specified buffer
	return (Length)Dest.File->Write(Buffer, Size);
//@'@Length Ret = Dest.File->Write(Buffer, Size);
//@'@printf("->0x%06x\n", (int)Dest.File->Tell());
//@'@return Ret;
}

