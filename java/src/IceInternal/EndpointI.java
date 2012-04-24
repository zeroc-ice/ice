// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

abstract public class EndpointI implements Ice.Endpoint, java.lang.Comparable<EndpointI>
{
    public EndpointI(Ice.ProtocolVersion protocol, Ice.EncodingVersion encoding)
    {
        _protocol = protocol;
        _encoding = encoding;
    }

    public EndpointI()
    {
        _protocol = (Ice.ProtocolVersion)Protocol.currentProtocol.clone();
        _encoding = (Ice.EncodingVersion)Protocol.currentEncoding.clone();
    }

    public String
    toString()
    {
        return _toString();
    }

    //
    // Marshal the endpoint.
    //
    public abstract void streamWrite(BasicStream s);

    //
    // Return the endpoint type.
    //
    public abstract short type();
    
    //
    // Return the timeout for the endpoint in milliseconds. 0 means
    // non-blocking, -1 means no timeout.
    //
    public abstract int timeout();

    //
    // Return a new endpoint with a different timeout value, provided
    // that timeouts are supported by the endpoint. Otherwise the same
    // endpoint is returned.
    //
    public abstract EndpointI timeout(int t);

    //
    // Return a new endpoint with a different connection id.
    //
    public abstract EndpointI connectionId(String connectionId);

    //
    // Return true if the endpoints support bzip2 compress, or false
    // otherwise.
    //
    public abstract boolean compress();
    
    //
    // Return a new endpoint with a different compression value,
    // provided that compression is supported by the
    // endpoint. Otherwise the same endpoint is returned.
    //
    public abstract EndpointI compress(boolean co);

    //
    // Return true if the endpoint is datagram-based.
    //
    public abstract boolean datagram();

    //
    // Return true if the endpoint is secure.
    //
    public abstract boolean secure();

    public Ice.ProtocolVersion protocol()
    {
        return _protocol;
    }

    public Ice.EncodingVersion encoding()
    {
        return _encoding;
    }

    //
    // Return a server side transceiver for this endpoint, or null if a
    // transceiver can only be created by an acceptor. In case a
    // transceiver is created, this operation also returns a new
    // "effective" endpoint, which might differ from this endpoint,
    // for example, if a dynamic port number is assigned.
    //
    public abstract Transceiver transceiver(EndpointIHolder endpoint);

    //
    // Return connectors for this endpoint, or empty list if no connector
    // is available.
    //
    public abstract java.util.List<Connector> connectors();
    public abstract void connectors_async(EndpointI_connectors callback);

    //
    // Return an acceptor for this endpoint, or null if no acceptors
    // is available. In case an acceptor is created, this operation
    // also returns a new "effective" endpoint, which might differ
    // from this endpoint, for example, if a dynamic port number is
    // assigned.
    //
    public abstract Acceptor acceptor(EndpointIHolder endpoint, String adapterName);

    //
    // Expand endpoint out in to separate endpoints for each local
    // host if listening on INADDR_ANY.
    //
    public abstract java.util.List<EndpointI> expand();

    //
    // Check whether the endpoint is equivalent to another one.
    //
    public abstract boolean equivalent(EndpointI endpoint);

    //
    // Compare endpoints for sorting purposes.
    //
    public boolean equals(java.lang.Object obj)
    {
        if(!(obj instanceof EndpointI))
        {
            return false;
        }
        return compareTo((EndpointI)obj) == 0;
    }

    public int compareTo(EndpointI p) // From java.lang.Comparable. 
    {
        if(_protocol.major < p._protocol.major)
        {
            return -1;
        }
        else if(p._protocol.major < _protocol.major)
        {
            return 1;
        }

        if(_protocol.minor < p._protocol.minor)
        {
            return -1;
        }
        else if(p._protocol.minor < _protocol.minor)
        {
            return 1;
        }

        if(_encoding.major < p._encoding.major)
        {
            return -1;
        }
        else if(p._encoding.major < _encoding.major)
        {
            return 1;
        }

        if(_encoding.minor < p._encoding.minor)
        {
            return -1;
        }
        else if(p._encoding.minor < _encoding.minor)
        {
            return 1;
        }

        return 0;
    }

    public java.util.List<Connector>
    connectors(java.util.List<java.net.InetSocketAddress> addresses)
    {
        //
        // This method must be extended by endpoints which use the EndpointHostResolver to create
        // connectors from IP addresses.
        //
        assert(false);
        return null;
    }

    protected void
    parseOption(String option, String arg, String desc, String str)
    {
        if(option.equals("-v"))
        {
            if(arg == null)
            {
                throw new Ice.EndpointParseException("no argument provided for -v option in endpoint `" +
                                                     desc + " "+ str + "'");
            }

            try
            {
                _protocol = Ice.Util.stringToProtocolVersion(arg);
            }
            catch(Ice.VersionParseException e)
            {
                throw new Ice.EndpointParseException("invalid protocol version `" + arg + "' in endpoint `" +
                                                     desc + " "+ str + "':\n" + e.str);
            }
        }            
        else if(option.equals("-e"))
        {
            if(arg == null)
            {
                throw new Ice.EndpointParseException("no argument provided for -e option in endpoint `" +
                                                     desc + " " + str + "'");
            }
            
            try
            {
                _encoding = Ice.Util.stringToEncodingVersion(arg);
            }
            catch(Ice.VersionParseException e)
            {
                throw new Ice.EndpointParseException("invalid encoding version `" + arg + "' in endpoint `" +
                                                     desc + " "+ str + "':\n" + e.str);
            }
        }
        else
        {
            throw new Ice.EndpointParseException("unknown option `" + option + "' in `" + desc + " " + str + "'");
        }
    }

    protected Ice.ProtocolVersion _protocol;
    protected Ice.EncodingVersion _encoding;
}
