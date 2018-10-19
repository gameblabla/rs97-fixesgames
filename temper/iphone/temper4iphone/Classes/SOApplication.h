#import <UIKit/UIKit.h>
#import <CoreFoundation/CoreFoundation.h>
#import "CoreSurface/CoreSurface.h"
#import "TabBar.h"
#import "AdMobDelegateProtocol.h"
#import "AdMobView.h"
#import "RootViewController.h"
#import "ShoutOutAppDelegate.h"
//#import "BookmarksController.h"
#import "NowPlayingController.h"
#import "SaveStatesController.h"
#import "RecentController.h"
#import "OptionsController.h"

#define SOApp ((SOApplication *)[UIApplication sharedApplication])
@interface SOApplication : UIApplication 
{
				RootViewController			* tblView;
//	IBOutlet	BookmarksController			* bookmarksView;
	IBOutlet	NowPlayingController		* nowPlayingView;
	IBOutlet	SaveStatesController		* saveStatesView;
	IBOutlet	RecentController			* recentView;
	IBOutlet	OptionsController			* optionsView;
}

@property(assign)	RootViewController			* tblView;
//@property(assign)	BookmarksController			* bookmarksView;
@property(assign)	NowPlayingController		* nowPlayingView;
@property(assign)	SaveStatesController		* saveStatesView;
@property(assign)	RecentController			* recentView;
@property(assign)	OptionsController			* optionsView;

@end
