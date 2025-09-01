// Copyright (c) ZeroC, Inc.

#import "include/ObjectPrx.h"
#import "Convert.h"
#import "include/Communicator.h"
#import "include/Connection.h"
#import "include/OutputStream.h"

#include <chrono>

@implementation ICEObjectPrx

- (ICEObjectPrx*)initWithObjectPrx:(ICEObjectPrx*)prx
{
    assert(prx);
    self = [super init];
    _prx = [prx prx];
    return self;
}

- (ICEObjectPrx*)initWithCppObjectPrx:(const Ice::ObjectPrx&)prx
{
    self = [super init];
    self->_prx = prx;

    return self;
}

- (nonnull NSString*)ice_toString
{
    return toNSString(_prx->ice_toString());
}

- (ICECommunicator*)ice_getCommunicator
{
    auto comm = _prx->ice_getCommunicator();
    return [ICECommunicator getHandle:comm];
}

- (void)ice_getIdentity:(NSString* __strong _Nonnull* _Nonnull)name
               category:(NSString* __strong _Nonnull* _Nonnull)category
{
    auto identity = _prx->ice_getIdentity();
    *name = toNSString(identity.name);
    *category = toNSString(identity.category);
}

- (instancetype)ice_identity:(NSString*)name category:(NSString*)category error:(NSError**)error
{
    try
    {
        auto prx = _prx->ice_identity(Ice::Identity{fromNSString(name), fromNSString(category)});
        return _prx == prx ? self : [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
    }
    catch (...)
    {
        *error = convertException(std::current_exception());
        return nil;
    }
}

- (NSDictionary<NSString*, NSString*>*)ice_getContext
{
    return toNSDictionary(_prx->ice_getContext());
}

- (instancetype)ice_context:(NSDictionary<NSString*, NSString*>*)context
{
    Ice::Context ctx;
    fromNSDictionary(context, ctx);

    auto prx = _prx->ice_context(ctx);
    return _prx == prx ? self : [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
}

- (NSString*)ice_getFacet
{
    return toNSString(_prx->ice_getFacet());
}

- (instancetype)ice_facet:(NSString*)facet
{
    auto prx = _prx->ice_facet(fromNSString(facet));
    return _prx == prx ? self : [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
}

- (NSString*)ice_getAdapterId
{
    return toNSString(_prx->ice_getAdapterId());
}

- (instancetype)ice_adapterId:(NSString*)id error:(NSError**)error
{
    try
    {
        auto prx = _prx->ice_adapterId(fromNSString(id));
        return _prx == prx ? self : [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
    }
    catch (...)
    {
        *error = convertException(std::current_exception());
        return nil;
    }
}

- (NSArray<ICEEndpoint*>*)ice_getEndpoints
{
    return toNSArray(_prx->ice_getEndpoints());
}

- (instancetype)ice_endpoints:(NSArray<ICEEndpoint*>*)endpoints error:(NSError**)error
{
    try
    {
        Ice::EndpointSeq endpts;
        fromNSArray(endpoints, endpts);

        auto prx = _prx->ice_endpoints(endpts);
        return _prx == prx ? self : [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
    }
    catch (...)
    {
        *error = convertException(std::current_exception());
        return nil;
    }
}

- (int32_t)ice_getLocatorCacheTimeout
{
    std::chrono::seconds timeout = std::chrono::duration_cast<std::chrono::seconds>(_prx->ice_getLocatorCacheTimeout());
    return static_cast<int32_t>(timeout.count());
}

- (instancetype)ice_locatorCacheTimeout:(int32_t)timeout error:(NSError**)error
{
    try
    {
        auto prx = _prx->ice_locatorCacheTimeout(timeout);
        return _prx == prx ? self : [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
    }
    catch (...)
    {
        *error = convertException(std::current_exception());
        return nil;
    }
}

- (int32_t)ice_getInvocationTimeout
{
    return static_cast<int32_t>(_prx->ice_getInvocationTimeout().count());
}

- (instancetype)ice_invocationTimeout:(int32_t)timeout error:(NSError**)error
{
    try
    {
        auto prx = _prx->ice_invocationTimeout(timeout);
        return _prx == prx ? self : [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
    }
    catch (...)
    {
        *error = convertException(std::current_exception());
        return nil;
    }
}

- (NSString*)ice_getConnectionId
{
    return toNSString(_prx->ice_getConnectionId());
}

- (instancetype)ice_connectionId:(NSString*)connectionId error:(NSError**)error
{
    try
    {
        auto prx = _prx->ice_connectionId(fromNSString(connectionId));
        return _prx == prx ? self : [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
    }
    catch (...)
    {
        *error = convertException(std::current_exception());
        return nil;
    }
}

- (bool)ice_isConnectionCached
{
    return _prx->ice_isConnectionCached();
}

- (instancetype)ice_connectionCached:(bool)cached error:(NSError**)error
{
    try
    {
        auto prx = _prx->ice_connectionCached(cached);
        return _prx == prx ? self : [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
    }
    catch (...)
    {
        *error = convertException(std::current_exception());
        return nil;
    }
}

- (std::uint8_t)ice_getEndpointSelection
{
    return static_cast<std::uint8_t>(_prx->ice_getEndpointSelection());
}

- (instancetype)ice_endpointSelection:(std::uint8_t)type error:(NSError**)error
{
    try
    {
        auto prx = _prx->ice_endpointSelection(Ice::EndpointSelectionType(type));
        return _prx == prx ? self : [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
    }
    catch (...)
    {
        *error = convertException(std::current_exception());
        return nil;
    }
}

- (instancetype)ice_encodingVersion:(std::uint8_t)major minor:(std::uint8_t)minor
{
    Ice::EncodingVersion encoding{major, minor};

    auto prx = _prx->ice_encodingVersion(encoding);
    return _prx == prx ? self : [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
}

- (void)ice_getEncodingVersion:(std::uint8_t*)major minor:(std::uint8_t*)minor
{
    Ice::EncodingVersion v = _prx->ice_getEncodingVersion();
    *major = v.major;
    *minor = v.minor;
}

- (ICEObjectPrx*)ice_getRouter
{
    std::optional<Ice::RouterPrx> router = _prx->ice_getRouter();
    if (router)
    {
        return [[ICEObjectPrx alloc] initWithCppObjectPrx:router.value()];
    }
    else
    {
        return nil;
    }
}

- (instancetype)ice_router:(ICEObjectPrx*)router error:(NSError**)error
{
    try
    {
        std::optional<Ice::ObjectPrx> r;
        if (router)
        {
            r = [router prx];
        }
        auto prx = _prx->ice_router(Ice::uncheckedCast<Ice::RouterPrx>(r));
        return _prx == prx ? self : [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
    }
    catch (...)
    {
        *error = convertException(std::current_exception());
        return nil;
    }
}

- (ICEObjectPrx*)ice_getLocator
{
    std::optional<Ice::LocatorPrx> locator = _prx->ice_getLocator();

    if (locator)
    {
        return [[ICEObjectPrx alloc] initWithCppObjectPrx:locator.value()];
    }
    else
    {
        return nil;
    }
}

- (instancetype)ice_locator:(ICEObjectPrx*)locator error:(NSError**)error
{
    try
    {
        std::optional<Ice::ObjectPrx> l;
        if (locator)
        {
            l = [locator prx];
        }
        auto prx = _prx->ice_locator(Ice::uncheckedCast<Ice::LocatorPrx>(l));
        return _prx == prx ? self : [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
    }
    catch (...)
    {
        *error = convertException(std::current_exception());
        return nil;
    }
}

- (bool)ice_isTwoway
{
    return _prx->ice_isTwoway();
}

- (nonnull instancetype)ice_twoway
{
    auto prx = _prx->ice_twoway();
    return _prx == prx ? self : [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
}

- (bool)ice_isOneway
{
    return _prx->ice_isOneway();
}

- (nonnull instancetype)ice_oneway
{
    auto prx = _prx->ice_oneway();
    return _prx == prx ? self : [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
}

- (bool)ice_isBatchOneway
{
    return _prx->ice_isBatchOneway();
}

- (instancetype)ice_batchOneway
{
    auto prx = _prx->ice_batchOneway();
    return _prx == prx ? self : [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
}

- (bool)ice_isDatagram
{
    return _prx->ice_isDatagram();
}

- (instancetype)ice_datagram
{
    auto prx = _prx->ice_datagram();
    return _prx == prx ? self : [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
}

- (bool)ice_isBatchDatagram
{
    return _prx->ice_isBatchDatagram();
}

- (instancetype)ice_batchDatagram
{
    auto prx = _prx->ice_batchDatagram();
    return _prx == prx ? self : [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
}

- (nullable id)ice_getCompress
{
    auto compress = _prx->ice_getCompress();
    if (!compress.has_value())
    {
        return nil;
    }
    return [NSNumber numberWithBool:compress.value() ? YES : NO];
}

- (instancetype)ice_compress:(bool)compress
{
    auto prx = _prx->ice_compress(compress);
    return _prx == prx ? self : [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
}

- (instancetype)ice_fixed:(ICEConnection*)connection error:(NSError**)error
{
    try
    {
        auto prx = _prx->ice_fixed([connection connection]);
        return _prx == prx ? self : [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
    }
    catch (...)
    {
        *error = convertException(std::current_exception());
        return nil;
    }
}

- (bool)ice_isFixed
{
    return _prx->ice_isFixed();
}

- (void)ice_getConnection:(void (^)(ICEConnection* _Nullable))response exception:(void (^)(NSError*))exception
{
    try
    {
        _prx->ice_getConnectionAsync(
            [response](std::shared_ptr<Ice::Connection> cppConnection)
            {
                @autoreleasepool
                {
                    response([ICEConnection getHandle:cppConnection]);
                }
            },
            [exception](std::exception_ptr e)
            {
                @autoreleasepool
                {
                    exception(convertException(e));
                }
            });
    }
    catch (...)
    {
        // Typically CommunicatorDestroyedException.
        exception(convertException(std::current_exception()));
    }
}

- (ICEConnection*)ice_getCachedConnection
{
    auto cppConnection = _prx->ice_getCachedConnection();
    return [ICEConnection getHandle:cppConnection];
}

- (void)ice_flushBatchRequests:(void (^)(NSError*))exception sent:(void (^)(bool))sent
{
    try
    {
        _prx->ice_flushBatchRequestsAsync(
            [exception](std::exception_ptr e)
            {
                @autoreleasepool
                {
                    exception(convertException(e));
                }
            },
            [sent](bool sentSynchronously) { sent(sentSynchronously); });
    }
    catch (...)
    {
        // Typically CommunicatorDestroyedException.
        exception(convertException(std::current_exception()));
    }
}

- (bool)ice_isCollocationOptimized
{
    return _prx->ice_isCollocationOptimized();
}

- (instancetype)ice_collocationOptimized:(bool)collocated error:(NSError* _Nullable* _Nullable)error
{
    try
    {
        auto prx = _prx->ice_collocationOptimized(collocated);
        return _prx == prx ? self : [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
    }
    catch (...)
    {
        *error = convertException(std::current_exception());
        return nil;
    }
}

+ (id)ice_read:(NSData*)data
     communicator:(ICECommunicator*)communicator
    encodingMajor:(std::uint8_t)major
    encodingMinor:(std::uint8_t)minor
        bytesRead:(NSInteger*)bytesRead
            error:(NSError**)error
{
    std::pair<const std::byte*, const std::byte*> p;
    p.first = static_cast<const std::byte*>(data.bytes);
    p.second = p.first + data.length;

    auto comm = [communicator communicator];

    try
    {
        Ice::InputStream ins(comm, Ice::EncodingVersion{major, minor}, p);

        std::optional<Ice::ObjectPrx> proxy;
        ins.read(proxy);

        *bytesRead = ins.pos();
        if (proxy)
        {
            return [[ICEObjectPrx alloc] initWithCppObjectPrx:proxy.value()];
        }
        else
        {
            return [NSNull null];
        }
    }
    catch (...)
    {
        *error = convertException(std::current_exception());
        return nil;
    }
}

- (void)ice_write:(id<ICEOutputStreamHelper>)os
    encodingMajor:(std::uint8_t)encodingMajor
    encodingMinor:(std::uint8_t)encodingMinor
{
    //
    // Marshal a proxy into a stream and return the encoded bytes.
    //
    auto communicator = _prx->ice_getCommunicator();
    Ice::EncodingVersion encoding{encodingMajor, encodingMinor};
    Ice::OutputStream out(communicator, encoding);
    out.write(_prx);
    auto p = out.finished();
    [os copy:p.first count:static_cast<long>(p.second - p.first)];
}

- (BOOL)enqueueBatch:(NSString*)op
                mode:(std::uint8_t)mode
            inParams:(NSData*)inParams
             context:(NSDictionary*)context
               error:(NSError**)error
{
    assert(_prx->ice_isBatchOneway() || _prx->ice_isBatchDatagram());

    std::pair<const std::byte*, const std::byte*> params(0, 0);
    params.first = static_cast<const std::byte*>(inParams.bytes);
    params.second = params.first + inParams.length;

    try
    {
        Ice::Context ctx;
        if (context)
        {
            fromNSDictionary(context, ctx);
        }

        std::vector<std::byte> ignored;
        // So long as the proxy is batch oneway or batch datagram this will never block.
        _prx->ice_invoke(
            fromNSString(op),
            static_cast<Ice::OperationMode>(mode),
            params,
            ignored,
            context ? ctx : Ice::noExplicitContext);
        return YES;
    }
    catch (...)
    {
        *error = convertException(std::current_exception());
        return NO;
    }
}

- (void)invoke:(NSString* _Nonnull)op
          mode:(std::uint8_t)mode
      inParams:(NSData*)inParams
       context:(NSDictionary* _Nullable)context
      response:(void (^)(bool, void*, long))response
     exception:(void (^)(NSError*))exception
          sent:(void (^_Nullable)(bool))sent
{
    assert(!_prx->ice_isBatchOneway() && !_prx->ice_isBatchDatagram());

    std::pair<const std::byte*, const std::byte*> params(0, 0);
    params.first = static_cast<const std::byte*>(inParams.bytes);
    params.second = params.first + inParams.length;

    try
    {
        Ice::Context ctx;
        if (context)
        {
            fromNSDictionary(context, ctx);
        }

        _prx->ice_invokeAsync(
            fromNSString(op),
            static_cast<Ice::OperationMode>(mode),
            params,
            [response](bool ok, std::pair<const std::byte*, const std::byte*> outParams)
            {
                // We need an autorelease pool in case the unmarshaling creates auto
                // release objects.
                @autoreleasepool
                {
                    response(
                        ok,
                        const_cast<std::byte*>(outParams.first),
                        static_cast<long>(outParams.second - outParams.first));
                }
            },
            [exception](std::exception_ptr e)
            {
                @autoreleasepool
                {
                    exception(convertException(e));
                }
            },
            [sent](bool sentSynchronously)
            {
                if (sent)
                {
                    sent(sentSynchronously);
                }
            },
            context ? ctx : Ice::noExplicitContext);
    }
    catch (...)
    {
        // Typically CommunicatorDestroyedException.
        exception(convertException(std::current_exception()));
    }
}

- (bool)isEqual:(ICEObjectPrx*)other
{
    return *_prx == *other.prx;
}

- (bool)proxyIdentityLess:(ICEObjectPrx*)other
{
    return Ice::proxyIdentityLess(_prx, other.prx);
}

- (bool)proxyIdentityEqual:(ICEObjectPrx*)other
{
    return Ice::proxyIdentityEqual(_prx, other.prx);
}

- (bool)proxyIdentityAndFacetLess:(ICEObjectPrx*)other
{
    return Ice::proxyIdentityAndFacetLess(_prx, other.prx);
}

- (bool)proxyIdentityAndFacetEqual:(ICEObjectPrx*)other
{
    return Ice::proxyIdentityAndFacetEqual(_prx, other.prx);
}
@end
