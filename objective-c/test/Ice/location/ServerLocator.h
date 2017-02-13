// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <LocationTest.h>

@interface ServerLocatorRegistry : TestLocationTestLocatorRegistry<TestLocationTestLocatorRegistry>
{
    NSMutableDictionary* adapters_;
    NSMutableDictionary* objects_;
}
-(id<ICEObjectPrx>) getAdapter:(NSString*)adapterId;
-(id<ICEObjectPrx>) getObject:(ICEIdentity*)ident;
-(void) addObject:(id<ICEObjectPrx>)prx;
@end

@interface ServerLocator : TestLocationTestLocator<TestLocationTestLocator>
{
    ServerLocatorRegistry* registry_;
    id<ICELocatorRegistryPrx> registryPrx_;
    int requestCount_;
}
-(id) init:(ServerLocatorRegistry*)registry proxy:(id<ICELocatorRegistryPrx>)proxy;
@end
