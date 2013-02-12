// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

final class OpaqueEndpointI extends EndpointI
{
    public
    OpaqueEndpointI(String str)
    {
        super("");

        _rawEncoding = Ice.Util.Encoding_1_0;

        int topt = 0;
        int vopt = 0;

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
            if(option.length() != 2 || option.charAt(0) != '-')
            {
                throw new Ice.EndpointParseException("expected an endpoint option but found `" + option +
                                                     "' in endpoint `opaque " + str + "'");
            }

            String argument = null;
            if(i < arr.length && arr[i].charAt(0) != '-')
            {
                argument = arr[i++];
            }

            switch(option.charAt(1))
            {
                case 't':
                {
                    if(argument == null)
                    {
                        throw new Ice.EndpointParseException("no argument provided for -t option in endpoint `opaque "
                                                             + str + "'");
                    }

                    int t;
                    try
                    {
                        t = Integer.parseInt(argument);
                    }
                    catch(NumberFormatException ex)
                    {
                        throw new Ice.EndpointParseException("invalid type value `" + argument + 
                                                             "' in endpoint `opaque " + str + "'");
                    }

                    if(t < 0 || t > 65535)
                    {
                        throw new Ice.EndpointParseException("type value `" + argument +
                                                             "' out of range in endpoint `opaque " + str + "'");
                    }

                    _type = (short)t;
                    ++topt;
                    if(topt > 1)
                    {
                        throw new Ice.EndpointParseException("multiple -t options in endpoint `opaque " + str + "'");
                    }
                    break;
                }

                case 'e':
                {
                    if(argument == null)
                    {
                        throw new Ice.EndpointParseException("no argument provided for -e option in endpoint `opaque "
                                                             + str + "'");
                    }

                    try
                    {
                        _rawEncoding = Ice.Util.stringToEncodingVersion(argument);
                    }
                    catch(Ice.VersionParseException e)
                    {
                        throw new Ice.EndpointParseException("invalid encoding version `" + argument + 
                                                             "' in endpoint `opaque " + str + "':\n" + e.str);
                    }
                    break;
                }

                case 'v':
                {
                    if(argument == null)
                    {
                        throw new Ice.EndpointParseException("no argument provided for -v option in endpoint `opaque "
                                                             + str + "'");
                    }

                    for(int j = 0; j < argument.length(); ++j)
                    {
                        if(!IceUtilInternal.Base64.isBase64(argument.charAt(j)))
                        {
                            throw new Ice.EndpointParseException("invalid base64 character `" + argument.charAt(j) +
                                                                 "' (ordinal " + ((int)argument.charAt(j)) +
                                                                 ") in endpoint `opaque " + str + "'");
                        }
                    }
                    _rawBytes = IceUtilInternal.Base64.decode(argument);
                    ++vopt;
                    if(vopt > 1)
                    {
                        throw new Ice.EndpointParseException("multiple -v options in endpoint `opaque " + str + "'");
                    }
                    break;
                }

                default:
                {
                    throw new Ice.EndpointParseException("invalid option `" + option + "' in endpoint `opaque " +
                                                         str + "'");
                }
            }
        }

        if(topt != 1)
        {
            throw new Ice.EndpointParseException("no -t option in endpoint `opaque " + str + "'");
        }
        if(vopt != 1)
        {
            throw new Ice.EndpointParseException("no -v option in endpoint `opaque " + str + "'");
        }
        calcHashValue();
    }

    public
    OpaqueEndpointI(short type, BasicStream s)
    {
        super("");
        _type = type;
        _rawEncoding = s.startReadEncaps();
        int sz = s.getReadEncapsSize();
        _rawBytes = s.readBlob(sz);
        s.endReadEncaps();
        calcHashValue();
    }

    //
    // Marshal the endpoint
    //
    public void
    streamWrite(BasicStream s)
    {
        s.writeShort(_type);
        s.startWriteEncaps(_rawEncoding, Ice.FormatType.DefaultFormat);
        s.writeBlob(_rawBytes);
        s.endWriteEncaps();
    }

    //
    // Convert the endpoint to its string form
    //
    public String
    _toString()
    {
        String val = IceUtilInternal.Base64.encode(_rawBytes);
        return "opaque -t " + _type + " -e " + Ice.Util.encodingVersionToString(_rawEncoding) + " -v " + val;
    }

    //
    // Return the endpoint information.
    //
    public Ice.EndpointInfo
    getInfo()
    {
        return new Ice.OpaqueEndpointInfo(-1, false, _rawEncoding, _rawBytes)
            {
                public short type()
                {
                    return _type;
                }
                
                public boolean datagram()
                {
                    return false;
                }
                
                public boolean secure()
                {
                    return false;
                }
        };
    }

    //
    // Return the endpoint type
    //
    public short
    type()
    {
        return _type;
    }

    //
    // Return the protocol name
    //
    public String
    protocol()
    {
        return "opaque";
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
    // Return a new endpoint with a different timeout value, provided
    // that timeouts are supported by the endpoint. Otherwise the same
    // endpoint is returned.
    //
    public EndpointI
    timeout(int t)
    {
        return this;
    }

    //
    // Return a new endpoint with a different connection id.
    //
    public EndpointI
    connectionId(String connectionId)
    {
        return this;
    }
    
    //
    // Return true if the endpoints support bzip2 compress, or false
    // otherwise.
    //
    public boolean
    compress()
    {
        return false;
    }

    //
    // Return a new endpoint with a different compression value,
    // provided that compression is supported by the
    // endpoint. Otherwise the same endpoint is returned.
    //
    public EndpointI
    compress(boolean compress)
    {
        return this;
    }

    //
    // Return true if the endpoint is datagram-based.
    //
    public boolean
    datagram()
    {
        return false;
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
    // Get the encoded endpoint.
    //
    public byte[]
    rawBytes()
    {
        return _rawBytes;
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
        endpoint.value = null;
        return null;
    }

    //
    // Return connectors for this endpoint, or empty list if no connector
    // is available.
    //
    public java.util.List<Connector>
    connectors(Ice.EndpointSelectionType selType)
    {
        return new java.util.ArrayList<Connector>();
    }

    public void
    connectors_async(Ice.EndpointSelectionType selType, EndpointI_connectors callback)
    {
        callback.connectors(new java.util.ArrayList<Connector>());
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
        endpoint.value = null;
        return null;
    }

    //
    // Expand endpoint out in to separate endpoints for each local
    // host if listening on INADDR_ANY on server side or if no host
    // was specified on client side.
    //
    public java.util.List<EndpointI>
    expand()
    {
        java.util.List<EndpointI> endps = new java.util.ArrayList<EndpointI>();
        endps.add(this);
        return endps;
    }

    //
    // Check whether the endpoint is equivalent to another one.
    //
    public boolean
    equivalent(EndpointI endpoint)
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
    public int
    compareTo(EndpointI obj) // From java.lang.Comparable
    {
        if(!(obj instanceof OpaqueEndpointI))
        {
            return type() < obj.type() ? -1 : 1;
        }

        OpaqueEndpointI p = (OpaqueEndpointI)obj;
        if(this == p)
        {
            return 0;
        }

        if(_type < p._type)
        {
            return -1;
        }
        else if(p._type < _type)
        {
            return 1;
        }

        if(_rawEncoding.major < p._rawEncoding.major)
        {
            return -1;
        }
        else if(p._rawEncoding.major < _rawEncoding.major)
        {
            return 1;
        }

        if(_rawEncoding.minor < p._rawEncoding.minor)
        {
            return -1;
        }
        else if(p._rawEncoding.minor < _rawEncoding.minor)
        {
            return 1;
        }

        if(_rawBytes.length < p._rawBytes.length)
        {
            return -1;
        }
        else if(p._rawBytes.length < _rawBytes.length)
        {
            return 1;
        }
        for(int i = 0; i < _rawBytes.length; i++)
        {
            if(_rawBytes[i] < p._rawBytes[i])
            {
                return -1;
            }
            else if(p._rawBytes[i] < _rawBytes[i])
            {
                return 1;
            }
        }

        return 0;
    }

    private void
    calcHashValue()
    {
        int h = 5381;
        h = IceInternal.HashUtil.hashAdd(h, _type);
        h = IceInternal.HashUtil.hashAdd(h, _rawEncoding);
        h = IceInternal.HashUtil.hashAdd(h, _rawBytes);
        _hashCode = h;
    }

    private short _type;
    private Ice.EncodingVersion _rawEncoding;
    private byte[] _rawBytes;
    private int _hashCode;
}
