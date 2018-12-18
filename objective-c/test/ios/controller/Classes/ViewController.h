// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <UIKit/UIKit.h>
#import <objc/Ice/Ice.h>

@protocol ViewController<NSObject>
-(void) print:(NSString*)msg;
-(void) println:(NSString*)msg;
@end

@interface ViewController : UIViewController<ViewController, UIPickerViewDataSource, UIPickerViewDelegate>
{
@private
    IBOutlet UIPickerView* interfaceIPv4;
    IBOutlet UIPickerView* interfaceIPv6;
    IBOutlet UITextView* output;
    NSMutableArray* interfacesIPv4;
    NSMutableArray* interfacesIPv6;
    id<ICECommunicator> communicator;
}
@end
