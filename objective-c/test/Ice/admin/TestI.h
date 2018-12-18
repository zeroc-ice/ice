// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#import <objc/Ice.h>
#import <AdminTest.h>

#import <Foundation/Foundation.h>

@interface TestAdminRemoteCommunicatorI : TestAdminRemoteCommunicator<TestAdminRemoteCommunicator,
                                                                      ICEPropertiesAdminUpdateCallback>
{
    id<ICECommunicator> _communicator;
    ICEMutablePropertyDict* _changes;
    BOOL _called;
    NSCondition* _cond;
}
+(id) remoteCommunicator:(id<ICECommunicator>)communicator;
@end

@interface TestAdminRemoteCommunicatorFactoryI : TestAdminRemoteCommunicatorFactory<TestAdminRemoteCommunicatorFactory>
@end

@interface TestAdminTestFacetI : TestAdminTestFacet<TestAdminTestFacet>
@end
