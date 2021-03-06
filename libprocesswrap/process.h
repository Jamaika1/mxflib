/*! \file	process.h
 *	\brief	Declarations for MXF wrapping functions
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

#ifndef _PROCESS_H_
#define _PROCESS_H_

#include <list>

#include "mxflib/mxflib.h"
using namespace mxflib;

#include "opmap.h"



//! return the text version of the productIDs
// TODO move somewhere else
std::string GetVersionText();


//! The empirically smallest Header
const UInt32 EmpiricalSmallestHeader=16*1024;

//! DM Dictionaries
typedef std::list<std::string> DMFileList;

//! DM Items - name-value pairs for insertion into DM Frameworks
typedef std::pair<std::string,std::string> DMItem;

//! UserComments
typedef std::pair<std::string,std::string> BinComment;

enum PartitionMode {		//!< Allowed modes of body partition insertion
			Body_None, 
			Body_Duration, 
			Body_Size 
			};	

struct _EssenceStreamInfo
{
	GCStreamID EssenceID;						// Essence stream ID for each essence stream
	TimecodeComponentPtr FPTimecodeComponent;	// Timecode component for each file package
	TrackPtr MPTrack;							// Material Package track for each essence stream
	TrackList MPSubTracks;						// Any extra Material Package tracks linked to the same essence (such as DV audio)
	TrackPtr SPEdgecodeTrack;					// Material Package track for edgecode if DPX name is used
	TrackPtr FPTrack;							// File Package track for each essence stream
	TrackList FPSubTracks;						// Any extra File Package tracks linked to the same essence (such as DV audio)
	TrackPtr TSPTrack;							// Physical Source Package track for each essence stream
	TrackPtr FSPTrack;							// Physical Source Package track for each essence stream
	TrackPtr SoundRollTrack;					// Track in Avid special Sound source package
	SourceClipPtr MPClip;						// Material Package SourceClip for each essence stream 
	ComponentPtr FPClip;						// File Package SourceClip for each essence stream 
	ComponentPtr TSPClip;						// Physical Source Package SourceClip for each essence stream 
	ComponentPtr FSPClip;						// Physical Source Package SourceClip for each essence stream 
	BodyStreamPtr Stream;						// BodyStream for each stream being built (master stream if frame-grouping)

};
typedef struct _EssenceStreamInfo  EssenceStreamInfo;

enum  TestFrameFormat {
	Test_None,
	Test_Bars,
	Test_Bars75,
	Test_GreyScale,
	Test_Triangle,
	Test_Random,
	Test_TrianglePlus,
	Test_FastBars,
	Test_SingleColour,
	Test_FieldOrder,
	Test_16MIRE
};


#ifndef THREEWAY_T
#define THREEWAY_T
//also defined in VC3Codec/DNxHD.h

//Used for R601 conversion action that needs 3 states
enum _ThreeWay {
	ForceOff,
	NoAction,
	ForceOn
};
typedef enum _ThreeWay ThreeWay_t;

#endif



struct __UUID__ 
{
	unsigned char data[16];

	__UUID__()
	{
		memset(data,0,16);
	}

	__UUID__* operator=(  unsigned char * RHS )
	{
		memcpy( data,RHS,16);
		return this;
	}

	bool isValid()
	{ //return false if NULL UUID - true otherwise
		int * pint=reinterpret_cast<int *>(data);
		if(*pint!=0)
			return true;
		if(*++pint!=0)
			return true;
		if(*++pint!=0)
			return true;
		if(*++pint!=0)
			return true;
		
		return false;
	}

	operator unsigned char *()
	{
		return data;
	};
};

typedef struct __UUID__  __UUID;



struct ProcessOptions
{
#define NUMB_DMGARBAGES 2
public:

	//! Debug flag for MXFLib
	bool DebugMode;
	std::string  TestMode;   //String to contol test modes

	bool Stats;				//!< enable basic Sdats output at final exit
	bool ShowTiming;			//!< Show timing 

	/*********************************************************
	***
	*** Options relating to Operational Patterns
	***
	**********************************************************/
	bool OPAtom;				//!< Is OP-Atom mode being forced?
	bool OPAtom2Part;			//!< Has a 2-partition OP-Atom file been requested (only works for VBR)


	/*********************************************************
	***
	*** Options relating to filenames
	***
	**********************************************************/
	std::string  TempDir;       //!< Location for temporary file - should be on the same device as the final location of the file
								//   If making file for Avid fast import must be on the same device
	std::string  TempBody;       //!< body of temproary filename

	std::string  FileName;   //!< Name of finished file. If Avid file being created use of this is not recommended. - use generated filename 
	std::string  FinalDir;   //!< Final directory of file if the file name is being generated by mxfwrap
	std::string  FileNameStart;  //!< optional prefix to generated file name

	//set limits on the number of files we can handle
	const static int MaxInFiles=32;
	const static int MaxOutFiles=17; //16 audio and 1 video
	const static int MaxFilenameLen=256;
	char InFilenameSet[MaxInFiles*MaxFilenameLen];	//!< The set of input filenames
	char InFilename[MaxInFiles][MaxFilenameLen];	//!< The list of input filenames
	int InFileGangCount;							//!< The number of sets of ganged files to process
	int InFileGangSize;								//!< The number of ganged files to process at a time
	std::string AudioFilename;						//!< The name of the input audio file
	std::string P2VideoFilename;					//!< Name of directory at top or P2 essence tree.

	std::string MXFVideoFilename;					//!< Name of directory at top or P2 essence tree.
	std::string MXFAudioFilename[MaxInFiles];		//!< Name of MXF file for audio input
	
	char OutFilenameSet[MaxOutFiles*MaxFilenameLen];//!< The set of output filenames
	char OutFilename[MaxOutFiles][MaxFilenameLen];	//!< The output filename
	int OutFileCount;								//!< The number of files to output





	/*********************************************************
	***
	*** Options relating to detailed control of format
	***
	**********************************************************/

	UInt32 KAGSize;
	UInt32 HeaderPadding ;					//!< The (minimum) number of bytes of padding to leave in the header
	UInt32 HeaderSize ;						//!< The (minimum) size of the header
	UInt32 BlockSize ;						//!< The BlockAlign size for this file
	Int32 BlockOffset ;						//!< The BlockAlign essence offset for this file
	Int32 BlockIndexOffset ;				//!< The BlockAlign index offset for this file

	bool UpdateHeader;						//!< Is the header going to be updated after writing the footer

	bool FrameGroup;						//!< Group all as a frame-wrapped group (in one essence container)
	bool ZeroPad;							//!< Pad streams with zero bytes if they end earlier than others in the same frame-group
	bool StreamMode ;						//!< Wrap in stream-mode

	unsigned int AudioLimit;				//!< Maximum number of audio channels per container, 0 for unlimited (will demux higher numbers)
	unsigned int AudioBits;					//!< The number of output bits when demuxing (0 = same as input bits)

	bool UseIndex	;						//!< Write complete index tables
	bool SparseIndex ;						//!< Write sparse index tables (one entry per partition)
	bool SprinkledIndex ;					//!< Write segmented index tables (one set per partition)
	bool IsolatedIndex ;					//!< Don't write essence and index in same partition
	bool VeryIsolatedIndex ;				//!< Don't write metadata and index in same partition
	bool IsolatedEssence ;					//!< Don't write essence and header metadata in same partition

	bool IncludeSubstreams;					//!< Should sub-streams also be wrapped

	bool PutTCTrack;						//!< If false suppresses the output or Timecode tracks


	bool ExtractTimecode;					//!< Should we attempt to set FP timecodes from extracted timecode?

	//options relating to body partitions
	PartitionMode BodyMode;					//!< The mode of body partition insertion
	UInt32 BodyRate;						//!< The rate of body partition insertion
	bool EditAlign ;						//!< Start new body partitions only at the start of a GOP


	Rational ForceEditRate;					//!< Edit rate to try and force
	Rational AudioEditRate;					//!< Edit rate to set the audio rate if different form force edit rate


	int SelectedWrappingOption;				//!< Selected wrapping option: -1 = auto, 0 = list choices
	std::list<int> SecondaryWrapping;		//!< A list of 2nd, 3rd, 4th etc -w= values for 2nd, 3rd, 4th etc. source files
	std::string SelectedWrappingOptionText;	//!< Selected wrapping option name
	StringList SecondaryWrappingText;		//!< A list of 2nd, 3rd, 4th etc -w= values for 2nd, 3rd, 4th etc. source files

	/*********************************************************
	***
	*** Options related to audio  buffers
	***
	**********************************************************/
	int nAudChannels;						//!< Number of Audio channels that will be passed by buffer
	int AudioSampleRate;					//!< Audio sample rate - currently only 48000 is supported
	int AudioBitDepth;						//!< Audio channel bit depth - currently only 16 is supported
	int ExtractAudio;						//!< Extract audio from compressed audio stream.

	/*********************************************************
	***
	*** Derived Options
	***
	**********************************************************/

	UL OPUL;								//!< The UL of the OP for this file

	/*********************************************************
	***
	*** Options relating to Dictionaries
	***
	**********************************************************/

	DMFileList DMDicts;						//!< DM Dictionaries
	DMFileList DMMaterialInstances;			//!< DM Instance data files for Material Package

	std::list<DMItem> DMItems;			//!< name-value pairs for insertion into DM Frameworks

	DMFileList DMFileInstances;			    //!< DM Instance data files for File Package

	std::string OrthodoxDict;				//!< Default filename of orthodox Dictionary
	bool OverrideDictionary;		//!< true to use Compiled dictionary in place of xml dictionary (or vice versa)




	std::string MobName ;					//!< Name of master mob


	/************************************************************************
	***
	*** Options relating to reading image files  
	***
	************************************************************************/




	/*********************************************************
	***
	*** Member Functions
	***
	**********************************************************/


	ProcessOptions():
		OPUL(OP1a_Data)
	{
		DebugMode = false;
		Stats = false;
		ShowTiming = true;

		SelectedWrappingOption=-1;

		OrthodoxDict = "";
		OverrideDictionary = false;


		KAGSize=1;

		HeaderPadding=0;					// The (minimum) number of bytes of padding to leave in the header
		HeaderSize=0 ;						// The (minimum) size of the header
		BlockSize=0 ;						// The BlockAlign size for this file
		BlockOffset=0 ;						// The BlockAlign essence offset for this file
		BlockIndexOffset=0 ;				// The BlockAlign index offset for this file

		UpdateHeader=false;

		AudioLimit = 0;

		OPAtom=false;		
		OPAtom2Part=false;	
		FrameGroup=false;
		ZeroPad = false;
		StreamMode=false;
		UseIndex=false	;		
		SparseIndex=false ;		
		SprinkledIndex=false ;	
		IsolatedIndex=false ;		
		VeryIsolatedIndex=false ;
		IsolatedEssence=false ;		
		EditAlign=false;

		IncludeSubstreams = true;

		PutTCTrack=true;

		// DRAGONS: deselect extraction of timecode to FP tracks
		ExtractTimecode = false;

		ForceEditRate = Rational(0,1);
		AudioEditRate = Rational(0,1);

		BodyRate=0;
		BodyMode=Body_None;




		MobName = "Material Package" ;	// Default Name of material package




		//zero out all character strings
		memset(InFilenameSet,0,sizeof(InFilenameSet));		
		memset(InFilename,0,sizeof(InFilename));		
		memset(OutFilenameSet,0,sizeof(OutFilenameSet));		
		memset(OutFilename,0,sizeof(OutFilename));	

		//Setting so for Audio buffers
		nAudChannels=0;;						// Number of Audio channels that will be passed by buffer
		AudioSampleRate=48000;					// Audio sample rate - currently only 48000 is supported
		AudioBitDepth=16;
		AudioBits=0;
		ExtractAudio=false;                     //Will only be valid if using compressed audio


	}

	~ProcessOptions()
	{
	}
};



struct _OutputFileInfo
{
	MetadataPtr  MData;                         //Metadata for each OPAtom Stream
	Length       Duration;
	IndexTablePtr IndexTable;

};
typedef struct _OutputFileInfo OutputFileInfo;


//! A pair with a file package index and a source to insert into that file package
typedef std::pair<int, EssenceSourcePtr> EssenceSourcePair;

/*!
This is the main process loop for writing an MXF file. It requires that
the file has been opened and that that essence parsers have been initialized 
with all the essence streams to be written to this file.
*/
Length Process(
				int					OutFileNum, //!< The index of the file to output. 
												//!<This will normally be 0 but when multipe atom files are
												//!<being written then the function must be called once for each OPAtom file
												//!<with this parameter incrementing each time
				MXFFilePtr			Out,		//!<The MXF file object for this output file.
				ProcessOptions		*pOpt,     //!<A pointer to the options that are reqruied for this output fle
				EssenceParser::WrappingConfigList WrappingList,  //!< A List of the acceptable Wrapping options for the essence being wrapped. In many cases there will only be one suitable option
				EssenceSourcePair	*Source,    //!< An array of pairs ( file package index and a source to insert into that file package).
												//!< There should be one entry in the array for each essence source in the file
				Rational			EditRate,   //!< The edit rate that is right for the master package.
				UMIDPtr				MPUMID,     //!< The UMID for the master package
				UMIDPtr				*FPUMID,    //!< An array of UMIDs for each file package. There should be as many UMIDs as there are stream of essence to wrap
				UMIDPtr				*SPUMID,    //!< An pair of UMIDs for the source packages.
				bool				*pReadyForEssenceFlag=NULL  //!A pointer to a common flag used to synchronize multiple wrapping threads
			);

#endif //_PROCESS_H_
