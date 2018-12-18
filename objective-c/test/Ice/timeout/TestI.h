// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <TimeoutTest.h>

@interface TimeoutI : TestTimeoutTimeout<TestTimeoutTimeout>
-(void) op:(ICECurrent *)current;
-(void) sendData:(TestTimeoutMutableByteSeq *)seq current:(ICECurrent *)current;
-(void) sleep:(ICEInt)to current:(ICECurrent *)current;
@end

@interface TimeoutControllerI : TestTimeoutController<TestTimeoutController>
{
    id<ICEObjectAdapter> adapter_;
}
+(id) controller:(id<ICEObjectAdapter>)adapter;
-(void) holdAdapter:(ICEInt)to current:(ICECurrent *)current;
-(void) resumeAdapter:(ICECurrent *)current;
-(void) shutdown:(ICECurrent *)current;
@end
