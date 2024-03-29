/*

File: Help.c

Abstract: Sets help balloons for the preferences window in Classic and help tags 
          for the preferences window and preferences dialog in Carbon.

Version: 3.0

Change History:
	
	<3.0>	no changes necessary
	<2.0>	added support for help tags
			this was a bit of a chore for the preferences window 
			because of the list
	<1.0>	first release version

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
#include "Help.h"
#include "PrefsDialog.h"

#if TARGET_API_MAC_OS8		// additional includes for this file under Classic
#include <Balloons.h>
#include <Lists.h>
#include <Quickdraw.h>
#endif

#if TARGET_API_MAC_OS8
enum
{
	kInList       = 0,
	kInStaticText = 1,
	kInUserPane   = 2,
	kNotInHotRect = 3
};
#endif

#if TARGET_API_MAC_CARBON
static HMWindowContentUPP gPrefsWindowHelpContent;
#endif

#if TARGET_API_MAC_CARBON
static pascal OSStatus displayPrefsWindowHelpTags(WindowRef prefsWindow, Point mouseLocation, 
													HMContentRequest request, 
													HMContentProvidedType *contentProvided, 
													HMHelpContentPtr helpContent);
#endif


#if TARGET_API_MAC_OS8
// --------------------------------------------------------------------------------------
void CalculateBalloonHelpRects(WindowRef prefsWindow)
{
	GrafPtr savedPort;
	ListHandle iconList;
	Point tips[3];
	Rect hotRects[3], userPaneBounds;
	ControlRef rootControl, userPane;
	
	GetPort(&savedPort);
	SetPortWindowPort(prefsWindow);
	
	GetWindowProperty(prefsWindow, kAppSignature, kIconListTag, sizeof(ListHandle), NULL, 
						&iconList);
	SetPt(&tips[0], kPlatinumWindowEdgeSpacing + kListWidth, kPlatinumWindowEdgeSpacing + 10);
	GetListViewBounds(iconList, &hotRects[0]);	// set the icon list tip and hot rectangle
	
	SetPt(&tips[1], 203, 219);			// set the static text tip and hot rectangle
	SetRect(&hotRects[1], 149, 213, 213, 229);	// these coordinates are all arbitrary
	
	GetRootControl(prefsWindow, &rootControl);
	GetIndexedSubControl(rootControl, 1, &userPane);	// the only thing embedded in the 
	GetControlBounds(userPane, &userPaneBounds);		// root control is the user panes and 
														// they're all the same size
	SetPt(&tips[2], userPaneBounds.right - 10, userPaneBounds.top + 10);
	SetRect(&hotRects[2], userPaneBounds.left, userPaneBounds.top, userPaneBounds.right, 
			userPaneBounds.bottom);		// set the user pane tip and hot rectangle
	
	SetWindowProperty(prefsWindow, kAppSignature, kBalloonTipsTag, 3 * sizeof(Point), tips);
	SetWindowProperty(prefsWindow, kAppSignature, kHotRectsTag, 3 * sizeof(Rect), hotRects);
	
	SetPort(savedPort);
}

// --------------------------------------------------------------------------------------
void DisplayHelpBalloons(WindowRef prefsWindow)
{
	GrafPtr savedPort;
	Point mouseLocation;
	Rect hotRects[3];
	short hotRectID;
	static int previousHotRectID = kNotInHotRect;
	
	GetPort(&savedPort);
	SetPortWindowPort(prefsWindow);
	
	GetGlobalMouse(&mouseLocation);
	GlobalToLocal(&mouseLocation);
	
	GetWindowProperty(prefsWindow, kAppSignature, kHotRectsTag, 3 * sizeof(Rect), NULL, 
						hotRects);
	
	for (hotRectID = kInList; hotRectID < kNotInHotRect; hotRectID++)
	{
		if (PtInRect(mouseLocation, &hotRects[hotRectID]))
		{
			if (hotRectID != previousHotRectID)
			{
				HMMessageRecord helpMessage;
				Point tips[3];
				
				helpMessage.hmmHelpType = khmmStringRes;
				helpMessage.u.hmmStringRes.hmmResID = rHelpStrings;
				helpMessage.u.hmmStringRes.hmmIndex = hotRectID + 1;
				
				GetWindowProperty(prefsWindow, kAppSignature, kBalloonTipsTag, 
									3 * sizeof(Point), NULL, tips);
				LocalToGlobal(&tips[hotRectID]);
				
				LocalToGlobal(&topLeft(hotRects[hotRectID]));
				LocalToGlobal(&botRight(hotRects[hotRectID]));
				
				HMShowBalloon(&helpMessage, tips[hotRectID], &hotRects[hotRectID], NULL, 
								kBalloonWDEFID, kTopLeftTipPointsLeftVariant, kHMRegularWindow);
			}
				
			break;
		}
	}
	
	previousHotRectID = hotRectID;
	SetPort(savedPort);
}

#else // TARGET_API_MAC_CARBON
#pragma mark -
// --------------------------------------------------------------------------------------
void InstallPrefsWindowHelpTags(WindowRef prefsWindow)
{
	HMHelpContentRec userPaneContent, staticTextContent;
	ControlRef rootControl, userPane, staticText;
	UInt16 panelIndex;
	
		// install a help tag callback for the list
	gPrefsWindowHelpContent = NewHMWindowContentUPP(displayPrefsWindowHelpTags);
	HMInstallWindowContentCallback(prefsWindow, gPrefsWindowHelpContent);
	
		// set the help tags for the user panes
	userPaneContent.version = kMacHelpVersion;
	SetRect(&userPaneContent.absHotRect, 0, 0, 0, 0);
	userPaneContent.tagSide = kHMDefaultSide;
	userPaneContent.content[kHMMinimumContentIndex].contentType = kHMStringResContent;
	userPaneContent.content[kHMMinimumContentIndex].u.tagStringRes.hmmResID = rHelpStrings;
	userPaneContent.content[kHMMinimumContentIndex].u.tagStringRes.hmmIndex = 3;
	userPaneContent.content[kHMMaximumContentIndex].contentType = kHMNoContent;
	
		// set the help tags for the static texts
	staticTextContent.version = kMacHelpVersion;
	SetRect(&staticTextContent.absHotRect, 0, 0, 0, 0);
	staticTextContent.tagSide = kHMDefaultSide;
	staticTextContent.content[kHMMinimumContentIndex].contentType = kHMStringResContent;
	staticTextContent.content[kHMMinimumContentIndex].u.tagStringRes.hmmResID = rHelpStrings;
	staticTextContent.content[kHMMinimumContentIndex].u.tagStringRes.hmmIndex = 2;
	staticTextContent.content[kHMMaximumContentIndex].contentType = kHMNoContent;
	
	GetRootControl(prefsWindow, &rootControl);
	for (panelIndex = 1; panelIndex <=  kNumberOfRows; panelIndex++)
	{
		GetIndexedSubControl(rootControl, panelIndex, &userPane);
		HMSetControlHelpContent(userPane, &userPaneContent);
		
		GetIndexedSubControl(userPane, 1, &staticText);
		HMSetControlHelpContent(staticText, &staticTextContent);
	}
}

// --------------------------------------------------------------------------------------
void DisposePrefsWindowHelpTags(void)
{
	DisposeHMWindowContentUPP(gPrefsWindowHelpContent);
}

// --------------------------------------------------------------------------------------
static pascal OSStatus displayPrefsWindowHelpTags(WindowRef prefsWindow, Point mouseLocation, 
													HMContentRequest request, 
													HMContentProvidedType *contentProvided, 
													HMHelpContentPtr helpContent)
{
	OSStatus result = noErr;
	GrafPtr savedPort;
	ListHandle iconList;
	Rect hotRect;
	
	switch (request)
	{
		case kHMSupplyContent:
			GetPort(&savedPort);
			SetPortWindowPort(prefsWindow);
			
			GlobalToLocal(&mouseLocation);
			
			GetWindowProperty(prefsWindow, kAppSignature, kIconListTag, sizeof(ListHandle), 
								NULL, &iconList);
			GetListViewBounds(iconList, &hotRect);
			
			if (!PtInRect(mouseLocation, &hotRect))
				*contentProvided = kHMContentNotProvided;
			else
			{
				*contentProvided = kHMContentProvided;
				
				helpContent->version = kMacHelpVersion;
				
				LocalToGlobal(&topLeft(hotRect));
				LocalToGlobal(&botRight(hotRect));
				SetRect(&helpContent->absHotRect, hotRect.left, hotRect.top, hotRect.right, 
						hotRect.bottom);
				
				helpContent->tagSide = kHMDefaultSide;
				
				helpContent->content[kHMMinimumContentIndex].contentType = kHMStringResContent;
				helpContent->content[kHMMinimumContentIndex].u.tagStringRes.hmmResID = rHelpStrings;
				helpContent->content[kHMMinimumContentIndex].u.tagStringRes.hmmIndex = 1;
				
				helpContent->content[kHMMaximumContentIndex].contentType = kHMNoContent;
			}
			
			SetPort(savedPort);
			break;
	}
	return result;
}

// --------------------------------------------------------------------------------------
void  SetPrefsDialogHelpTags(DialogRef prefsDialog)
{
	HMHelpContentRec helpContent;
	ControlRef control;
	SInt16 itemIndex;
	
	helpContent.version = kMacHelpVersion;
	SetRect(&helpContent.absHotRect, 0, 0, 0, 0);	// use the current location of the controls
	helpContent.tagSide = kHMDefaultSide;
	
		// set the help tag for the list box control
	helpContent.content[kHMMinimumContentIndex].contentType = kHMStringResContent;
	helpContent.content[kHMMinimumContentIndex].u.tagStringRes.hmmResID = rHelpStrings;
	helpContent.content[kHMMinimumContentIndex].u.tagStringRes.hmmIndex = 4;
	
	helpContent.content[kHMMaximumContentIndex].contentType = kHMNoContent;
	
	GetDialogItemAsControl(prefsDialog, iIconList, &control);
	HMSetControlHelpContent(control, &helpContent);
	
		// set the help tags for the user panes
	helpContent.content[kHMMinimumContentIndex].contentType = kHMStringResContent;
	helpContent.content[kHMMinimumContentIndex].u.tagStringRes.hmmResID = rHelpStrings;
	helpContent.content[kHMMinimumContentIndex].u.tagStringRes.hmmIndex = 3;
	
	helpContent.content[kHMMaximumContentIndex].contentType = kHMNoContent;
	
	for (itemIndex = iUserPane1; itemIndex <= iUserPane10; itemIndex += 2)
	{
		GetDialogItemAsControl(prefsDialog, itemIndex, &control);
		HMSetControlHelpContent(control, &helpContent);
	}
	
		// set the help tags for the static texts
	helpContent.content[kHMMinimumContentIndex].contentType = kHMStringResContent;
	helpContent.content[kHMMinimumContentIndex].u.tagStringRes.hmmResID = rHelpStrings;
	helpContent.content[kHMMinimumContentIndex].u.tagStringRes.hmmIndex = 2;
	
	helpContent.content[kHMMaximumContentIndex].contentType = kHMNoContent;
	
	for (itemIndex = iUserPane1 + 1; itemIndex <= iUserPane10 + 1; itemIndex += 2)
	{
		GetDialogItemAsControl(prefsDialog, itemIndex, &control);
		HMSetControlHelpContent(control, &helpContent);
	}
}
#endif // TARGET_API_MAC_OS8