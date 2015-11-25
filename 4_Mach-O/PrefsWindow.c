/*

File: PrefsWindow.c

Abstract: Handles all Window and List related functions for the Demo.

Version: 4.0

Change History:
	
	<4.0>	removed all Classic code
			made AdjustControls, HandleDrawContent, and 
			HandleActivate, internal functions
			made HandleContentClick an internal function and changed 
			its last parameter from an EventModifiers to a UInt32
			made HandleKeyDown an internal function and changed its 
			first parameter from a char to a UInt32
			made ClosePrefsWindow an internal function
			adjusted the RunningInMacOSX dependent stuff
			changed windowEventHander to call through to the 
			standard handler for default behavior on activation and 
			close events instead of implementing it on top of the 
			custom behavior already implemented
	<3.0>	converted the Carbon version to use Carbon Events and 
			standard handlers
			added windowEventHandler to support Carbon Events
			windowEventHandler has some similarities to handleEvents 
			in ExamplePrefs.c
			Carbon Events has richer support for mouse events, we 
			support a mouse wheel in Carbon
	<2.0>	Carbonized
			for the Carbon version we're using CreateCustomList to 
			create the list
			introduced some new globals to account for the different 
			theme metrics between the Platinum and Aqua appearances
	<1.0>	first release version
			this draws a list directly into a window and mimics the 
			behavior of a list box control
			it turns out this was actually quite difficult, there is 
			a LOT of background color/pattern changing you have to 
			do to achieve the desired effect

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
#include "MissingConstants.h"	// for the virtual key codes used in handleKeyDown
#include "PrefsWindow.h"

static short gPanelNumber;
static ListDefUPP gIconListDef;
static EventHandlerUPP gWindowEventHandler;

static void closePrefsWindow(WindowRef prefsWindow);
static void adjustControls(WindowRef prefsWindow);
static pascal OSStatus windowEventHandler(EventHandlerCallRef nextHandler, EventRef event, 
											void *junk);
static void handleDrawContent(WindowRef prefsWindow);
static void handleActivate(WindowRef window, Boolean activate);
static void handleContentClick(WindowRef window, Point mouseLocation, UInt32 modifiers);
static void handleKeyDown(UInt32 keyCode, WindowRef prefsWindow);
static ListHandle createIconList(WindowRef window, Rect listRect);
static void drawFrameAndFocus(ListHandle list, Boolean active, WindowRef window);
static void changePanel(WindowRef window, short newPanel);


// --------------------------------------------------------------------------------------
void OpenPrefsWindow(void)
{	
	OSStatus error;
	WindowRef window;
	ControlRef containerControl, embeddedControl;
	SInt16 pixelDepth;
	Boolean isColorDevice;
	short iconListBottom;
	Rect iconListRect;
	ListHandle iconList;
	EventTypeSpec windowEvents[] = {
									{kEventClassWindow, kEventWindowActivated},
									{kEventClassWindow, kEventWindowDeactivated},
									{kEventClassWindow, kEventWindowHandleContentClick},
									{kEventClassWindow, kEventWindowGetMinimumSize},
									{kEventClassWindow, kEventWindowResizeCompleted},
									{kEventClassWindow, kEventWindowClose},
									{kEventClassWindow, kEventWindowDrawContent},
									{kEventClassWindow, kEventWindowContextualMenuSelect},
									{kEventClassControl, kEventControlClick},
									{kEventClassMouse, kEventMouseWheelMoved},
									{kEventClassTextInput, kEventTextInputUnicodeForKeyEvent}
	                               };
	
	error = CreateWindowFromResource(rPrefsWindow, &window);
	if (error != noErr)
		ExitToShell();
	RepositionWindow(window, NULL, kWindowCascadeOnMainScreen);		// CreateWindowFromResource
											// doesn't call this for you like GetNewCWindow does
		// we would just set the standard handler attribute in the Platinum 'wind' resource 
		// but that makes it not work under CarbonLib for some reason
	ChangeWindowAttributes(window, kWindowStandardHandlerAttribute, kWindowNoAttributes);
	
	SetPortWindowPort(window);
	
	SetThemeWindowBackground(window, kThemeBrushModelessDialogBackgroundActive, true);
	
	CreateRootControl(window, &containerControl);
	
		/* Get each user pane and embed each preference panel's controls (for the 
		   demonstration there is only some static text identifying each panel number).  
		   We could just as easily have used AutoEmbedControl but why make the system 
		   figure out which control to use as the embedder when we already know? */
	containerControl = GetNewControl(cWindowUserPaneVisible, window);
	embeddedControl = GetNewControl(cStaticText, window);
	EmbedControl(embeddedControl, containerControl);
	SetControlData(embeddedControl, kControlEntireControl, kControlStaticTextTextTag, 7, 
					"Panel 1");
	
	containerControl = GetNewControl(cWindowUserPaneInvisible, window);
	embeddedControl = GetNewControl(cStaticText, window);
	EmbedControl(embeddedControl, containerControl);
	SetControlData(embeddedControl, kControlEntireControl, kControlStaticTextTextTag, 7, 
					"Panel 2");
	
	containerControl = GetNewControl(cWindowUserPaneInvisible, window);
	embeddedControl = GetNewControl(cStaticText, window);
	EmbedControl(embeddedControl, containerControl);
	SetControlData(embeddedControl, kControlEntireControl, kControlStaticTextTextTag, 7, 
					"Panel 3");
	
	containerControl = GetNewControl(cWindowUserPaneInvisible, window);
	embeddedControl = GetNewControl(cStaticText, window);
	EmbedControl(embeddedControl, containerControl);
	SetControlData(embeddedControl, kControlEntireControl, kControlStaticTextTextTag, 7, 
					"Panel 4");
	
	containerControl = GetNewControl(cWindowUserPaneInvisible, window);
	embeddedControl = GetNewControl(cStaticText, window);
	EmbedControl(embeddedControl, containerControl);
	SetControlData(embeddedControl, kControlEntireControl, kControlStaticTextTextTag, 7, 
					"Panel 5");
	
	containerControl = GetNewControl(cWindowUserPaneInvisible, window);
	embeddedControl = GetNewControl(cStaticText, window);
	EmbedControl(embeddedControl, containerControl);
	SetControlData(embeddedControl, kControlEntireControl, kControlStaticTextTextTag, 7, 
					"Panel 6");
	
	containerControl = GetNewControl(cWindowUserPaneInvisible, window);
	embeddedControl = GetNewControl(cStaticText, window);
	EmbedControl(embeddedControl, containerControl);
	SetControlData(embeddedControl, kControlEntireControl, kControlStaticTextTextTag, 7, 
					"Panel 7");
	
	containerControl = GetNewControl(cWindowUserPaneInvisible, window);
	embeddedControl = GetNewControl(cStaticText, window);
	EmbedControl(embeddedControl, containerControl);
	SetControlData(embeddedControl, kControlEntireControl, kControlStaticTextTextTag, 7, 
					"Panel 8");
	
	containerControl = GetNewControl(cWindowUserPaneInvisible, window);
	embeddedControl = GetNewControl(cStaticText, window);
	EmbedControl(embeddedControl, containerControl);
	SetControlData(embeddedControl, kControlEntireControl, kControlStaticTextTextTag, 7, 
					"Panel 9");
	
	containerControl = GetNewControl(cWindowUserPaneInvisible, window);
	embeddedControl = GetNewControl(cStaticText, window);
	EmbedControl(embeddedControl, containerControl);
	SetControlData(embeddedControl, kControlEntireControl, kControlStaticTextTextTag, 8, 
					"Panel 10");
	
	gPanelNumber = 1;
	
	GetWindowDeviceDepthAndColor(window, &pixelDepth, &isColorDevice);	// draw the list with a 
	SetThemeBackground(kThemeBrushWhite, pixelDepth, isColorDevice);	// white background
																// get the bottom-most 
	iconListBottom = kPrefsWindowHeight - kWindowEdgeSpacing;	// coordinate we can use
	iconListBottom -= (kPrefsWindowHeight - kWindowEdgeSpacing - kWindowEdgeSpacing) % 
						kCellHeight;	// then subtract out the partial cell height that would 
									// be drawn on the bottom so that it's not actually drawn
	SetRect(&iconListRect, kWindowEdgeSpacing, kWindowEdgeSpacing, 
			kWindowEdgeSpacing + kListWidth, iconListBottom);
	iconList = createIconList(window, iconListRect);
	SetWindowProperty(window, kAppSignature, kIconListTag, sizeof(ListHandle), &iconList);
	
	gWindowEventHandler = NewEventHandlerUPP(windowEventHandler);
	InstallWindowEventHandler(window, gWindowEventHandler, GetEventTypeCount(windowEvents), 
									windowEvents, NULL, NULL);
	InstallPrefsWindowHelpTags(window);
	
	DisableMenuItem(GetMenuRef(mDemonstration), iPrefsWindow);
	EnableMenuItem(GetMenuRef(mFile), iClose);
	
	ShowWindow(window);
} // OpenPrefsWindow

// --------------------------------------------------------------------------------------
static void closePrefsWindow(WindowRef prefsWindow)
{
	ListHandle iconList;
	
	HideWindow(prefsWindow);
	DisableMenuItem(GetMenuRef(mFile), iClose);
	
	GetWindowProperty(prefsWindow, kAppSignature, kIconListTag, sizeof(ListHandle), NULL, 
						&iconList);
	ReleaseIconListIcons(iconList);
	LDispose(iconList);
	DisposeListDefUPP(gIconListDef);
	DisposeEventHandlerUPP(gWindowEventHandler);
	DisposePrefsWindowHelpTags();
	
	EnableMenuItem(GetMenuRef(mDemonstration), iPrefsWindow);
}

// --------------------------------------------------------------------------------------
static void adjustControls(WindowRef prefsWindow)
{
	Rect contentRect, listViewRect;
	ControlRef rootControl, userPane;
	SInt16 userPaneWidth, userPaneHeight;
	UInt16 panelIndex;
	ListHandle iconList;
	short oldListHeight, newListHeight;
	
	GetWindowBounds(prefsWindow, kWindowContentRgn, &contentRect);
	
	userPaneWidth = (contentRect.right - contentRect.left) - 
					(kWindowEdgeSpacing + kListWidth + kScrollBarWidth + kControlSpacing) - 
					(kSizeBoxWidth + kMinimumSpacing);
	userPaneHeight = (contentRect.bottom - contentRect.top) - kWindowEdgeSpacing - 
						(kSizeBoxWidth + kMinimumSpacing);
	
	GetRootControl(prefsWindow, &rootControl);
	for (panelIndex = 1; panelIndex <= kNumberOfRows; panelIndex++)
	{
		GetIndexedSubControl(rootControl, panelIndex, &userPane);
		SizeControl(userPane, userPaneWidth, userPaneHeight);
	}
	
	GetWindowProperty(prefsWindow, kAppSignature, kIconListTag, sizeof(ListHandle), NULL, 
						&iconList);
	
	GetListViewBounds(iconList, &listViewRect);
	oldListHeight = listViewRect.bottom - listViewRect.top;
	
	newListHeight = (contentRect.bottom - contentRect.top) - kWindowEdgeSpacing - 
					kWindowEdgeSpacing;
	newListHeight -= newListHeight % kCellHeight;	// make the list height a multiple 
	if (newListHeight > kMaxListHeight)				// of the cell height and don't make 
		newListHeight = kMaxListHeight;				// it bigger than we have cells for
	
	if (newListHeight != oldListHeight)
	{
		Rect invalRect;
		
		listViewRect.right += kScrollBarWidth;	// we will need to redraw the scroll bar as well
			// we need to invalidate the area where a cell will be drawn or erased
		if (newListHeight > oldListHeight)
			SetRect(&invalRect, listViewRect.left - 5, listViewRect.bottom - 5, 
					listViewRect.right + 5, contentRect.bottom);	// the extra 5 pixels are 
									// to cause the bottom of the list box frame to get erased
		else
			SetRect(&invalRect, listViewRect.left - 5, kWindowEdgeSpacing + newListHeight - 5, 
					listViewRect.right + 5, contentRect.bottom);
		
			// the drawing section is as far down as possible to avoid any screen flickering
		DrawThemeFocusRect(&listViewRect, false);			// erase the focus rectangle
		LSize(kListWidth, newListHeight, iconList);			// resize the List
		drawFrameAndFocus(iconList, true, prefsWindow);		// draw the focus rectangle back
		InvalWindowRect(prefsWindow, &invalRect);
	}
} // adjustControls

// --------------------------------------------------------------------------------------
void RedrawPrefsWindowList(WindowRef prefsWindow)
{
	ListHandle iconList;
	ListBounds visibleCells;
	SInt16 pixelDepth;
	Boolean isColorDevice;
	short row;
	Cell theCell;
	
	GetWindowProperty(prefsWindow, kAppSignature, kIconListTag, sizeof(ListHandle), NULL, 
						&iconList);
	GetListVisibleCells(iconList, &visibleCells);
	
	GetWindowDeviceDepthAndColor(prefsWindow, &pixelDepth, &isColorDevice);
	SetThemeBackground(kThemeBrushWhite, pixelDepth, isColorDevice);
	
	for (row = visibleCells.top; row < visibleCells.bottom; row++)	// redraw just the 
	{																// visible cells
		SetPt(&theCell, 0, row);
		LDraw(theCell, iconList);
	}
}

#pragma mark -
// --------------------------------------------------------------------------------------
static pascal OSStatus windowEventHandler(EventHandlerCallRef nextHandler, EventRef event, 
											void *junk)
{
#pragma unused (nextHandler, junk)

	OSStatus result = eventNotHandledErr;
	UInt32 eventClass, eventKind;
	WindowRef prefsWindow;
	Point mouseLocation, minWindowBounds;
	UInt32 modifiers;
	ControlRef listScrollBar;
	ListHandle iconList;
	Rect iconListRect;
	
	eventClass = GetEventClass(event);
	eventKind = GetEventKind(event);
	
	switch (eventClass)
	{
		case kEventClassWindow:
			GetEventParameter(event, kEventParamDirectObject, typeWindowRef, NULL, 
								sizeof(WindowRef), NULL, &prefsWindow);
			
			switch (eventKind)
			{
				case kEventWindowActivated:
					handleActivate(prefsWindow, true);
					break;		// propogate this event to the standard handler
				
				case kEventWindowDeactivated:
					handleActivate(prefsWindow, false);
					break;		// propogate this event to the standard handler
				
				case kEventWindowHandleContentClick:
					GetEventParameter(event, kEventParamMouseLocation, typeQDPoint, NULL, 
										sizeof(Point), NULL, &mouseLocation);
					GetEventParameter(event, kEventParamKeyModifiers, typeUInt32, NULL, 
										sizeof(UInt32), NULL, &modifiers);
					
					handleContentClick(prefsWindow, mouseLocation, modifiers);
					result = noErr;
					break;
				
				case kEventWindowGetMinimumSize:
					SetPt(&minWindowBounds, kPrefsWindowWidth, kPrefsWindowHeight);
					SetEventParameter(event, kEventParamDimensions, typeQDPoint, sizeof(Point), 
										&minWindowBounds);
					result = noErr;
					break;
				
				case kEventWindowResizeCompleted:
					adjustControls(prefsWindow);
					result = noErr;
					break;
				
				case kEventWindowClose:
					closePrefsWindow(prefsWindow);
					break;		// propogate this event to the standard handler
				
				case kEventWindowDrawContent: 
					handleDrawContent(prefsWindow);
					break;		// propogate this event to the standard handler
				
				case kEventWindowContextualMenuSelect:
					result = noErr;		// eat contextual menu clicks
					break;
			}
			break;
		
		case kEventClassControl:	// we need to respond to clicks in the list's scroll bar
			switch (eventKind)				// kEventControlClick instead of kEventControlHit 
			{								// because the control click must be tracked 
				case kEventControlClick:	// with LClick instead of the default handler
					GetEventParameter(event, kEventParamDirectObject, typeControlRef, 
										NULL, sizeof(ControlRef), NULL, &listScrollBar);
					
					prefsWindow = GetControlOwner(listScrollBar);
					GetWindowProperty(prefsWindow, kAppSignature, kIconListTag, 
										sizeof(ListHandle), NULL, &iconList);
					if (listScrollBar == GetListVerticalScrollBar(iconList))
					{
						GetEventParameter(event, kEventParamMouseLocation, typeQDPoint, NULL, 
											sizeof(Point), NULL, &mouseLocation);
						GetEventParameter(event, kEventParamKeyModifiers, typeUInt32, NULL, 
											sizeof(UInt32), NULL, &modifiers);
						
						handleContentClick(prefsWindow, mouseLocation, modifiers);
						result = noErr;
					}
					break;
			}
			break;
		
		case kEventClassMouse:
			switch (eventKind)
			{
				case kEventMouseWheelMoved:
					GetEventParameter(event, kEventParamMouseLocation, typeQDPoint, NULL, 
										sizeof(Point), NULL, &mouseLocation);
					GlobalToLocal(&mouseLocation);
					
					GetEventParameter(event, kEventParamWindowRef, typeWindowRef, NULL, 
										sizeof(WindowRef), NULL, &prefsWindow);
					GetWindowProperty(prefsWindow, kAppSignature, kIconListTag, 
										sizeof(ListHandle), NULL, &iconList);
					GetListViewBounds(iconList, &iconListRect);
					iconListRect.right += kScrollBarWidth;
					
					if (PtInRect(mouseLocation, &iconListRect))
					{
						EventMouseWheelAxis axis;
						long mouseWheelDelta;
						SInt16 pixelDepth;
						Boolean isColorDevice;
						
						GetEventParameter(event, kEventParamMouseWheelAxis, typeMouseWheelAxis, 
											NULL, sizeof(EventMouseWheelAxis), NULL, &axis);
						GetEventParameter(event, kEventParamMouseWheelDelta, typeLongInteger, 
											NULL, sizeof(long), NULL, &mouseWheelDelta);
							
						GetWindowDeviceDepthAndColor(prefsWindow, &pixelDepth, &isColorDevice);
						SetThemeBackground(kThemeBrushWhite, pixelDepth, isColorDevice);
										// LScroll draws the newly visible cells immediately 
						if (axis == kEventMouseWheelAxisX)			// (no update event)
							LScroll(-mouseWheelDelta, 0, iconList);
						else	// axis == kEventMouseWheelAxisY
							LScroll(0, -mouseWheelDelta, iconList);
						
						result = noErr;
					}
					break;
			}
			break;
		
		case kEventClassTextInput:
			switch (eventKind)
			{
				case kEventTextInputUnicodeForKeyEvent:
					prefsWindow = ActiveNonFloatingWindow();
					
					if (prefsWindow != NULL)
					{
						EventRef keyboardEvent;
						UInt32 keyCode;
						
						GetEventParameter(event, kEventParamTextInputSendKeyboardEvent, 
											typeEventRef, NULL, sizeof(EventRef), NULL, 
											&keyboardEvent);
						GetEventParameter(keyboardEvent, kEventParamKeyCode, typeUInt32, NULL, 
											sizeof(UInt32), NULL, &keyCode);
						
						handleKeyDown(keyCode, prefsWindow);
						result = noErr;
					}
					break;
			}
			break;
	}
	
	return result;
} // windowEventHandler

// --------------------------------------------------------------------------------------
static void handleDrawContent(WindowRef prefsWindow)
{
	RgnHandle visibleRegion;
	ListHandle iconList;
	
	GetWindowProperty(prefsWindow, kAppSignature, kIconListTag, sizeof(ListHandle), NULL, 
						&iconList);
	
	visibleRegion = NewRgn();
	GetPortVisibleRegion(GetWindowPort(prefsWindow), visibleRegion);
	
	if (visibleRegion != NULL)
	{
		Boolean active = IsWindowHilited(prefsWindow);
		SInt16 pixelDepth;
		Boolean isColorDevice;
		
		if (active)
			SetThemeWindowBackground(prefsWindow, kThemeBrushModelessDialogBackgroundActive, 
										false);
		else
			SetThemeWindowBackground(prefsWindow, 
										kThemeBrushModelessDialogBackgroundInactive, false);
		
		EraseRgn(visibleRegion);
		
		GetWindowDeviceDepthAndColor(prefsWindow, &pixelDepth, &isColorDevice);
		SetThemeBackground(kThemeBrushWhite, pixelDepth, isColorDevice);
		LUpdate(visibleRegion, iconList);
		drawFrameAndFocus(iconList, active, prefsWindow);
		DisposeRgn(visibleRegion);
	}
}

// --------------------------------------------------------------------------------------
void  handleActivate(WindowRef window, Boolean activate)
{
	ListHandle iconList;
	SInt16 pixelDepth;
	Boolean isColorDevice;

	GetWindowProperty(window, kAppSignature, kIconListTag, sizeof(ListHandle), NULL, &iconList);
	SetPortWindowPort(window);
	GetWindowDeviceDepthAndColor(window, &pixelDepth, &isColorDevice);
	
	if (activate)
	{
		SetThemeTextColor(kThemeTextColorModelessDialogActive, pixelDepth, isColorDevice);
		SetThemeBackground(kThemeBrushWhite, pixelDepth, isColorDevice);
		LActivate(true, iconList);
		RedrawPrefsWindowList(window);	// redraw the list with the active appearance
		drawFrameAndFocus(iconList, true, window);
		
		EnableMenuItem(GetMenuRef(mFile), iClose);
	}
	else	// deactivate
	{
		SetThemeTextColor(kThemeTextColorModelessDialogInactive, pixelDepth, isColorDevice);
		SetThemeBackground(kThemeBrushWhite, pixelDepth, isColorDevice);
		LActivate(false, iconList);
		RedrawPrefsWindowList(window);	// redraw the  list with the inactive appearance
		drawFrameAndFocus(iconList, false, window);
		
		DisableMenuItem(GetMenuRef(mFile), iClose);
	}
}

// --------------------------------------------------------------------------------------
static void handleContentClick(WindowRef window, Point mouseLocation, UInt32 modifiers)
{
	ListHandle iconList;
	Rect iconListRect;
	Boolean isDoubleClick;
	Cell theCell;
	
	GetWindowProperty(window, kAppSignature, kIconListTag, sizeof(ListHandle), NULL, 
						&iconList);
	
	GetListViewBounds(iconList, &iconListRect);
	
	iconListRect.right += kScrollBarWidth;	
	
	SetPortWindowPort(window);
	GlobalToLocal(&mouseLocation);
	
	if (PtInRect(mouseLocation, &iconListRect))
	{
		SInt16 pixelDepth;
		Boolean isColorDevice;
		
		GetWindowDeviceDepthAndColor(window, &pixelDepth, &isColorDevice);
		SetThemeBackground(kThemeBrushWhite, pixelDepth, isColorDevice);
							// if LClick causes the list selection to change, or the scroll 
		isDoubleClick = LClick(mouseLocation, (EventModifiers)modifiers, iconList);	// bar 
		SetPt(&theCell, 0, 0);						// to change, the affected cells are 
		LGetSelect(true, &theCell, iconList);		// immediately drawn (no update event)
		
		if ((theCell.v + 1) != gPanelNumber)
			changePanel(window, theCell.v + 1);
	}
}

// --------------------------------------------------------------------------------------
static void handleKeyDown(UInt32 keyCode, WindowRef prefsWindow)
{	
		/* Why use the virtual key code instead of the character code?  When the control 
		   key is held down it often masks out bit 7 of the character code, thus making 
		   it impossible to distinguish between some key presses such as page down 
		   (0x0C) and control-L (0x4C & ~bit7 = 0x0C).  The virtual key codes, on the 
		   other hand, are unaffected by modifier keys. */
	if ( (keyCode == kUpArrowKeyCode) || (keyCode == kDownArrowKeyCode) )
	{
		ListHandle iconList;
		ListBounds bounds;
		short lastRow;
		Cell selectedCell;
		
		GetWindowProperty(prefsWindow, kAppSignature, kIconListTag, sizeof(ListHandle), NULL, 
							&iconList);
		GetListDataBounds(iconList, &bounds);
		lastRow = bounds.bottom - bounds.top - 1;
		
		SetPt(&selectedCell, 0, 0);
		LGetSelect(true, &selectedCell, iconList);
		
		if ( (keyCode == kUpArrowKeyCode) && (selectedCell.v > 0) )
		{
			SInt16 pixelDepth;
			Boolean isColorDevice;
			
			GetWindowDeviceDepthAndColor(prefsWindow, &pixelDepth, &isColorDevice);
			SetThemeBackground(kThemeBrushWhite, pixelDepth, isColorDevice);
			
			LSetSelect(false, selectedCell, iconList);	// LSetSelect causes the indicated 
			selectedCell.v--;	// cell to be highlighted immediately (no update event)
			LSetSelect(true, selectedCell, iconList);
			
			LAutoScroll(iconList);	// scroll the list in case the selected cell isn't in view
			changePanel(prefsWindow, selectedCell.v + 1);
		}
		else if ( (keyCode == kDownArrowKeyCode) && (selectedCell.v < lastRow) )
		{
			SInt16 pixelDepth;
			Boolean isColorDevice;
			
			GetWindowDeviceDepthAndColor(prefsWindow, &pixelDepth, &isColorDevice);
			SetThemeBackground(kThemeBrushWhite, pixelDepth, isColorDevice);
			
			LSetSelect(false, selectedCell, iconList);
			selectedCell.v++;
			LSetSelect(true, selectedCell, iconList);
			
			LAutoScroll(iconList);	// scroll the list in case the selected cell isn't in view
			changePanel(prefsWindow, selectedCell.v + 1);
		}
	}
	else if ( (keyCode == kPageUpKeyCode) || (keyCode == kPageDownKeyCode) )
	{
		ListHandle iconList;
		ListBounds visibleCells;
		SInt16 pixelDepth;
		Boolean isColorDevice;
		
		GetWindowProperty(prefsWindow, kAppSignature, kIconListTag, sizeof(ListHandle), NULL, 
							&iconList);
		GetListVisibleCells(iconList, &visibleCells);
		
		GetWindowDeviceDepthAndColor(prefsWindow, &pixelDepth, &isColorDevice);
		SetThemeBackground(kThemeBrushWhite, pixelDepth, isColorDevice);
							// LScroll causes the affected cells to be drawn immediately 
		if (keyCode == kPageUpKeyCode)								// (no update event)
			LScroll(0, -(visibleCells.bottom - 1 - visibleCells.top), iconList);
		else	// keyCode == kPageDownKeyCode
			LScroll(0, (visibleCells.bottom - 1 - visibleCells.top), iconList);
	}
} // handleKeyDown

#pragma mark -
// --------------------------------------------------------------------------------------
static ListHandle createIconList(WindowRef window, Rect listRect)
{
	ListBounds dataBounds;
	Point cellSize;
	ListHandle iconList;
	Cell theCell;
	ListDefSpec listSpec;
	OSStatus error;
	
	SetRect(&dataBounds, 0, 0, 1, 0);		// initially there are no rows
	SetPt(&cellSize, kListWidth, kCellHeight);
	
		// we could use RegisterListDefinition and LNew in Carbon instead 
		// but we already show how to do that in PrefsDialog.c
	gIconListDef = NewListDefUPP(IconListDef);
	
	listSpec.defType = kListDefUserProcType;
	listSpec.u.userProc = gIconListDef;
	
	error = CreateCustomList(&listRect, &dataBounds, cellSize, &listSpec, window, 
								true, false, false, true, &iconList);
	
	if ( (error != noErr) && (iconList != NULL) )
	{
		LDispose(iconList);
		DisposeListDefUPP(gIconListDef);
		iconList = NULL;
	}
	
	if (iconList != NULL)
	{
		SetListSelectionFlags(iconList, lOnlyOne);
		
		AddRowsAndDataToIconList(iconList, rIconListIconBaseID);
		
		SetPt(&theCell, 0, 0);
		LSetSelect(true, theCell, iconList);	// select the first Cell
		drawFrameAndFocus(iconList, true, window);
	}
	
	return iconList;
}

// --------------------------------------------------------------------------------------
static void drawFrameAndFocus(ListHandle list, Boolean active, WindowRef window)
{
	Rect borderRect;
	
	GetListViewBounds(list, &borderRect);
	borderRect.right += kScrollBarWidth;
	
	if (active)
	{
		SetThemeWindowBackground(window, kThemeBrushModelessDialogBackgroundActive, false);
		DrawThemeListBoxFrame(&borderRect, kThemeStateActive);
		DrawThemeFocusRect(&borderRect, true);
	}
	else
	{
		SetThemeWindowBackground(window, kThemeBrushModelessDialogBackgroundInactive, false);
		DrawThemeFocusRect(&borderRect, false);
		DrawThemeListBoxFrame(&borderRect, kThemeStateInactive);
	}
}

// --------------------------------------------------------------------------------------
static void changePanel(WindowRef window, short newPanel)
{
	ControlRef rootControl, userPane;
	
	GetRootControl(window, &rootControl);
	
	GetIndexedSubControl(rootControl, gPanelNumber, &userPane);
	SetControlVisibility(userPane, false, false);	// hide the currently active panel
	
	GetIndexedSubControl(rootControl, newPanel, &userPane);
	SetControlVisibility(userPane, true, true);		// and show the newly selected panel
	gPanelNumber = newPanel;
}