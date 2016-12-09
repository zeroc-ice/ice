// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <UIKit/UIKit.h>

@interface AppDelegate : NSObject <UIApplicationDelegate>
{
@private

    UIWindow *window;
    IBOutlet UINavigationController *navigationController;
    NSArray* testSuites;
    NSInteger currentTestSuite;
    NSString* protocol;
    BOOL loop;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet UINavigationController* navigationController;
@property (nonatomic, readonly) NSArray* testSuites;
@property (nonatomic) NSInteger currentTestSuite;
@property (nonatomic, retain) NSString* protocol;
@property (nonatomic) BOOL loop;

-(BOOL)testCompleted:(BOOL)success;

@end

