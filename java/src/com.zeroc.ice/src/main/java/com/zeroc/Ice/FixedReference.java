// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.time.Duration;

public class FixedReference extends Reference {
    public FixedReference(
            Instance instance,
            Communicator communicator,
            Identity identity,
            String facet,
            int mode,
            boolean secure,
            java.util.Optional<Boolean> compress,
            ProtocolVersion protocol,
            EncodingVersion encoding,
            ConnectionI connection,
            Duration invocationTimeout,
            java.util.Map<String, String> context) {
        super(
                instance,
                communicator,
                identity,
                facet,
                mode,
                secure,
                compress,
                protocol,
                encoding,
                invocationTimeout,
                context);
        _fixedConnection = connection;
    }

    @Override
    public EndpointI[] getEndpoints() {
        return _emptyEndpoints;
    }

    @Override
    public String getAdapterId() {
        return "";
    }

    @Override
    public LocatorInfo getLocatorInfo() {
        return null;
    }

    @Override
    public RouterInfo getRouterInfo() {
        return null;
    }

    @Override
    public boolean getCollocationOptimized() {
        return false;
    }

    @Override
    public final boolean getCacheConnection() {
        return true;
    }

    @Override
    public boolean getPreferSecure() {
        return false;
    }

    @Override
    public final EndpointSelectionType getEndpointSelection() {
        return EndpointSelectionType.Random;
    }

    @Override
    public Duration getLocatorCacheTimeout() {
        return Duration.ZERO;
    }

    @Override
    public String getConnectionId() {
        return "";
    }

    @Override
    public ThreadPool getThreadPool() {
        return _fixedConnection.getThreadPool();
    }

    @Override
    public ConnectionI getConnection() {
        return _fixedConnection;
    }

    @Override
    public Reference changeEndpoints(EndpointI[] newEndpoints) {
        throw new FixedProxyException();
    }

    @Override
    public Reference changeAdapterId(String newAdapterId) {
        throw new FixedProxyException();
    }

    @Override
    public Reference changeLocator(LocatorPrx newLocator) {
        throw new FixedProxyException();
    }

    @Override
    public Reference changeRouter(RouterPrx newRouter) {
        throw new FixedProxyException();
    }

    @Override
    public Reference changeCollocationOptimized(boolean newCollocationOptimized) {
        throw new FixedProxyException();
    }

    @Override
    public final Reference changeCacheConnection(boolean newCache) {
        throw new FixedProxyException();
    }

    @Override
    public Reference changePreferSecure(boolean prefSec) {
        throw new FixedProxyException();
    }

    @Override
    public final Reference changeEndpointSelection(EndpointSelectionType newType) {
        throw new FixedProxyException();
    }

    @Override
    public Reference changeLocatorCacheTimeout(Duration newTimeout) {
        throw new FixedProxyException();
    }

    @Override
    public Reference changeConnectionId(String connectionId) {
        throw new FixedProxyException();
    }

    @Override
    public Reference changeConnection(ConnectionI connection) {
        FixedReference r = (FixedReference) getInstance().referenceFactory().copy(this);
        r._fixedConnection = connection;
        return r;
    }

    @Override
    public boolean isIndirect() {
        return false;
    }

    @Override
    public boolean isWellKnown() {
        return false;
    }

    @Override
    public void streamWrite(OutputStream s) throws MarshalException {
        throw new FixedProxyException();
    }

    @Override
    public java.util.Map<String, String> toProperty(String prefix) {
        throw new FixedProxyException();
    }

    @Override
    RequestHandler getRequestHandler() {
        // We need to perform all these checks here and not in the constructor because
        // `changeConnection()` clones then sets the connection.

        switch (getMode()) {
            case Reference.ModeTwoway:
            case Reference.ModeOneway:
            case Reference.ModeBatchOneway:
                {
                    if (_fixedConnection.endpoint().datagram()) {
                        throw NoEndpointException.fromProxyString(toString());
                    }
                    break;
                }

            case Reference.ModeDatagram:
            case Reference.ModeBatchDatagram:
                {
                    if (!_fixedConnection.endpoint().datagram()) {
                        throw NoEndpointException.fromProxyString(toString());
                    }
                    break;
                }
        }

        //
        // If a secure connection is requested or secure overrides is set, check if the connection is secure.
        //
        boolean secure;
        DefaultsAndOverrides defaultsAndOverrides = getInstance().defaultsAndOverrides();
        if (defaultsAndOverrides.overrideSecure.isPresent()) {
            secure = defaultsAndOverrides.overrideSecure.get();
        } else {
            secure = getSecure();
        }
        if (secure && !_fixedConnection.endpoint().secure()) {
            throw NoEndpointException.fromProxyString(toString());
        }

        _fixedConnection.throwException(); // Throw in case our connection is already destroyed.

        boolean compress =
                defaultsAndOverrides.overrideCompress.isPresent()
                        ? defaultsAndOverrides.overrideCompress.get()
                        : getCompress().orElse(false);
        return new ConnectionRequestHandler(this, _fixedConnection, compress);
    }

    @Override
    public BatchRequestQueue getBatchRequestQueue() {
        return _fixedConnection.getBatchRequestQueue();
    }

    @Override
    public boolean equals(java.lang.Object obj) {
        if (this == obj) {
            return true;
        }
        if (!(obj instanceof FixedReference)) {
            return false;
        }
        FixedReference rhs = (FixedReference) obj;
        if (!super.equals(rhs)) {
            return false;
        }
        return _fixedConnection.equals(rhs._fixedConnection);
    }

    private ConnectionI _fixedConnection;
    private static EndpointI[] _emptyEndpoints = new EndpointI[0];
}
