// Copyright (c) ZeroC, Inc.

#import "AppDelegate.h"

@implementation AppDelegate

- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions
{
    // Ensure device does not become idle
    [[UIApplication sharedApplication] setIdleTimerDisabled:YES];

    return YES;
}

@end
