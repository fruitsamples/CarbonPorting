/*

File: PrefsDialog.h

Abstract: Defines the public constants and functions used for the Preferences 
          dialog

Version: 2.0

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

#ifndef REZ

#if TARGET_API_MAC_CARBON
#include <Carbon.h>
#else
#include <Dialogs.h>
#include <Events.h>
#endif

#else

#if TARGET_API_MAC_CARBON
#include "Carbon.r"
#else
#include "ConditionalMacros.r"
#endif

#endif // ifndef REZ

enum
{
	rPrefsDialogPlatinum = 128,
	rPrefsDialogAqua     = 129,
	iIconList            = 3,
	iUserPane1           = 4,
	iUserPane2           = 6,
	iUserPane3           = 8,
	iUserPane4           = 10,
	iUserPane5           = 12,
	iUserPane6           = 14,
	iUserPane7           = 16,
	iUserPane8           = 18,
	iUserPane9           = 20,
	iUserPane10          = 22
};
enum
{
	kPrefsDialogPlatinumHeight = 500,
	kPrefsDialogPlatinumWidth  = 275,
	kPrefsDialogAquaHeight     = 516,
	kPrefsDialogAquaWidth      = 291
};
enum					// refer to PrefsWindow.h to figure out the appropriate starting ID
{
	cPlatinumIconList                = 133,
	cAquaIconList                    = 134,
	cPlatinumDialogUserPaneVisible   = 135,
	cAquaDialogUserPaneVisible       = 136,
	cPlatinumDialogUserPaneInvisible = 137,
	cAquaDialogUserPaneInvisible     = 138
};

#define kListBoxPlatinumBounds kPlatinumWindowEdgeSpacing, kPlatinumWindowEdgeSpacing, \
	kPrefsDialogPlatinumHeight - kPlatinumWindowEdgeSpacing - \
	(((kPrefsDialogPlatinumHeight - kPlatinumWindowEdgeSpacing) - \
	kPlatinumWindowEdgeSpacing) % kCellHeight), \
	kPlatinumWindowEdgeSpacing + kListWidth + kScrollBarWidth
#define kListBoxAquaBounds kAquaWindowEdgeSpacing, kAquaWindowEdgeSpacing, \
	kPrefsDialogAquaHeight - kAquaWindowEdgeSpacing - \
	(((kPrefsDialogAquaHeight - kAquaWindowEdgeSpacing) - kAquaWindowEdgeSpacing) % \
	kCellHeight), \
	kAquaWindowEdgeSpacing + kListWidth + kScrollBarWidth
#define kUserPanePlatinumBounds kPlatinumWindowEdgeSpacing, \
	kPlatinumWindowEdgeSpacing + kListWidth + kScrollBarWidth + kControlSpacing, \
	kPrefsDialogPlatinumHeight - kPlatinumWindowEdgeSpacing - kPushButtonHeight - \
	kControlSpacing, \
	kPrefsDialogPlatinumWidth - kPlatinumWindowEdgeSpacing
#define kUserPaneAquaBounds kAquaWindowEdgeSpacing, \
	kAquaWindowEdgeSpacing + kListWidth + kScrollBarWidth + kControlSpacing, \
	kPrefsDialogAquaHeight - kAquaWindowEdgeSpacing - kPushButtonHeight - kControlSpacing, \
	kPrefsDialogAquaWidth - kAquaWindowEdgeSpacing
#define kStaticTextBounds 224, 153, 240, 217	// these coordinates are arbitrary

#ifndef REZ

void OpenPrefsDialog(void);
void ClosePrefsDialog(DialogRef prefsDialog);
void RedrawPrefsDialogList(DialogRef prefsDialog);
void HandleDialogEvents(EventRecord *event);

#endif // ifndef REZ