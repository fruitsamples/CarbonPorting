/*

File: PrefsDialog.c

Abstract: Handles all Dialog related functions for the Demo.

Version: 4.0

Change History:
	
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
#include "IconListDef.h"
#include "IconListUtilities.h"
#include "PrefsDialog.h"

static short gPanelNumber;
static EventHandlerUPP gDialogEventHandler;
static EventHandlerUPP gListBoxControlEventHandler;

static pascal OSStatus dialogEventHandler(EventHandlerCallRef nextHandler, EventRef event, 
											void *prefsDialog);
static pascal OSStatus listBoxControlEventHandler(EventHandlerCallRef nextHandler, 
													EventRef event, void *prefsDialog);
static void handleDialogItemHit(DialogRef prefsDialog, DialogItemIndex itemHit);
static void changePanel(DialogRef dialog, short newPanel);


// --------------------------------------------------------------------------------------
void  OpenPrefsDialog(void)
{
	ListDefSpec listSpec;
	DialogRef dialog;
	WindowRef dialogWindow;
	EventTypeSpec dialogEvents[] = {
									{kEventClassControl, kEventControlHit}
	                               };
	ControlID controlID = {kAppSignature, 0};
	ControlRef control;
	EventTypeSpec listBoxControlEvents[] = {
											{kEventClassTextInput, 
												kEventTextInputUnicodeForKeyEvent}
	                                       };
	ListHandle iconList;
	Cell theCell;
	
	listSpec.defType = kListDefUserProcType;
	listSpec.u.userProc = NewListDefUPP(IconListDef);	// this is automatically disposed 
														// when the program exits
	RegisterListDefinition(kIconListLDEF, &listSpec);
	
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
	
	GetDialogItemAsControl(dialog, iIconList, &control);
	controlID.id = iIconList;
	SetControlID(control, &controlID);
		/* We need to postprocess keyboard events on the icon list so that we can change 
		   panels after the user changes the selected cell by using the keyboard. */
	gListBoxControlEventHandler = NewEventHandlerUPP(listBoxControlEventHandler);
	InstallControlEventHandler(control, gListBoxControlEventHandler, 
								GetEventTypeCount(listBoxControlEvents), listBoxControlEvents, 
								(void *)dialog, NULL);
	
	GetControlData(control, kControlEntireControl, kControlListBoxListHandleTag, 
					sizeof(ListHandle), &iconList, NULL);
 	
	AddRowsAndDataToIconList(iconList, rIconListIconBaseID);
	SetListSelectionFlags(iconList, lOnlyOne);
	
	SetPt(&theCell, 0, 0);
	LSetSelect(true, theCell, iconList);
	SetKeyboardFocus(dialogWindow, control, kControlFocusNextPart);
	gPanelNumber = 0;
	
	SetPrefsDialogHelpTags(dialog);
	
	DisableMenuItem(GetMenuRef(mDemonstration), iPrefsDialog);
	ShowWindow(dialogWindow);
} // OpenPrefsDialog

// --------------------------------------------------------------------------------------
void ClosePrefsDialog(DialogRef prefsDialog)
{
	ControlRef listBoxControl;
	ListHandle iconList;
	
	HideWindow(GetDialogWindow(prefsDialog));
	GetDialogItemAsControl(prefsDialog, iIconList, &listBoxControl);
	GetControlData(listBoxControl, kControlEntireControl, kControlListBoxListHandleTag, 
					sizeof(ListHandle), &iconList, NULL);
	ReleaseIconListIcons(iconList);
	DisposeEventHandlerUPP(gDialogEventHandler);
	DisposeEventHandlerUPP(gListBoxControlEventHandler);
	RegisterListDefinition(kIconListLDEF, NULL);	// unregister the list definition
	DisposeDialog(prefsDialog);
	EnableMenuItem(GetMenuRef(mDemonstration), iPrefsDialog);
}

// --------------------------------------------------------------------------------------
void RedrawPrefsDialogList(DialogRef prefsDialog)
{
	ControlRef listBoxControl;
	
	GetDialogItemAsControl(prefsDialog, iIconList, &listBoxControl);
	Draw1Control(listBoxControl);	// redraw the list box control
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
static pascal OSStatus listBoxControlEventHandler(EventHandlerCallRef nextHandler, 
													EventRef event, void *prefsDialog)
{
	OSStatus result = eventNotHandledErr;
	UInt32 eventClass, eventKind;
	DialogRef dialog;
	WindowRef dialogWindow;
	
	eventClass = GetEventClass(event);
	eventKind = GetEventKind(event);
	
	switch (eventClass)
	{
		case kEventClassTextInput:
			switch (eventKind)
			{
				case kEventTextInputUnicodeForKeyEvent:
						/* The strategy here is to first let the default handler handle 
						   the event (i.e. change the selected cell in the category list 
						   box control), then react to that change by showing the 
						   correct category panel.  However the key pressed could 
						   potentially cause the default or cancel button to get hit.  
						   In this case, our window handler will be called which will 
						   dispose of the dialog.  If this is the case, we need to not 
						   postprocess the event.  We will test for this by getting the 
						   dialog's window, retaining it, calling the default handler, 
						   then getting the window's retain count.  If the retain count 
						   is back to 1, then we know the dialog is already disposed. */
					dialog = (DialogRef)prefsDialog;
					dialogWindow = GetDialogWindow(dialog);
					RetainWindow(dialogWindow);		// hold onto the dialog's window
					
					result = CallNextEventHandler(nextHandler, event);
					if (result == noErr)	// we don't need to postprocess if nothing happened
					{
						ItemCount retainCount;
						
						retainCount = GetWindowRetainCount(dialogWindow);
						if (retainCount > 1)		// if we're the last one holding the window
							handleDialogItemHit(dialog, iIconList);		// then there's no 
					}			// need to postprocess anything because it's about to go away
					
					ReleaseWindow(dialogWindow);
					break;
			}
			break;
	}
	
	return result;
}

#pragma mark -
// --------------------------------------------------------------------------------------
static void handleDialogItemHit(DialogRef prefsDialog, DialogItemIndex itemHit)
{
	if (itemHit == iIconList)
	{
		ControlRef listBoxControl;
		ListHandle iconList;
		Cell theCell;
		
		GetDialogItemAsControl(prefsDialog, iIconList, &listBoxControl);
		GetControlData(listBoxControl, kControlEntireControl, 
						kControlListBoxListHandleTag, sizeof(ListHandle), 
						&iconList, NULL);
 		
 		SetPt(&theCell, 0, 0);
		LGetSelect(true, &theCell, iconList);
		
		if ((theCell.v) != gPanelNumber)
			changePanel(prefsDialog, theCell.v);
	}
	else if ( (itemHit == kStdOkItemIndex) || (itemHit == kStdCancelItemIndex) )
		ClosePrefsDialog(prefsDialog);
}

// --------------------------------------------------------------------------------------
static void changePanel(DialogRef dialog, short newPanel)
{
	ControlRef userPane;
	static const SInt16 panelIndexes[kNumberOfRows] = {
														iUserPane1,
														iUserPane2,
														iUserPane3,
														iUserPane4,
														iUserPane5,
														iUserPane6,
														iUserPane7,
														iUserPane8,
														iUserPane9,
														iUserPane10
	                                                  };

	GetDialogItemAsControl(dialog, panelIndexes[gPanelNumber], &userPane);
	SetControlVisibility(userPane, false, false);	// hide the currently active panel
	
	GetDialogItemAsControl(dialog, panelIndexes[newPanel], &userPane);
	SetControlVisibility(userPane, true, true);		// and show the newly selected panel
	gPanelNumber = newPanel;
}