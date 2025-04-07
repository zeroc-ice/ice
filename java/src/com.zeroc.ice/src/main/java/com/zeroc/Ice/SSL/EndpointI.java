// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice.SSL;

import com.zeroc.Ice.*;

import java.util.ArrayList;
import java.util.List;
import java.util.stream.Collectors;

final class EndpointI extends com.zeroc.Ice.EndpointI {
    public EndpointI(Instance instance, com.zeroc.Ice.EndpointI delegate) {
        _instance = instance;
        _delegate = delegate;
    }

    @Override
    public void streamWriteImpl(OutputStream s) {
        _delegate.streamWriteImpl(s);
    }

    //
    // Return the endpoint information.
    //
    @Override
    public com.zeroc.Ice.EndpointInfo getInfo() {
        return new EndpointInfo(_delegate.getInfo());
    }

    @Override
    public short type() {
        return _delegate.type();
    }

    @Override
    public String protocol() {
        return _delegate.protocol();
    }

    @Override
    public int timeout() {
        return _delegate.timeout();
    }

    @Override
    public com.zeroc.Ice.EndpointI timeout(int timeout) {
        if (timeout == _delegate.timeout()) {
            return this;
        } else {
            return new EndpointI(_instance, _delegate.timeout(timeout));
        }
    }

    @Override
    public String connectionId() {
        return _delegate.connectionId();
    }

    @Override
    public com.zeroc.Ice.EndpointI connectionId(String connectionId) {
        if (connectionId == _delegate.connectionId()) {
            return this;
        } else {
            return new EndpointI(_instance, _delegate.connectionId(connectionId));
        }
    }

    @Override
    public boolean compress() {
        return _delegate.compress();
    }

    @Override
    public com.zeroc.Ice.EndpointI compress(boolean compress) {
        if (compress == _delegate.compress()) {
            return this;
        } else {
            return new EndpointI(_instance, _delegate.compress(compress));
        }
    }

    @Override
    public boolean datagram() {
        return _delegate.datagram();
    }

    @Override
    public boolean secure() {
        return _delegate.secure();
    }

    //
    // Return a server side transceiver for this endpoint, or null if a
    // transceiver can only be created by an acceptor.
    //
    @Override
    public Transceiver transceiver() {
        return null;
    }

    @Override
    public void connectors_async(final EndpointI_connectors callback) {
        IPEndpointInfo ipInfo = null;
        for (com.zeroc.Ice.EndpointInfo p = _delegate.getInfo(); p != null; p = p.underlying) {
            if (p instanceof IPEndpointInfo) {
                ipInfo = (IPEndpointInfo) p;
            }
        }
        final String host = ipInfo != null ? ipInfo.host : "";
        EndpointI_connectors cb =
                new EndpointI_connectors() {
                    @Override
                    public void connectors(List<Connector> connectors) {
                        List<Connector> l = new ArrayList<>();
                        for (Connector c : connectors) {
                            l.add(new ConnectorI(_instance, c, host));
                        }
                        callback.connectors(l);
                    }

                    @Override
                    public void exception(LocalException ex) {
                        callback.exception(ex);
                    }
                };
        _delegate.connectors_async(cb);
    }

    //
    // Return an acceptor for this endpoint, or null if no acceptors
    // is available.
    //
    @Override
    public Acceptor acceptor(String adapterName, SSLEngineFactory factory) {
        return new AcceptorI(
                this, _instance, _delegate.acceptor(adapterName, null), adapterName, factory);
    }

    public EndpointI endpoint(com.zeroc.Ice.EndpointI delEndpoint) {
        if (delEndpoint == _delegate) {
            return this;
        } else {
            return new EndpointI(_instance, delEndpoint);
        }
    }

    @Override
    public List<com.zeroc.Ice.EndpointI> expandHost() {
        return _delegate.expandHost().stream().map(this::endpoint).collect(Collectors.toList());
    }

    @Override
    public boolean isLoopbackOrMulticast() {
        return _delegate.isLoopbackOrMulticast();
    }

    @Override
    public com.zeroc.Ice.EndpointI toPublishedEndpoint(String publishedHost) {
        return endpoint(_delegate.toPublishedEndpoint(publishedHost));
    }

    @Override
    public boolean equivalent(com.zeroc.Ice.EndpointI endpoint) {
        if (!(endpoint instanceof EndpointI)) {
            return false;
        }
        EndpointI endpointI = (EndpointI) endpoint;
        return _delegate.equivalent(endpointI._delegate);
    }

    @Override
    public synchronized int hashCode() {
        return _delegate.hashCode();
    }

    @Override
    public String options() {
        return _delegate.options();
    }

    //
    // Compare endpoints for sorting purposes
    //
    @Override
    public int compareTo(com.zeroc.Ice.EndpointI obj) // From java.lang.Comparable
            {
        if (!(obj instanceof EndpointI)) {
            return type() < obj.type() ? -1 : 1;
        }

        EndpointI p = (EndpointI) obj;
        if (this == p) {
            return 0;
        }

        return _delegate.compareTo(p._delegate);
    }

    @Override
    protected boolean checkOption(String option, String argument, String endpoint) {
        return false;
    }

    private final Instance _instance;
    private final com.zeroc.Ice.EndpointI _delegate;
}
