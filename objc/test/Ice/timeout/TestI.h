// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <TimeoutTest.h>

@interface TimeoutI : TestTimeoutTimeout<TestTimeoutTimeout>
-(void) op:(ICECurrent *)current;
-(void) sendData:(TestTimeoutMutableByteSeq *)seq current:(ICECurrent *)current;
-(void) sleep:(ICEInt)to current:(ICECurrent *)current;
-(void) holdAdapter:(ICEInt)to current:(ICECurrent *)current;
-(void) shutdown:(ICECurrent *)current;
@end
