//
//  ShoutOutAppDelegate.h
//  ShoutOut
//
//  Created by Spookysoft on 9/6/08.
//  Copyright Spookysoft 2008. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface ShoutOutAppDelegate : NSObject <UIApplicationDelegate> {
	
	IBOutlet UIWindow*					window;
	IBOutlet UINavigationController*	navigationController;
	IBOutlet UITabBar*					tabBar;
}

- (void)switchToBrowse;
- (void)switchToSaveStates;
//- (void)switchToBookmarks;
- (void)switchToNowPlaying;
- (void)switchToRecent;
- (void)switchToOptions;
- (void)reportAppOpen;

@property (nonatomic, retain) UIWindow *window;
@property (nonatomic, retain) UINavigationController *navigationController;

@end

