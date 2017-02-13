// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <ACMTest.h>

#import <Foundation/Foundation.h>

@interface RemoteCommunicatorI : TestACMRemoteCommunicator<TestACMRemoteCommunicator>
@end

@interface RemoteObjectAdapterI : TestACMRemoteObjectAdapter<TestACMRemoteObjectAdapter>
{
    id<ICEObjectAdapter> _adapter;
    id<TestACMTestIntfPrx> _testIntf;
}
-(id) initWithAdapter:(id<ICEObjectAdapter>)adapter;
@end

@interface TestACMTestIntfI : TestACMTestIntf<TestACMTestIntf>
{
    NSCondition* _cond;
}
@end
