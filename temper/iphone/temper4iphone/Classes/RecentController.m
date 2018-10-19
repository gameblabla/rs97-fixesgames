//
//  BookmarksController.m
//  ShoutOut
//
//  Created by ME on 9/13/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "SOApplication.h"


@implementation RecentController

- (void)dealloc {
	[ super dealloc ];
}

- (void)viewDidLoad {
	[super viewDidLoad];
	self.navigationItem.hidesBackButton = YES;
}

-(void)awakeFromNib
{
	self.navigationItem.hidesBackButton = YES;
	
	// always put any sort of initializations in here. They will only be called once.
	adNotReceived = 0;

	[self getRecent];
}

- (void)viewWillAppear:(BOOL)animated {
	[super viewWillAppear:animated];
	self.navigationItem.hidesBackButton = YES;
}

- (void)viewDidAppear:(BOOL)animated {
	[super viewDidAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated {
}

- (void)viewDidDisappear:(BOOL)animated {
}

- (void)didReceiveMemoryWarning {
	[super didReceiveMemoryWarning];
}

- (NSString*)getDocumentsDirectory
{
	/*
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString *documentsDirectory = [paths objectAtIndex: 0];
	return documentsDirectory;
	*/
	return @"/Applications/temper4iphone.app";
}

- (void)addRecent:(NSString*)thisPath withFile:(NSString*)thisFile withDir:(NSString*)thisDir {
	if(thisPath && thisFile && thisDir)
	{
		[recentArray addObject:[NSDictionary dictionaryWithObjectsAndKeys:
								thisPath, @"path",
								thisFile, @"file",
								thisDir, @"directory",								   
								nil]];
		if([recentArray count] > 7)
		{
			[recentArray removeObjectAtIndex:1];
		}
		[tableview reloadData];
		
	}
	else
	{
		return;
	}
	
	NSString *path=[[self getDocumentsDirectory] stringByAppendingPathComponent:@"recent.bin"];
	NSData *plistData;
	
	NSString *error;
	
	
	
	plistData = [NSPropertyListSerialization dataFromPropertyList:recentArray
				 
												format:NSPropertyListBinaryFormat_v1_0
				 
												errorDescription:&error];
	
	if(plistData)
		
	{
		
		NSLog(@"No error creating plist data.");
		
		[plistData writeToFile:path atomically:NO];
		
	}
	else
	{
		
		NSLog(error);
		
		[error release];
		
	}
}

- (void)getRecent {
	NSString *path=[[self getDocumentsDirectory] stringByAppendingPathComponent:@"recent.bin"];
	
	NSData *plistData;
	id plist;
	NSString *error;
	
	NSPropertyListFormat format;
	
	plistData = [NSData dataWithContentsOfFile:path];
	
	
	
	plist = [NSPropertyListSerialization propertyListFromData:plistData
			 
											 mutabilityOption:NSPropertyListImmutable
			 
													   format:&format
			 
											 errorDescription:&error];
	
	if(!plist)
	{
		
		NSLog(error);
		
		[error release];
		
		recentArray = [[NSMutableArray alloc] init];
	}
	else
	{
		recentArray = [[NSMutableArray alloc] initWithArray:plist];
	}
}

// *** Tablesource stuffs.
- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
	// Number of sections is the number of region dictionaries
	
	if(recentArray==nil) {
		return 1;
	} else {
		return 2;
	}
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
	if(recentArray==nil) {
		return 1;
	}
	
	if(section < 1)
	{
		return 1;
	}

	return [recentArray count];
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath {
	if(indexPath.section < 1) {
		return 48.0; // this is the height of the AdMob ad
	}
	
	return 44.0; // this is the generic cell height
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
	UITableViewCell *cell;
	
	if(indexPath.section < 1) 
	{
		cell = [tableview dequeueReusableCellWithIdentifier:@"adCell"];
		if(adNotReceived)
		{
			if(cell != nil)
			{
				[cell release];
				cell = nil;
			}
			
			adNotReceived = 0;
		}
		if(cell == nil)
		{
			cell = [[UITableViewCell alloc] initWithFrame:CGRectZero reuseIdentifier:@"adCell"];
			// Request an AdMob ad for this table view cell
			adMobView = [AdMobView requestAdWithDelegate:self];
			[cell.contentView addSubview:adMobView];
		}
		else
		{
			[adMobView requestFreshAd];
		}
		
		cell.text = @"";
	}
	else
	{
		cell = [tableview dequeueReusableCellWithIdentifier:@"cell"];
		if (cell == nil) 
		{
			cell = [[[UITableViewCell alloc] initWithFrame:CGRectZero reuseIdentifier:@"cell"] autorelease];
		}
		cell.accessoryType = UITableViewCellAccessoryNone;
		if([[[recentArray objectAtIndex:indexPath.row] objectForKey:@"path"] compare:@""] != NSOrderedSame)
		{
			UILabel *label = [[UILabel alloc] initWithFrame:CGRectMake(0, 0, 320, 44)];
			label.numberOfLines = 1;
			label.adjustsFontSizeToFitWidth = YES;
			label.minimumFontSize = 9.0f;
			label.lineBreakMode = UILineBreakModeMiddleTruncation;
			label.text = [[[recentArray objectAtIndex:indexPath.row] objectForKey:@"path"] lastPathComponent];
			[cell.contentView addSubview:label];
			[label release];
		}
	}
	
	// Set up the cell
	return cell;
}

- (void)setCurrentlyPlaying:(NSString*) str
{	
	self.navigationItem.prompt = str;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	if( indexPath.section < 1 )
	{
		return;
	}
	char *cListingsPath;
	if([[[recentArray objectAtIndex:indexPath.row] objectForKey:@"path"] compare:@""] != NSOrderedSame )
	{
    cListingsPath = (char*)[[[[recentArray objectAtIndex:indexPath.row] objectForKey:@"path"] lastPathComponent] UTF8String];
		[SOApp.nowPlayingView setCurrentStation:[[recentArray objectAtIndex:indexPath.row] objectForKey:@"path"] 
		 withFile:[[recentArray objectAtIndex:indexPath.row] objectForKey:@"file"] 
		 withDir:[[recentArray objectAtIndex:indexPath.row] objectForKey:@"directory"]];
	}
	
	
	[SOApp.nowPlayingView startEmu:cListingsPath];
	[SOApp.delegate switchToNowPlaying];
	//[tabBar didMoveToWindowNowPlaying];
}

#if 0
- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath
{	
	if (editingStyle == UITableViewCellEditingStyleDelete) {
		// Delete the row from the data source
		//[tableview deleteRowAtIndexPaths:[NSArray arrayWithObject:indexPath] withRowAnimation:YES];
		[bookmarksArray removeObjectAtIndex:indexPath.row];
		[tableview reloadData];
		NSString *path=[[self getDocumentsDirectory] stringByAppendingPathComponent:@"bookmarks.bin"];
		NSData *plistData;
		
		NSString *error;
		
		
		
		plistData = [NSPropertyListSerialization dataFromPropertyList:bookmarksArray
					 
															   format:NSPropertyListBinaryFormat_v1_0
					 
													 errorDescription:&error];
		
		if(plistData)
			
		{
			
			NSLog(@"No error creating plist data.");
			
			[plistData writeToFile:path atomically:YES];
			
		}
		else
		{
			
			NSLog(error);
			
			[error release];
			
		}
		
	}	
	if (editingStyle == UITableViewCellEditingStyleInsert) {
		// Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view
	}	
}
#endif


#pragma mark -
#pragma mark AdMobDelegate methods

- (NSString *)publisherId {
	return @"a149a090beddb43"; // this should be prefilled; if not, get it from www.admob.com
}

- (UIColor *)adBackgroundColor {
	return [UIColor colorWithRed:0 green:0 blue:0 alpha:1]; // this should be prefilled; if not, provide a UIColor
}

- (UIColor *)adTextColor {
	return [UIColor colorWithRed:1 green:1 blue:1 alpha:1]; // this should be prefilled; if not, provide a UIColor
}

- (BOOL)mayAskForLocation {
	return NO; // this should be prefilled; if not, see AdMobProtocolDelegate.h for instructions
}

- (void)didReceiveAd:(AdMobView *)adView {
	NSLog(@"AdMob: Did receive ad");
}

- (void)didFailToReceiveAd:(AdMobView *)adView {
	NSLog(@"AdMob: Did fail to receive ad");
	adNotReceived = 1;
	//[self.tableView cellForRowAtIndexPath:[NSIndexPath indexPathForRow:0 inSection:0]];
	[tableview reloadData];
}

@end