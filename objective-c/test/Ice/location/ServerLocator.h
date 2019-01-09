// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
