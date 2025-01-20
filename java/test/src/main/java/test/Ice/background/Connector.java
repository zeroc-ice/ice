// Copyright (c) ZeroC, Inc.

package test.Ice.background;

final class Connector implements com.zeroc.Ice.Connector {
    @Override
    public com.zeroc.Ice.Transceiver connect() {
        _configuration.checkConnectException();
        return new Transceiver(_configuration, _connector.connect());
    }

    @Override
    public short type() {
        return (short) (EndpointI.TYPE_BASE + _connector.type());
    }

    @Override
    public String toString() {
        return _connector.toString();
    }

    @Override
    public int hashCode() {
        return _connector.hashCode();
    }

    //
    // Only for use by Endpoint
    //
    Connector(Configuration configuration, com.zeroc.Ice.Connector connector) {
        _configuration = configuration;
        _connector = connector;
    }

    @Override
    public boolean equals(java.lang.Object obj) {
        Connector p = null;

        try {
            p = (Connector) obj;
        } catch (ClassCastException ex) {
            return false;
        }

        if (this == p) {
            return true;
        }

        return _connector.equals(p._connector);
    }

    private final com.zeroc.Ice.Connector _connector;
    private final Configuration _configuration;
}
