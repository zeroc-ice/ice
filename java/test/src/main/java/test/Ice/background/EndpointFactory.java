// Copyright (c) ZeroC, Inc.

package test.Ice.background;

final class EndpointFactory implements com.zeroc.Ice.EndpointFactory {
    EndpointFactory(Configuration configuration, com.zeroc.Ice.EndpointFactory factory) {
        _configuration = configuration;
        _factory = factory;
    }

    @Override
    public short type() {
        return (short) (EndpointI.TYPE_BASE + _factory.type());
    }

    @Override
    public String protocol() {
        return "test-" + _factory.protocol();
    }

    @Override
    public com.zeroc.Ice.EndpointI create(java.util.ArrayList<String> args, boolean server) {
        return new EndpointI(_configuration, _factory.create(args, server));
    }

    @Override
    public com.zeroc.Ice.EndpointI read(com.zeroc.Ice.InputStream s) {
        short type = s.readShort();
        assert (type == _factory.type());

        s.startEncapsulation();
        com.zeroc.Ice.EndpointI endpoint = new EndpointI(_configuration, _factory.read(s));
        s.endEncapsulation();
        return endpoint;
    }

    @Override
    public com.zeroc.Ice.EndpointFactory clone(com.zeroc.Ice.ProtocolInstance instance) {
        return this;
    }

    private Configuration _configuration;
    private com.zeroc.Ice.EndpointFactory _factory;
}
