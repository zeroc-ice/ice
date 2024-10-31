//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * @hidden Public because it's used by IceBT, SSL, and the 'Ice/background' test.
 */
public class ProtocolInstance {
    public ProtocolInstance(
            Communicator communicator, short type, String protocol, boolean secure) {
        _instance = communicator.getInstance();
        _traceLevel = _instance.traceLevels().network;
        _traceCategory = _instance.traceLevels().networkCat;
        _logger = _instance.initializationData().logger;
        _properties = _instance.initializationData().properties;
        _type = type;
        _protocol = protocol;
        _secure = secure;
    }

    public int traceLevel() {
        return _traceLevel;
    }

    public String traceCategory() {
        return _traceCategory;
    }

    public Logger logger() {
        return _logger;
    }

    public EndpointFactory getEndpointFactory(short type) {
        return _instance.endpointFactoryManager().get(type);
    }

    public String protocol() {
        return _protocol;
    }

    public short type() {
        return _type;
    }

    public boolean secure() {
        return _secure;
    }

    public Properties properties() {
        return _properties;
    }

    public boolean preferIPv6() {
        return _instance.preferIPv6();
    }

    public int protocolSupport() {
        return _instance.protocolSupport();
    }

    public String defaultHost() {
        return _instance.defaultsAndOverrides().defaultHost;
    }

    public java.net.InetSocketAddress defaultSourceAddress() {
        return _instance.defaultsAndOverrides().defaultSourceAddress;
    }

    public EncodingVersion defaultEncoding() {
        return _instance.defaultsAndOverrides().defaultEncoding;
    }

    public NetworkProxy networkProxy() {
        return _instance.networkProxy();
    }

    public int messageSizeMax() {
        return _instance.messageSizeMax();
    }

    public void resolve(
            String host,
            int port,
            EndpointSelectionType type,
            IPEndpointI endpt,
            EndpointI_connectors callback) {
        _instance.endpointHostResolver().resolve(host, port, type, endpt, callback);
    }

    public BufSizeWarnInfo getBufSizeWarn(short type) {
        return _instance.getBufSizeWarn(type);
    }

    public void setSndBufSizeWarn(short type, int size) {
        _instance.setSndBufSizeWarn(type, size);
    }

    public void setRcvBufSizeWarn(short type, int size) {
        _instance.setRcvBufSizeWarn(type, size);
    }

    ProtocolInstance(Instance instance, short type, String protocol, boolean secure) {
        _instance = instance;
        _traceLevel = _instance.traceLevels().network;
        _traceCategory = _instance.traceLevels().networkCat;
        _logger = _instance.initializationData().logger;
        _properties = _instance.initializationData().properties;
        _type = type;
        _protocol = protocol;
        _secure = secure;
    }

    protected final Instance _instance;
    protected int _traceLevel;
    protected String _traceCategory;
    protected Logger _logger;
    protected Properties _properties;
    protected String _protocol;
    protected short _type;
    protected boolean _secure;
}
