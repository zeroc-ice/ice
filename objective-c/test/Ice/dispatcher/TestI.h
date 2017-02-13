// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <DispatcherTest.h>

@interface TestDispatcherTestIntfI : TestDispatcherTestIntf
-(void) op:(ICECurrent *)current;
-(void) opWithPayload:(ICEMutableByteSeq*)seq current:(ICECurrent *)current;
-(void) shutdown:(ICECurrent *)current;
@end

@interface TestDispatcherTestIntfControllerI : TestDispatcherTestIntfController
{
    id<ICEObjectAdapter> _adapter;
}
-(id) initWithAdapter:(id<ICEObjectAdapter>)adapter;
-(void) holdAdapter:(ICECurrent*)current;
-(void) resumeAdapter:(ICECurrent*)current;
@end
