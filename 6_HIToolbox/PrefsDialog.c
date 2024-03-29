/*

File: PrefsDialog.c

Abstract: Handles all Dialog related functions for the Demo.

Version: 6.0

Change History:
	
	<6.0>	replaced menu item enabling/disabling with menu command 
			enabling/disabling (because we aren't setting menu IDs 
			in our nib which makes it impossible to use GetMenuRef)
			converted the Mach-O version to use an Interface Builder 
			nib interface (which technically means we're using a 
			window now and not a dialog)
			changed the dialog's window event handler to handle 
			commands instead of control hits
			changed ClosePrefsDialog and 
			RedrawPrefsDialogDataBrowser to take a window instead of 
			a dialog
			removed handleDialogItemHit and put its necessary 
			functionality into dialogEventHandler
			added support for HIView/compositing mode
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
#if !TARGET_API_MAC_OSX
#include "Help.h"
#endif
#include "IconDBUtilities.h"
#include "PrefsDialog.h"

static EventHandlerUPP gDialogEventHandler;

static pascal OSStatus dialogEventHandler(EventHandlerCallRef nextHandler, EventRef event, 
											void *prefsDialogWindow);


// --------------------------------------------------------------------------------------
void  OpenPrefsDialog(void)
{
	DialogRef dialog;
	WindowRef dialogWindow;
	EventTypeSpec dialogEvents[] = {
									{kEventClassCommand, kEventCommandProcess}
	                               };
	ControlID controlID = {kAppSignature, 0};
	ControlRef control, userPanes[10];
	short iconDBBottom;
	Rect iconDBRect;
	OSStatus error;
	IBNibRef nib;
	FourCharCode prefsWindowKind = kPrefsWindowKindDialog;
	UInt16 panelIndex;
	
#if !TARGET_API_MAC_OSX
#pragma unused (error, nib, panelIndex)
	dialog = GetNewDialog(rPrefsDialog, NULL, kFirstWindowOfClass);
	if (dialog == NULL)
		ExitToShell();
	SetPortDialogPort(dialog);
	dialogWindow = GetDialogWindow(dialog);
																// install window handlers
	ChangeWindowAttributes(dialogWindow, kWindowStandardHandlerAttribute, kWindowNoAttributes);
	
	GetDialogItemAsControl(dialog, kStdOkItemIndex, &control);		// set control IDs to 
	controlID.id = kStdOkItemIndex;									// match dialog item 
	SetControlID(control, &controlID);								// indices which are 
	SetControlCommandID(control, kHICommandOK);						// not tracked by any 
	SetWindowDefaultButton(dialogWindow, control);					// standard handlers
	
	GetDialogItemAsControl(dialog, kStdCancelItemIndex, &control);	// also set the default 
	controlID.id = kStdCancelItemIndex;				// and cancel buttons (because Mac OS 8/9 
	SetControlID(control, &controlID);				// under CarbonLib doesn't respect the 
	SetControlCommandID(control, kHICommandCancel);	// dialog's default and cancel buttons)
	SetWindowCancelButton(dialogWindow, control);
	
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
	
	control = CreateIconDataBrowser(dialogWindow, &iconDBRect);
	SetupIconDataBrowser(control, userPanes);
	InsertDialogItem(dialog, kStdCancelItemIndex, kControlDialogItem, (Handle)control, 
						&iconDBRect);
	
	SetPrefsDialogHelpTags(dialog);
#else
#pragma unused (dialog, iconDBBottom, iconDBRect)
	error = CreateNibReference(CFSTR("PrefsDialog"), &nib);
	if (error != noErr)
		ExitToShell();
	
	if (MacOSVersion() < 0x1030)
		error = CreateWindowFromNib(nib, CFSTR("PrefsDlg"), &dialogWindow);
	else
		error = CreateWindowFromNib(nib, CFSTR("PrefsDlgComp"), &dialogWindow);
	
	if (error != noErr)
		ExitToShell();
	
	DisposeNibReference(nib);
	
	SetWindowProperty(dialogWindow, kAppSignature, kPrefsWindowKindTag, sizeof(FourCharCode), 
						&prefsWindowKind);
	RepositionWindow(dialogWindow, NULL, kWindowCascadeOnMainScreen);
	SetPortWindowPort(dialogWindow);
	
	if (MacOSVersion() < 0x1030)
	{
		GetRootControl(dialogWindow, &control);
		
			/* We use iUserPane because the control index of the first user pane in our 
			   nib is the same as the item number of the first user pane in the dialog 
			   (although none of the other user panes match up). */
		for (panelIndex = iUserPane1; panelIndex <= kNumberOfRows + iUserPane1; panelIndex++)
			GetIndexedSubControl(control, panelIndex, &userPanes[panelIndex - iUserPane1]);
		
		controlID.id = iIconDataBrowser;
		GetControlByID(dialogWindow, &controlID, &control);
	}
	else
	{
		HIViewFindByID(HIViewGetRoot(dialogWindow), kHIViewWindowContentID, &control);
		userPanes[0] = HIViewGetLastSubview(control);						// get the first 
		for (panelIndex = kStdCancelItemIndex; panelIndex <= iUserPane1; panelIndex++)	// user 
			userPanes[0] = HIViewGetPreviousView(userPanes[0]);							// pane
		
		for (panelIndex = 1; panelIndex < kNumberOfRows; panelIndex++)	// get the rest of them
			userPanes[panelIndex] = HIViewGetPreviousView(userPanes[panelIndex - 1]);
		
		controlID.id = iIconDataBrowser;
		HIViewFindByID(control, controlID, &control);
	}
	
	SetupIconDataBrowser(control, userPanes);
#endif
	SetKeyboardFocus(dialogWindow, control, kControlFocusNextPart);
	
	gDialogEventHandler = NewEventHandlerUPP(dialogEventHandler);
	InstallWindowEventHandler(dialogWindow, gDialogEventHandler, 
								GetEventTypeCount(dialogEvents), dialogEvents, 
								(void *)dialogWindow, NULL);
	
	DisableMenuCommand(NULL, kCommandPrDlg);
	ShowWindow(dialogWindow);
} // OpenPrefsDialog

// --------------------------------------------------------------------------------------
void ClosePrefsDialog(WindowRef prefsDialogWindow)
{
	DialogRef prefsDialog;
	ControlRef iconDataBrowser;
	ControlID iconDataBrowserID = {kAppSignature, iIconDataBrowser};
	
	HideWindow(prefsDialogWindow);
	
#if !TARGET_API_MAC_OSX
#pragma unused (iconDataBrowserID)
	prefsDialog = GetDialogFromWindow(prefsDialogWindow);
	GetDialogItemAsControl(prefsDialog, iIconDataBrowser, &iconDataBrowser);
#else
#pragma unused (prefsDialog)
	if (MacOSVersion() < 0x1030)
		GetControlByID(prefsDialogWindow, &iconDataBrowserID, &iconDataBrowser);
	else
		HIViewFindByID(HIViewGetRoot(prefsDialogWindow), iconDataBrowserID, &iconDataBrowser);
#endif
	ReleaseIconDataBrowserItemData(iconDataBrowser);
	DisposeEventHandlerUPP(gDialogEventHandler);
#if !TARGET_API_MAC_OSX
	DisposeDialog(prefsDialog);
#else
	DisposeWindow(prefsDialogWindow);
#endif
		
	EnableMenuCommand(NULL, kCommandPrDlg);
}

// --------------------------------------------------------------------------------------
void RedrawPrefsDialogDataBrowser(WindowRef prefsDialogWindow)
{
	DialogRef prefsDialog;
	ControlRef iconDataBrowser;
	ControlID iconDataBrowserID = {kAppSignature, iIconDataBrowser};
	Rect iconDBRect;
	
#if !TARGET_API_MAC_OSX
#pragma unused (iconDataBrowserID)
	prefsDialog = GetDialogFromWindow(prefsDialogWindow);
	GetDialogItemAsControl(prefsDialog, iIconDataBrowser, &iconDataBrowser);
	GetControlBounds(iconDataBrowser, &iconDBRect);
	
	iconDBRect.right -= kScrollBarWidth;
	InsetRect(&iconDBRect, kDataBrowserInset, kDataBrowserInset);
	InvalWindowRect(prefsDialogWindow, &iconDBRect);
#else
#pragma unused (prefsDialog)
	if (MacOSVersion() < 0x1030)
	{
		GetControlByID(prefsDialogWindow, &iconDataBrowserID, &iconDataBrowser);
		GetControlBounds(iconDataBrowser, &iconDBRect);
		
		iconDBRect.right -= kScrollBarWidth;
		InsetRect(&iconDBRect, kDataBrowserInset, kDataBrowserInset);
		InvalWindowRect(prefsDialogWindow, &iconDBRect);
	}
	else
	{
		HIViewFindByID(HIViewGetRoot(prefsDialogWindow), iconDataBrowserID, &iconDataBrowser);
		HIViewSetNeedsDisplay(iconDataBrowser, true);
	}
#endif
}

#pragma mark -
// --------------------------------------------------------------------------------------
static pascal OSStatus dialogEventHandler(EventHandlerCallRef nextHandler, EventRef event, 
											void *prefsDialogWindow)
{
#pragma unused (nextHandler)

	OSStatus result = eventNotHandledErr;
	UInt32 eventClass, eventKind;
	HICommand command;
	
	eventClass = GetEventClass(event);
	eventKind = GetEventKind(event);
	
	switch (eventClass)
	{
		case kEventClassCommand:
			switch (eventKind)
			{
				case kEventCommandProcess:
					GetEventParameter(event, kEventParamDirectObject, typeHICommand, NULL, 
										sizeof(HICommand), NULL, &command);
					
					switch (command.commandID)
					{
						case kHICommandOK:
						case kHICommandCancel:
							ClosePrefsDialog((WindowRef)prefsDialogWindow);
							result = noErr;
							
							break;
					}
					break;
			}
			break;
	}
	
	return result;
}