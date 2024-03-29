/*

File: Help.c

Abstract: Sets help balloons for the preferences window.

Version: 1.0

Change History:
	
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

#include <Balloons.h>
#include <Lists.h>
#include <Quickdraw.h>

enum
{
	kInList       = 0,
	kInStaticText = 1,
	kInUserPane   = 2,
	kNotInHotRect = 3
};


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
	SetRect(&hotRects[0], (*iconList)->rView.left, (*iconList)->rView.top, 
			(*iconList)->rView.right, (*iconList)->rView.bottom);	// set the icon list tip 
																	// and hot rectangle
	
	SetPt(&tips[1], 203, 219);			// set the static text tip and hot rectangle
	SetRect(&hotRects[1], 149, 213, 213, 229);	// these coordinates are all arbitrary
	
	GetRootControl(prefsWindow, &rootControl);			// the only thing embedded in the 
	GetIndexedSubControl(rootControl, 1, &userPane);	// root control is the user panes and 
	SetRect(&userPaneBounds, (*userPane)->contrlRect.left, (*userPane)->contrlRect.top, 
			(*userPane)->contrlRect.right, (*userPane)->contrlRect.bottom);		// they're 
																		// all the same size
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
	
	GetMouse(&mouseLocation);
	
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
								kBalloonWDEFID, kTopLeftTipPointsLeftVariant, 
								kHMRegularWindow);
			}
				
			break;
		}
	}
	
	previousHotRectID = hotRectID;
	SetPort(savedPort);
}