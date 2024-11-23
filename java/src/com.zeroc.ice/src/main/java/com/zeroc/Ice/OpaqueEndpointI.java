//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

import com.zeroc.Ice.SSL.SSLEngineFactory;

import java.util.Collections;

final class OpaqueEndpointI extends EndpointI {
    public OpaqueEndpointI(java.util.ArrayList<String> args) {
        _type = -1;
        _rawEncoding = Util.Encoding_1_0;
        _rawBytes = new byte[0];

        initWithOptions(args);

        if (_type < 0) {
            throw new ParseException("no -t option in endpoint '" + toString() + "'");
        }
        if (_rawBytes.length == 0) {
            throw new ParseException("no -v option in endpoint '" + toString() + "'");
        }
    }

    public OpaqueEndpointI(short type, InputStream s) {
        _type = type;
        _rawEncoding = s.getEncoding();
        int sz = s.getEncapsulationSize();
        _rawBytes = s.readBlob(sz);
    }

    //
    // Marshal the endpoint
    //
    @Override
    public void streamWrite(OutputStream s) {
        s.startEncapsulation(_rawEncoding, null);
        s.writeBlob(_rawBytes);
        s.endEncapsulation();
    }

    @Override
    public void streamWriteImpl(OutputStream s) {
        assert (false);
    }

    //
    // Return the endpoint information.
    //
    @Override
    public EndpointInfo getInfo() {
        var info =
                new OpaqueEndpointInfo() {
                    @Override
                    public short type() {
                        return _type;
                    }

                    @Override
                    public boolean datagram() {
                        return false;
                    }

                    @Override
                    public boolean secure() {
                        return false;
                    }
                };

        info.rawEncoding = _rawEncoding;
        info.rawBytes = _rawBytes;
        return info;
    }

    //
    // Return the endpoint type
    //
    @Override
    public short type() {
        return _type;
    }

    //
    // Return the protocol name
    //
    @Override
    public String protocol() {
        return "opaque";
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
    public EndpointI timeout(int t) {
        return this;
    }

    @Override
    public String connectionId() {
        return "";
    }

    //
    // Return a new endpoint with a different connection id.
    //
    @Override
    public EndpointI connectionId(String connectionId) {
        return this;
    }

    //
    // Return true if the endpoints support bzip2 compress, or false
    // otherwise.
    //
    @Override
    public boolean compress() {
        return false;
    }

    //
    // Return a new endpoint with a different compression value,
    // provided that compression is supported by the
    // endpoint. Otherwise the same endpoint is returned.
    //
    @Override
    public EndpointI compress(boolean compress) {
        return this;
    }

    //
    // Return true if the endpoint is datagram-based.
    //
    @Override
    public boolean datagram() {
        return false;
    }

    //
    // Return true if the endpoint is secure.
    //
    @Override
    public boolean secure() {
        return false;
    }

    //
    // Return a server side transceiver for this endpoint, or null if a
    // transceiver can only be created by an acceptor.d.
    //
    @Override
    public Transceiver transceiver() {
        return null;
    }

    //
    // Return connectors for this endpoint, or empty list if no connector
    // is available.
    //
    @Override
    public void connectors_async(EndpointSelectionType selType, EndpointI_connectors callback) {
        callback.connectors(new java.util.ArrayList<>());
    }

    //
    // Return an acceptor for this endpoint, or null if no acceptors
    // is available.
    //
    @Override
    public Acceptor acceptor(String adapterName, SSLEngineFactory factory) {
        assert (factory == null);
        return null;
    }

    @Override
    public java.util.List<EndpointI> expandHost() {
        return Collections.singletonList(this);
    }

    @Override
    public boolean isLoopbackOrMulticast() {
        return false;
    }

    @Override
    public EndpointI toPublishedEndpoint(String publishedHost) {
        return this;
    }

    //
    // Check whether the endpoint is equivalent to another one.
    //
    @Override
    public boolean equivalent(EndpointI endpoint) {
        return false;
    }

    @Override
    public int hashCode() {
        int h = 5381;
        h = HashUtil.hashAdd(h, _type);
        h = HashUtil.hashAdd(h, _rawBytes);
        return h;
    }

    @Override
    public String options() {
        String s = "";
        if (_type > -1) {
            s += " -t " + _type;
        }
        s += " -e " + Util.encodingVersionToString(_rawEncoding);
        if (_rawBytes.length > 0) {
            s += " -v " + Base64.encode(_rawBytes);
        }
        return s;
    }

    //
    // Compare endpoints for sorting purposes
    //
    @Override
    public int compareTo(EndpointI obj) // From java.lang.Comparable
            {
        if (!(obj instanceof OpaqueEndpointI)) {
            return type() < obj.type() ? -1 : 1;
        }

        OpaqueEndpointI p = (OpaqueEndpointI) obj;
        if (this == p) {
            return 0;
        }

        if (_type < p._type) {
            return -1;
        } else if (p._type < _type) {
            return 1;
        }

        if (_rawEncoding.major < p._rawEncoding.major) {
            return -1;
        } else if (p._rawEncoding.major < _rawEncoding.major) {
            return 1;
        }

        if (_rawEncoding.minor < p._rawEncoding.minor) {
            return -1;
        } else if (p._rawEncoding.minor < _rawEncoding.minor) {
            return 1;
        }

        if (_rawBytes.length < p._rawBytes.length) {
            return -1;
        } else if (p._rawBytes.length < _rawBytes.length) {
            return 1;
        }
        for (int i = 0; i < _rawBytes.length; i++) {
            if (_rawBytes[i] < p._rawBytes[i]) {
                return -1;
            } else if (p._rawBytes[i] < _rawBytes[i]) {
                return 1;
            }
        }

        return 0;
    }

    @Override
    protected boolean checkOption(String option, String argument, String endpoint) {
        switch (option.charAt(1)) {
            case 't':
                {
                    if (_type > -1) {
                        throw new ParseException(
                                "multiple -t options in endpoint '" + endpoint + "'");
                    }
                    if (argument == null) {
                        throw new ParseException(
                                "no argument provided for -t option in endpoint '"
                                        + endpoint
                                        + "'");
                    }

                    int t;
                    try {
                        t = Integer.parseInt(argument);
                    } catch (NumberFormatException ex) {
                        throw new ParseException(
                                "invalid type value '"
                                        + argument
                                        + "' in endpoint '"
                                        + endpoint
                                        + "'",
                                ex);
                    }

                    if (t < 0 || t > 65535) {
                        throw new ParseException(
                                "type value '"
                                        + argument
                                        + "' out of range in endpoint '"
                                        + endpoint
                                        + "'");
                    }

                    _type = (short) t;
                    return true;
                }

            case 'v':
                {
                    if (_rawBytes.length > 0) {
                        throw new ParseException(
                                "multiple -v options in endpoint '" + endpoint + "'");
                    }
                    if (argument == null) {
                        throw new ParseException(
                                "no argument provided for -v option in endpoint '"
                                        + endpoint
                                        + "'");
                    }

                    try {
                        _rawBytes = Base64.decode(argument);
                    } catch (IllegalArgumentException ex) {
                        throw new ParseException(
                                "invalid Base64 input in endpoint '" + endpoint + "'", ex);
                    }
                    return true;
                }

            case 'e':
                {
                    if (argument == null) {
                        throw new ParseException(
                                "no argument provided for -e option in endpoint '"
                                        + endpoint
                                        + "'");
                    }

                    try {
                        _rawEncoding = Util.stringToEncodingVersion(argument);
                    } catch (ParseException ex) {
                        throw new ParseException(
                                "invalid encoding version '"
                                        + argument
                                        + "' in endpoint '"
                                        + endpoint
                                        + "'",
                                ex);
                    }
                    return true;
                }

            default:
                {
                    return false;
                }
        }
    }

    private short _type;
    private EncodingVersion _rawEncoding;
    private byte[] _rawBytes;
}
