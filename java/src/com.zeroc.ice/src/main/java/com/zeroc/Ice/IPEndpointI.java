// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

abstract class IPEndpointI extends EndpointI {
    protected IPEndpointI(
            ProtocolInstance instance,
            String host,
            int port,
            InetSocketAddress sourceAddr,
            String connectionId) {
        _instance = instance;
        _host = host;
        _normalizedHost = normalizeHost(host);
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
    public void connectors_async(EndpointI_connectors callback) {
        _instance.resolve(_host, _port, this, callback);
    }

    @Override
    public List<EndpointI> expandHost() {

        // If this endpoint has an empty host (wildcard address), don't expand, just return this
        // endpoint.
        if (_host.isEmpty()) {
            return Collections.singletonList(this);
        }

        List<InetSocketAddress> addresses =
                Network.getAddresses(
                        _host, _port, _instance.protocolSupport(), _instance.preferIPv6(), true);

        var result = new ArrayList<EndpointI>(addresses.size());
        for (InetSocketAddress addr : addresses) {
            String host = addr.getAddress().getHostAddress();
            result.add(createEndpoint(host, addr.getPort(), _connectionId));
        }

        return result;
    }

    @Override
    public boolean isLoopbackOrMulticast() {
        if (_host.isEmpty()) {
            return false;
        } else {
            try {
                var address = InetAddress.getByName(_host);
                return address.isLoopbackAddress() || address.isMulticastAddress();
            } catch (UnknownHostException ex) {
                return false;
            }
        }
    }

    @Override
    public boolean equivalent(EndpointI endpoint) {
        if (!(endpoint instanceof IPEndpointI)) {
            return false;
        }

        IPEndpointI ipEndpointI = (IPEndpointI) endpoint;
        return ipEndpointI.type() == type()
                && ipEndpointI._normalizedHost.equals(_normalizedHost)
                && ipEndpointI._port == _port;
    }

    public List<Connector> connectors(
            List<InetSocketAddress> addresses, NetworkProxy proxy) {
        List<Connector> connectors = new ArrayList<>();
        for (InetSocketAddress p : addresses) {
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
        // methods called to generate parts of the reference string could break these features.
        // Please review for all features that depend on the
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

    void initWithOptions(ArrayList<String> args, boolean oaEndpoint) {
        super.initWithOptions(args);

        if (_host == null || _host.isEmpty()) {
            _host = _instance.defaultHost();
            _normalizedHost = normalizeHost(_host);
        } else if ("*".equals(_host)) {
            if (oaEndpoint) {
                _host = "";
                _normalizedHost = "";
            } else {
                throw new ParseException(
                        "'-h *' not valid for proxy endpoint '" + toString() + "'");
            }
        }

        if (_host == null) {
            _host = "";
            _normalizedHost = "";
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
        if ("-h".equals(option)) {
            if (argument == null) {
                throw new ParseException(
                        "no argument provided for -h option in endpoint '" + endpoint + "'");
            }
            _host = argument;
            _normalizedHost = normalizeHost(argument);
        } else if ("-p".equals(option)) {
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
        } else if ("--sourceAddress".equals(option)) {
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
            } catch (UnknownHostException ex) {
                // Ignore - don't normalize host.
            }
        }
        return host;
    }

    protected abstract Connector createConnector(
            InetSocketAddress addr, NetworkProxy proxy);

    protected abstract IPEndpointI createEndpoint(String host, int port, String connectionId);

    protected final ProtocolInstance _instance;
    protected String _host;
    protected int _port;
    protected InetSocketAddress _sourceAddr;
    protected final String _connectionId;

    // Set when we set _host; used by the implementation of equivalent.
    private String _normalizedHost;
}
