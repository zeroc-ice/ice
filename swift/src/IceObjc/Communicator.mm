// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import "Ice-Objc.h"

#include "Util.h"

@implementation ICECommunicator

-(instancetype) initWithCppCommunicator:(std::shared_ptr<Ice::Communicator>)communicator
{
    self = [super initWithLocalObject:communicator.get()];
    if(self)
    {
        self->_communicator = communicator;
    }
    return self;
}

-(void) destroy
{
    _communicator->destroy();
}

-(void) shutdown
{
    _communicator->destroy();
}

-(void) waitForShutdown
{
    _communicator->waitForShutdown();
}

-(bool) isShutdown
{
    return _communicator->isShutdown();
}

-(id) stringToProxy:(NSString*)str error:(NSError**)error
{
    try
    {
        ICEObjectPrx* prx = [[ICEObjectPrx alloc] initWithCppObjectPrx:_communicator->stringToProxy(fromNSString(str))];
        if(prx)
        {
            return prx;
        }
        else
        {
            return [NSNull null];
        }
    }
    catch(const std::exception& e)
    {
        *error = convertException(e);
    }

    return nil;
}

-(NSString*) proxyToString:(ICEObjectPrx*)prx error:(NSError* _Nullable * _Nullable)error
{
    try
    {
        return toNSString(_communicator->proxyToString([prx objectPrx]));
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(nullable id) propertyToProxy:(NSString*)property error:(NSError* _Nullable * _Nullable)error
{
    try
    {
        auto prx = _communicator->propertyToProxy(fromNSString(property));
        if(prx)
        {
            return [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
        }
        return [NSNull null];
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(NSDictionary<NSString*, NSString*>*) proxyToProperty:(ICEObjectPrx*)prx property:(NSString*)property error:(NSError* _Nullable * _Nullable)error
{
    return toNSDictionary(_communicator->proxyToProperty([prx objectPrx], fromNSString(property)));
}

//-(ObjectAdapterI*) createObjectAdapter:(NSString*)name error:(NSError* _Nullable * _Nullable)error
//{
////    try
////    {
//////        communicator->createObjectAdapter(fromNSString(name));
////    }
////    catch(const std::exception& ex)
////    {
////        *error = convertException(ex);
////        return nil;
////    }
//    assert(false);
//}
//
//-(ObjectAdapterI*) createObjectAdapterWithEndpoints:(NSString*)name endpoints:(NSString*)endpoints error:(NSError* _Nullable * _Nullable)error
//{
//    assert(false);
//}
//
//-(ObjectAdapterI*) createObjectAdapterWithRouter:(NSString*)name router:(_RouterPrxI*)router error:(NSError* _Nullable * _Nullable)error
//{
//    assert(false);
//}

-(ICEImplicitContext*) getImplicitContext
{
    auto implicitContext = _communicator->getImplicitContext();
    return createLocalObject<Ice::ImplicitContext>(implicitContext, [&implicitContext] () -> id
    {
        return [[ICEImplicitContext alloc] initWithCppImplicitContext:implicitContext];
    });
}

-(id<ICELoggerProtocol>) getLogger
{
    auto logger = _communicator->getLogger();
    return createLocalObject<Ice::Logger>(logger, [&logger] () -> id
    {
        return [[ICELogger alloc] initWithCppLogger:logger];
    });
}

@end
