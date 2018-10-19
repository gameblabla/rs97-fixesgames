//
//  BookmarksController.h
//  ShoutOut
//
//  Created by ME on 9/13/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface SaveStatesController : UIViewController < UITableViewDataSource, UITableViewDelegate, AdMobDelegate > {
	NSMutableArray			*browseArray;
	NSMutableArray			*indexedLetters;
	NSMutableArray			*displayList;
	IBOutlet UITableView	*tableview;
	IBOutlet UIWindow		*window;
	IBOutlet UITabBar		*tabBar;
	AdMobView				*adMobView;
	NSUInteger				adNotReceived;
	NSString*				currentPath;
}

- (void)refreshData:(NSString*)path;
- (void)setupIndexedData;
- (NSMutableArray*)getDisplayList;
- (UITableView*)getTableview;

@end
