// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <location/ServerLocator.h>

@implementation ServerLocatorRegistry
-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    adapters_ = [[NSMutableDictionary alloc] init];
    objects_ = [[NSMutableDictionary alloc] init];
    return self;
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [adapters_ release];
    [objects_ release];
    [super dealloc];
}
#endif

-(void) setAdapterDirectProxy:(NSMutableString *)adapter proxy:(id<ICEObjectPrx>)proxy current:(ICECurrent *)__unused current
{
    if(proxy == nil)
    {
        [adapters_ removeObjectForKey:adapter];
    }
    else
    {
        [adapters_ setObject:proxy forKey:adapter];
    }
}
-(void) setReplicatedAdapterDirectProxy:(NSMutableString *)adapterId
                         replicaGroupId:(NSMutableString *)replicaGroupId
                                      p:(id<ICEObjectPrx>)p
                                current:(ICECurrent *)__unused current
{
    if(p == nil)
    {
        [adapters_ removeObjectForKey:adapterId];
        [adapters_ removeObjectForKey:replicaGroupId];
    }
    else
    {
        [adapters_ setObject:p forKey:adapterId];
        [adapters_ setObject:p forKey:replicaGroupId];
    }
}
-(void) setServerProcessProxy:(NSMutableString *)__unused id_ proxy:(id<ICEProcessPrx>)__unused proxy current:(ICECurrent *)__unused current
{
}
-(void) addObject:(id<ICEObjectPrx>)object current:(ICECurrent*)__unused current
{
    [self addObject:object];
}
-(id<ICEObjectPrx>) getAdapter:(NSString*)adapterId
{
    id<ICEObjectPrx> proxy = [adapters_ objectForKey:adapterId];
    if(proxy == nil)
    {
        @throw [ICEAdapterNotFoundException adapterNotFoundException];
    }
    return proxy;
}
-(id<ICEObjectPrx>) getObject:(ICEIdentity*)ident
{
    id<ICEObjectPrx> proxy = [objects_ objectForKey:ident];
    if(proxy == nil)
    {
        @throw [ICEObjectNotFoundException objectNotFoundException];
    }
    return proxy;
}
-(void) addObject:(id<ICEObjectPrx>)object
{
    [objects_ setObject:object forKey:[object ice_getIdentity]];
}
@end

@implementation ServerLocator
-(id) init:(ServerLocatorRegistry*)registry proxy:(id<ICELocatorRegistryPrx>)registryPrx
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    registry_ = registry;
    registryPrx_ = registryPrx;
    requestCount_ = 0;
    return self;
}
-(id<ICEObjectPrx>) findObjectById:(ICEIdentity *)id_ current:(ICECurrent *)__unused current
{
    ++requestCount_;
    return [registry_ getObject:id_];
}
-(id<ICEObjectPrx>) findAdapterById:(NSMutableString *)id_ current:(ICECurrent *)current
{
    ++requestCount_;
    if([id_ isEqualToString:@"TestAdapter10"] || [id_ isEqualToString:@"TestAdapter10-2"])
    {
        NSAssert([current.encoding isEqual:ICEEncoding_1_0], @"unexpected encoding");
        return [registry_ getAdapter:@"TestAdapter"];
    }
    return [registry_ getAdapter:id_];
}
-(id<ICELocatorRegistryPrx>) getRegistry:(ICECurrent *)__unused current
{
    return registryPrx_;
}
-(int) getRequestCount:(ICECurrent*)__unused current
{
    return requestCount_;
}
@end
