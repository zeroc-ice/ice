// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import "ObjectPrx.h"

#import "Communicator.h"
#import "Connection.h"
#import "InputStream.h"
#import "OutputStream.h"
#import "Util.h"

@implementation ICEObjectPrx

-(ICEObjectPrx*) initWithObjectPrx:(ICEObjectPrx*)prx
{
    assert(prx);
    self = [super init];
    _objectPrx = std::shared_ptr<Ice::ObjectPrx>([prx objectPrx]);
    return self;
}

-(ICEObjectPrx*) initWithCppObjectPrx:(std::shared_ptr<Ice::ObjectPrx>)prx
{
    if(!prx)
    {
        return nil;
    }

    self = [super init];
    if(!self)
    {
        return nil;
    }

    self->_objectPrx = prx;

    return self;
}

-(nonnull NSString*) ice_toString
{
    return toNSString(_objectPrx->ice_toString());
}

-(ICECommunicator*) ice_getCommunicator
{
    auto comm = _objectPrx->ice_getCommunicator().get();
    return [ICECommunicator fromLocalObject:comm];
}

-(void) ice_getIdentity:(NSString* __strong _Nonnull * _Nonnull)name
               category:(NSString* __strong _Nonnull * _Nonnull)category
{
    // TODO: verify that __strong does not leak back in Swift
    auto identity = _objectPrx->ice_getIdentity();
    *name = toNSString(identity.name);
    *category = toNSString(identity.category);
}

-(instancetype) ice_identity:(NSString*)name
                    category:(NSString*)category
                       error:(NSError**)error
{
    try
    {
        return [[ICEObjectPrx alloc] initWithCppObjectPrx:_objectPrx->ice_identity(Ice::Identity{fromNSString(name),
                                                                                     fromNSString(category)})];
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(NSDictionary<NSString*, NSString*>*) ice_getContext
{
    return toNSDictionary(_objectPrx->ice_getContext());
}

-(instancetype) ice_context:(NSDictionary<NSString*, NSString*>*)context
{
    Ice::Context ctx;
    fromNSDictionary(context, ctx);
    return [[ICEObjectPrx alloc] initWithCppObjectPrx:_objectPrx->ice_context(ctx)];
}

-(NSString*) ice_getFacet
{
    return toNSString(_objectPrx->ice_getFacet());
}

-(instancetype) ice_facet:(NSString*)facet
{
    return [[ICEObjectPrx alloc] initWithCppObjectPrx:_objectPrx->ice_facet(fromNSString(facet))];
}

-(NSString*) ice_getAdapterId
{
    return toNSString(_objectPrx->ice_getAdapterId());
}

-(instancetype) ice_adapterId:(NSString*)id error:(NSError**)error
{
    try
    {
        return [[ICEObjectPrx alloc] initWithCppObjectPrx:_objectPrx->ice_adapterId(fromNSString(id))];
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(NSArray<ICEEndpoint*>*) ice_getEndpoints
{
    return toNSArray(_objectPrx->ice_getEndpoints());
}

-(instancetype) ice_endpoints:(NSArray<ICEEndpoint*>*)endpoints error:(NSError**)error
{
    try
    {
        Ice::EndpointSeq endpts;
        fromNSArray(endpoints, endpts);
        return [[ICEObjectPrx alloc] initWithCppObjectPrx:_objectPrx->ice_endpoints(endpts)];
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(int32_t) ice_getLocatorCacheTimeout
{
    return _objectPrx->ice_getLocatorCacheTimeout();
}

-(instancetype) ice_locatorCacheTimeout:(int32_t)timeout error:(NSError**)error
{
    try
    {
        return [[ICEObjectPrx alloc] initWithCppObjectPrx:_objectPrx->ice_locatorCacheTimeout(timeout)];
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(int32_t) ice_getInvocationTimeout
{
    return _objectPrx->ice_getInvocationTimeout();
}

-(instancetype) ice_invocationTimeout:(int32_t)timeout error:(NSError**)error
{
    try
    {
        return [[ICEObjectPrx alloc] initWithCppObjectPrx:_objectPrx->ice_invocationTimeout(timeout)];
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(NSString*) ice_getConnectionId
{
    return toNSString(_objectPrx->ice_getConnectionId());
}

-(instancetype) ice_connectionId:(NSString*)connectionId error:(NSError**)error
{
    try
    {
        return [[ICEObjectPrx alloc] initWithCppObjectPrx:_objectPrx->ice_connectionId(fromNSString(connectionId))];
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(bool) ice_isConnectionCached
{
    return _objectPrx->ice_isConnectionCached();
}

-(instancetype) ice_connectionCached:(bool)cached error:(NSError**)error
{
    try
    {
        return [[ICEObjectPrx alloc] initWithCppObjectPrx:_objectPrx->ice_connectionCached(cached)];
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(uint8_t) ice_getEndpointSelection
{
    return static_cast<uint8_t>(_objectPrx->ice_getEndpointSelection());
}

-(instancetype) ice_endpointSelection:(uint8_t)type error:(NSError**)error
{
    try
    {
        return [[ICEObjectPrx alloc] initWithCppObjectPrx:_objectPrx->ice_endpointSelection(Ice::EndpointSelectionType(type))];
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(instancetype) ice_encodingVersion:(uint8_t)major minor:(uint8_t)minor
{
    Ice::EncodingVersion encoding{major, minor};
    return [[ICEObjectPrx alloc] initWithCppObjectPrx:_objectPrx->ice_encodingVersion(encoding)];
}

-(void) ice_getEncodingVersion:(uint8_t*)major minor:(uint8_t*)minor
{
    Ice::EncodingVersion v = _objectPrx->ice_getEncodingVersion();
    *major = v.major;
    *minor = v.minor;
}

-(ICEObjectPrx*) ice_getRouter
{
    return [[ICEObjectPrx alloc] initWithCppObjectPrx:_objectPrx->ice_getRouter()];
}

-(instancetype) ice_router:(ICEObjectPrx*)router error:(NSError**)error
{
    try
    {
        auto r = router ? [router objectPrx] : nullptr;
        auto prx = _objectPrx->ice_router(Ice::uncheckedCast<Ice::RouterPrx>(r));
        return [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(ICEObjectPrx*) ice_getLocator
{
    return [[ICEObjectPrx alloc] initWithCppObjectPrx:_objectPrx->ice_getLocator()];
}

-(instancetype) ice_locator:(ICEObjectPrx*)locator error:(NSError**)error
{
    try
    {
        auto l = locator ? [locator objectPrx] : nullptr;
        auto prx = _objectPrx->ice_locator(Ice::uncheckedCast<Ice::LocatorPrx>(l));
        return [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(bool) ice_isSecure
{
    return _objectPrx->ice_isSecure();
}

-(instancetype) ice_secure:(bool)b
{
    return [[ICEObjectPrx alloc] initWithCppObjectPrx:_objectPrx->ice_secure(b)];
}

-(bool) ice_isPreferSecure
{
    return _objectPrx->ice_isPreferSecure();
}

-(instancetype) ice_preferSecure:(bool)b error:(NSError**)error
{
    try
    {
        return [[ICEObjectPrx alloc] initWithCppObjectPrx:_objectPrx->ice_preferSecure(b)];
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(bool) ice_isTwoway
{
    return _objectPrx->ice_isTwoway();
}

-(nonnull instancetype) ice_twoway
{
    return [[ICEObjectPrx alloc] initWithCppObjectPrx:_objectPrx->ice_twoway()];
}

-(bool) ice_isOneway
{
    return _objectPrx->ice_isOneway();
}

-(nonnull instancetype) ice_oneway
{
    return [[ICEObjectPrx alloc] initWithCppObjectPrx:_objectPrx->ice_oneway()];
}

-(bool) ice_isBatchOneway
{
    return _objectPrx->ice_isBatchOneway();
}

-(instancetype) ice_batchOneway
{
    return [[ICEObjectPrx alloc] initWithCppObjectPrx:_objectPrx->ice_batchOneway()];
}

-(bool) ice_isDatagram
{
    return _objectPrx->ice_isDatagram();
}

-(instancetype) ice_datagram
{
    return [[ICEObjectPrx alloc] initWithCppObjectPrx:_objectPrx->ice_datagram()];
}

-(bool) ice_isBatchDatagram
{
    return _objectPrx->ice_isBatchDatagram();
}

-(instancetype) ice_batchDatagram
{
    return [[ICEObjectPrx alloc] initWithCppObjectPrx:_objectPrx->ice_batchDatagram()];
}

-(nullable id) ice_getCompress
{
    auto compress = _objectPrx->ice_getCompress();
    if(!compress.has_value())
    {
        return nil;
    }
    return [NSNumber numberWithBool: compress.value() ? YES : NO];
}

-(instancetype) ice_compress:(bool)compress
{
    return [[ICEObjectPrx alloc] initWithCppObjectPrx:_objectPrx->ice_compress(compress)];
}

-(id) ice_getTimeout
{
    auto timeout = _objectPrx->ice_getTimeout();
    if(!timeout.has_value())
    {
        return nil;
    }
    return [NSNumber numberWithInt:timeout.value()];
}

-(instancetype) ice_timeout:(int32_t)timeout error:(NSError**)error
{
    try
    {
        return [[ICEObjectPrx alloc] initWithCppObjectPrx:_objectPrx->ice_timeout(timeout)];
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(instancetype) ice_fixed:(ICEConnection*)connection error:(NSError**)error
{
    try
    {
        auto prx = _objectPrx->ice_fixed([connection connection]);
        return [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(id)ice_getConnection:(NSError**)error
{
    try
    {
        auto c = _objectPrx->ice_getConnection();

        ICEConnection* connection = createLocalObject<Ice::Connection>(c, [&c]() -> id
        {
            return [[ICEConnection alloc] initWithCppConnection:c];
        });

        return connection ? connection : [NSNull null];
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(ICEConnection*) ice_getCachedConnection
{
    auto c = _objectPrx->ice_getCachedConnection();

    return createLocalObject<Ice::Connection>(c, [&c]() -> id
    {
        return [[ICEConnection alloc] initWithCppConnection:c];
    });
}

-(BOOL) ice_flushBatchRequests:(NSError**)error
{
    try
    {
        _objectPrx->ice_flushBatchRequests();
        return YES;
    }
    catch(const std::exception& ex) {
        *error = convertException(ex);
        return NO;
    }
}

+(id) iceRead:(void*)start size:(NSInteger)size communicator:(ICECommunicator*)communicator error:(NSError* _Nullable * _Nullable)error
{

    std::pair<const Ice::Byte*, const Ice::Byte*> p;
    p.first = reinterpret_cast<Ice::Byte*>(start);
    p.second = p.first + size;

    auto comm = [communicator communicator];

//    Ice::InputStream in(deref<Ice::Communicator>(communicator), ev, p);
    //TODO

    try
    {
        // TODO add encoding
        Ice::InputStream in(comm, p);
//        Ice::EncodingVersion ev;
//        getEncodingVersion(encoding, ev);

//        Ice::InputStream in(deref<Ice::Communicator>(communicator), ev, p);
        std::shared_ptr<Ice::ObjectPrx> proxy;
        in.read(proxy);
        if(proxy)
        {
            return [[ICEObjectPrx alloc] initWithCppObjectPrx:proxy];
        }
        else
        {
            return [NSNull null];
        }
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(void) iceWrite:(id<ICEOutputStreamHelper>)os
{
    //
    // Marshal a proxy into a stream and return the encoded bytes.
    //
    try
    {
        auto communicator = _objectPrx->ice_getCommunicator();
        auto encoding = _objectPrx->ice_getEncodingVersion();
        Ice::OutputStream out(communicator, encoding);
        out.write(_objectPrx);
        std::pair<const Ice::Byte*, const Ice::Byte*> p = out.finished();
        int count = static_cast<int>(p.second - p.first);
        [os copy:p.first count:[NSNumber numberWithInt:count]];
    }
    catch(const std::exception& ex)
    {
        //TODO can this actually throw?
        assert(false);
//        *error = convertException(ex);
//        return NO;
    }
}

-(ICEInputStream*) iceInvoke:(NSString*)op
                         mode:(NSInteger)mode
                     inParams:(void*)inParams
                       inSize:(NSInteger)inSize
                      context:(NSDictionary*)context
                  returnValue:(bool*)returnValue
                        error:(NSError**)error
{
    std::pair<const Ice::Byte*, const Ice::Byte*> params(0, 0);
    params.first = reinterpret_cast<Ice::Byte*>(inParams);
    params.second = params.first + inSize;

    try
    {
        Ice::Context ctx;
        if(context)
        {
            fromNSDictionary(context, ctx);
        }
        else
        {
            ctx = Ice::noExplicitContext;
        }

        std::vector<Ice::Byte> v;
        *returnValue = _objectPrx->ice_invoke(fromNSString(op), static_cast<Ice::OperationMode>(mode), params, v, ctx);
        return [[ICEInputStream alloc] initWithBytes:std::move(v)];
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}
@end
