// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * Provides overloads of the proxy factory methods with covariant return types so that applications
 * do not need to downcast to the derived proxy type.
 *
 * @hidden Public because it's used by the generated code.
 */
@SuppressWarnings("unchecked")
public abstract class _ObjectPrxFactoryMethods<T extends ObjectPrx> extends _ObjectPrxI {
    protected _ObjectPrxFactoryMethods(Reference ref) {
        super(ref);
    }

    protected _ObjectPrxFactoryMethods(ObjectPrx obj) {
        super(obj);
    }

    @Override
    public T ice_context(java.util.Map<String, String> newContext) {
        return (T) super.ice_context(newContext);
    }

    @Override
    public T ice_adapterId(String newAdapterId) {
        return (T) super.ice_adapterId(newAdapterId);
    }

    @Override
    public T ice_endpoints(Endpoint[] newEndpoints) {
        return (T) super.ice_endpoints(newEndpoints);
    }

    @Override
    public T ice_locatorCacheTimeout(int newTimeout) {
        return (T) super.ice_locatorCacheTimeout(newTimeout);
    }

    @Override
    public T ice_invocationTimeout(int newTimeout) {
        return (T) super.ice_invocationTimeout(newTimeout);
    }

    @Override
    public T ice_connectionCached(boolean newCache) {
        return (T) super.ice_connectionCached(newCache);
    }

    @Override
    public T ice_endpointSelection(EndpointSelectionType newType) {
        return (T) super.ice_endpointSelection(newType);
    }

    @Override
    public T ice_secure(boolean b) {
        return (T) super.ice_secure(b);
    }

    @Override
    public T ice_encodingVersion(EncodingVersion e) {
        return (T) super.ice_encodingVersion(e);
    }

    @Override
    public T ice_preferSecure(boolean b) {
        return (T) super.ice_preferSecure(b);
    }

    @Override
    public T ice_router(RouterPrx router) {
        return (T) super.ice_router(router);
    }

    @Override
    public T ice_locator(LocatorPrx locator) {
        return (T) super.ice_locator(locator);
    }

    @Override
    public T ice_collocationOptimized(boolean b) {
        return (T) super.ice_collocationOptimized(b);
    }

    @Override
    public T ice_twoway() {
        return (T) super.ice_twoway();
    }

    @Override
    public T ice_oneway() {
        return (T) super.ice_oneway();
    }

    @Override
    public T ice_batchOneway() {
        return (T) super.ice_batchOneway();
    }

    @Override
    public T ice_datagram() {
        return (T) super.ice_datagram();
    }

    @Override
    public T ice_batchDatagram() {
        return (T) super.ice_batchDatagram();
    }

    @Override
    public T ice_compress(boolean co) {
        return (T) super.ice_compress(co);
    }

    @Override
    public T ice_connectionId(String connectionId) {
        return (T) super.ice_connectionId(connectionId);
    }

    @Override
    public T ice_fixed(com.zeroc.Ice.Connection connection) {
        return (T) super.ice_fixed(connection);
    }

    private static final long serialVersionUID = 0L;
}
