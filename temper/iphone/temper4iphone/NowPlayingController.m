//
//  NowPlayingController.m
//  ShoutOut
//
//  Created by ME on 9/13/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "SOApplication.h"
#import "../iphone_event.h"
#import <pthread.h>

#define M_PI        3.14159265358979323846264338327950288   /* pi */

char romfile[1024];
int				tArgc;
char**		tArgv;
unsigned short* screenbuffer;
int iphone_touches = 0;
int iphone_layout = 0;

extern unsigned long gp2x_pad_status;
extern float __audioVolume;
unsigned short BaseAddress[320*240];
extern int __emulation_run;
extern int __emulation_saving;
extern int tArgc;
extern char** tArgv;
extern pthread_t main_tid;
extern unsigned char gamepak_filename[512];
extern void set_save_state(void);
extern int temper_main(int argc, char *argv[]);
extern void app_DemuteSound(int buffersize);
extern void app_MuteSound(void);
int iphone_soundon = 0;


ScreenView* sharedInstance = nil;

void updateScreen()
{
	/*static int delayer = 0;
	
	delayer++;
	if(delayer >= 2)
	{
	*/
		[sharedInstance performSelectorOnMainThread:@selector(setNeedsDisplay) withObject:nil waitUntilDone:NO];
	/*	delayer = 0;
	}
	*/
}

void *app_Thread_Start(void *args)
{
	temper_main(tArgc, tArgv);
	return NULL;
}

@implementation ControllerView
- (id)initWithFrame:(CGRect)frame
{
	if ((self = [super initWithFrame:frame])!=nil)
	{
	}
	
	return self;
}

- (void)drawRect:(CGRect)rect
{
  
}
@end

@implementation ScreenView
- (id)initWithFrame:(CGRect)frame {
	if ((self = [super initWithFrame:frame])!=nil) {
		CFMutableDictionaryRef dict;
	int w = 320; //rect.size.width;
	int h = 240; //rect.size.height;
	
	int pitch = w * 2, allocSize = 2 * w * h;
	char *pixelFormat = "565L";
	
		self.opaque = YES;
		self.clearsContextBeforeDrawing = YES;
		self.userInteractionEnabled = NO;
		self.multipleTouchEnabled = NO;
		self.exclusiveTouch = NO;
		self.contentMode = UIViewContentModeTopLeft;
		
		[[self layer] setMagnificationFilter:0];
		[[self layer] setMinificationFilter:0];
		
		dict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
										 &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		CFDictionarySetValue(dict, kCoreSurfaceBufferGlobal, kCFBooleanTrue);
		CFDictionarySetValue(dict, kCoreSurfaceBufferMemoryRegion,
							 @"PurpleGFXMem");
		CFDictionarySetValue(dict, kCoreSurfaceBufferPitch,
							 CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &pitch));
		CFDictionarySetValue(dict, kCoreSurfaceBufferWidth,
							 CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &w));
		CFDictionarySetValue(dict, kCoreSurfaceBufferHeight,
							 CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &h));
		CFDictionarySetValue(dict, kCoreSurfaceBufferPixelFormat,
							 CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, pixelFormat));
		CFDictionarySetValue(dict, kCoreSurfaceBufferAllocSize,
							 CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &allocSize));
		
		_screenSurface = CoreSurfaceBufferCreate(dict);
		CoreSurfaceBufferLock(_screenSurface, 3);
		
    	//CALayer* screenLayer = [CALayer layer];  
		screenLayer = [[CALayer layer] retain];
		/*
		CGAffineTransform affineTransform = CGAffineTransformIdentity;
		affineTransform = CGAffineTransformConcat( affineTransform, CGAffineTransformMakeRotation(90));
		self.transform = affineTransform;
		*/

	if(iphone_layout == 0 || iphone_layout == 1)
	{
		screenLayer.frame = CGRectMake(0.0f, 0.0f, 320.0f, 242.0f);
	}
	else
	{
		if([SOApp.optionsView getCurrentScaling])
		{
			CGAffineTransform transform = CGAffineTransformMakeRotation(M_PI / 2.0f); // = CGAffineTransformMakeTranslation(1.0, 1.0);
			[screenLayer setAffineTransform:transform];
			screenLayer.frame = CGRectMake(0.0f, 27.0f, 320.0f, 426.0f);
			//[screenLayer setCenter:CGPointMake(240.0f,160.0f)];
		}
		else
		{
			CGAffineTransform transform = CGAffineTransformMakeRotation(M_PI / 2.0f); // = CGAffineTransformMakeTranslation(1.0, 1.0);
			[screenLayer setAffineTransform:transform];
			screenLayer.frame = CGRectMake(40.0f, 80.0f, 240.0f, 320.0f);
			//[screenLayer setCenter:CGPointMake(240.0f,160.0f)];
		}
	}
  [screenLayer setOpaque:YES];
	[screenLayer setMinificationFilter: 0];
	[screenLayer setMagnificationFilter: 0];
	screenLayer.contents = (id)_screenSurface;
	[[self layer] addSublayer:screenLayer];
	
	/*
	 screenLayer = [CALayer layer];
	 screenLayer.doubleSided = NO;
	 screenLayer.bounds = rect;
	 screenLayer.contents = (id)_screenSurface;
	 screenLayer.anchorPoint = CGPointMake(0, 0); // set anchor point to top-left
	 [self.layer addSublayer: screenLayer];
	 */
	CoreSurfaceBufferUnlock(_screenSurface);
	
	screenbuffer = CoreSurfaceBufferGetBaseAddress(_screenSurface);
	
		//[NSThread detachNewThreadSelector:@selector(updateScreen) toTarget:self withObject:nil];
		
		/*
		timer = [NSTimer scheduledTimerWithTimeInterval:0.01f
												 target:self
											   selector:@selector(updateScreen)
											   userInfo:nil
												repeats:YES];
		*/
	}
    
	sharedInstance = self;
	
	return self;
}

- (void)dealloc
{
	[timer invalidate];
	[ screenLayer release ];
	[ super dealloc ];
}

- (CoreSurfaceBufferRef)getSurface
{
	return _screenSurface;
}


- (void)drawRect:(CGRect)rect
{
	if(screenbuffer)
	{
		memcpy(screenbuffer, BaseAddress, 320*240*2);
	}
	//memcpy(screenbuffer, BaseAddress, 240*160*2);
	//memcpy(screenbuffer, BaseAddress, 240*160*2);
}

- (void)dummy
{
	
}

- (void)updateScreen
{
	static unsigned long long last_upd_ticks = 0;
		
	if(last_upd_ticks != 0)
	{
		struct timeval current_time;
		gettimeofday(&current_time, NULL);
		unsigned long long current_ticks = 
		((unsigned long long)current_time.tv_sec * 1000000ll) + current_time.tv_usec;
		
		if((current_ticks - last_upd_ticks) >= (iphone_touches > 1 ? 40000 : 33333))
		{
			[self setNeedsDisplay];
			struct timeval new_time;
			gettimeofday(&new_time, NULL);
			last_upd_ticks = ((unsigned long long)new_time.tv_sec * 1000000ll) + new_time.tv_usec;
		}
	}
	else
	{
		[self setNeedsDisplay];
		struct timeval new_time;
		gettimeofday(&new_time, NULL);
		last_upd_ticks = ((unsigned long long)new_time.tv_sec * 1000000ll) + new_time.tv_usec;
	}
}

@end

@implementation NowPlayingController

- (void)alertView:(UIAlertView *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex {
	// the user clicked one of the OK/Cancel buttons
	if(__emulation_run)
	{
		if (buttonIndex == 0)
		{
			__emulation_saving = 2;
			__emulation_run = 0;
			//pthread_join(main_tid, NULL);
			while(__emulation_saving)
		  {
        usleep(10000);
		  }
			[screenView release];
			[controllerImageView release];
			__emulation_run = 0;
			__emulation_saving = 0;			
			[SOApp.delegate switchToSaveStates];
			[tabBar didMoveToWindowSaveStates];
		}
		else if (buttonIndex == 1)
		{
			__emulation_saving = 1;
			__emulation_run = 0;
			//pthread_join(main_tid, NULL);
			while(__emulation_saving)
		  {
        usleep(10000);
		  }
			[screenView release];
			[controllerImageView release];
			__emulation_run = 0;
			__emulation_saving = 0;			
			[SOApp.delegate switchToSaveStates];
			[tabBar didMoveToWindowSaveStates];
		}
		else			
		{
			__emulation_saving = 0;
			__emulation_run = 0;
			//pthread_join(main_tid, NULL);
			[screenView release];
			[controllerImageView release];
			__emulation_run = 0;
			__emulation_saving = 0;
		}
		
		if(tArgv)
  	{
  		int i;
  		for(i = 0; i < tArgc; i++)
  		{
  			if(tArgv[i] != NULL)
  				free(tArgv[i]);

  			tArgv[i] = NULL;
  		}
  		free(tArgv);
  		tArgv = NULL;
  	}
	}
	else
	{
	  iphone_layout = buttonIndex;
		if (buttonIndex == 0)
		{
			iphone_soundon = 1;
			[ self getControllerCoords:0 ];
			[ self fixRects ];
			numFingers = 0;
			__emulation_run = 1;
			screenView = [ [ScreenView alloc] initWithFrame: CGRectMake(0, 0, 1, 1)];
			screenView.userInteractionEnabled = NO;
			screenView.multipleTouchEnabled = NO;
			screenView.clearsContextBeforeDrawing = YES;
			[self.view addSubview: screenView];
			controllerImageView = [ [ ControllerView alloc ] initWithImage:[UIImage imageNamed:[NSString stringWithFormat:@"controller_hs%d.png", [SOApp.optionsView getCurrentSkin]]]];
			controllerImageView.frame = CGRectMake(0.0f, 240.0f, 320.0f, 240.0f); // Set the frame in which the UIImage should be drawn in.
			controllerImageView.userInteractionEnabled = NO;
			controllerImageView.multipleTouchEnabled = NO;
			controllerImageView.clearsContextBeforeDrawing = YES;
			[controllerImageView setOpaque:YES];
			[controllerImageView setAlpha:1.0f];
			[self.view addSubview: controllerImageView]; // Draw the image in self.view.
		}
		else if (buttonIndex == 1)
		{
			iphone_soundon = 0;
			[ self getControllerCoords:0 ];
			[ self fixRects ];
			numFingers = 0;
			__emulation_run = 1;
			screenView = [ [ScreenView alloc] initWithFrame: CGRectMake(0, 0, 1, 1)];
			screenView.userInteractionEnabled = NO;
			screenView.multipleTouchEnabled = NO;
			screenView.clearsContextBeforeDrawing = YES;			
			[self.view addSubview: screenView];
			controllerImageView = [ [ ControllerView alloc ] initWithImage:[UIImage imageNamed:[NSString stringWithFormat:@"controller_hs%d.png", [SOApp.optionsView getCurrentSkin]]]];
			controllerImageView.frame = CGRectMake(0.0f, 240.0f, 320.0f, 240.0f); // Set the frame in which the UIImage should be drawn in.
			controllerImageView.userInteractionEnabled = NO;
			controllerImageView.multipleTouchEnabled = NO;
			controllerImageView.clearsContextBeforeDrawing = YES;
			[controllerImageView setOpaque:YES];
			[controllerImageView setAlpha:1.0f];
			[self.view addSubview: controllerImageView]; // Draw the image in self.view.
		}
		else if (buttonIndex == 2)
		{
			iphone_soundon = 1;
			[ self getControllerCoords:1 ];
			[ self fixRects ];
			numFingers = 0;
			__emulation_run = 1;
			screenView = [ [ScreenView alloc] initWithFrame: CGRectMake(0, 0, 1, 1)];
			screenView.userInteractionEnabled = NO;
			screenView.multipleTouchEnabled = NO;
			screenView.clearsContextBeforeDrawing = YES;	
			[self.view addSubview: screenView];
			controllerImageView = [ [ ControllerView alloc ] initWithImage:[UIImage imageNamed:[NSString stringWithFormat:@"controller_fs%d.png", [SOApp.optionsView getCurrentSkin]]]];
			controllerImageView.frame = CGRectMake(0.0f, 0.0f, 320.0f, 480.0f); // Set the frame in which the UIImage should be drawn in.
			controllerImageView.userInteractionEnabled = NO;
			controllerImageView.multipleTouchEnabled = NO;
			controllerImageView.clearsContextBeforeDrawing = YES;
			[controllerImageView setOpaque:YES];
			[controllerImageView setAlpha:0.25f];
			[self.view addSubview: controllerImageView]; // Draw the image in self.view.
		}
		else
		{
			iphone_soundon = 0;
			[ self getControllerCoords:1 ];
			[ self fixRects ];
			numFingers = 0;
			__emulation_run = 1;
			screenView = [ [ScreenView alloc] initWithFrame: CGRectMake(0, 0, 1, 1)];
			screenView.userInteractionEnabled = NO;
			screenView.multipleTouchEnabled = NO;
			screenView.clearsContextBeforeDrawing = YES;
			[self.view addSubview: screenView];
			controllerImageView = [ [ ControllerView alloc ] initWithImage:[UIImage imageNamed:[NSString stringWithFormat:@"controller_fs%d.png", [SOApp.optionsView getCurrentSkin]]]];
			controllerImageView.frame = CGRectMake(0.0f, 0.0f, 320.0f, 480.0f); // Set the frame in which the UIImage should be drawn in.
			[controllerImageView setOpaque:YES];
			[controllerImageView setAlpha:0.25f];
			controllerImageView.clearsContextBeforeDrawing = YES;
			controllerImageView.userInteractionEnabled = NO;
			controllerImageView.multipleTouchEnabled = NO;
			[self.view addSubview: controllerImageView]; // Draw the image in self.view.				
		}
    iphone_touches = 0;
		pthread_create(&main_tid, NULL, app_Thread_Start, NULL);
		//[NSThread detachNewThreadSelector:@selector(runProgram) toTarget:self withObject:nil];
	}
}

- (void)runProgram
{
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	temper_main(tArgc, tArgv);
  [pool release];
}

- (void)startEmu:(char*)path {
	int i;

	tArgc = 0;
	tArgv = (char**) malloc (sizeof(*tArgv) * 32);
	
	if (tArgv == NULL) 
	{
		return;
	}
		
	tArgv[tArgc] = (char*)malloc(strlen("/var/mobile/Media/ROMs/TEMPER/temper4iphone") + 1);
	sprintf(tArgv[tArgc], "/var/mobile/Media/ROMs/TEMPER/temper4iphone");
	tArgc++;
	
	tArgv[tArgc] = (char*)malloc(512);
	sprintf(tArgv[tArgc], "%s", path);
	tArgc++;

  if( (!strcasecmp(path + (strlen(path)-4), ".svs")) )
	{
	  tArgv[tArgc] = (char*)malloc(strlen("-s") + 1);
	  sprintf(tArgv[tArgc], "-s");
	  tArgc++;
	}
	
  tArgv[tArgc] = NULL;
  	
	UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Screen Orientation & Sound" message:@"Choose Your Screen Orientation" delegate:self cancelButtonTitle:nil otherButtonTitles:@"Portrait & Sound", @"Portrait & No Sound", @"Landscape & Sound", @"Landscape & No Sound", nil];
	[alert show];
	[alert release];
}

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
	if (self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil]) {
		// Initialization code
	}
	return self;
}

- (void)awakeFromNib {
	//[ self setTapDelegate: self ];
	//[ self setGestureDelegate: self ];
	//[ self setMultipleTouchEnabled: YES ];
	[ self getControllerCoords:0 ];
	[ self fixRects ];
	numFingers = 0;
	iphone_touches = 0;
	self.navigationItem.hidesBackButton = YES;
	
	self.view.opaque = YES;
	self.view.clearsContextBeforeDrawing = NO;
	self.view.userInteractionEnabled = YES;
	self.view.multipleTouchEnabled = YES;
	self.view.exclusiveTouch = YES;
	self.view.contentMode = UIViewContentModeTopLeft;
	
	[[self.view layer] setMagnificationFilter:0];
	[[self.view layer] setMinificationFilter:0];
}

- (void)drawRect:(CGRect)rect
{
}


- (void)runSound
{
}

- (void)fixRects {

}


#define MyCGRectContainsPoint(rect, point)						\
	(((point.x >= rect.origin.x) &&								\
		(point.y >= rect.origin.y) &&							\
		(point.x <= rect.origin.x + rect.size.width) &&			\
		(point.y <= rect.origin.y + rect.size.height)) ? 1 : 0)

#if 1
- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {	
	int i;
	//Get all the touches.
	NSSet *allTouches = [event allTouches];
	int touchcount = [allTouches count];
	
	gp2x_pad_status = 0;
	
	iphone_touches = touchcount;
			
	for (i = 0; i < touchcount; i++) 
	{
		UITouch *touch = [[allTouches allObjects] objectAtIndex:i];
		
		if(touch == nil)
		{
			return;
		}
		
		/*if(touch.phase == UITouchPhaseBegan)
		{
			NSLog([NSString stringWithFormat:@"%s", test_print_buffer]);
		}*/
		if( touch.phase == UITouchPhaseBegan		||
			touch.phase == UITouchPhaseMoved		||
			touch.phase == UITouchPhaseStationary	)
		{
			struct CGPoint point;
			point = [touch locationInView:self.view];
			
			if (MyCGRectContainsPoint(Left, point)) {
				gp2x_pad_status |= GP2X_LEFT;
			}
			else if (MyCGRectContainsPoint(Right, point)) {
				gp2x_pad_status |= GP2X_RIGHT;
			}
			else if (MyCGRectContainsPoint(Up, point)) {
				gp2x_pad_status |= GP2X_UP;
			}
			else if (MyCGRectContainsPoint(Down, point)) {
				gp2x_pad_status |= GP2X_DOWN;
			}
			else if (MyCGRectContainsPoint(ButtonAB, point)) {
				gp2x_pad_status |= GP2X_B | GP2X_X;
			}
			else if (MyCGRectContainsPoint(ButtonA, point)) {
				gp2x_pad_status |= GP2X_B;
			}
			else if (MyCGRectContainsPoint(ButtonB, point)) {
				gp2x_pad_status |= GP2X_X;
			}
			else if (MyCGRectContainsPoint(UpLeft, point)) {
				gp2x_pad_status |= GP2X_UP | GP2X_LEFT;
			} 
			else if (MyCGRectContainsPoint(DownLeft, point)) {
				gp2x_pad_status |= GP2X_DOWN | GP2X_LEFT;
			}
			else if (MyCGRectContainsPoint(UpRight, point)) {
				gp2x_pad_status |= GP2X_UP | GP2X_RIGHT;
			}
			else if (MyCGRectContainsPoint(DownRight, point)) {
				gp2x_pad_status |= GP2X_DOWN | GP2X_RIGHT;
			}
			else if (MyCGRectContainsPoint(Select, point)) {
				gp2x_pad_status |= GP2X_SELECT;
			}
			else if (MyCGRectContainsPoint(Start, point)) {
				gp2x_pad_status |= GP2X_START;
			}
			else if (MyCGRectContainsPoint(LPad, point)) {
				gp2x_pad_status |= GP2X_L;
			}
			else if (MyCGRectContainsPoint(RPad, point)) {
				gp2x_pad_status |= GP2X_R;
			}			
			else if (MyCGRectContainsPoint(Menu, point)) {
				if(touch.phase == UITouchPhaseBegan || touch.phase == UITouchPhaseStationary)
				{
					[SOApp.delegate switchToBrowse];
					[tabBar didMoveToWindow];
					if(__emulation_run)
					{
						UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Save Game State" message:@"Save your state? You can overwrite a currently loaded savestate, save to a new file, or cancel." delegate:self cancelButtonTitle:nil otherButtonTitles:@"Save [Currently Loaded] State",@"Save State To New File",@"Don't Save?!", nil];
						[alert show];
						[alert release];
					}
				}
			}
		}
	}
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
	[self touchesBegan:touches withEvent:event];
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event {
	[self touchesBegan:touches withEvent:event];
	iphone_touches = 0;
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
	[self touchesBegan:touches withEvent:event];
	iphone_touches = 0;
}
#endif
/*
- (void)view:(UIView *)view handleTapWithCount:(int)count event:(myGSEvent *)event {
	//NSLog(@"handleTapWithCount: %d", count);
	[self dumpEvent: event];
}
*/
/*
- (double)viewTouchPauseThreshold:(UIView *)view {
	NSLog(@"TouchPause!");
	return 0.0;
}
*/

#if 0
- (void)mouseEvent:(myGSEvent*)event {
	int i;
	int touchcount = event->fingerCount;
	
	gp2x_pad_status = 0;
	
	for (i = 0; i < touchcount; i++) 
	{
		struct CGPoint point = CGPointMake(event->points[i].x, event->points[i].y);
		
		if (MyCGRectContainsPoint(Left, point)) {
			gp2x_pad_status |= GP2X_LEFT;
		}
		else if (MyCGRectContainsPoint(Right, point)) {
			gp2x_pad_status |= GP2X_RIGHT;
		}
		else if (MyCGRectContainsPoint(Up, point)) {
			gp2x_pad_status |= GP2X_UP;
		}
		else if (MyCGRectContainsPoint(Down, point)) {
			gp2x_pad_status |= GP2X_DOWN;
		}
		else if (MyCGRectContainsPoint(A, point)) {
			gp2x_pad_status |= GP2X_B;
		}
		else if (MyCGRectContainsPoint(B, point)) {
			gp2x_pad_status |= GP2X_X;
		}
		else if (MyCGRectContainsPoint(AB, point)) {
			gp2x_pad_status |= GP2X_B | GP2X_X;
		}			
		else if (MyCGRectContainsPoint(UpLeft, point)) {
			gp2x_pad_status |= GP2X_UP | GP2X_LEFT;
		} 
		else if (MyCGRectContainsPoint(DownLeft, point)) {
			gp2x_pad_status |= GP2X_DOWN | GP2X_LEFT;
		}
		else if (MyCGRectContainsPoint(UpRight, point)) {
			gp2x_pad_status |= GP2X_UP | GP2X_RIGHT;
		}
		else if (MyCGRectContainsPoint(DownRight, point)) {
			gp2x_pad_status |= GP2X_DOWN | GP2X_RIGHT;
		}			
		else if (MyCGRectContainsPoint(LPad, point)) {
			gp2x_pad_status |= GP2X_L;
		}
		else if (MyCGRectContainsPoint(RPad, point)) {
			gp2x_pad_status |= GP2X_R;
		}			
		else if (MyCGRectContainsPoint(Select, point)) {
			gp2x_pad_status |= GP2X_SELECT;
		}
		else if (MyCGRectContainsPoint(Start, point)) {
			gp2x_pad_status |= GP2X_START;
		}
		else if (MyCGRectContainsPoint(Menu, point)) {
			//if(touch.phase == UITouchPhaseBegan || touch.phase == UITouchPhaseStationary)
			{
				[SOApp.delegate switchToBrowse];
				[tabBar didMoveToWindow];
				if(__emulation_run)
				{
					UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Save Game State" message:@"Save Game State?" delegate:self cancelButtonTitle:nil otherButtonTitles:@"Save Game!",@"Don't Save?", nil];
					[alert show];
					[alert release];
				}
			}
		}
	}
}

- (void)mouseDown:(myGSEvent*)event {
	//NSLog(@"mouseDown:");
	[self mouseEvent: event];
}

- (void)mouseDragged:(myGSEvent*)event {
	//NSLog(@"mouseDragged:");
	[self mouseEvent: event];
}

- (void)mouseEntered:(myGSEvent*)event {		
	//NSLog(@"mouseEntered:");
	[self mouseEvent: event];
}

- (void)mouseExited:(myGSEvent*)event {		
	//NSLog(@"mouseExited:");
	[self mouseEvent: event];
}

- (void)mouseMoved:(myGSEvent*)event {
	//NSLog(@"mouseMoved:");
	[self mouseEvent: event];
}

- (void)mouseUp:(myGSEvent*)event {
	[self mouseEvent: event];
}
#endif

/*
- (BOOL)isFirstResponder {
	return YES;
}
*/

- (void)getControllerCoords:(int)orientation {
    char string[256];
    FILE *fp;
	
	if(!orientation)
	{
		fp = fopen([[NSString stringWithFormat:@"/Applications/temper4iphone.app/controller_hs%d.txt", [SOApp.optionsView getCurrentSkin]] UTF8String], "r");
  }
	else
	{
		fp = fopen([[NSString stringWithFormat:@"/Applications/temper4iphone.app/controller_fs%d.txt", [SOApp.optionsView getCurrentSkin]] UTF8String], "r");
	}
	
	if (fp) 
	{
		int i = 0;
    
    while(fgets(string, 256, fp) != NULL && i < 16) 
    {
			char* result = strtok(string, ",");
			int coords[4];
			int i2 = 1;
			while( result != NULL && i2 < 5 )
			{
				coords[i2 - 1] = atoi(result);
				result = strtok(NULL, ",");
				i2++;
			}
	
	    switch(i)
			{
			case 0:    DownLeft   	= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
			case 1:    Down   		  = CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
			case 2:    DownRight    = CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
			case 3:    Left  		    = CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
			case 4:    Right  		  = CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
			case 5:    UpLeft     	= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
			case 6:    Up     	  	= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
			case 7:    UpRight    	= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
			case 8:    Select	    	= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
			case 9:	   Start	    	= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
			case 10:   ButtonA     	= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
			case 11:   ButtonB     	= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
			case 12:   ButtonAB    	= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
			case 13:   LPad	    		= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
			case 14:   RPad	    		= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
			case 15:   Menu	    		= CGRectMake( coords[0], coords[1], coords[2], coords[3] ); break;
			}
      i++;
    }
    fclose(fp);
  }
}

/*
 Implement loadView if you want to create a view hierarchy programmatically
- (void)loadView {
}
 */

/*
 If you need to do additional setup after loading the view, override viewDidLoad.
- (void)viewDidLoad {
}
 */


- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
	// Return YES for supported orientations
	return (interfaceOrientation == UIInterfaceOrientationPortrait);
}


- (void)didReceiveMemoryWarning {
	[super didReceiveMemoryWarning]; // Releases the view if it doesn't have a superview
	// Release anything that's not essential, such as cached data
}


- (void)dealloc {
	if(currentPath)
		[ currentPath release ];
	if(currentFile)
		[ currentFile release ];
	if(currentDir)
		[ currentDir release ];
	  
	[super dealloc];
}

- (void)volumeChanged:(id)sender
{
#if 0
	__audioVolume = volumeSlider.value;
#endif
}

- (void)setCurrentStation:(NSString*)thePath withFile:(NSString*)theFile withDir:(NSString*)theDir {
	if(currentPath)
	{	[ currentPath release ]; currentPath = NULL; }
	if(currentFile)
	{	[ currentFile release ]; currentFile = NULL; }
	if(currentDir)
	{	[ currentDir release ]; currentDir = NULL; }
	
	if(thePath)
	{
		currentPath = [[NSString alloc] initWithString: thePath];	
	}
	if(theFile)
	{
		currentFile = [[NSString alloc] initWithString: theFile];	
	}
	if(theDir)
	{
		currentDir = [[NSString alloc] initWithString: theDir];	
	}
}

#if 0
- (void)setBookmark:(id)sender 
{
	if(__emulation_run)
	{
		UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Save Game State" message:@"Save Game State?" delegate:self cancelButtonTitle:@"Dont Save?" otherButtonTitles:@"Save Game!", nil];
		[alert show];
		[alert release];
		__emulation_saving = 1;
		__emulation_run = 0;
		pthread_join(main_tid, NULL);
		[screenView release];
	}	
	if(currentPath && currentFile && currentDir)
	{
		[SOApp.bookmarksView addBookmark:currentPath withFile:currentFile withDir:currentDir];
		[SOApp.delegate switchToBookmarks];
		[tabBar didMoveToWindowBookmarks];		
	}
	else
	{
		[SOApp.delegate switchToBookmarks];
		[tabBar didMoveToWindowBookmarks];
	}
}
#endif

- (void)setSaveState:(id)sender 
{
#if 0
	if(__emulation_run)
	{
		UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Save Game State" message:@"Save Game State?" delegate:self cancelButtonTitle:@"Dont Save?" otherButtonTitles:@"Save Game!", nil];
		[alert show];
		[alert release];
		__emulation_saving = 1;
		__emulation_run = 0;
		pthread_join(main_tid, NULL);
		[screenView release];
	}	
	[SOApp.delegate switchToBrowse];
	[tabBar didMoveToWindow];	
#endif
}

- (void)setCurrentlyPlaying:(NSString*) str
{
}

@end
