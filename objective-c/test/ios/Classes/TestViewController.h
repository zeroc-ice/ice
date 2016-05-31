// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <UIKit/UIKit.h>

typedef enum
{
    TestConfigOptionDefault,
    TestConfigOptionSliced,
    TestConfigOptionEncoding10
} TestConfigOption;

@class Test;

@interface TestViewController : UIViewController<UITableViewDataSource, UITableViewDelegate>
{
@private
    IBOutlet UITableView* output;
    IBOutlet UIActivityIndicatorView* activity;
    IBOutlet UIButton* nextButton;
    
    NSMutableString* currentMessage;
    NSMutableArray* messages;
    NSOperationQueue* queue;
    Test* test;
    NSEnumerator* testRunEnumerator;
}
-(IBAction)next:(id)sender;
-(NSOperationQueue*) queue;
@end

