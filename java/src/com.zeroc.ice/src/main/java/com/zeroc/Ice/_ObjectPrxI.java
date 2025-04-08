// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.io.IOException;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.time.Duration;
import java.util.*;
import java.util.concurrent.CompletableFuture;

/** Concrete proxy implementation. */
class _ObjectPrxI implements ObjectPrx, Serializable {
    public _ObjectPrxI(Reference ref) {
        _reference = ref;
        _requestHandlerCache = new RequestHandlerCache(ref);
    }

    public _ObjectPrxI(ObjectPrx obj) {
        _ObjectPrxI source = (_ObjectPrxI) obj;
        _reference = source._reference;
        _requestHandlerCache = source._requestHandlerCache;
    }

    public Communicator ice_getCommunicator() {
        return _reference.getCommunicator();
    }

    public boolean ice_isA(String id) {
        return ice_isA(id, ObjectPrx.noExplicitContext);
    }

    public boolean ice_isA(String id, Map<String, String> context) {
        return _iceI_ice_isAAsync(id, context, true).waitForResponse();
    }

    public CompletableFuture<Boolean> ice_isAAsync(String id) {
        return _iceI_ice_isAAsync(id, ObjectPrx.noExplicitContext, false);
    }

    public CompletableFuture<Boolean> ice_isAAsync(String id, Map<String, String> context) {
        return _iceI_ice_isAAsync(id, context, false);
    }

    private OutgoingAsync<Boolean> _iceI_ice_isAAsync(
            String id, Map<String, String> context, boolean sync) {
        OutgoingAsync<Boolean> f =
            new OutgoingAsync<>(this, "ice_isA", OperationMode.Idempotent, sync, null);
        f.invoke(true, context, null, ostr -> ostr.writeString(id), istr -> istr.readBool());
        return f;
    }

    public void ice_ping() {
        ice_ping(ObjectPrx.noExplicitContext);
    }

    public void ice_ping(Map<String, String> context) {
        _iceI_ice_pingAsync(context, true).waitForResponse();
    }

    public CompletableFuture<Void> ice_pingAsync() {
        return _iceI_ice_pingAsync(ObjectPrx.noExplicitContext, false);
    }

    public CompletableFuture<Void> ice_pingAsync(Map<String, String> context) {
        return _iceI_ice_pingAsync(context, false);
    }

    private OutgoingAsync<Void> _iceI_ice_pingAsync(Map<String, String> context, boolean sync) {
        OutgoingAsync<Void> f =
            new OutgoingAsync<>(this, "ice_ping", OperationMode.Idempotent, sync, null);
        f.invoke(false, context, null, null, null);
        return f;
    }

    public String[] ice_ids() {
        return ice_ids(ObjectPrx.noExplicitContext);
    }

    public String[] ice_ids(Map<String, String> context) {
        return _iceI_ice_idsAsync(context, true).waitForResponse();
    }

    public CompletableFuture<String[]> ice_idsAsync() {
        return _iceI_ice_idsAsync(ObjectPrx.noExplicitContext, false);
    }

    public CompletableFuture<String[]> ice_idsAsync(Map<String, String> context) {
        return _iceI_ice_idsAsync(context, false);
    }

    private OutgoingAsync<String[]> _iceI_ice_idsAsync(Map<String, String> context, boolean sync) {
        OutgoingAsync<String[]> f =
            new OutgoingAsync<>(this, "ice_ids", OperationMode.Idempotent, sync, null);
        f.invoke(true, context, null, null, istr -> istr.readStringSeq());
        return f;
    }

    public String ice_id() {
        return ice_id(ObjectPrx.noExplicitContext);
    }

    public String ice_id(Map<String, String> context) {
        return _iceI_ice_idAsync(context, true).waitForResponse();
    }

    public CompletableFuture<String> ice_idAsync() {
        return _iceI_ice_idAsync(ObjectPrx.noExplicitContext, false);
    }

    public CompletableFuture<String> ice_idAsync(Map<String, String> context) {
        return _iceI_ice_idAsync(context, false);
    }

    private OutgoingAsync<String> _iceI_ice_idAsync(Map<String, String> context, boolean sync) {
        OutgoingAsync<String> f =
            new OutgoingAsync<>(this, "ice_id", OperationMode.Idempotent, sync, null);
        f.invoke(true, context, null, null, istr -> istr.readString());
        return f;
    }

    public Object.Ice_invokeResult ice_invoke(
            String operation, OperationMode mode, byte[] inParams) {
        return ice_invoke(operation, mode, inParams, ObjectPrx.noExplicitContext);
    }

    public Object.Ice_invokeResult ice_invoke(
            String operation, OperationMode mode, byte[] inParams, Map<String, String> context) {
        return _iceI_ice_invokeAsync(operation, mode, inParams, context, true).waitForResponse();
    }

    public CompletableFuture<Object.Ice_invokeResult> ice_invokeAsync(
            String operation, OperationMode mode, byte[] inParams) {
        return ice_invokeAsync(operation, mode, inParams, ObjectPrx.noExplicitContext);
    }

    public CompletableFuture<Object.Ice_invokeResult> ice_invokeAsync(
            String operation, OperationMode mode, byte[] inParams, Map<String, String> context) {
        return _iceI_ice_invokeAsync(operation, mode, inParams, context, false);
    }

    private ProxyIceInvoke _iceI_ice_invokeAsync(
            String operation,
            OperationMode mode,
            byte[] inParams,
            Map<String, String> context,
            boolean sync) {
        ProxyIceInvoke f = new ProxyIceInvoke(this, operation, mode, sync);
        f.invoke(inParams, context);
        return f;
    }

    public Identity ice_getIdentity() {
        return _reference.getIdentity().clone();
    }

    public ObjectPrx ice_identity(Identity newIdentity) {
        if (newIdentity.name == null || newIdentity.name.isEmpty()) {
            throw new IllegalArgumentException(
                "The name of an Ice object identity cannot be empty.");
        }
        if (newIdentity.equals(_reference.getIdentity())) {
            return this;
        } else {
            return new _ObjectPrxI(_reference.changeIdentity(newIdentity));
        }
    }

    public Map<String, String> ice_getContext() {
        return new HashMap<>(_reference.getContext());
    }

    public String ice_getFacet() {
        return _reference.getFacet();
    }

    public ObjectPrx ice_facet(String newFacet) {
        if (newFacet == null) {
            newFacet = "";
        }

        if (newFacet.equals(_reference.getFacet())) {
            return this;
        } else {
            return new _ObjectPrxI(_reference.changeFacet(newFacet));
        }
    }

    public String ice_getAdapterId() {
        return _reference.getAdapterId();
    }

    public Endpoint[] ice_getEndpoints() {
        return _reference.getEndpoints().clone();
    }

    public Duration ice_getLocatorCacheTimeout() {
        return _reference.getLocatorCacheTimeout();
    }

    public Duration ice_getInvocationTimeout() {
        return _reference.getInvocationTimeout();
    }

    public String ice_getConnectionId() {
        return _reference.getConnectionId();
    }

    public boolean ice_isConnectionCached() {
        return _reference.getCacheConnection();
    }

    public EndpointSelectionType ice_getEndpointSelection() {
        return _reference.getEndpointSelection();
    }

    public boolean ice_isFixed() {
        return _reference instanceof FixedReference;
    }

    public boolean ice_isSecure() {
        return _reference.getSecure();
    }

    public EncodingVersion ice_getEncodingVersion() {
        return _reference.getEncoding().clone();
    }

    public boolean ice_isPreferSecure() {
        return _reference.getPreferSecure();
    }

    public RouterPrx ice_getRouter() {
        RouterInfo ri = _reference.getRouterInfo();
        return ri != null ? ri.getRouter() : null;
    }

    public LocatorPrx ice_getLocator() {
        LocatorInfo ri = _reference.getLocatorInfo();
        return ri != null ? ri.getLocator() : null;
    }

    public boolean ice_isCollocationOptimized() {
        return _reference.getCollocationOptimized();
    }

    public boolean ice_isTwoway() {
        return _reference.isTwoway();
    }

    public boolean ice_isOneway() {
        return _reference.getMode() == Reference.ModeOneway;
    }

    public boolean ice_isBatchOneway() {
        return _reference.getMode() == Reference.ModeBatchOneway;
    }

    public boolean ice_isDatagram() {
        return _reference.getMode() == Reference.ModeDatagram;
    }

    public boolean ice_isBatchDatagram() {
        return _reference.getMode() == Reference.ModeBatchDatagram;
    }

    public Optional<Boolean> ice_getCompress() {
        return _reference.getCompress();
    }

    public Connection ice_getConnection() {
        return _iceI_ice_getConnectionAsync().waitForResponse();
    }

    public CompletableFuture<Connection> ice_getConnectionAsync() {
        return _iceI_ice_getConnectionAsync();
    }

    private ProxyGetConnection _iceI_ice_getConnectionAsync() {
        ProxyGetConnection r = new ProxyGetConnection(this);
        r.invoke();
        return r;
    }

    public Connection ice_getCachedConnection() {
        return _requestHandlerCache.getCachedConnection();
    }

    public void ice_flushBatchRequests() {
        _iceI_ice_flushBatchRequestsAsync().waitForResponse();
    }

    public CompletableFuture<Void> ice_flushBatchRequestsAsync() {
        return _iceI_ice_flushBatchRequestsAsync();
    }

    private ProxyFlushBatch _iceI_ice_flushBatchRequestsAsync() {
        ProxyFlushBatch f = new ProxyFlushBatch(this);
        try {
            f.invoke();
        } catch (LocalException ex) {
            f.abort(ex);
        }
        return f;
    }

    @Override
    public boolean equals(java.lang.Object r) {
        if (this == r) {
            return true;
        }

        if (r instanceof _ObjectPrxI) {
            return _reference.equals(((_ObjectPrxI) r)._reference);
        }

        return false;
    }

    @Override
    public final int hashCode() {
        return _reference.hashCode();
    }

    @Override
    public final String toString() {
        return _reference.toString();
    }

    @Override
    public void _write(OutputStream os) {
        _reference.getIdentity().ice_writeMembers(os);
        _reference.streamWrite(os);
    }

    @Override
    public Reference _getReference() {
        return _reference;
    }

    @Override
    public ObjectPrx ice_context(Map<String, String> newContext) {
        return _newInstance(_reference.changeContext(newContext));
    }

    @Override
    public ObjectPrx ice_adapterId(String newAdapterId) {
        if (newAdapterId == null) {
            newAdapterId = "";
        }

        if (newAdapterId.equals(_reference.getAdapterId())) {
            return this;
        } else {
            return _newInstance(_reference.changeAdapterId(newAdapterId));
        }
    }

    @Override
    public ObjectPrx ice_endpoints(Endpoint[] newEndpoints) {
        if (Arrays.equals(newEndpoints, _reference.getEndpoints())) {
            return this;
        } else {
            EndpointI[] edpts = new EndpointI[newEndpoints.length];
            edpts = Arrays.asList(newEndpoints).toArray(edpts);
            return _newInstance(_reference.changeEndpoints(edpts));
        }
    }

    @Override
    public ObjectPrx ice_fixed(Connection connection) {
        if (connection == null) {
            throw new IllegalArgumentException("invalid null connection passed to ice_fixed");
        }
        if (!(connection instanceof ConnectionI)) {
            throw new IllegalArgumentException("invalid connection passed to ice_fixed");
        }
        if (connection == _reference.getConnection()) {
            return this;
        } else {
            return _newInstance(
                _reference.changeConnection((ConnectionI) connection));
        }
    }

    @Override
    public ObjectPrx ice_locatorCacheTimeout(int newTimeout) {
        return this.ice_locatorCacheTimeout(Duration.ofSeconds(newTimeout));
    }

    @Override
    public ObjectPrx ice_locatorCacheTimeout(Duration newTimeout) {
        if (newTimeout.equals(_reference.getLocatorCacheTimeout())) {
            return this;
        } else {
            return _newInstance(_reference.changeLocatorCacheTimeout(newTimeout));
        }
    }

    @Override
    public ObjectPrx ice_invocationTimeout(int newTimeout) {
        return this.ice_invocationTimeout(Duration.ofMillis(newTimeout));
    }

    @Override
    public ObjectPrx ice_invocationTimeout(Duration newTimeout) {
        if (newTimeout.equals(_reference.getInvocationTimeout())) {
            return this;
        } else {
            return _newInstance(_reference.changeInvocationTimeout(newTimeout));
        }
    }

    @Override
    public ObjectPrx ice_connectionCached(boolean newCache) {
        if (newCache == _reference.getCacheConnection()) {
            return this;
        } else {
            return _newInstance(_reference.changeCacheConnection(newCache));
        }
    }

    @Override
    public ObjectPrx ice_endpointSelection(EndpointSelectionType newType) {
        if (newType == _reference.getEndpointSelection()) {
            return this;
        } else {
            return _newInstance(_reference.changeEndpointSelection(newType));
        }
    }

    @Override
    public ObjectPrx ice_secure(boolean b) {
        if (b == _reference.getSecure()) {
            return this;
        } else {
            return _newInstance(_reference.changeSecure(b));
        }
    }

    @Override
    public ObjectPrx ice_encodingVersion(EncodingVersion e) {
        if (e.equals(_reference.getEncoding())) {
            return this;
        } else {
            return _newInstance(_reference.changeEncoding(e));
        }
    }

    @Override
    public ObjectPrx ice_preferSecure(boolean b) {
        if (b == _reference.getPreferSecure()) {
            return this;
        } else {
            return _newInstance(_reference.changePreferSecure(b));
        }
    }

    @Override
    public ObjectPrx ice_router(RouterPrx newRouter) {
        RouterInfo routerInfo = _reference.getRouterInfo();
        RouterPrx router = routerInfo != null ? routerInfo.getRouter() : null;
        if (router == newRouter || (router != null && router.equals(newRouter))) {
            return this;
        } else {
            return _newInstance(_reference.changeRouter(newRouter));
        }
    }

    @Override
    public ObjectPrx ice_locator(LocatorPrx newLocator) {
        LocatorInfo locatorInfo = _reference.getLocatorInfo();
        LocatorPrx locator = locatorInfo != null ? locatorInfo.getLocator() : null;
        if (locator == newLocator || (locator != null && locator.equals(newLocator))) {
            return this;
        } else {
            return _newInstance(_reference.changeLocator(newLocator));
        }
    }

    @Override
    public ObjectPrx ice_collocationOptimized(boolean b) {
        if (b == _reference.getCollocationOptimized()) {
            return this;
        } else {
            return _newInstance(_reference.changeCollocationOptimized(b));
        }
    }

    @Override
    public ObjectPrx ice_twoway() {
        return ice_isTwoway() ? this : _newInstance(_reference.changeMode(Reference.ModeTwoway));
    }

    @Override
    public ObjectPrx ice_oneway() {
        return ice_isOneway() ? this : _newInstance(_reference.changeMode(Reference.ModeOneway));
    }

    @Override
    public ObjectPrx ice_batchOneway() {
        return ice_isBatchOneway()
            ? this
            : _newInstance(_reference.changeMode(Reference.ModeBatchOneway));
    }

    @Override
    public ObjectPrx ice_datagram() {
        return ice_isDatagram()
            ? this
            : _newInstance(_reference.changeMode(Reference.ModeDatagram));
    }

    @Override
    public ObjectPrx ice_batchDatagram() {
        return ice_isBatchDatagram()
            ? this
            : _newInstance(_reference.changeMode(Reference.ModeBatchDatagram));
    }

    @Override
    public ObjectPrx ice_compress(boolean compress) {
        var value = _reference.getCompress();
        if (value.isPresent() && value.get() == compress) {
            return this;
        } else {
            return _newInstance(_reference.changeCompress(compress));
        }
    }

    @Override
    public ObjectPrx ice_connectionId(String connectionId) {
        if (connectionId.equals(_reference.getConnectionId())) {
            return this;
        } else {
            return _newInstance(_reference.changeConnectionId(connectionId));
        }
    }

    public StreamPair _getCachedMessageBuffers() {
        synchronized (this) {
            if (_streamCache != null && !_streamCache.isEmpty()) {
                return _streamCache.remove(0);
            }
        }
        return null;
    }

    public void _cacheMessageBuffers(InputStream is, OutputStream os) {
        synchronized (this) {
            if (_streamCache == null) {
                _streamCache = new LinkedList<>();
            }
            _streamCache.add(new StreamPair(is, os));
        }
    }

    public RequestHandlerCache _getRequestHandlerCache() {
        return _requestHandlerCache;
    }

    private void writeObject(ObjectOutputStream out) throws IOException {
        out.writeUTF(toString());
    }

    private void readObject(java.io.ObjectInputStream in)
        throws IOException, ClassNotFoundException {
        String s = in.readUTF();
        try {
            Communicator communicator = ((ObjectInputStream) in).getCommunicator();
            if (communicator == null) {
                throw new IOException("Cannot deserialize proxy: no communicator provided");
            }
            var ref = communicator.getInstance().referenceFactory().create(s, null);
            var proxy = new _ObjectPrxI(ref);
            _reference = proxy._reference;
            _requestHandlerCache = proxy._requestHandlerCache;
        } catch (ClassCastException ex) {
            IOException e =
                new IOException(
                    "Cannot deserialize proxy: ObjectInputStream not found");
            e.initCause(ex);
            throw e;
        } catch (LocalException ex) {
            IOException e =
                new IOException("Failure occurred while deserializing proxy");
            e.initCause(ex);
            throw e;
        }
    }

    static class StreamPair {
        StreamPair(InputStream is, OutputStream os) {
            this.is = is;
            this.os = os;
        }

        public InputStream is;
        public OutputStream os;
    }

    protected transient Reference _reference;
    private transient RequestHandlerCache _requestHandlerCache;
    private transient List<StreamPair> _streamCache;
    private static final long serialVersionUID = 0L;
}
