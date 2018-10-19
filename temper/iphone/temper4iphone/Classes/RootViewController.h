//
//  RootViewController.h
//  ShoutOut
//
//  Created by Spookysoft on 9/6/08.
//  Copyright Spookysoft 2008. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "AdMobDelegateProtocol.h"

extern void *app_Thread_Start(void *args);

@interface RootViewController : UITableViewController<AdMobDelegate> {
	NSMutableArray			*browseArray;
	NSMutableArray			*indexedLetters;
	NSMutableArray			*displayList;
	IBOutlet UITableView	*tableView;
	IBOutlet UIWindow		*window;
	IBOutlet UITabBar		*tabBar;
	AdMobView				*adMobView;
	NSUInteger				adNotReceived;
	NSString*				currentPath;
}

- (void)backClicked:(id)sender;
- (void)refreshData:(NSString*)path;
- (void)setupIndexedData;
- (NSMutableArray*)getDisplayList;
- (void)setCurrentlyPlaying:(NSString*) str;
- (void)initRootData;

@end
