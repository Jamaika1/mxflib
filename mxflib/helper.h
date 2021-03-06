/*! \file	helper.h
 *	\brief	Verious helper function declarations
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

#ifndef MXFLIB__HELPER_H
#define MXFLIB__HELPER_H


#include <time.h>
#include <string>

namespace mxflib
{
	// Forware declare MDObjectPtr and EssenceSource
	class MDObjectPtr;
	class EssenceSource;

	//! Return the MXF version as the year of the main MXF specification document
	/*! /ret 2004 if we are writing files as per SMPTE 377M-2004
	 *! /ret 2009 if we are writing files as per SMPTE 377-1-2009
	 */
	int MXFVersion(void);

	//! Set the MXF version as the year of the main MXF specification document
	/*! Use 2004 if we are writing files as per SMPTE 377M-2004
	 *! Use 2009 if we are writing files as per SMPTE 377-1-2009
	 */
	void SetMXFVersion(int Year);

	// Declare the global null-ul which is 16 zero bytes
	extern const UL Null_UL;
//_atoi64 is s wondows only - rewrite to linux version
#ifndef _WIN32
#define _atoi64 atoll
#endif
	
	inline Int64 ato_Int64(const std::string str) { return _atoi64(str.c_str()); };
	inline Int64 ato_UInt64(const std::string str) { return (UInt64)_atoi64(str.c_str()); };



	//! Make a string containing a number
	inline std::string Int2String(int Num, int Digits = 0)
	{
		char Buffer[18];
		if(Digits > 16) Digits = 16;
		sprintf(Buffer, "%0*d", Digits, Num);
		return std::string(Buffer);
	}

	//! Make a string containing an unsigned number
	inline std::string UInt2String(int Num, int Digits = 0)
	{
		char Buffer[18];
		if(Digits > 16) Digits = 16;
		sprintf(Buffer, "%0*u", Digits, Num);
		return std::string(Buffer);
	}

	// Support old capitalization of unsigned integers
	inline std::string Uint2String(int Num, int Digits = 0) { return UInt2String(Num, Digits); }

	//! Make a hex string containing a number
	inline std::string Int2HexString(int Num, int Digits = 0)
	{
		char Buffer[18];
		if(Digits > 16) Digits = 16;
		sprintf(Buffer, "%0*x", Digits, Num);
		return std::string(Buffer);
	}

	//! Convert a time to an ISO 8601 string
	/*! \note ISO 8601 suggests "T" as a separator between date and time. 
	 *	To get this behaviour set StrictISO to true
	 *	\note ANSI-C doesn't seem to have a way to get milliseconds */
	inline std::string Time2String(full_time Time, bool StrictISO = false)
	{
		char Buffer[32];
		
		if(StrictISO)
			strftime(Buffer, 31, "%Y-%m-%dT%H:%M:%S", gmtime( &Time.time ));
		else
			strftime(Buffer, 31, "%Y-%m-%d %H:%M:%S", gmtime( &Time.time ));

		// Append the milliseconds
		sprintf(&Buffer[strlen(Buffer)], ".%03dZ", Time.msBy4 * 4);

		return std::string(Buffer);
	}


	//! Convert an ISO 8601 Date/Time string (as per TimeStamp) to a struct tm
	/*! \ret Converted time, or all zero on error - can be detected by tm_mday which is never normally 0 */
	struct tm StringToTm(std::string String);

	//! Convert an ISO 8601 Date/Time string (as per TimeStamp) to a full_time
	/*! \ret Converted time, or Ret.msBy4 = -1 on error */
	inline full_time StringToTime(std::string String)
	{
		full_time Ret;
		struct tm Time = StringToTm(String);
		if((Time.tm_mday == 0) && (String.length() < 20))
		{
			Ret.time = 0;
			Ret.msBy4 = -1;
			return Ret;
		}
		
		Ret.time = mktime(&Time);
		Ret.msBy4 = atoi(&String.c_str()[20]) / 4;
		return Ret;
	}


	//! Get the current time as an ISO 8601 string
	/*! \note ISO 8601 suggests "T" as a separator between date and time. 
	 *	To get this behaviour set StrictISO to true */
	inline std::string Now2String(bool StrictISO = false)
	{
		full_time now = GetTime();
		
		return Time2String(now, StrictISO);
	}

	//! Build a BER length
	/*! \param Data		A pointer to the buffer to receive the length
	 *	\param MazSize	The maximum length that can be written to the buffer
	 *	\param Length	The length to be converted to BER
	 *	\param Size		The total number of bytes to use for BER length (or 0 for auto)
	 *	\return The number of bytes written
	 *	\note If the size is specified it will be overridden for lengths that will not fit in Size,
	 *        <b>providing</b> they will fit in MaxSize. However an error message will be produced.
	 */
	UInt32 MakeBER(UInt8 *Data, int MaxSize, UInt64 Length, UInt32 Size = 0);


	//! Build a BER length
	/*! \param Length	The length to be converted to BER
	 *	\param Size		The total number of bytes to use for BER length (or 0 for auto)
	 *	\note If the size is specified it will be overridden for lengths
	 *		  that will not fit. However an error message will be produced.
	 */
	inline DataChunkPtr MakeBER(UInt64 Length, UInt32 Size = 0)
	{
		// Buffer for building BER
		UInt8 Buff[9];

		UInt32 Bytes = MakeBER(Buff, 9, Length, Size);

		// Return as a DataChunk
		return new DataChunk(Bytes, Buff);
	}

	//! Read a BER length
	Length ReadBER(UInt8 const **Data, int MaxSize);

	//! Read a BER length
	inline Length ReadBER(UInt8 **Data, int MaxSize)
	{
		// DRAGONS: This is always safe as we are actually adding a const!
		return ReadBER(const_cast<const UInt8**>(Data), MaxSize);
	}

	//! Encode a UInt64 as a BER OID subid (7 bits per byte)
	//! length > 0: length is maximum length of subid
	//! length == 0: as long as necessary
	//! length < 0: -length is exact length of subid
	//! returns number of bytes used
	int EncodeOID( UInt8* presult, UInt64 subid, int length );

	//! Build a new UMID
	UMIDPtr MakeUMID(int Type, const UUID *AssetID = NULL);

	//! Build a new UMID - for a specific source
	UMIDPtr MakeUMID(EssenceSource *Source, const UUID *AssetID = NULL);

	//! Build a new UMID from given values
	/*! AssetID is 16 bytes with the desired UUID */
	UMIDPtr MakeUMIDFromUUID(  int Type,  const UInt8 * AssetID );

	//! Read a "Chunk" from a non-MXF file
	DataChunkPtr FileReadChunk(FileHandle InFile, size_t Size);


	//! Read an IFF chunk header (from an open file)
	/*! The Chunk ID is read as a big-endian UInt32 and returned as the first
	 *	part of the returned pair. The chunk size is read as a specified-endian
	 *	number and returned as the second part of the returned pair
	 *	\return <0,0> if the header counld't be read
	 */
	U32Pair ReadIFFHeader(FileHandle InFile, bool BigEndian = true);


	//! Read a RIFF chunk header (from an open file)
	/*! The Chunk ID is read as a big-endian UInt32 and returned as the first
	 *	part of the returned pair. The chunk size is read as a little-endian
	 *	number and returned as the second part of the returned pair
	 *	\return <0,0> if the header counld't be read
	 */
	inline U32Pair ReadRIFFHeader(FileHandle InFile)
	{
		return ReadIFFHeader(InFile, false);
	}


	//! Read a AIFF chunk header (from an open file)
	/*! The Chunk ID is read as a big-endian UInt32 and returned as the first
	 *	part of the returned pair. The chunk size is read as a big-endian
	 *	number and returned as the second part of the returned pair
	 *	\return <0,0> if the header counld't be read
	 */
	inline U32Pair ReadAIFFHeader(FileHandle InFile)
	{
		return ReadIFFHeader(InFile, true);
	}


	//! Read a QuickTime Atom header (from an open file)
	/*! The Atom Type ID is read as a big-endian UInt32 and returned as the first
	 *	part of the returned pair. The Atom size is read as a big-endian
	 *	number and returned as the second part of the returned pair.
	 *  Extended sizes are automatically read if used.
	 *  If SkipWide is omitted (or true) any "wide" atoms are read and skipped automatically.
	 *	\return <0,0> if the header counld't be read
	 */
	std::pair<UInt32, Length> ReadAtomHeader(FileHandle InFile, bool SkipWide = true);


	//! Set a data chunk from a hex string
	DataChunkPtr Hex2DataChunk(std::string Hex);

	//! Set the search path to be used for dictionary files
	void SetDictionaryPath(std::string NewPath);

	//! Set the search path to be used for dictionary files
	inline void SetDictionaryPath(const char *NewPath) { SetDictionaryPath(std::string(NewPath)); }

	//! Search for a file of a specified name in the current dictionary search path
	/*! If the filname is either absolute, or relative to "." or ".." then the 
	 *  paths are not searched - just the location specified by that filename.
	 *  \return the full path and name of the file, or "" if not found
	 */
	std::string LookupDictionaryPath(const char *Filename);
	
	//! Search for a file of a specified name in the current dictionary search path
	inline std::string LookupDictionaryPath(std::string Filename) { return LookupDictionaryPath(Filename.c_str()); }

	//! Search a path list for a specified file
	/*! If the filname is either absolute, or relative to "." or ".." then the 
	 *  paths are not searched - just the location specified by that filename.
	 *  \return the full path and name of the file, or "" if not found
	 */
	std::string SearchPath(const char *Path, const char *Filename);

	//! Search a path list for a specified file
	inline std::string SearchPath(std::string Path, std::string Filename) { return SearchPath(Path.c_str(), Filename.c_str()); }



	// File read primitives

	//! Read 8-bit unsigned integer
	inline UInt8 ReadU8(FileHandle Handle) { unsigned char Buffer[1]; if(FileRead(Handle, Buffer, 1) == 1) return GetU8(Buffer); else return 0; }

	//! Read 16-bit unsigned integer
	inline UInt16 ReadU16(FileHandle Handle) { unsigned char Buffer[2]; if(FileRead(Handle, Buffer, 2) == 2) return GetU16(Buffer); else return 0; }

	//! Read 32-bit unsigned integer
	inline UInt32 ReadU32(FileHandle Handle) { unsigned char Buffer[4]; if(FileRead(Handle, Buffer, 4) == 4) return GetU32(Buffer); else return 0; }

	//! Read 64-bit unsigned integer
	inline UInt64 ReadU64(FileHandle Handle) { unsigned char Buffer[8]; if(FileRead(Handle, Buffer, 8) == 8) return GetU64(Buffer); else return 0; }

	//! Read 8-bit signed integer (casts from unsigned version)
	inline Int8 ReadI8(FileHandle Handle) { return (Int8)ReadU8(Handle); }

	//! Read 16-bit signed integer (casts from unsigned version)
	inline Int16 ReadI16(FileHandle Handle) { return (Int16)ReadU16(Handle); }
	
	//! Read 32-bit signed integer (casts from unsigned version)
	inline Int32 ReadI32(FileHandle Handle) { return (Int32)ReadU32(Handle); }
	
	//! Read 64-bit signed integer (casts from unsigned version)
	inline Int64 ReadI64(FileHandle Handle) { return (Int64)ReadU64(Handle); }

	//! Read 8-bit unsigned integer Little-Endian
	inline UInt8 ReadU8_LE(FileHandle Handle) { unsigned char Buffer[1]; if(FileRead(Handle, Buffer, 1) == 1) return GetU8_LE(Buffer); else return 0; }

	//! Read 16-bit unsigned integer Little-Endian
	inline UInt16 ReadU16_LE(FileHandle Handle) { unsigned char Buffer[2]; if(FileRead(Handle, Buffer, 2) == 2) return GetU16_LE(Buffer); else return 0; }

	//! Read 32-bit unsigned integer Little-Endian
	inline UInt32 ReadU32_LE(FileHandle Handle) { unsigned char Buffer[4]; if(FileRead(Handle, Buffer, 4) == 4) return GetU32_LE(Buffer); else return 0; }

	//! Read 64-bit unsigned integer Little-Endian
	inline UInt64 ReadU64_LE(FileHandle Handle) { unsigned char Buffer[8]; if(FileRead(Handle, Buffer, 8) == 8) return GetU64_LE(Buffer); else return 0; }

	//! Read 8-bit signed integer Little-Endian (casts from unsigned version)
	inline Int8 ReadI8_LE(FileHandle Handle) { return (Int8)ReadU8_LE(Handle); }

	//! Read 16-bit signed integer Little-Endian (casts from unsigned version)
	inline Int16 ReadI16_LE(FileHandle Handle) { return (Int16)ReadU16_LE(Handle); }
	
	//! Read 32-bit signed integer Little-Endian (casts from unsigned version)
	inline Int32 ReadI32_LE(FileHandle Handle) { return (Int32)ReadU32_LE(Handle); }
	
	//! Read 64-bit signed integer Little-Endian (casts from unsigned version)
	inline Int64 ReadI64_LE(FileHandle Handle) { return (Int64)ReadU64_LE(Handle); }



	//! Is a given sequence of bytes a partition pack key?
	bool IsPartitionKey(const UInt8 *Key);

	//! Does a given std::string contain a "wide" string in UTF8?
	/*! \note This currently only checks if any bytes contain >127 so it is only safe to test strings that are either 7-bit ASCII or UTF-8 */
	bool IsWideString(std::string &String);

	//! Read hex values separated by any of 'Sep'
	/*! \note Modifies the value of Source to point to the following byte
	 *  \return number of values read */
	int ReadHexString(const char **Source, int Max, UInt8 *Dest, const char *Sep);

	//! Read hex values separated by any of 'Sep', if it is a urn read any of the supported formats via StringToUL
	/*! \return number of values read 
	 */
	int ReadHexStringOrUL(const char *Source, int Max, UInt8 *Dest, const char *Sep);

	//! Read hex values separated by any of 'Sep'
	/*! \note This version does not modify the value of parameter Source
	 *  \return number of values read */
	inline int ReadHexString(const char *Source, int Max, UInt8 *Dest, const char *Sep)
	{
		const char *p = Source;
		return ReadHexString(&p, Max, Dest, Sep);
	}

	//! Build a UL from a character string, writing the bytes into a 16-byte buffer
	/*! \return true if a full 16 bytes were read into the buffer, else false
	 */
	bool StringToUL(UInt8 *Data, std::string Val);
	
	//! Build a UL from a character string, returning a smart pointer to it
	/*! \return A pointer to the new UL if a full 16 bytes were read, else NULL
	 */
	inline ULPtr StringToUL(std::string Val)
	{
		UInt8 Data[16];
		if(!StringToUL(Data, Val)) return NULL;
		return new UL(Data);
	}

	//! Build a new UUID and return it as a UL (via a ULPtr) - effectively a random UL
	/*! This is useful when a UL is required for a value, but as not been defined. 
	 * Using an end-swapped UL will allow the code to continue, presumable after a suitable error or warning message.
	 */
	inline ULPtr RandomUL(void)
	{
		UUIDPtr TempID = new mxflib::UUID;
		return new UL(TempID);
	}

	//! Split a StringArray that contains several zero terminated strings into a std::list of std::strings
	/*! DRAGONS: We use the current UTF16String GetString trait to ensure that we always have the correct handling,
	 *           even if the user wants these strings handled differently (i.e. we do it their way!)
	 */
	std::list<std::string> SplitStringArray(const MDObjectPtr &Array);

	//! Set a StringArray property that contains several zero terminated strings from a std::list of std::strings
	/*! DRAGONS: We use the current UTF16String SetString trait to ensure that we always have the correct handling,
	 *           even if the user wants these strings handled differently (i.e. we do it their way!)
	 */
	void SetStringArray(MDObjectPtr &Array, const std::list<std::string> &Strings);

	// Convert timecode to frame count
	// FIXME: Needs some work...
	inline Position TCtoFrames(int FrameRate, bool DropFrame, int Hours, int Mins, int Secs, int Frames)
	{
		Int64 f = Frames + FrameRate*( Secs + 60*( Mins + 60*Hours ) );

		if( (FrameRate == 30) && DropFrame )
		{
			UInt16 m = Mins + 60*Hours;
			f -= 2*( m - m/10 );
		}

		return f;
	}

	//! Read a complete line from a file and return it as a string, treats CR, LF, CRLF and LFCR as valid line ends
	std::string FileGets(FileHandle File);

	//! Check for a request for version number in the supplied arguments
	/*! DRAGONS: Will display version message and call exit(0) if the "-ver" or "--ver" is found at the start of an option
	 */
	void HandleVersionRequest(int argc, char *argv[], std::string VersionText);

#ifndef _WIN32
    //! Return an array of random bytes under linux. Uses the kernel random generator if possible
	void getRandnumbers( Uint8 * buff, int nbytes);
#endif
}

#endif // MXFLIB__HELPER_H
