// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <BindingTest.h>

@interface RemoteCommunicatorI : TestBindingRemoteCommunicator<TestBindingRemoteCommunicator>
{
    int nextPort_;
}
-(id<TestBindingRemoteObjectAdapterPrx>) createObjectAdapter:(NSMutableString *)name
    endpoints:(NSMutableString *)endpoints current:(ICECurrent *)current;
-(void) deactivateObjectAdapter:(id<TestBindingRemoteObjectAdapterPrx>)adapter current:(ICECurrent *)current;
-(void) shutdown:(ICECurrent *)current;
@end

@interface RemoteObjectAdapterI : TestBindingRemoteObjectAdapter<TestBindingRemoteObjectAdapter>
{
    id<ICEObjectAdapter> adapter_;
    id<TestBindingTestIntfPrx> testIntf_;
}
-(id)initWithAdapter:(id<ICEObjectAdapter>)adapter;
-(id<TestBindingTestIntfPrx>) getTestIntf:(ICECurrent *)current;
-(void) deactivate:(ICECurrent *)current;
@end

@interface TestBindingI : TestBindingTestIntf<TestBindingTestIntf>
-(NSString *) getAdapterName:(ICECurrent *)current;
@end

