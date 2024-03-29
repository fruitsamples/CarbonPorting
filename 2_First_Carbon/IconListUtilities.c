/*

File: IconListUtilities.c

Abstract: Handles adding and removing data from the custom Icon List.

Version: 2.0

Change History:
	
	<2.0>	changed ReleaseListIcons to make sure the icons get 
			released when they are unregistered (see the comment 
			in the function for discussion)
	<1.0>	first release version
			though we're not using them, Mac OS 8.6 introduces 
			packages; we made getCurrentResourceFSSpec to allow for 
			the possibility that our main application file might not 
			contain the 'icns' resources at some point in the future

� Copyright 2005 Apple Computer, Inc. All rights reserved.

IMPORTANT:  This Apple software is supplied to 
you by Apple Computer, Inc. ("Apple") in 
consideration of your agreement to the following 
terms, and your use, installation, modification 
or redistribution of this Apple software 
constitutes acceptance of these terms.  If you do 
not agree with these terms, please do not use, 
install, modify or redistribute this Apple 
software.

In consideration of your agreement to abide by 
the following terms, and subject to these terms, 
Apple grants you a personal, non-exclusive 
license, under Apple's copyrights in this 
original Apple software (the "Apple Software"), 
to use, reproduce, modify and redistribute the 
Apple Software, with or without modifications, in 
source and/or binary forms; provided that if you 
redistribute the Apple Software in its entirety 
and without modifications, you must retain this 
notice and the following text and disclaimers in 
all such redistributions of the Apple Software. 
Neither the name, trademarks, service marks or 
logos of Apple Computer, Inc. may be used to 
endorse or promote products derived from the 
Apple Software without specific prior written 
permission from Apple.  Except as expressly 
stated in this notice, no other rights or 
licenses, express or implied, are granted by 
Apple herein, including but not limited to any 
patent rights that may be infringed by your 
derivative works or by other works in which the 
Apple Software may be incorporated.

The Apple Software is provided by Apple on an "AS 
IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED 
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING 
THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
OR IN COMBINATION WITH YOUR PRODUCTS.

IN NO EVENT SHALL APPLE BE LIABLE FOR ANY 
SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF 
THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
UNDER THEORY OF CONTRACT, TORT (INCLUDING 
NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.

*/

#include "ExamplePrefs.h"
#include "IconListDef.h"
#include "IconListUtilities.h"

#if TARGET_API_MAC_OS8		// additional includes for this file under Classic
#include <Files.h>
#include <Folders.h>
#include <MacMemory.h>
#include <Processes.h>
#include <Quickdraw.h>
#include <Resources.h>
#include <TextUtils.h>
#endif

static Boolean gIconsRegistered;

static OSErr getCurrentResourceFSSpec(FSSpec *curResSpec);


// --------------------------------------------------------------------------------------
void ClearIconsAreRegistered(void)
{
	gIconsRegistered = false;
}

// --------------------------------------------------------------------------------------
void AddRowsAndDataToIconList(ListHandle iconList, SInt16 iconFamilyBaseID)
{
	short dataLength, rowNumber;
	IconListCellDataRec cellData;
	Cell theCell;
	
	if (!gIconsRegistered)		// if we didn't register our icons already, we need to
	{
		OSErr error;
		FSSpec iconResFile;
		
			/* The first thing we need to do to register an IconRef is to get the FSSpec 
			   for the file containing the icon resources.  This could be the 
			   application file itself or a flattened resource file in a bundle.  Either 
			   way, in this program it's the "current" resource file so we call our 
			   utility function that converts the file reference number returned by 
			   CurResFile to an FSSpec. */
		error = getCurrentResourceFSSpec(&iconResFile);
		
		if (error != noErr)		// if we can't get our icons, this program is kind of useless
			ExitToShell();
		
			// we've got the FSSpec, now get the icons out of it
		dataLength = sizeof(IconListCellDataRec);
		
		for (rowNumber = 0; rowNumber < kNumberOfRows; rowNumber++)
		{
			RegisterIconRefFromResource(kAppSignature, 'LIc0' + rowNumber, &iconResFile, 
										iconFamilyBaseID + rowNumber, &cellData.icon);
			GetIndString(cellData.name, rIconListStrings, rowNumber + 1);
			
			rowNumber = LAddRow(1, rowNumber, iconList);	// add each row to the 
			SetPt(&theCell, 0, rowNumber);					// bottom of the List
			LSetCell(&cellData, dataLength, theCell, iconList);
		}
		
		gIconsRegistered = true;
	}
	else	// the icons are already registered so we just have to get them
	{
		dataLength = sizeof(IconListCellDataRec);
		
		for (rowNumber = 0; rowNumber < kNumberOfRows; rowNumber++)
		{
			GetIconRef(kOnSystemDisk, kAppSignature, 'LIc0' + rowNumber, &cellData.icon);
			
			GetIndString(cellData.name, rIconListStrings, rowNumber + 1);
				
			rowNumber = LAddRow(1, rowNumber, iconList);	// add each row to the 
			SetPt(&theCell, 0, rowNumber);					// bottom of the List
			LSetCell(&cellData, dataLength, theCell, iconList);
		}
	}
}

// --------------------------------------------------------------------------------------
void ReleaseIconListIcons(ListHandle iconList)
{
	short dataLength;
	IconListCellDataRec cellData;
	Cell theCell;
	UInt16 referenceCount;
	Boolean needRelease = false;
	
	dataLength = sizeof(IconListCellDataRec);
	
	SetPt(&theCell, 0, 0);
	LGetCell(&cellData, &dataLength, theCell, iconList);
	GetIconRefOwners(cellData.icon, &referenceCount);
	
	if (referenceCount == 1)	// if this is the last instance of the 
	{							// IconRefs we should unregister them
		OSType iconType;
		
			/* UnregisterIconRef doesn't decrement the reference count on some versions 
			   of Mac OS X (it does on 10.1.5, doesn't on 10.2.8, and does on 10.3.4).  
			   To account for this we will retain/acquire the icon, unregister it, then 
			   check the reference/owner count.  If it's the same then we will also 
			   release the icons.  We can't release the icons first or UnregisterIconRef 
			   will return noSuchIconErr (because the icons will already be disposed).  
			   Likewise we can't just release the icons afterwards because they may get 
			   disposed of when they're unregistered. */
		AcquireIconRef(cellData.icon);
		
		for (iconType = 'LIc0'; iconType <= 'LIc9'; iconType++)
			UnregisterIconRef(kAppSignature, iconType);
		
		gIconsRegistered = false;
		
		GetIconRefOwners(cellData.icon, &referenceCount);
		if (referenceCount > 1)
			needRelease = true;
		
		ReleaseIconRef(cellData.icon);
	}
	else						// otherwise simply release the icons
		needRelease = true;
	
	if (needRelease)
	{
		short rowNumber;
		
		ReleaseIconRef(cellData.icon);
		
		for (rowNumber = 1; rowNumber < kNumberOfRows; rowNumber++)
		{
			dataLength = sizeof(IconListCellDataRec);
			SetPt(&theCell, 0, rowNumber);
			LGetCell(&cellData, &dataLength, theCell, iconList);
			ReleaseIconRef(cellData.icon);
		}
	}
}

// --------------------------------------------------------------------------------------
static OSErr getCurrentResourceFSSpec(FSSpec *curResSpec)
{
	OSErr error;
	FCBPBPtr fileInfo;		// file control block parameter block pointer
	StrFileName fileName;
	
		/* In the age of application packages and bundles, it's less likely that the 
		   current resource file is the application file itself.  It's more likely to be 
		   a flattened resource file in a bundle so we need a generic method instead of 
		   the traditional Process Manager method of getting the current application's 
		   FSSpec of it's executable file */
	fileInfo = (FCBPBPtr)NewPtr(sizeof(FCBPBRec));
	
	fileInfo->ioNamePtr = fileName;		// to hold the resource file's name
	fileInfo->ioVRefNum = 0;			// 0 to search through all open files on all volumes
	fileInfo->ioRefNum = CurResFile();	// get info about the main resource file 
										// by using its reference number
	fileInfo->ioFCBIndx = 0;			// 0 to ignore this parameter and use the 
										// ioRefNum parameter instead
	
	error = PBGetFCBInfoSync(fileInfo);	// synchronous because we don't have 
						// anything else to do while we're waiting on the filesystem
	
	if (error == noErr)		// if we got the necessary file info, make an FSSpec out of it
		FSMakeFSSpec(fileInfo->ioFCBVRefNum, fileInfo->ioFCBParID, fileName, curResSpec);
						
	DisposePtr((Ptr)fileInfo);
	
	return error;
}