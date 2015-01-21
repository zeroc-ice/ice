// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <AdminTest.h>

#import <Foundation/Foundation.h>

@interface RemoteCommunicatorI : TestAdminRemoteCommunicator<TestAdminRemoteCommunicator,
                                                             ICEPropertiesAdminUpdateCallback>
{
    id<ICECommunicator> _communicator;
    ICEMutablePropertyDict* _changes;
    BOOL _called;
    NSCondition* _cond;
}
+(id) remoteCommunicator:(id<ICECommunicator>)communicator;
@end

@interface RemoteCommunicatorFactoryI : TestAdminRemoteCommunicatorFactory<TestAdminRemoteCommunicatorFactory>
@end

@interface TestFacetI : TestAdminTestFacet<TestAdminTestFacet>
@end
