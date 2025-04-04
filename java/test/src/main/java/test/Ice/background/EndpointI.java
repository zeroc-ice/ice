// Copyright (c) ZeroC, Inc.

package test.Ice.background;

import java.util.stream.Collectors;

final class EndpointI extends com.zeroc.Ice.EndpointI {
    static final short TYPE_BASE = 100;

    public EndpointI(Configuration configuration, com.zeroc.Ice.EndpointI endpoint) {
        _endpoint = endpoint;
        _configuration = configuration;
    }

    @Override
    public String _toString() {
        return "test-" + _endpoint.toString();
    }

    @Override
    public com.zeroc.Ice.EndpointInfo getInfo() {
        return _endpoint.getInfo();
    }

    @Override
    public void streamWriteImpl(com.zeroc.Ice.OutputStream s) {
        s.writeShort(_endpoint.type());
        _endpoint.streamWrite(s);
    }

    @Override
    public short type() {
        return (short) (TYPE_BASE + _endpoint.type());
    }

    @Override
    public String protocol() {
        return _endpoint.protocol();
    }

    @Override
    public int timeout() {
        return _endpoint.timeout();
    }

    @Override
    public com.zeroc.Ice.EndpointI timeout(int timeout) {
        com.zeroc.Ice.EndpointI endpoint = _endpoint.timeout(timeout);
        if (endpoint == _endpoint) {
            return this;
        } else {
            return new EndpointI(_configuration, endpoint);
        }
    }

    @Override
    public String connectionId() {
        return _endpoint.connectionId();
    }

    @Override
    public com.zeroc.Ice.EndpointI connectionId(String connectionId) {
        com.zeroc.Ice.EndpointI endpoint = _endpoint.connectionId(connectionId);
        if (endpoint == _endpoint) {
            return this;
        } else {
            return new EndpointI(_configuration, endpoint);
        }
    }

    @Override
    public boolean compress() {
        return _endpoint.compress();
    }

    @Override
    public com.zeroc.Ice.EndpointI compress(boolean compress) {
        com.zeroc.Ice.EndpointI endpoint = _endpoint.compress(compress);
        if (endpoint == _endpoint) {
            return this;
        } else {
            return new EndpointI(_configuration, endpoint);
        }
    }

    @Override
    public boolean datagram() {
        return _endpoint.datagram();
    }

    @Override
    public boolean secure() {
        return _endpoint.secure();
    }

    @Override
    public com.zeroc.Ice.Transceiver transceiver() {
        com.zeroc.Ice.Transceiver transceiver = _endpoint.transceiver();
        if (transceiver != null) {
            return new Transceiver(_configuration, transceiver);
        } else {
            return null;
        }
    }

    @Override
    public void connectors_async(final com.zeroc.Ice.EndpointI_connectors cb) {
        class Callback implements com.zeroc.Ice.EndpointI_connectors {
            @Override
            public void connectors(java.util.List<com.zeroc.Ice.Connector> cons) {
                java.util.List<com.zeroc.Ice.Connector> connectors = new java.util.ArrayList<>();
                for (com.zeroc.Ice.Connector p : cons) {
                    connectors.add(new Connector(_configuration, p));
                }
                cb.connectors(connectors);
            }

            @Override
            public void exception(com.zeroc.Ice.LocalException exception) {
                cb.exception(exception);
            }
        }

        try {
            _configuration.checkConnectorsException();
            _endpoint.connectors_async(new Callback());
        } catch (com.zeroc.Ice.LocalException ex) {
            cb.exception(ex);
        }
    }

    @Override
    public com.zeroc.Ice.Acceptor acceptor(
            String adapterName, com.zeroc.Ice.SSL.SSLEngineFactory sslEngineFactory) {
        return new Acceptor(
                this, _configuration, _endpoint.acceptor(adapterName, sslEngineFactory));
    }

    public EndpointI endpoint(com.zeroc.Ice.EndpointI delEndp) {
        if (delEndp == _endpoint) {
            return this;
        } else {
            return new EndpointI(_configuration, delEndp);
        }
    }

    @Override
    public java.util.List<com.zeroc.Ice.EndpointI> expandHost() {
        return _endpoint.expandHost().stream().map(this::endpoint).collect(Collectors.toList());
    }

    @Override
    public boolean isLoopbackOrMulticast() {
        return _endpoint.isLoopbackOrMulticast();
    }

    @Override
    public com.zeroc.Ice.EndpointI toPublishedEndpoint(String publishedHost) {
        return endpoint(_endpoint.toPublishedEndpoint(publishedHost));
    }

    @Override
    public boolean equivalent(com.zeroc.Ice.EndpointI endpoint) {
        EndpointI testEndpoint = null;
        try {
            testEndpoint = (EndpointI) endpoint;
        } catch (ClassCastException ex) {
            return false;
        }
        return testEndpoint._endpoint.equivalent(_endpoint);
    }

    @Override
    public String options() {
        return _endpoint.options();
    }

    @Override
    public int hashCode() {
        return _endpoint.hashCode();
    }

    //
    // Compare endpoints for sorting purposes
    //
    @Override
    public boolean equals(java.lang.Object obj) {
        try {
            return compareTo((com.zeroc.Ice.EndpointI) obj) == 0;
        } catch (ClassCastException ee) {
            assert false;
            return false;
        }
    }

    @Override
    public int compareTo(com.zeroc.Ice.EndpointI obj) {
        EndpointI p = null;
        try {
            p = (EndpointI) obj;
        } catch (ClassCastException ex) {
            return type() < obj.type() ? -1 : 1;
        }

        if (this == p) {
            return 0;
        }

        return _endpoint.compareTo(p._endpoint);
    }

    public com.zeroc.Ice.EndpointI delegate() {
        return _endpoint;
    }

    private com.zeroc.Ice.EndpointI _endpoint;
    private Configuration _configuration;
}
