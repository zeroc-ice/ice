//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * @hidden
 * Provides overloads of the proxy factory methods with covariant return types
 * so that applications do not need to downcast to the derived proxy type.
 **/
@SuppressWarnings("unchecked")
public interface _ObjectPrxFactoryMethods<T extends ObjectPrx> extends ObjectPrx
{
    @Override
    default T ice_context(java.util.Map<String, String> newContext)
    {
        return (T)ObjectPrx.super.ice_context(newContext);
    }

    @Override
    default T ice_adapterId(String newAdapterId)
    {
        return (T)ObjectPrx.super.ice_adapterId(newAdapterId);
    }

    @Override
    default T ice_endpoints(Endpoint[] newEndpoints)
    {
        return (T)ObjectPrx.super.ice_endpoints(newEndpoints);
    }

    @Override
    default T ice_locatorCacheTimeout(int newTimeout)
    {
        return (T)ObjectPrx.super.ice_locatorCacheTimeout(newTimeout);
    }

    @Override
    default T ice_invocationTimeout(int newTimeout)
    {
        return (T)ObjectPrx.super.ice_invocationTimeout(newTimeout);
    }

    @Override
    default T ice_connectionCached(boolean newCache)
    {
        return (T)ObjectPrx.super.ice_connectionCached(newCache);
    }

    @Override
    default T ice_endpointSelection(EndpointSelectionType newType)
    {
        return (T)ObjectPrx.super.ice_endpointSelection(newType);
    }

    @Override
    default T ice_secure(boolean b)
    {
        return (T)ObjectPrx.super.ice_secure(b);
    }

    @Override
    default T ice_encodingVersion(EncodingVersion e)
    {
        return (T)ObjectPrx.super.ice_encodingVersion(e);
    }

    @Override
    default T ice_preferSecure(boolean b)
    {
        return (T)ObjectPrx.super.ice_preferSecure(b);
    }

    @Override
    default T ice_router(RouterPrx router)
    {
        return (T)ObjectPrx.super.ice_router(router);
    }

    @Override
    default T ice_locator(LocatorPrx locator)
    {
        return (T)ObjectPrx.super.ice_locator(locator);
    }

    @Override
    default T ice_collocationOptimized(boolean b)
    {
        return (T)ObjectPrx.super.ice_collocationOptimized(b);
    }

    @Override
    default T ice_twoway()
    {
        return (T)ObjectPrx.super.ice_twoway();
    }

    @Override
    default T ice_oneway()
    {
        return (T)ObjectPrx.super.ice_oneway();
    }

    @Override
    default T ice_batchOneway()
    {
        return (T)ObjectPrx.super.ice_batchOneway();
    }

    @Override
    default T ice_datagram()
    {
        return (T)ObjectPrx.super.ice_datagram();
    }

    @Override
    default T ice_batchDatagram()
    {
        return (T)ObjectPrx.super.ice_batchDatagram();
    }

    @Override
    default T ice_compress(boolean co)
    {
        return (T)ObjectPrx.super.ice_compress(co);
    }

    @Override
    default T ice_timeout(int t)
    {
        return (T)ObjectPrx.super.ice_timeout(t);
    }

    @Override
    default T ice_connectionId(String connectionId)
    {
        return (T)ObjectPrx.super.ice_connectionId(connectionId);
    }
}
