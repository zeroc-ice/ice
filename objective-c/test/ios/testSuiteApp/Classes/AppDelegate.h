// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <UIKit/UIKit.h>

@class TestViewController;

@interface AppDelegate : NSObject <UIApplicationDelegate>
{
@private
    UIWindow *window;
    IBOutlet UINavigationController *navigationController;
    NSArray* tests;
    NSInteger currentTest;
    NSString* protocol;
    BOOL loop;
    BOOL runAll;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet UINavigationController* navigationController;
@property (nonatomic, readonly) NSArray* tests;
@property (nonatomic) NSInteger currentTest;
@property (nonatomic, retain) NSString* protocol;
@property (nonatomic) BOOL loop;
@property (readonly) BOOL runAll;

-(BOOL)testCompleted:(BOOL)success;

@end

