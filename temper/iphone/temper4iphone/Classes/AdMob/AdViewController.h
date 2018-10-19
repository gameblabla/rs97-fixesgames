/**
 * AdViewController.h
 * AdMob iPhone SDK publisher code.
 *
 * Helper file for using IB with AdMob ads. To integrate an ad using Interface Builder
 * and this code, add an NSObject and set its type to AdViewController. Then drag a UIView
 * where you want the ad to display; its dimensions must be 320x48. Set the
 * AdViewController's view outlet to that UIView. In this file's implementation, check that
 * your publisher id is set correctly. Build and run. If you have questions, look at the
 * sample IB-based project.
 *
 * Note that top level objects in nibs other than MainWindow.xib in Cocoa Touch are autoreleased, not retained like in OS X.
 * Be sure to add [self retain] to -awakeFromNib if this is part of a custom nib.
 * See http://developer.apple.com/releasenotes/DeveloperTools/RN-InterfaceBuilder/index.html#//apple_ref/doc/uid/TP40001016-SW5
 */

#define AD_REFRESH_PERIOD 60.0 // display fresh ads once per minute

#import <UIKit/UIKit.h>
#import "AdMobDelegateProtocol.h";
@class AdMobView;

@interface AdViewController : UIViewController<AdMobDelegate> {

  AdMobView *adMobAd;  // the actual ad; self.view is a placeholder to indicate where the ad should be placed; intentially _not_ an IBOutlet
  NSTimer *autoslider; // timer to slide in fresh ads

}

- (void)refreshAd:(NSTimer *)timer;

@end