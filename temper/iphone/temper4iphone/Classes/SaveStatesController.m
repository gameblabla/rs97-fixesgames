//
//  BookmarksController.m
//  ShoutOut
//
//  Created by ME on 9/13/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "SOApplication.h"


@implementation SaveStatesController

- (void)awakeFromNib {
	self.navigationItem.hidesBackButton = YES;
	self.navigationItem.prompt = @"www.zodttd.com";
	
	adNotReceived = 0;
	
	browseArray = [[NSMutableArray alloc] init];
	
	currentPath = [[NSString alloc] initWithString:@"/var/mobile/Media/ROMs/TEMPER/save_states/"];
	[ self refreshData:currentPath ];
}

- (void)refreshData:(NSString*)path 
{
	[browseArray removeAllObjects];
	
	[currentPath release];
	
	if([[path substringWithRange:NSMakeRange([path length]-1,1)] compare:@"/"] == NSOrderedSame)
	{
		currentPath = [[NSString alloc] initWithFormat:@"%@",path];
	}
	else
	{
		currentPath = [[NSString alloc] initWithFormat:@"%@/",path];
	}
	
	int i;
	NSFileManager *fileManager = [NSFileManager defaultManager];
    NSInteger entries = [[fileManager directoryContentsAtPath: currentPath] count];
	
	for ( i = 0; i < entries; i++ ) 
  {
		if(([[[[fileManager directoryContentsAtPath: currentPath]  objectAtIndex: i ] substringWithRange:NSMakeRange([[[fileManager directoryContentsAtPath: currentPath]  objectAtIndex: i ] length]-6,6)] caseInsensitiveCompare:@"_0.svs"] == NSOrderedSame) || 
		  ([[[[fileManager directoryContentsAtPath: currentPath]  objectAtIndex: i ] substringWithRange:NSMakeRange([[[fileManager directoryContentsAtPath: currentPath]  objectAtIndex: i ] length]-4,4)] caseInsensitiveCompare:@".svs"] != NSOrderedSame))
		{
			continue;
		}
		//checks if directory
		BOOL isDir = FALSE;
		NSMutableString* dummyString;
		dummyString = [[NSMutableString alloc] initWithString: currentPath];
		if([[dummyString substringWithRange:NSMakeRange([dummyString length]-1,1)] compare:@"/"] == NSOrderedSame)
		{
			[dummyString appendString: [[fileManager directoryContentsAtPath: currentPath]  objectAtIndex: i ]];			
		}
		else
		{
			[dummyString stringByAppendingPathComponent: [[fileManager directoryContentsAtPath: currentPath]  objectAtIndex: i ]];
		}
		
		[fileManager fileExistsAtPath:dummyString isDirectory:&isDir];
		
		//NSLog(@"isDir %d", isDir);
		
		if(!isDir)
		{
			[browseArray addObject:[NSDictionary dictionaryWithObjectsAndKeys:
									currentPath, @"path",
                  [[fileManager directoryContentsAtPath: currentPath]  objectAtIndex: i ], @"object",
									@"0", @"directory",
									@"1", @"file",
									nil]];
		}
		
		[ dummyString release ];
    }
	
	[self setupIndexedData];
	[tableview reloadData];
	
	self.navigationItem.prompt = currentPath;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
	UITableViewCell			*cell;
	
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
		cell = [tableview dequeueReusableCellWithIdentifier:@"labelCell"];
		if (cell == nil) 
		{
			cell = [[[UITableViewCell alloc] initWithFrame:CGRectZero reuseIdentifier:@"labelCell"] autorelease];
		}
		NSDictionary *letterDictionary = [displayList objectAtIndex:indexPath.section - 1];
		NSMutableArray *zonesForLetter = [letterDictionary objectForKey:@"listings"];
		cell.accessoryType = UITableViewCellAccessoryNone;
		
		UILabel *label = [[UILabel alloc] initWithFrame:CGRectMake(0, 0, 320, 44)];
		label.numberOfLines = 1;
		label.adjustsFontSizeToFitWidth = YES;
		label.minimumFontSize = 9.0f;
		label.lineBreakMode = UILineBreakModeMiddleTruncation;
		label.text = [[zonesForLetter objectAtIndex:indexPath.row] objectForKey:@"object"];
		[cell.contentView addSubview:label];
		[label release];		
	}
	
	// Set up the cell
	return cell;
}


- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	if( indexPath.section < 1 )
	{
		return;
	}
	
	NSDictionary *letterDictionary = [displayList objectAtIndex:indexPath.section - 1];
	NSMutableArray *zonesForLetter = [letterDictionary objectForKey:@"listings"];
	
	NSMutableString *listingsPath = [[NSMutableString alloc] initWithString:[[zonesForLetter objectAtIndex:indexPath.row] objectForKey:@"path"]];
	if([[listingsPath substringWithRange:NSMakeRange([listingsPath length]-1,1)] compare:@"/"] == NSOrderedSame)
	{
		[listingsPath appendString:[[zonesForLetter objectAtIndex:indexPath.row] objectForKey:@"object"]];
	}
	else
	{
		[listingsPath stringByAppendingPathComponent:[[zonesForLetter objectAtIndex:indexPath.row] objectForKey:@"object"]];
	}
  char *cListingsPath = (char*)[[[zonesForLetter objectAtIndex:indexPath.row] objectForKey:@"object"] UTF8String];
	
	if( [[[zonesForLetter objectAtIndex:indexPath.row] objectForKey:@"directory"] compare:@"1"] == NSOrderedSame )
	{
		[self refreshData:listingsPath];
		[listingsPath release];
		return;
	}
	
	[SOApp.recentView addRecent:listingsPath withFile:[[zonesForLetter objectAtIndex:indexPath.row] objectForKey:@"file"] 
	 withDir:[[zonesForLetter objectAtIndex:indexPath.row] objectForKey:@"directory"]];
	
	[SOApp.nowPlayingView setCurrentStation:listingsPath 
	 withFile:[[zonesForLetter objectAtIndex:indexPath.row] objectForKey:@"file"] 
	 withDir:[[zonesForLetter objectAtIndex:indexPath.row] objectForKey:@"directory"]];
	
	[SOApp.nowPlayingView startEmu:cListingsPath];
	[SOApp.delegate switchToNowPlaying];
	//[tabBar didMoveToWindowNowPlaying];
	
	[listingsPath release];
}

// Override if you support editing the list
- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath { 
 if (editingStyle == UITableViewCellEditingStyleDelete) {
	 // Delete the row from the data source
	 if( indexPath.section < 1 )
	 {
		 return;
	 }
	 
	 NSDictionary *letterDictionary = [displayList objectAtIndex:indexPath.section - 1];
	 NSMutableArray *zonesForLetter = [letterDictionary objectForKey:@"listings"];
	 
	 NSMutableString *listingsPath = [[NSMutableString alloc] initWithString:[[zonesForLetter objectAtIndex:indexPath.row] objectForKey:@"path"]];
	 if([[listingsPath substringWithRange:NSMakeRange([listingsPath length]-1,1)] compare:@"/"] == NSOrderedSame)
	 {
		 [listingsPath appendString:[[zonesForLetter objectAtIndex:indexPath.row] objectForKey:@"object"]];
	 }
	 else
	 {
		 [listingsPath stringByAppendingPathComponent:[[zonesForLetter objectAtIndex:indexPath.row] objectForKey:@"object"]];
	 }
	 char *cListingsPath = (char*)[listingsPath UTF8String];

	 remove(cListingsPath);
	 
	 [listingsPath release];
	 
	 [ self refreshData:currentPath ];
 }
}


// Override if you support conditional editing of the list
/*
- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath {
 // Return NO if you do not want the specified item to be editable.
	if( indexPath.section < 1 )
	{
		return NO;
	}
	return NO;
}
*/
// Override if you support rearranging the list
/*
 - (void)tableView:(UITableView *)tableView moveRowAtIndexPath:(NSIndexPath *)fromIndexPath toIndexPath:(NSIndexPath *)toIndexPath {

}
*/

// Override if you support conditional rearranging of the list
/*
 - (BOOL)tableView:(UITableView *)tableView canMoveRowAtIndexPath:(NSIndexPath *)indexPath {
 // Return NO if you do not want the item to be re-orderable.
 return YES;
 }
 */

- (void)viewWillAppear:(BOOL)animated {
	[super viewWillAppear:animated];
}

- (void)viewDidAppear:(BOOL)animated {
	[super viewDidAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated {
}

- (void)viewDidDisappear:(BOOL)animated {
}


- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
	// Return YES for supported orientations
	return NO; //YES; //(interfaceOrientation == UIInterfaceOrientationPortrait);
}


- (void)didReceiveMemoryWarning {
	[super didReceiveMemoryWarning]; // Releases the view if it doesn't have a superview
	// Release anything that's not essential, such as cached data
}


- (void)dealloc {
	[currentPath release];
	[browseArray release];
	
	[super dealloc];
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
	// Number of sections is the number of region dictionaries
	
	if(displayList==nil) {
		return 1;
	} else {
		return [displayList count] + 1;
	}
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
	if(displayList==nil) {
		return 1;
	}
	
	if(section < 1)
	{
		return 1;
	}
	// Number of rows is the number of names in the region dictionary for the specified section
	NSDictionary *letterDictionary = [displayList objectAtIndex:section - 1];
	NSMutableArray *zonesForLetter = [letterDictionary objectForKey:@"listings"];
	return [zonesForLetter count];
}


- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section {
	if(section < 1)
	{
		return @"";
	}
	NSDictionary *sectionDictionary = [displayList objectAtIndex:section - 1];
	return [sectionDictionary valueForKey:@"letter"];
}

#if 1
- (NSArray *)sectionIndexTitlesForTableView:(UITableView *)tableView {
	/*
	 Return the index titles for each of the sections (e.g. "A", "B", "C"...).
	 Use key-value coding to get the value for the key @"letter" in each of the dictionaries in list.
	 */
	return [displayList valueForKey:@"letter"];
}


- (NSInteger)tableView:(UITableView *)tableView sectionForSectionIndexTitle:(NSString *)title atIndex:(NSInteger)index {
	return [indexedLetters indexOfObject:title];
}
#endif

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath {
	if(indexPath.section < 1) {
		return 48.0; // this is the height of the AdMob ad
	}
	
	return 44.0; // this is the generic cell height
}

- (void)setupIndexedData {
	int i;
	NSMutableDictionary *indexedNames = [[NSMutableDictionary alloc] init];
	for (i = 0;  i < [browseArray count]; i++) 
	{
		NSString *theLetter = [[[[browseArray objectAtIndex:i] objectForKey:@"object"] substringToIndex:1] lowercaseString];
		NSCharacterSet *special = [NSCharacterSet characterSetWithCharactersInString:@"1234567890qwertyuiopasdfghjklzxcvbnm"];
		NSRange range = [theLetter rangeOfCharacterFromSet:special options:NSLiteralSearch];
		NSString *firstLetter = nil;
		if( range.location == NSNotFound )
		{
			firstLetter = [[NSString alloc] initWithString: @"~"];
		}
		else
		{
			firstLetter = [[NSString alloc] initWithString: theLetter];
		}
		NSMutableArray *indexArray = [indexedNames objectForKey:firstLetter];
		if (indexArray == nil) {
			indexArray = [[NSMutableArray alloc] init];
			[indexedNames setObject:indexArray forKey:firstLetter];
			[indexArray release];
		}
		
		[indexArray addObject:[browseArray objectAtIndex:i]];
		
		[ firstLetter release ];
	}
	NSMutableArray *listings = [[NSMutableArray alloc] init];
	indexedLetters = (NSMutableArray*) [[[indexedNames allKeys] sortedArrayUsingSelector:@selector(compare:)] retain];
	NSSortDescriptor *sortDescriptor = [[NSSortDescriptor alloc] initWithKey:@"object" ascending:YES];
	NSArray *sortDescriptors = [NSArray arrayWithObject:sortDescriptor];
	for (NSString *indexLetter in indexedLetters) {
		NSMutableArray *arr = [indexedNames objectForKey:indexLetter];
		[arr sortUsingDescriptors:sortDescriptors];
		
		NSDictionary *letterDictionary = [[NSDictionary alloc] initWithObjectsAndKeys:indexLetter, @"letter", arr, @"listings", nil];
		[listings addObject:letterDictionary];
		[letterDictionary release];
	}
	[sortDescriptor release];
	displayList = [listings retain];
	[listings release];
	[indexedNames release];
}

- (NSMutableArray*)getDisplayList
{
	return displayList;
}

- (UITableView*)getTableview
{
	return tableview;
}

- (void)setCurrentlyPlaying:(NSString*) str
{	
	[SOApp.nowPlayingView setCurrentlyPlaying:str];
}


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