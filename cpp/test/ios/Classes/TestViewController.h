// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <UIKit/UIKit.h>
#import <TestUtil.h>

@interface TestViewController : UIViewController<UITableViewDataSource, UITableViewDelegate>
{
@private
    IBOutlet UITableView* output;
    IBOutlet UIActivityIndicatorView* activity;
    IBOutlet UIButton* nextButton;

    NSMutableString* currentMessage;
    NSMutableArray* messages;
    NSOperationQueue* queue;
    TestSuite* testSuite;
    NSEnumerator* testRunEnumator;
    bool reloadScheduled;
}
-(IBAction)next:(id)sender;
-(NSOperationQueue*) queue;
@end

