/*

File: MissingConstants.r

Abstract: Constants that are missing from the system Rez includes

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

/*
	These constants were added from MacWindows.r 145.33~1 and are necessary to make a 
	'wind' resource.
*/
#ifndef kStoredWindowSystemTag		// these are defined in MacWindows.r 145.33~1 and later
#define kStoredWindowSystemTag 			'appl'				/*  Only Apple collection items will be of this tag */
#endif
#ifndef kStoredBasicWindowDescriptionID
#define kStoredBasicWindowDescriptionID  'sbas'				/*  BasicWindowDescription */
#endif
#ifndef kStoredWindowPascalTitleID
#define kStoredWindowPascalTitleID 		's255'				/*  pascal title string */
#endif
#ifndef kStoredWindowTitleCFStringID
#define kStoredWindowTitleCFStringID 	'cfst'				/*  CFString title string */
#endif

/*
	The following constants are modified from MacWindows.r 145.33~1 to be suitable for 
	'wind' resources.  The constants defined in MacWindows.r are hexidecimal values but 
	they need to be hex strings to be used in 'wind' resources.
*/
#define kAlertWindowClassHexStr			$"00000001"
#define kMovableAlertWindowClassHexStr 	$"00000002"
#define kModalWindowClassHexStr 		$"00000003"
#define kMovableModalWindowClassHexStr 	$"00000004"
#define kFloatingWindowClassHexStr 		$"00000005"
#define kDocumentWindowClassHexStr 		$"00000006"
#define kUtilityWindowClassHexStr 		$"00000008"
#define kHelpWindowClassHexStr 			$"0000000A"
#define kSheetWindowClassHexStr 		$"0000000B"
#define kToolbarWindowClassHexStr 		$"0000000C"
#define kPlainWindowClassHexStr 		$"0000000D"
#define kOverlayWindowClassHexStr 		$"0000000E"
#define kSheetAlertWindowClassHexStr 	$"0000000F"
#define kAltPlainWindowClassHexStr 		$"00000010"
#define kSimpleWindowClassHexStr 		$"00000012"
#define kDrawerWindowClassHexStr 		$"00000014"
#define kAllWindowClassesHexStr 		$"FFFFFFFF"

#define kWindowNoAttributesHexStr 		$"00000000"
#define kWindowCloseBoxAttributeHexStr 	$"00000001"
#define kWindowHorizontalZoomAttributeHexStr  $"00000002"
#define kWindowVerticalZoomAttributeHexStr  $"00000004"
#define kWindowFullZoomAttributeHexStr 	$"00000006"
#define kWindowCollapseBoxAttributeHexStr  $"00000008"
#define kWindowResizableAttributeHexStr  $"00000010"
#define kWindowSideTitlebarAttributeHexStr 	$"00000020"
#define kWindowToolbarButtonAttributeHexStr  $"00000040"
#define kWindowMetalAttributeHexStr 	$"00000100"
#define kWindowDoesNotCycleAttributeHexStr  $"00008000"
#define kWindowNoUpdatesAttributeHexStr  $"00010000"
#define kWindowNoActivatesAttributeHexStr  $"00020000"
#define kWindowOpaqueForEventsAttributeHexStr  $"00040000"
#define kWindowCompositingAttributeHexStr  $"00080000"
#define kWindowNoShadowAttributeHexStr 	$"00200000"
#define kWindowHideOnSuspendAttributeHexStr  $"01000000"
#define kWindowAsyncDragAttributeHexStr  $"00800000"
#define kWindowStandardHandlerAttributeHexStr  $"02000000"
#define kWindowHideOnFullScreenAttributeHexStr  $"04000000"
#define kWindowInWindowMenuAttributeHexStr  $"08000000"
#define kWindowLiveResizeAttributeHexStr  $"10000000"
#define kWindowIgnoreClicksAttributeHexStr  $"20000000"
#define kWindowNoConstrainAttributeHexStr  $"80000000"
#define kWindowStandardDocumentAttributesHexStr  $"0000001F"
#define kWindowStandardFloatingAttributesHexStr  $"00000009"

#define kWindowCenterOnMainScreenHexStr  $"00000001"
#define kWindowCenterOnParentWindowHexStr  $"00000002"
#define kWindowCenterOnParentWindowScreenHexStr  $"00000003"
#define kWindowCascadeOnMainScreenHexStr  $"00000004"
#define kWindowCascadeOnParentWindowHexStr  $"00000005"
#define kWindowCascadeOnParentWindowScreenHexStr  $"00000006"
#define kWindowCascadeStartAtParentWindowScreenHexStr  $"0000000A"
#define kWindowAlertPositionOnMainScreenHexStr  $"00000007"
#define kWindowAlertPositionOnParentWindowHexStr  $"00000008"
#define kWindowAlertPositionOnParentWindowScreenHexStr  $"00000009"

#define kWindowDefinitionVersionOneHexStr  $"00000001"
#define kWindowDefinitionVersionTwoHexStr  $"00000002"

#define kWindowIsCollapsedStateHexStr 	$"00000001"

/*
	These constants are from Balloons.h and are necessary for most help balloon resources.
*/
#define kTopLeftTipPointsLeftVariant 	0
#define kTopLeftTipPointsUpVariant 		1
#define kTopRightTipPointsUpVariant 	2
#define kTopRightTipPointsRightVariant 	3
#define kBottomRightTipPointsRightVariant  4
#define kBottomRightTipPointsDownVariant  5
#define kBottomLeftTipPointsDownVariant  6
#define kBottomLeftTipPointsLeftVariant  7
#define kBalloonVariantCount 			8