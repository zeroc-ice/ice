// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

final class UdpEndpointI extends EndpointI
{
    final static short TYPE = 3;

    public
    UdpEndpointI(Instance instance, String ho, int po, String mif, int mttl, boolean conn, String conId, boolean co,
                 boolean oae)
    {
        _instance = instance;
        _host = ho;
        _port = po;
        _mcastInterface = mif;
        _mcastTtl = mttl;
        _protocolMajor = Protocol.protocolMajor;
        _protocolMinor = Protocol.protocolMinor;
        _encodingMajor = Protocol.encodingMajor;
        _encodingMinor = Protocol.encodingMinor;
        _connect = conn;
        _connectionId = conId;
        _compress = co;
        _oaEndpoint = oae;
        calcHashValue();
    }

    public
    UdpEndpointI(Instance instance, String str, boolean oaEndpoint)
    {
        _instance = instance;
        _host = null;
        _port = 0;
        _protocolMajor = Protocol.protocolMajor;
        _protocolMinor = Protocol.protocolMinor;
        _encodingMajor = Protocol.encodingMajor;
        _encodingMinor = Protocol.encodingMinor;
        _connect = false;
        _compress = false;
        _oaEndpoint = oaEndpoint;

        String[] arr = str.split("[ \t\n\r]+");

        int i = 0;
        while(i < arr.length)
        {
            if(arr[i].length() == 0)
            {
                i++;
                continue;
            }

            String option = arr[i++];
            if(option.charAt(0) != '-')
            {
                throw new Ice.EndpointParseException("udp " + str);
            }

            String argument = null;
            if(i < arr.length && arr[i].charAt(0) != '-')
            {
                argument = arr[i++];
            }

            if(option.equals("-v"))
            {
                if(argument == null)
                {
                    throw new Ice.EndpointParseException("udp " + str);
                }

                int pos = argument.indexOf('.');
                if(pos == -1)
                {
                    throw new Ice.EndpointParseException("udp " + str);
                }

                String majStr = argument.substring(0, pos);
                String minStr = argument.substring(pos + 1, argument.length());
                int majVersion;
                int minVersion;
                try
                {
                    majVersion = Integer.parseInt(majStr);
                    minVersion = Integer.parseInt(minStr);
                }
                catch(NumberFormatException ex)
                {
                    throw new Ice.EndpointParseException("udp " + str);
                }

                if(majVersion < 1 || majVersion > 255 || minVersion < 0 || minVersion > 255)
                {
                    throw new Ice.EndpointParseException("udp " + str);
                }

                if(majVersion != Protocol.protocolMajor)
                {
                    Ice.UnsupportedProtocolException e = new Ice.UnsupportedProtocolException();
                    e.badMajor = majVersion < 0 ? majVersion + 255 : majVersion;
                    e.badMinor = minVersion < 0 ? minVersion + 255 : minVersion;
                    e.major = Protocol.protocolMajor;
                    e.minor = Protocol.protocolMinor;
                    throw e;
                }

                _protocolMajor = (byte)majVersion;
                _protocolMinor = (byte)minVersion;
            }
            else if(option.equals("-e"))
            {
                if(argument == null)
                {
                    throw new Ice.EndpointParseException("udp " + str);
                }

                int pos = argument.indexOf('.');
                if(pos == -1)
                {
                    throw new Ice.EndpointParseException("udp " + str);
                }

                String majStr = argument.substring(0, pos);
                String minStr = argument.substring(pos + 1, argument.length());
                int majVersion;
                int minVersion;
                try
                {
                    majVersion = Integer.parseInt(majStr);
                    minVersion = Integer.parseInt(minStr);
                }
                catch(NumberFormatException ex)
                {
                    throw new Ice.EndpointParseException("udp " + str);
                }

                if(majVersion < 1 || majVersion > 255 || minVersion < 0 || minVersion > 255)
                {
                    throw new Ice.EndpointParseException("udp " + str);
                }

                if(majVersion != Protocol.encodingMajor)
                {
                    Ice.UnsupportedEncodingException e = new Ice.UnsupportedEncodingException();
                    e.badMajor = majVersion < 0 ? majVersion + 255 : majVersion;
                    e.badMinor = minVersion < 0 ? minVersion + 255 : minVersion;
                    e.major = Protocol.encodingMajor;
                    e.minor = Protocol.encodingMinor;
                    throw e;
                }

                _encodingMajor = (byte)majVersion;
                _encodingMinor = (byte)minVersion;
            }
            else if(option.equals("-h"))
            {
                if(argument == null)
                {
                    throw new Ice.EndpointParseException("udp " + str);
                }

                _host = argument;
            }
            else if(option.equals("-p"))
            {
                if(argument == null)
                {
                    throw new Ice.EndpointParseException("udp " + str);
                }

                try
                {
                    _port = Integer.parseInt(argument);
                }
                catch(NumberFormatException ex)
                {
                    throw new Ice.EndpointParseException("udp " + str);
                }

                if(_port < 0 || _port > 65535)
                {
                    throw new Ice.EndpointParseException("udp " + str);
                }
            }
            else if(option.equals("-c"))
            {
                if(argument != null)
                {
                    throw new Ice.EndpointParseException("udp " + str);
                }

                _connect = true;
                break;
            }
            else if(option.equals("-z"))
            {
                if(argument != null)
                {
                    throw new Ice.EndpointParseException("udp " + str);
                }

                _compress = true;
                break;
            }
            else if(option.equals("--interface"))
            {
                if(argument == null)
                {
                    throw new Ice.EndpointParseException("udp " + str);
                }

                _mcastInterface = argument;
            }
            else if(option.equals("--ttl"))
            {
                if(argument == null)
                {
                    throw new Ice.EndpointParseException("udp " + str);
                }

                try
                {
                    _mcastTtl = Integer.parseInt(argument);
                }
                catch(NumberFormatException ex)
                {
                    throw new Ice.EndpointParseException("udp " + str);
                }

                if(_mcastTtl < 0)
                {
                    throw new Ice.EndpointParseException("udp " + str);
                }
            }
            else
            {
                throw new Ice.EndpointParseException("udp " + str);
            }
        }

        if(_host == null)
        {
            _host = _instance.defaultsAndOverrides().defaultHost;
            if(_host == null)
            {
                if(_oaEndpoint)
                {
                    _host = "0.0.0.0";
                }
                else
                {
                    _host = "127.0.0.1";
                }
            }
        }
        else if(_host.equals("*"))
        {
            _host = "0.0.0.0";
        }
        calcHashValue();
    }

    public
    UdpEndpointI(BasicStream s)
    {
        _instance = s.instance();
        s.startReadEncaps();
        _host = s.readString();
        _port = s.readInt();
        _protocolMajor = s.readByte();
        _protocolMinor = s.readByte();
        _encodingMajor = s.readByte();
        _encodingMinor = s.readByte();
        if(_protocolMajor != Protocol.protocolMajor)
        {
            Ice.UnsupportedProtocolException e = new Ice.UnsupportedProtocolException();
            e.badMajor = _protocolMajor < 0 ? _protocolMajor + 255 : _protocolMajor;
            e.badMinor = _protocolMinor < 0 ? _protocolMinor + 255 : _protocolMinor;
            e.major = Protocol.protocolMajor;
            e.minor = Protocol.protocolMinor;
            throw e;
        }
        if(_encodingMajor != Protocol.encodingMajor)
        {
            Ice.UnsupportedEncodingException e = new Ice.UnsupportedEncodingException();
            e.badMajor = _encodingMajor < 0 ? _encodingMajor + 255 : _encodingMajor;
            e.badMinor = _encodingMinor < 0 ? _encodingMinor + 255 : _encodingMinor;
            e.major = Protocol.encodingMajor;
            e.minor = Protocol.encodingMinor;
            throw e;
        }
        // Not transmitted.
        //_connect = s.readBool();
        _connect = false;
        _compress = s.readBool();
        s.endReadEncaps();
        _oaEndpoint = false;
        calcHashValue();
    }

    //
    // Marshal the endpoint
    //
    public void
    streamWrite(BasicStream s)
    {
        s.writeShort(TYPE);
        s.startWriteEncaps();
        s.writeString(_host);
        s.writeInt(_port);
        s.writeByte(_protocolMajor);
        s.writeByte(_protocolMinor);
        s.writeByte(_encodingMajor);
        s.writeByte(_encodingMinor);
        // Not transmitted.
        //s.writeBool(_connect);
        s.writeBool(_compress);
        s.endWriteEncaps();
    }

    //
    // Convert the endpoint to its string form
    //
    public String
    _toString()
    {
        //
        // WARNING: Certain features, such as proxy validation in Glacier2,
        // depend on the format of proxy strings. Changes to toString() and
        // methods called to generate parts of the reference string could break
        // these features. Please review for all features that depend on the
        // format of proxyToString() before changing this and related code.
        //
        String s = "udp";

        if((int)_protocolMajor != 1 || (int)_protocolMinor != 0)
        {
            s += " -v " + (_protocolMajor < 0 ? (int)_protocolMajor + 255 : _protocolMajor)
                + "." + (_protocolMinor < 0 ? (int)_protocolMinor + 255 : _protocolMinor);
        }

        if((int)_encodingMajor != 1 || (int)_encodingMinor != 0)
        {
            s += " -e " + (_encodingMajor < 0 ? (int)_encodingMajor + 255 : _encodingMajor)
                + "." + (_encodingMinor < 0 ? (int)_encodingMinor + 255 : _encodingMinor);
        }

        s += " -h " + _host + " -p " + _port;

        if(_mcastInterface.length() != 0)
        {
            s += " --interface " + _mcastInterface;
        }

        if(_mcastTtl != -1)
        {
            s += " --ttl " + _mcastTtl;
        }

        if(_connect)
        {
            s += " -c";
        }

        if(_compress)
        {
            s += " -z";
        }

        return s;
    }

    //
    // Return the endpoint type
    //
    public short
    type()
    {
        return TYPE;
    }

    //
    // Return the timeout for the endpoint in milliseconds. 0 means
    // non-blocking, -1 means no timeout.
    //
    public int
    timeout()
    {
        return -1;
    }

    //
    // Return true if the endpoints support bzip2 compress, or false
    // otherwise.
    //
    public boolean
    compress()
    {
        return _compress;
    }

    //
    // Return a new endpoint with a different compression value,
    // provided that compression is supported by the
    // endpoint. Otherwise the same endpoint is returned.
    //
    public EndpointI
    compress(boolean compress)
    {
        if(compress == _compress)
        {
            return this;
        }
        else
        {
            return new UdpEndpointI(_instance, _host, _port, _mcastInterface, _mcastTtl, _connect, _connectionId,
                                    compress, _oaEndpoint);
        }
    }

    //
    // Return a new endpoint with a different connection id.
    //
    public EndpointI
    connectionId(String connectionId)
    {
        if(connectionId.equals(_connectionId))
        {
            return this;
        }
        else
        {
            return new UdpEndpointI(_instance, _host, _port, _mcastInterface, _mcastTtl, _connect, connectionId,
                                    _compress, _oaEndpoint);
        }
    }

    //
    // Return a new endpoint with a different timeout value, provided
    // that timeouts are supported by the endpoint. Otherwise the same
    // endpoint is returned.
    //
    public EndpointI
    timeout(int timeout)
    {
        return this;
    }

    //
    // Return true if the endpoint is datagram-based.
    //
    public boolean
    datagram()
    {
        return true;
    }

    //
    // Return true if the endpoint is secure.
    //
    public boolean
    secure()
    {
        return false;
    }

    //
    // Return true if the endpoint type is unknown.
    //
    public boolean
    unknown()
    {
        return false;
    }

    //
    // Return a server side transceiver for this endpoint, or null if a
    // transceiver can only be created by an acceptor. In case a
    // transceiver is created, this operation also returns a new
    // "effective" endpoint, which might differ from this endpoint,
    // for example, if a dynamic port number is assigned.
    //
    public Transceiver
    transceiver(EndpointIHolder endpoint)
    {
        UdpTransceiver p = new UdpTransceiver(_instance, _host, _port, _mcastInterface, _connect);
        endpoint.value = new UdpEndpointI(_instance, _host, p.effectivePort(), _mcastInterface, _mcastTtl, _connect,
                                          _connectionId, _compress, _oaEndpoint);
        return p;
    }

    //
    // Return connectors for this endpoint, or empty list if no connector
    // is available.
    //
    public java.util.ArrayList
    connectors()
    {
        java.util.ArrayList connectors = new java.util.ArrayList();
        java.util.ArrayList addresses = Network.getAddresses(_host, _port);
        java.util.Iterator p = addresses.iterator();
        while(p.hasNext())
        {
            connectors.add(
                new UdpConnector(_instance, (java.net.InetSocketAddress)p.next(), _mcastInterface, _mcastTtl,
                                 _protocolMajor, _protocolMinor, _encodingMajor, _encodingMinor, _connectionId));
        }
        return connectors;
    }

    //
    // Return an acceptor for this endpoint, or null if no acceptors
    // is available. In case an acceptor is created, this operation
    // also returns a new "effective" endpoint, which might differ
    // from this endpoint, for example, if a dynamic port number is
    // assigned.
    //
    public Acceptor
    acceptor(EndpointIHolder endpoint, String adapterName)
    {
        endpoint.value = this;
        return null;
    }

    //
    // Expand endpoint out in to separate endpoints for each local
    // host if endpoint was configured with no host set. 
    //
    public java.util.ArrayList
    expand()
    {
        java.util.ArrayList endps = new java.util.ArrayList();
        if(_host.equals("0.0.0.0"))
        {
            java.util.ArrayList hosts = Network.getLocalHosts();
            java.util.Iterator iter = hosts.iterator();
            while(iter.hasNext())
            {
                String host = (String)iter.next();
                if(!_oaEndpoint || hosts.size() == 1 || !host.equals("127.0.0.1"))
                {
                    endps.add(new UdpEndpointI(_instance, host, _port, _mcastInterface, _mcastTtl,  _connect,
                                               _connectionId, _compress, _oaEndpoint));
                }
            }
        }
        else
        {
            endps.add(this);
        }
        return endps;
    }

    //
    // Check whether the endpoint is equivalent to a specific
    // Transceiver or Acceptor
    //
    public boolean
    equivalent(Transceiver transceiver)
    {
        UdpTransceiver udpTransceiver = null;
        try
        {
            udpTransceiver = (UdpTransceiver)transceiver;
        }
        catch(ClassCastException ex)
        {
            return false;
        }
        return udpTransceiver.equivalent(_host, _port);
    }

    public boolean
    equivalent(Acceptor acceptor)
    {
        return false;
    }

    public int
    hashCode()
    {
        return _hashCode;
    }

    //
    // Compare endpoints for sorting purposes
    //
    public boolean
    equals(java.lang.Object obj)
    {
        return compareTo(obj) == 0;
    }

    public int
    compareTo(java.lang.Object obj) // From java.lang.Comparable
    {
        UdpEndpointI p = null;

        try
        {
            p = (UdpEndpointI)obj;
        }
        catch(ClassCastException ex)
        {
            try
            {
                EndpointI e = (EndpointI)obj;
                return type() < e.type() ? -1 : 1;
            }
            catch(ClassCastException ee)
            {
                assert(false);
            }
        }

        if(this == p)
        {
            return 0;
        }

        if(_port < p._port)
        {
            return -1;
        }
        else if(p._port < _port)
        {
            return 1;
        }

        if(!_connect && p._connect)
        {
            return -1;
        }
        else if(!p._connect && _connect)
        {
            return 1;
        }

        if(!_connectionId.equals(p._connectionId))
        {
            return _connectionId.compareTo(p._connectionId);
        }

        if(!_compress && p._compress)
        {
            return -1;
        }
        else if(!p._compress && _compress)
        {
            return 1;
        }

        if(_protocolMajor < p._protocolMajor)
        {
            return -1;
        }
        else if(p._protocolMajor < _protocolMajor)
        {
            return 1;
        }

        if(_protocolMinor < p._protocolMinor)
        {
            return -1;
        }
        else if(p._protocolMinor < _protocolMinor)
        {
            return 1;
        }

        if(_encodingMajor < p._encodingMajor)
        {
            return -1;
        }
        else if(p._encodingMajor < _encodingMajor)
        {
            return 1;
        }

        if(_encodingMinor < p._encodingMinor)
        {
            return -1;
        }
        else if(p._encodingMinor < _encodingMinor)
        {
            return 1;
        }

        if(_mcastTtl < p._mcastTtl)
        {
            return -1;
        }
        else if(p._mcastTtl < _mcastTtl)
        {
            return 1;
        }

        int rc = _mcastInterface.compareTo(p._mcastInterface);
        if(rc != 0)
        {
            return rc;
        }

        return _host.compareTo(p._host);
    }

    public boolean
    requiresThreadPerConnection()
    {
        return false;
    }

    private void
    calcHashValue()
    {
        try
        {
            java.net.InetSocketAddress addr = Network.getAddress(_host, _port);
            _hashCode = addr.getAddress().getHostAddress().hashCode();
        }
        catch(Ice.DNSException ex)
        {
            _hashCode = _host.hashCode();
        }
        _hashCode = 5 * _hashCode + _port;
        _hashCode = 5 * _hashCode + _mcastInterface.hashCode();
        _hashCode = 5 * _hashCode + _mcastTtl;
        _hashCode = 5 * _hashCode + (_connect ? 1 : 0);
        _hashCode = 5 * _hashCode + _connectionId.hashCode();
        _hashCode = 5 * _hashCode + (_compress ? 1 : 0);
    }

    private Instance _instance;
    private String _host;
    private int _port;
    private String _mcastInterface = "";
    private int _mcastTtl = -1;
    private byte _protocolMajor;
    private byte _protocolMinor;
    private byte _encodingMajor;
    private byte _encodingMinor;
    private boolean _connect;
    private String _connectionId = "";
    private boolean _compress;
    private boolean _oaEndpoint;
    private int _hashCode;
}
