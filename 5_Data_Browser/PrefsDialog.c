/*

File: PrefsDialog.c

Abstract: Handles all Dialog related functions for the Demo.

Version: 5.0

Change History:
	
	<5.0>	converted everything from a List Box control to a Data 
			Browser control
			removed changePanel and listBoxControlEventHandler
			renamed RedrawPrefsDialogList to 
			RedrawPrefsDialogDataBrowser which doesn't actually draw 
			the data browser but rather just invalidates its content 
			rectangle
	<4.0>	removed all Classic code
			removed HandleDialogEvents and dialogFilter
			adjusted the RunningInMacOSX dependent stuff
	<3.0>	converted the Carbon version to use Carbon Events and 
			standard handlers
			added dialogEventHandler and listBoxControlEventHandler 
			to support Carbon Events
			the Carbon Event model doesn't mix all that well with 
			the Dialog Manager, as a workaround we gave the controls 
			Control IDs that match their Dialog Item IDs
	<2.0>	Carbonized
			we have to register our list definition procedure in 
			Carbon because code resources are unsupported
			also we have to flush the QuickDraw port after our calls 
			to HiliteControl
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
#include "IconDBUtilities.h"
#include "PrefsDialog.h"

static EventHandlerUPP gDialogEventHandler;

static pascal OSStatus dialogEventHandler(EventHandlerCallRef nextHandler, EventRef event, 
											void *prefsDialog);
static void handleDialogItemHit(DialogRef prefsDialog, DialogItemIndex itemHit);


// --------------------------------------------------------------------------------------
void  OpenPrefsDialog(void)
{
	DialogRef dialog;
	WindowRef dialogWindow;
	EventTypeSpec dialogEvents[] = {
									{kEventClassControl, kEventControlHit}
	                               };
	ControlID controlID = {kAppSignature, 0};
	ControlRef control, userPanes[10];
	short iconDBBottom;
	Rect iconDBRect;
	
	dialog = GetNewDialog(rPrefsDialog, NULL, kFirstWindowOfClass);
	if (dialog == NULL)
		ExitToShell();
	SetPortDialogPort(dialog);
	dialogWindow = GetDialogWindow(dialog);
																// install window handlers
	ChangeWindowAttributes(dialogWindow, kWindowStandardHandlerAttribute, kWindowNoAttributes);
	gDialogEventHandler = NewEventHandlerUPP(dialogEventHandler);
	InstallWindowEventHandler(dialogWindow, gDialogEventHandler, 
								GetEventTypeCount(dialogEvents), dialogEvents, (void *)dialog, 
								NULL);
	
	GetDialogItemAsControl(dialog, kStdOkItemIndex, &control);		// set control IDs to 
	controlID.id = kStdOkItemIndex;									// match dialog item 
	SetControlID(control, &controlID);								// indices which are 
	SetWindowDefaultButton(dialogWindow, control);					// not tracked by any 
																	// standard handlers
	GetDialogItemAsControl(dialog, kStdCancelItemIndex, &control);	// also set the default 
	controlID.id = kStdCancelItemIndex;				// and cancel buttons (because Mac OS 8/9 
	SetControlID(control, &controlID);				// under CarbonLib doesn't respect the 
	SetWindowCancelButton(dialogWindow, control);	// dialog's default and cancel buttons)
	
																// get the bottom-most 
	iconDBBottom = kPrefsDialogHeight - kWindowEdgeSpacing;		// coordinate we can use
	iconDBBottom -= (kPrefsDialogHeight - kWindowEdgeSpacing - kDataBrowserInset - 
						(kWindowEdgeSpacing + kDataBrowserInset) ) % 
						kRowHeight;	// then subtract out the partial cell height that would 
									// be drawn on the bottom so that it's not actually drawn
	SetRect(&iconDBRect, kWindowEdgeSpacing , kWindowEdgeSpacing, 
			kWindowEdgeSpacing + kDataBrowserInset + kColumnWidth + kDataBrowserInset + 
			kScrollBarWidth, iconDBBottom);
	
	GetDialogItemAsControl(dialog, iUserPane1 - 1, &userPanes[0]);	// because the data browser 
	GetDialogItemAsControl(dialog, iUserPane2 - 1, &userPanes[1]);	// hasn't been inserted 
	GetDialogItemAsControl(dialog, iUserPane3 - 1, &userPanes[2]);	// yet, the user panes are 
	GetDialogItemAsControl(dialog, iUserPane4 - 1, &userPanes[3]);	// one item number lower 
	GetDialogItemAsControl(dialog, iUserPane5 - 1, &userPanes[4]);	// than their final values
	GetDialogItemAsControl(dialog, iUserPane6 - 1, &userPanes[5]);
	GetDialogItemAsControl(dialog, iUserPane7 - 1, &userPanes[6]);
	GetDialogItemAsControl(dialog, iUserPane8 - 1, &userPanes[7]);
	GetDialogItemAsControl(dialog, iUserPane9 - 1, &userPanes[8]);
	GetDialogItemAsControl(dialog, iUserPane10 - 1, &userPanes[9]);
	
	control = CreateIconDataBrowser(dialogWindow, &iconDBRect, userPanes);
	InsertDialogItem(dialog, kStdCancelItemIndex, kControlDialogItem, (Handle)control, 
						&iconDBRect);
	
	SetPrefsDialogHelpTags(dialog);
	
	DisableMenuItem(GetMenuRef(mDemonstration), iPrefsDialog);
	ShowWindow(dialogWindow);
} // OpenPrefsDialog

// --------------------------------------------------------------------------------------
void ClosePrefsDialog(DialogRef prefsDialog)
{
	ControlRef iconDataBrowser;
	
	HideWindow(GetDialogWindow(prefsDialog));
	
	GetDialogItemAsControl(prefsDialog, iIconDataBrowser, &iconDataBrowser);
	ReleaseIconDataBrowserItemData(iconDataBrowser);
	DisposeEventHandlerUPP(gDialogEventHandler);
	DisposeDialog(prefsDialog);
	
	EnableMenuItem(GetMenuRef(mDemonstration), iPrefsDialog);
}

// --------------------------------------------------------------------------------------
void RedrawPrefsDialogDataBrowser(DialogRef prefsDialog)
{
	ControlRef iconDataBrowser;
	Rect iconDBRect;
	
	GetDialogItemAsControl(prefsDialog, iIconDataBrowser, &iconDataBrowser);
	GetControlBounds(iconDataBrowser, &iconDBRect);
	
	iconDBRect.right -= kScrollBarWidth;
	InsetRect(&iconDBRect, kDataBrowserInset, kDataBrowserInset);
	InvalWindowRect(GetDialogWindow(prefsDialog), &iconDBRect);
}

#pragma mark -
// --------------------------------------------------------------------------------------
static pascal OSStatus dialogEventHandler(EventHandlerCallRef nextHandler, EventRef event, 
											void *prefsDialog)
{
#pragma unused (nextHandler)

	OSStatus result = eventNotHandledErr;
	UInt32 eventClass, eventKind;
	ControlRef controlHit;
	ControlID controlID;
	
	eventClass = GetEventClass(event);
	eventKind = GetEventKind(event);
	
	switch (eventClass)
	{
		case kEventClassControl:
			switch (eventKind)
			{
				case kEventControlHit:
					GetEventParameter(event, kEventParamDirectObject, typeControlRef, NULL, 
										sizeof(ControlRef), NULL, &controlHit);
					GetControlID(controlHit, &controlID);
					
					handleDialogItemHit((DialogRef)prefsDialog, (DialogItemIndex)controlID.id);
					result = noErr;
					break;
			}
			break;
	}
	
	return result;
}

// --------------------------------------------------------------------------------------
static void handleDialogItemHit(DialogRef prefsDialog, DialogItemIndex itemHit)
{
	if ( (itemHit == kStdOkItemIndex) || (itemHit == kStdCancelItemIndex) )
		ClosePrefsDialog(prefsDialog);
}