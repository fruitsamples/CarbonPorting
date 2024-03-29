/*

File: Help.c

Abstract: Sets help tags for the preferences window and preferences dialog.

Version: 6.0

Change History:
	
	<6.0>	this file is now only necessary in the PEF version 
			(because the Mach-O version now uses nibs which have the 
			help tags included)
	<5.0>	renamed InstallPrefsWindowHelpTags to 
			SetPrefsWindowHelpTags which just sets help tags on the 
			window's controls
			removed DisposePrefsWindowHelpTags and 
			displayPrefsWindowHelpTags
	<4.0>	removed all Classic code
			removed CalculateBalloonHelpRects and DisplayHelpBalloons
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
#include "PrefsWindow.h"


#if !TARGET_API_MAC_OSX
// --------------------------------------------------------------------------------------
void SetPrefsWindowHelpTags(WindowRef prefsWindow)
{
	CFBundleRef mainBundle;
	CFStringRef dataBrowserString, userPaneString, staticTextString;
	HMHelpContentRec dataBrowserContent, userPaneContent, staticTextContent;
	ControlID dataBrowserID = {kAppSignature, kIconDataBrowserID};
	ControlRef dataBrowser, rootControl, userPane, staticText;
	UInt16 panelIndex;
	
	mainBundle = CFBundleGetMainBundle();
	
		// set the help tag for the data browser
	dataBrowserString = CFCopyLocalizedStringFromTableInBundle(CFSTR("Window Data Browser"), 
																NULL, mainBundle, NULL);
	
	dataBrowserContent.version = kMacHelpVersion;
	SetRect(&dataBrowserContent.absHotRect, 0, 0, 0, 0);
	dataBrowserContent.tagSide = kHMDefaultSide;
	dataBrowserContent.content[kHMMinimumContentIndex].contentType = kHMCFStringContent;
	dataBrowserContent.content[kHMMinimumContentIndex].u.tagCFString = dataBrowserString;
	dataBrowserContent.content[kHMMaximumContentIndex].contentType = kHMNoContent;
	
	GetControlByID(prefsWindow, &dataBrowserID, &dataBrowser);
	HMSetControlHelpContent(dataBrowser, &dataBrowserContent);
	CFRelease(dataBrowserString);
	
		// set the help tags for the user panes
	userPaneString = CFCopyLocalizedStringFromTableInBundle(CFSTR("User Pane"), NULL, 
															mainBundle, NULL);
	
	userPaneContent.version = kMacHelpVersion;
	SetRect(&userPaneContent.absHotRect, 0, 0, 0, 0);
	userPaneContent.tagSide = kHMDefaultSide;
	userPaneContent.content[kHMMinimumContentIndex].contentType = kHMCFStringContent;
	userPaneContent.content[kHMMinimumContentIndex].u.tagCFString = userPaneString;
	userPaneContent.content[kHMMaximumContentIndex].contentType = kHMNoContent;
	
		// set the help tags for the static texts
	staticTextString = CFCopyLocalizedStringFromTableInBundle(CFSTR("Static Text"), NULL, 
																mainBundle, NULL);
	
	staticTextContent.version = kMacHelpVersion;
	SetRect(&staticTextContent.absHotRect, 0, 0, 0, 0);
	staticTextContent.tagSide = kHMDefaultSide;
	staticTextContent.content[kHMMinimumContentIndex].contentType = kHMCFStringContent;
	staticTextContent.content[kHMMinimumContentIndex].u.tagCFString = staticTextString;
	staticTextContent.content[kHMMaximumContentIndex].contentType = kHMNoContent;
	
	GetRootControl(prefsWindow, &rootControl);
	for (panelIndex = 1; panelIndex <=  kNumberOfRows; panelIndex++)
	{
		GetIndexedSubControl(rootControl, panelIndex, &userPane);
		HMSetControlHelpContent(userPane, &userPaneContent);
		
		GetIndexedSubControl(userPane, 1, &staticText);
		HMSetControlHelpContent(staticText, &staticTextContent);
	}
	CFRelease(userPaneString);
	CFRelease(staticTextString);
}

// --------------------------------------------------------------------------------------
void  SetPrefsDialogHelpTags(DialogRef prefsDialog)
{
	HMHelpContentRec helpContent;
	CFBundleRef mainBundle;
	CFStringRef helpString;
	ControlRef control;
	SInt16 itemIndex;
	
	helpContent.version = kMacHelpVersion;
	SetRect(&helpContent.absHotRect, 0, 0, 0, 0);	// use the current location of the controls
	helpContent.tagSide = kHMDefaultSide;
	
	mainBundle = CFBundleGetMainBundle();
	
		// set the help tag for the data browser
	helpString = CFCopyLocalizedStringFromTableInBundle(CFSTR("Dialog Data Browser"), NULL, 
														mainBundle, NULL);
	
	helpContent.content[kHMMinimumContentIndex].contentType = kHMCFStringContent;
	helpContent.content[kHMMinimumContentIndex].u.tagCFString = helpString;
	helpContent.content[kHMMaximumContentIndex].contentType = kHMNoContent;
	
	GetDialogItemAsControl(prefsDialog, iIconDataBrowser, &control);
	HMSetControlHelpContent(control, &helpContent);		// this retains the help string
	CFRelease(helpString);								// so we can safely release it
	
		// set the help tags for the user panes
	helpString = CFCopyLocalizedStringFromTableInBundle(CFSTR("User Pane"), NULL, mainBundle, 
														NULL);
	
	helpContent.content[kHMMinimumContentIndex].contentType = kHMCFStringContent;
	helpContent.content[kHMMinimumContentIndex].u.tagCFString = helpString;
	helpContent.content[kHMMaximumContentIndex].contentType = kHMNoContent;
	
	for (itemIndex = iUserPane1; itemIndex <= iUserPane10; itemIndex += 2)
	{
		GetDialogItemAsControl(prefsDialog, itemIndex, &control);
		HMSetControlHelpContent(control, &helpContent);
	}
	CFRelease(helpString);
	
		// set the help tags for the static texts
	helpString = CFCopyLocalizedStringFromTableInBundle(CFSTR("Static Text"), NULL, mainBundle, 
														NULL);
	
	helpContent.content[kHMMinimumContentIndex].contentType = kHMCFStringContent;
	helpContent.content[kHMMinimumContentIndex].u.tagCFString = helpString;
	helpContent.content[kHMMaximumContentIndex].contentType = kHMNoContent;
	
	for (itemIndex = iUserPane1 + 1; itemIndex <= iUserPane10 + 1; itemIndex += 2)
	{
		GetDialogItemAsControl(prefsDialog, itemIndex, &control);
		HMSetControlHelpContent(control, &helpContent);
	}
	CFRelease(helpString);
}
#endif