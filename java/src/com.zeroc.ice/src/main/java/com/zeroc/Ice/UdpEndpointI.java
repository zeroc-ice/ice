//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

import com.zeroc.Ice.SSL.SSLEngineFactory;

final class UdpEndpointI extends IPEndpointI {
    public UdpEndpointI(
            ProtocolInstance instance,
            String ho,
            int po,
            java.net.InetSocketAddress sourceAddr,
            String mcastInterface,
            int mttl,
            boolean conn,
            String conId,
            boolean co) {
        super(instance, ho, po, sourceAddr, conId);
        _mcastInterface = mcastInterface;
        _mcastTtl = mttl;
        _connect = conn;
        _compress = co;
    }

    public UdpEndpointI(ProtocolInstance instance) {
        super(instance);
        _connect = false;
        _compress = false;
    }

    public UdpEndpointI(ProtocolInstance instance, InputStream s) {
        super(instance, s);
        if (s.getEncoding().equals(Util.Encoding_1_0)) {
            s.readByte();
            s.readByte();
            s.readByte();
            s.readByte();
        }
        // Not transmitted.
        // _connect = s.readBool();
        _connect = false;
        _compress = s.readBool();
    }

    //
    // Return the endpoint information.
    //
    @Override
    public EndpointInfo getInfo() {
        return new UDPEndpointInfo(
                _compress,
                _host,
                _port,
                _sourceAddr == null ? "" : _sourceAddr.getAddress().getHostAddress(),
                _mcastInterface,
                _mcastTtl);
    }

    //
    // Return the timeout for the endpoint in milliseconds. 0 means
    // non-blocking, -1 means no timeout.
    //
    @Override
    public int timeout() {
        return -1;
    }

    //
    // Return a new endpoint with a different timeout value, provided
    // that timeouts are supported by the endpoint. Otherwise the same
    // endpoint is returned.
    //
    @Override
    public EndpointI timeout(int timeout) {
        return this;
    }

    //
    // Return true if the endpoints support bzip2 compress, or false
    // otherwise.
    //
    @Override
    public boolean compress() {
        return _compress;
    }

    //
    // Return a new endpoint with a different compression value,
    // provided that compression is supported by the
    // endpoint. Otherwise the same endpoint is returned.
    //
    @Override
    public EndpointI compress(boolean compress) {
        if (compress == _compress) {
            return this;
        } else {
            return new UdpEndpointI(
                    _instance,
                    _host,
                    _port,
                    _sourceAddr,
                    _mcastInterface,
                    _mcastTtl,
                    _connect,
                    _connectionId,
                    compress);
        }
    }

    //
    // Return true if the endpoint is datagram-based.
    //
    @Override
    public boolean datagram() {
        return true;
    }

    //
    // Return a server side transceiver for this endpoint, or null if a
    // transceiver can only be created by an acceptor.
    //
    @Override
    public Transceiver transceiver() {
        java.net.InetSocketAddress addr =
                Network.getAddressForServer(
                        _host, _port, _instance.protocolSupport(), _instance.preferIPv6());
        if (Util.isAndroid() && addr.getAddress().isMulticastAddress()) {
            return new UdpMulticastServerTransceiver(this, _instance, addr, _mcastInterface);
        } else {
            return new UdpTransceiver(this, _instance, addr, _mcastInterface, _connect);
        }
    }

    //
    // Return an acceptor for this endpoint, or null if no acceptor is available.
    //
    @Override
    public Acceptor acceptor(String adapterName, SSLEngineFactory factory) {
        assert (factory == null);
        return null;
    }

    public UdpEndpointI endpoint(UdpTransceiver transceiver) {
        int port = transceiver.effectivePort();
        if (port == _port) {
            return this;
        } else {
            return new UdpEndpointI(
                    _instance,
                    _host,
                    port,
                    _sourceAddr,
                    _mcastInterface,
                    _mcastTtl,
                    _connect,
                    _connectionId,
                    _compress);
        }
    }

    public UdpEndpointI endpoint(UdpMulticastServerTransceiver transceiver) {
        int port = transceiver.effectivePort();
        if (port == _port) {
            return this;
        } else {
            return new UdpEndpointI(
                    _instance,
                    _host,
                    port,
                    _sourceAddr,
                    _mcastInterface,
                    _mcastTtl,
                    _connect,
                    _connectionId,
                    _compress);
        }
    }

    @Override
    public void initWithOptions(java.util.ArrayList<String> args, boolean oaEndpoint) {
        super.initWithOptions(args, oaEndpoint);

        if (_mcastInterface.equals("*")) {
            if (oaEndpoint) {
                _mcastInterface = "";
            } else {
                throw new ParseException(
                        "'--interface *' not valid for proxy endpoint '" + toString() + "'");
            }
        }
    }

    //
    // Convert the endpoint to its string form
    //
    @Override
    public String options() {
        //
        // WARNING: Certain features, such as proxy validation in Glacier2,
        // depend on the format of proxy strings. Changes to toString() and
        // methods called to generate parts of the reference string could break
        // these features. Please review for all features that depend on the
        // format of proxyToString() before changing this and related code.
        //
        String s = super.options();

        if (!_mcastInterface.isEmpty()) {
            s += " --interface ";
            boolean addQuote = _mcastInterface.indexOf(':') != -1;
            if (addQuote) {
                s += "\"";
            }
            s += _mcastInterface;
            if (addQuote) {
                s += "\"";
            }
        }

        if (_mcastTtl != -1) {
            s += " --ttl " + _mcastTtl;
        }

        if (_connect) {
            s += " -c";
        }

        if (_compress) {
            s += " -z";
        }

        return s;
    }

    @Override
    public int compareTo(EndpointI obj) {
        if (!(obj instanceof UdpEndpointI)) {
            return type() < obj.type() ? -1 : 1;
        }

        UdpEndpointI p = (UdpEndpointI) obj;
        if (this == p) {
            return 0;
        }

        if (!_connect && p._connect) {
            return -1;
        } else if (!p._connect && _connect) {
            return 1;
        }

        if (!_compress && p._compress) {
            return -1;
        } else if (!p._compress && _compress) {
            return 1;
        }

        if (_mcastTtl < p._mcastTtl) {
            return -1;
        } else if (p._mcastTtl < _mcastTtl) {
            return 1;
        }

        int rc = _mcastInterface.compareTo(p._mcastInterface);
        if (rc != 0) {
            return rc;
        }

        return super.compareTo(obj);
    }

    //
    // Marshal the endpoint
    //
    @Override
    public void streamWriteImpl(OutputStream s) {
        super.streamWriteImpl(s);
        if (s.getEncoding().equals(Util.Encoding_1_0)) {
            Util.Protocol_1_0.ice_writeMembers(s);
            Util.Encoding_1_0.ice_writeMembers(s);
        }
        // Not transmitted.
        // s.writeBool(_connect);
        s.writeBool(_compress);
    }

    @Override
    public int hashCode() {
        int h = super.hashCode();
        h = HashUtil.hashAdd(h, _mcastInterface);
        h = HashUtil.hashAdd(h, _mcastTtl);
        h = HashUtil.hashAdd(h, _connect);
        h = HashUtil.hashAdd(h, _compress);
        return h;
    }

    @Override
    public EndpointI toPublishedEndpoint(String publishedHost) {
        return new UdpEndpointI(
                _instance,
                publishedHost.isEmpty() ? _host : publishedHost,
                _port,
                null,
                "",
                -1,
                false, // for "connect"
                "",
                _compress);
    }

    @Override
    protected boolean checkOption(String option, String argument, String endpoint) {
        if (super.checkOption(option, argument, endpoint)) {
            return true;
        }

        if (option.equals("-c")) {
            if (argument != null) {
                throw new ParseException(
                        "unexpected argument '"
                                + argument
                                + "' provided for -c option in '"
                                + endpoint
                                + "'");
            }

            _connect = true;
        } else if (option.equals("-z")) {
            if (argument != null) {
                throw new ParseException(
                        "unexpected argument '"
                                + argument
                                + "' provided for -z option in '"
                                + endpoint
                                + "'");
            }

            _compress = true;
        } else if (option.equals("-v") || option.equals("-e")) {
            if (argument == null) {
                throw new ParseException(
                        "no argument provided for "
                                + option
                                + " option in endpoint '"
                                + endpoint
                                + "'");
            }

            try {
                EncodingVersion v = Util.stringToEncodingVersion(argument);
                if (v.major != 1 || v.minor != 0) {
                    _instance.logger().warning("deprecated udp endpoint option: " + option);
                }
            } catch (ParseException ex) {
                throw new ParseException(
                        "invalid version '" + argument + "' in endpoint '" + endpoint + "'", ex);
            }
        } else if (option.equals("--ttl")) {
            if (argument == null) {
                throw new ParseException(
                        "no argument provided for --ttl option in endpoint '" + endpoint + "'");
            }

            try {
                _mcastTtl = Integer.parseInt(argument);
            } catch (NumberFormatException ex) {
                throw new ParseException(
                        "invalid TTL value '" + argument + "' in endpoint '" + endpoint + "'", ex);
            }

            if (_mcastTtl < 0) {
                throw new ParseException(
                        "TTL value '" + argument + "' out of range in endpoint '" + endpoint + "'");
            }
        } else if (option.equals("--interface")) {
            if (argument == null) {
                throw new ParseException(
                        "no argument provided for --interface option in endpoint '"
                                + endpoint
                                + "'");
            }
            _mcastInterface = argument;
        } else {
            return false;
        }
        return true;
    }

    @Override
    protected Connector createConnector(java.net.InetSocketAddress addr, NetworkProxy proxy) {
        return new UdpConnector(
                _instance, addr, _sourceAddr, _mcastInterface, _mcastTtl, _connectionId);
    }

    @Override
    protected IPEndpointI createEndpoint(String host, int port, String connectionId) {
        return new UdpEndpointI(
                _instance,
                host,
                port,
                _sourceAddr,
                _mcastInterface,
                _mcastTtl,
                _connect,
                connectionId,
                _compress);
    }

    private String _mcastInterface = "";
    private int _mcastTtl = -1;
    private boolean _connect;
    private boolean _compress;
}
