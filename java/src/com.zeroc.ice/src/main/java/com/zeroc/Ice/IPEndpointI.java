//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

import java.net.InetAddress;
import java.util.Collections;

abstract class IPEndpointI extends EndpointI {
    protected IPEndpointI(
            ProtocolInstance instance,
            String host,
            int port,
            java.net.InetSocketAddress sourceAddr,
            String connectionId) {
        _instance = instance;
        _host = normalizeHost(host);
        _port = port;
        _sourceAddr = sourceAddr;
        _connectionId = connectionId;
    }

    protected IPEndpointI(ProtocolInstance instance) {
        this(instance, null, 0, null, "");
    }

    protected IPEndpointI(ProtocolInstance instance, InputStream s) {
        this(instance, s.readString(), s.readInt(), null, "");
    }

    @Override
    public EndpointInfo getInfo() {
        IPEndpointInfo info =
                new IPEndpointInfo() {
                    @Override
                    public short type() {
                        return IPEndpointI.this.type();
                    }

                    @Override
                    public boolean datagram() {
                        return IPEndpointI.this.datagram();
                    }

                    @Override
                    public boolean secure() {
                        return IPEndpointI.this.secure();
                    }
                };
        fillEndpointInfo(info);
        return info;
    }

    @Override
    public short type() {
        return _instance.type();
    }

    @Override
    public String protocol() {
        return _instance.protocol();
    }

    @Override
    public boolean secure() {
        return _instance.secure();
    }

    @Override
    public String connectionId() {
        return _connectionId;
    }

    @Override
    public EndpointI connectionId(String connectionId) {
        if (connectionId.equals(_connectionId)) {
            return this;
        } else {
            return createEndpoint(_host, _port, connectionId);
        }
    }

    @Override
    public void connectors_async(EndpointSelectionType selType, EndpointI_connectors callback) {
        _instance.resolve(_host, _port, selType, this, callback);
    }

    @Override
    public java.util.List<EndpointI> expandHost() {

        // If this endpoint has an empty host (wildcard address), don't expand, just return this
        // endpoint.
        if (_host.isEmpty()) {
            return Collections.singletonList(this);
        }

        java.util.List<java.net.InetSocketAddress> addresses =
                Network.getAddresses(
                        _host,
                        _port,
                        _instance.protocolSupport(),
                        EndpointSelectionType.Ordered,
                        _instance.preferIPv6(),
                        true);

        var result = new java.util.ArrayList<EndpointI>(addresses.size());
        for (java.net.InetSocketAddress addr : addresses) {
            String host = addr.getAddress().getHostAddress();
            result.add(createEndpoint(host, addr.getPort(), _connectionId));
        }

        return result;
    }

    @Override
    public boolean isLoopback() {
        try {
            return !_host.isEmpty() && java.net.InetAddress.getByName(_host).isLoopbackAddress();
        } catch (java.net.UnknownHostException ex) {
            return false;
        }
    }

    @Override
    public EndpointI withPublishedHost(String host) {
        return createEndpoint(host, _port, _connectionId);
    }

    @Override
    public boolean equivalent(EndpointI endpoint) {
        if (!(endpoint instanceof IPEndpointI)) {
            return false;
        }

        IPEndpointI ipEndpointI = (IPEndpointI) endpoint;
        return ipEndpointI.type() == type()
                && ipEndpointI._host.equals(_host)
                && ipEndpointI._port == _port
                && java.util.Objects.equals(ipEndpointI._sourceAddr, _sourceAddr);
    }

    public java.util.List<Connector> connectors(
            java.util.List<java.net.InetSocketAddress> addresses, NetworkProxy proxy) {
        java.util.List<Connector> connectors = new java.util.ArrayList<>();
        for (java.net.InetSocketAddress p : addresses) {
            connectors.add(createConnector(p, proxy));
        }
        return connectors;
    }

    @Override
    public int hashCode() {
        int h = 5381;
        h = HashUtil.hashAdd(h, type());
        h = HashUtil.hashAdd(h, _host);
        h = HashUtil.hashAdd(h, _port);
        h = HashUtil.hashAdd(h, _connectionId);
        if (_sourceAddr != null) {
            h = HashUtil.hashAdd(h, _sourceAddr.getAddress().getHostAddress());
        }
        return h;
    }

    @Override
    public String options() {
        //
        // WARNING: Certain features, such as proxy validation in Glacier2,
        // depend on the format of proxy strings. Changes to toString() and
        // methods called to generate parts of the reference string could break
        // these features. Please review for all features that depend on the
        // format of proxyToString() before changing this and related code.
        //
        String s = "";

        if (_host != null && !_host.isEmpty()) {
            s += " -h ";
            boolean addQuote = _host.indexOf(':') != -1;
            if (addQuote) {
                s += '"';
            }
            s += _host;
            if (addQuote) {
                s += '"';
            }
        }

        s += " -p " + _port;

        if (_sourceAddr != null) {
            String sourceAddr = _sourceAddr.getAddress().getHostAddress();
            s += " --sourceAddress ";
            boolean addQuote = sourceAddr.indexOf(':') != -1;
            if (addQuote) {
                s += "\"";
            }
            s += sourceAddr;
            if (addQuote) {
                s += "\"";
            }
        }

        return s;
    }

    @Override
    public int compareTo(EndpointI obj) {
        if (!(obj instanceof IPEndpointI)) {
            return type() < obj.type() ? -1 : 1;
        }

        IPEndpointI p = (IPEndpointI) obj;
        if (this == p) {
            return 0;
        }

        int v = _host.compareTo(p._host);
        if (v != 0) {
            return v;
        }

        if (_port < p._port) {
            return -1;
        } else if (p._port < _port) {
            return 1;
        }

        int rc = Network.compareAddress(_sourceAddr, p._sourceAddr);
        if (rc != 0) {
            return rc;
        }

        return _connectionId.compareTo(p._connectionId);
    }

    @Override
    public void streamWriteImpl(OutputStream s) {
        s.writeString(_host);
        s.writeInt(_port);
    }

    public void fillEndpointInfo(IPEndpointInfo info) {
        info.timeout = timeout();
        info.compress = compress();
        info.host = _host;
        info.port = _port;
        info.sourceAddress = _sourceAddr == null ? "" : _sourceAddr.getAddress().getHostAddress();
    }

    public void initWithOptions(java.util.ArrayList<String> args, boolean oaEndpoint) {
        super.initWithOptions(args);

        if (_host == null || _host.isEmpty()) {
            _host = normalizeHost(_instance.defaultHost());
        } else if (_host.equals("*")) {
            if (oaEndpoint) {
                _host = "";
            } else {
                throw new ParseException(
                        "'-h *' not valid for proxy endpoint '" + toString() + "'");
            }
        }

        if (_host == null) {
            _host = "";
        }

        if (_sourceAddr == null) {
            if (!oaEndpoint) {
                _sourceAddr = _instance.defaultSourceAddress();
            }
        } else if (oaEndpoint) {
            throw new ParseException(
                    "'--sourceAddress' not valid for object adapter endpoint '" + toString() + "'");
        }
    }

    @Override
    protected boolean checkOption(String option, String argument, String endpoint) {
        if (option.equals("-h")) {
            if (argument == null) {
                throw new ParseException(
                        "no argument provided for -h option in endpoint '" + endpoint + "'");
            }
            _host = normalizeHost(argument);
        } else if (option.equals("-p")) {
            if (argument == null) {
                throw new ParseException(
                        "no argument provided for -p option in endpoint '" + endpoint + "'");
            }

            try {
                _port = Integer.parseInt(argument);
            } catch (NumberFormatException ex) {
                throw new ParseException(
                        "invalid port value '" + argument + "' in endpoint '" + endpoint + "'", ex);
            }

            if (_port < 0 || _port > 65535) {
                throw new ParseException(
                        "port value '"
                                + argument
                                + "' out of range in endpoint '"
                                + endpoint
                                + "'");
            }
        } else if (option.equals("--sourceAddress")) {
            if (argument == null) {
                throw new ParseException(
                        "no argument provided for --sourceAddress option in endpoint '"
                                + endpoint
                                + "'");
            }
            _sourceAddr = Network.getNumericAddress(argument);
            if (_sourceAddr == null) {
                throw new ParseException(
                        "invalid IP address provided for --sourceAddress option in endpoint '"
                                + endpoint
                                + "'");
            }
        } else {
            return false;
        }
        return true;
    }

    private String normalizeHost(String host) {
        if (host != null && host.contains(":")) {
            // Could be an IPv6 address that we need to normalize.
            try {
                var address = InetAddress.getByName(host);
                host = address.getHostAddress(); // normalized host
            } catch (java.net.UnknownHostException ex) {
                // Ignore - don't normalize host.
            }
        }
        return host;
    }

    protected abstract Connector createConnector(
            java.net.InetSocketAddress addr, NetworkProxy proxy);

    protected abstract IPEndpointI createEndpoint(String host, int port, String connectionId);

    protected final ProtocolInstance _instance;
    protected String _host;
    protected int _port;
    protected java.net.InetSocketAddress _sourceAddr;
    protected final String _connectionId;
}
