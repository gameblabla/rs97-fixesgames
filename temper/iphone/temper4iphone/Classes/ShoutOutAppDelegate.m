//
//  ShoutOutAppDelegate.m
//  ShoutOut
//
//  Created by Spookysoft on 9/6/08.
//  Copyright Spookysoft 2008. All rights reserved.
//

#import "SOApplication.h"

extern unsigned long gp2x_pad_status;

@implementation ShoutOutAppDelegate

@synthesize window;
@synthesize navigationController;


- (id)init {
	if (self = [super init]) {
		// 
	}
	return self;
}


- (void)applicationDidFinishLaunching:(UIApplication *)application {	
	[[UIApplication sharedApplication] setStatusBarHidden:NO animated:NO];
	[[UIApplication sharedApplication] setIdleTimerDisabled:YES];
	application.delegate = self;
	
	CGRect frame = window.frame;
	frame.origin.y = window.frame.origin.y+window.frame.size.height-49;
	frame.size.height = 49;
	tabBar.frame = frame;
	[window addSubview:tabBar];
	CGRect navFrame = [navigationController view].frame;
	navFrame.origin.y = 20;
	navFrame.size.height = 460 - 49;
	[navigationController view].frame = navFrame;
	
	window.opaque = YES;
	window.clearsContextBeforeDrawing = YES;
	window.userInteractionEnabled = YES;
	window.multipleTouchEnabled = YES;
	window.exclusiveTouch = YES;
	window.contentMode = UIViewContentModeTopLeft;
	
	[[window layer] setMagnificationFilter:0];
	[[window layer] setMinificationFilter:0];
//	tabBar.selectedItem = 0;
	// Configure and show the window
	//[SOApp.nowPlayingView startEmu:"/var/mobile/Media/ROMs/SNES/snes-SUPERMARIOKART.zip"];
	//[window addSubview:[SOApp.nowPlayingView view]];
	//[window bringSubviewToFront:[SOApp.nowPlayingView view]];
	//[window makeKeyAndVisible];
	[window addSubview:[navigationController view]];
	[window makeKeyAndVisible];	
}

#pragma mark TabBar Actions
- (void)switchToBrowse {
	[[UIApplication sharedApplication] setStatusBarHidden:NO animated:NO];
	[tabBar setHidden:NO];
	CGRect navFrame = [navigationController view].frame;
	navFrame.origin.y = 20;
	navFrame.size.height = 460 - 49;
	[navigationController view].frame = navFrame;
	navigationController.navigationBarHidden = FALSE;
	navigationController.navigationBar.hidden = FALSE;
	
	[[self navigationController] popToRootViewControllerAnimated:NO];
}
- (void)switchToSaveStates {
	[[UIApplication sharedApplication] setStatusBarHidden:NO animated:NO];
	[tabBar setHidden:NO];
	CGRect navFrame = [navigationController view].frame;
	navFrame.origin.y = 20;
	navFrame.size.height = 460 - 49;
	[navigationController view].frame = navFrame;
	navigationController.navigationBarHidden = FALSE;
	navigationController.navigationBar.hidden = FALSE;
	
	[SOApp.saveStatesView refreshData:@"/var/mobile/Media/ROMs/TEMPER/save_states/"];
	if ([[[self navigationController] viewControllers] containsObject:SOApp.saveStatesView]) {
		[[self navigationController] popToViewController:SOApp.saveStatesView animated:NO];
	} else {
		[[self navigationController] pushViewController:SOApp.saveStatesView animated:NO];
	}
}
- (void)switchToNowPlaying
{
	[[UIApplication sharedApplication] setStatusBarHidden:YES animated:NO];
	[tabBar setHidden:YES];
	CGRect navFrame = [navigationController view].frame;
	navFrame.origin.y = 0;
	navFrame.size.height = 480;
	[navigationController view].frame = navFrame;
	navigationController.navigationBarHidden = TRUE;
	navigationController.navigationBar.hidden = TRUE;

	gp2x_pad_status = 0;
	
	if ([[[self navigationController] viewControllers] containsObject:SOApp.nowPlayingView]) {
		[[self navigationController] popToViewController:SOApp.nowPlayingView animated:NO];
	} else {
		[[self navigationController] pushViewController:SOApp.nowPlayingView animated:NO];
	}
}

- (void)switchToRecent {
	[[UIApplication sharedApplication] setStatusBarHidden:NO animated:NO];
	[tabBar setHidden:NO];
	CGRect navFrame = [navigationController view].frame;
	navFrame.origin.y = 20;
	navFrame.size.height = 460 - 49;
	[navigationController view].frame = navFrame;
	navigationController.navigationBarHidden = FALSE;
	navigationController.navigationBar.hidden = FALSE;
	
	if ([[[self navigationController] viewControllers] containsObject:SOApp.recentView]) {
		[[self navigationController] popToViewController:SOApp.recentView animated:NO];
	} else {
		[[self navigationController] pushViewController:SOApp.recentView animated:NO];
	}
}
- (void)switchToOptions {
	[[UIApplication sharedApplication] setStatusBarHidden:NO animated:NO];
	[tabBar setHidden:NO];
	CGRect navFrame = [navigationController view].frame;
	navFrame.origin.y = 20;
	navFrame.size.height = 460 - 49;
	[navigationController view].frame = navFrame;
	navigationController.navigationBarHidden = FALSE;
	navigationController.navigationBar.hidden = FALSE;
	
	if ([[[self navigationController] viewControllers] containsObject:SOApp.optionsView]) {
		[[self navigationController] popToViewController:SOApp.optionsView animated:NO];
	} else {
		[[self navigationController] pushViewController:SOApp.optionsView animated:NO];
	}
}

- (void)applicationWillTerminate:(UIApplication *)application {
	// Save data if appropriate
}

- (void)dealloc {
	[ navigationController release ];
	[ window release ];
	[ super dealloc ];
}

@end
