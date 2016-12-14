// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <UIKit/UIKit.h>

@protocol ViewController
-(void) print:(NSString*)msg;
-(void) println:(NSString*)msg;
@end

@interface ViewController : UIViewController<ViewController>
{
@private
    IBOutlet UITextView* output;
    void (*stopController)(id<ViewController>);
}
@end

