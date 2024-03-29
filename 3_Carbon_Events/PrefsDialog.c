/*

File: PrefsDialog.c

Abstract: Handles all Dialog related functions for the Demo.

Version: 3.0

Change History:
	
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
#include "MissingConstants.h"	// for the virtual key codes used in dialogFilter
#include "PrefsDialog.h"

#if TARGET_API_MAC_OS8		// additional includes for this file under Classic
#include <CarbonEvents.h>
#include <ControlDefinitions.h>
#include <Controls.h>
#include <MacWindows.h>
#include <OSUtils.h>
#include <Processes.h>
#include <Quickdraw.h>
#include <ToolUtils.h>
#endif

static short gPanelNumber;
#if TARGET_API_MAC_CARBON
static EventHandlerUPP gDialogEventHandler;
static EventHandlerUPP gListBoxControlEventHandler;
#endif

#if TARGET_API_MAC_OS8
static Boolean dialogFilter(EventRecord *event, DialogRef *theDialog, DialogItemIndex *itemHit);
#else
static pascal OSStatus dialogEventHandler(EventHandlerCallRef nextHandler, EventRef event, 
											void *prefsDialog);
static pascal OSStatus listBoxControlEventHandler(EventHandlerCallRef nextHandler, 
													EventRef event, void *prefsDialog);
#endif
static void handleDialogItemHit(DialogRef prefsDialog, DialogItemIndex itemHit);
static void changePanel(DialogRef dialog, short newPanel);


// --------------------------------------------------------------------------------------
void  OpenPrefsDialog(void)
{
	ListDefSpec listSpec;
	DialogRef dialog;
	SInt16 dialogResourceID;
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
	
	if (!RunningInMacOSX())
		dialogResourceID = rPrefsDialogPlatinum;
	else
		dialogResourceID = rPrefsDialogAqua;
	
#if TARGET_API_MAC_OS8
#pragma unused (listSpec)
#else
	listSpec.defType = kListDefUserProcType;
	listSpec.u.userProc = NewListDefUPP(IconListDef);	// this is automatically disposed 
														// when the program exits
	RegisterListDefinition(kIconListLDEF, &listSpec);
#endif
	
	dialog = GetNewDialog(dialogResourceID, NULL, kFirstWindowOfClass);
	if (dialog == NULL)
		ExitToShell();
	SetPortDialogPort(dialog);
	dialogWindow = GetDialogWindow(dialog);
	
#if TARGET_API_MAC_OS8		// Mac OS 8/9 under CarbonLb doesn't respect the dialog's 
	SetDialogDefaultItem(dialog, kStdOkItemIndex);			// default or cancel items 
	SetDialogCancelItem(dialog, kStdCancelItemIndex);
#else
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
	controlID.id = kStdCancelItemIndex;								// and cancel buttons
	SetControlID(control, &controlID);					// (because CarbonLib doesn't respect 
	SetWindowCancelButton(dialogWindow, control);	// the dialog's default and cancel buttons)
	
	GetDialogItemAsControl(dialog, iIconList, &control);
	controlID.id = iIconList;
	SetControlID(control, &controlID);
		/* We need to postprocess keyboard events on the icon list so that we can change 
		   panels after the user changes the selected cell by using the keyboard. */
	gListBoxControlEventHandler = NewEventHandlerUPP(listBoxControlEventHandler);
	InstallControlEventHandler(control, gListBoxControlEventHandler, 
								GetEventTypeCount(listBoxControlEvents), listBoxControlEvents, 
								(void *)dialog, NULL);
#endif
	
	GetDialogItemAsControl(dialog, iIconList, &control);
	GetControlData(control, kControlEntireControl, kControlListBoxListHandleTag, 
					sizeof(ListHandle), &iconList, NULL);
 	
	AddRowsAndDataToIconList(iconList, rIconListIconBaseID);
	SetListSelectionFlags(iconList, lOnlyOne);
	
	SetPt(&theCell, 0, 0);
	LSetSelect(true, theCell, iconList);
	SetKeyboardFocus(dialogWindow, control, kControlFocusNextPart);
	gPanelNumber = 0;
	
#if TARGET_API_MAC_CARBON
	SetPrefsDialogHelpTags(dialog);
#endif
	
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
#if TARGET_API_MAC_CARBON
	DisposeEventHandlerUPP(gDialogEventHandler);
	DisposeEventHandlerUPP(gListBoxControlEventHandler);
	RegisterListDefinition(kIconListLDEF, NULL);	// unregister the list definition
#endif
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
#if TARGET_API_MAC_OS8
// --------------------------------------------------------------------------------------
void HandleDialogEvents(EventRecord *event)
{
	Boolean eventHandled;
	DialogRef prefsDialog;
	DialogItemIndex itemHit;
	
	eventHandled = dialogFilter(event, &prefsDialog, &itemHit);		// we call this 
		// before and not after DialogSelect to filter out key presses that should simulate 
		// clicks on the OK or Cancel buttons and to respond to menu key equivalents
	
	if (!eventHandled)	// handle most other events
		eventHandled = DialogSelect(event, &prefsDialog, &itemHit);
	
	if (eventHandled)
	{
		handleDialogItemHit(prefsDialog, itemHit);
	}
}

// --------------------------------------------------------------------------------------
static Boolean dialogFilter(EventRecord *event, DialogRef *theDialog, DialogItemIndex *itemHit)
{
		/* See the comments for HandleKeyDown in PrefsWindow.c about why we're using 
		   virtual key codes instead of character codes. */
	Boolean eventHandled = false;
	char charCode, keyCode;
	
	switch (event->what)
	{
		case keyDown:	// handle key presses not handled by DialogSelect
		case autoKey:
			*theDialog = GetDialogFromWindow(FrontNonFloatingWindow());
			SetPortDialogPort(*theDialog);
			charCode = event->message & charCodeMask;
			keyCode = (event->message & keyCodeMask) >> 8;
			
			if ((event->modifiers & cmdKey) != 0)		// check to see if any menu commands 
			{					// were selected - this isn't necessarily handling the event
				UInt32 menuChoice;
				OSErr error;
				MenuCommand commandID;
				
				menuChoice = MenuEvent(event);
				error = GetMenuItemCommandID(GetMenuRef(HiWord(menuChoice)), 
												LoWord(menuChoice), &commandID);
				if (error == noErr)
				{
					if (commandID == 0)		// if the menu item chosen does not have a command 
						commandID = (MenuCommand)menuChoice;		// ID (but they all should)
					HandleMenuChoice(commandID);
				}
			}
			
			if ( (keyCode == kEnterKeyCode) || (keyCode == kReturnKeyCode) || 
				(keyCode == kEscapeKeyCode) || 
				( ((event->modifiers & cmdKey) != 0) && (charCode == '.') ) )
			{
				ControlRef button;
				unsigned long finalTicks;
				
				if ( (keyCode == kEnterKeyCode) || (keyCode == kReturnKeyCode) )
				{
					GetDialogItemAsControl(*theDialog, kStdOkItemIndex, &button);
					*itemHit = kStdOkItemIndex;
				}
				else
				{
					GetDialogItemAsControl(*theDialog, kStdCancelItemIndex, &button);
					*itemHit = kStdCancelItemIndex;
				}
				
				HiliteControl(button, kControlButtonPart);
				Delay(8, &finalTicks);
				HiliteControl(button, kControlNoPart);
				eventHandled = true;
			}
	}
	
	return eventHandled;
}

#else // TARGET_API_MAC_CARBON
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
#endif

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