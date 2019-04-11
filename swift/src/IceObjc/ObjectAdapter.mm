// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import "IceObjcConfig.h"
#import "IceObjcObjectAdapter.h"
#import "IceObjcUtil.h"
#import "IceObjcObjectPrx.h"
#import "IceObjcInputStream.h"
#import "IceObjcConnection.h"
#import "IceObjcCommunicator.h"
#import "IceObjcBlobjectFacade.h"

@implementation ICEObjectAdapter

-(instancetype) initWithCppObjectAdapter:(std::shared_ptr<Ice::ObjectAdapter>)objectAdapter
{
    self = [super initWithLocalObject:objectAdapter.get()];
    if(self)
    {
        self->_objectAdapter = objectAdapter;
    }
    return self;
}

-(NSString*) getName
{
    return toNSString(_objectAdapter->getName());
}

-(ICECommunicator*) getCommunicator
{
    auto comm = _objectAdapter->getCommunicator().get();
    return [ICECommunicator fromLocalObject:comm];
}

-(BOOL) activate:(NSError* _Nullable * _Nullable)error
{
    try
    {
        _objectAdapter->activate();
        return YES;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return NO;
    }
}

-(BOOL) hold:(NSError* _Nullable * _Nullable)error
{
    try
    {
        _objectAdapter->hold();
        return YES;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return NO;
    }
}

-(BOOL) waitForHold:(NSError* _Nullable * _Nullable)error
{
    try
    {
        _objectAdapter->waitForHold();
        return YES;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return NO;
    }
}

-(void) deactivate
{
    _objectAdapter->deactivate();
}

-(void) waitForDeactivate
{
    _objectAdapter->waitForDeactivate();
}

-(BOOL) isDeactivated
{
    return _objectAdapter->isDeactivated();
}

-(void) destroy
{
    _objectAdapter->destroy();
}

-(nullable ICEObjectPrx*) createProxy:(NSString*)name category:(NSString*)category error:(NSError* _Nullable * _Nullable)error
{
    try
    {
        auto prx = _objectAdapter->createProxy(Ice::Identity{fromNSString(name), fromNSString(category)});
        return [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(nullable ICEObjectPrx*) createDirectProxy:(NSString*)name category:(NSString*)category error:(NSError* _Nullable * _Nullable)error
{
    try
    {
        auto prx = _objectAdapter->createDirectProxy(Ice::Identity{fromNSString(name), fromNSString(category)});
        return [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(nullable ICEObjectPrx*) createIndirectProxy:(NSString*)name category:(NSString*)category error:(NSError* _Nullable * _Nullable)error
{
    try
    {
        auto prx = _objectAdapter->createIndirectProxy(Ice::Identity{fromNSString(name), fromNSString(category)});
        return [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(NSArray<ICEEndpoint*>*) getEndpoints
{
    return toNSArray(_objectAdapter->getEndpoints());
}

-(BOOL) refreshPublishedEndpoints:(NSError* _Nullable * _Nullable)error
{
    try
    {
        _objectAdapter->refreshPublishedEndpoints();
        return YES;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return NO;
    }
}

-(NSArray<ICEEndpoint*>*) getPublishedEndpoints
{
    return toNSArray(_objectAdapter->getPublishedEndpoints());
}

-(BOOL) setPublishedEndpoints:(NSArray<ICEEndpoint*>*)newEndpoints error:(NSError* _Nullable * _Nullable)error
{
    try
    {
        Ice::EndpointSeq endpts;
        fromNSArray(newEndpoints, endpts);

        _objectAdapter->setPublishedEndpoints(endpts);
        return YES;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return NO;
    }
}

-(void) registerDefaultServant:(id<ICEBlobjectFacade>)facade
{
    auto servant = std::make_shared<BlobjectFacade>(facade);
    _objectAdapter->addDefaultServant(servant, "");
}

@end
