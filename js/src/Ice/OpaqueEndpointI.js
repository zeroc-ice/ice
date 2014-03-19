// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(global){
    require("Ice/Class");
    require("Ice/Base64");
    require("Ice/Debug");
    require("Ice/FormatType");
    require("Ice/HashUtil");
    require("Ice/Protocol");
    require("Ice/StringUtil");
    require("Ice/Endpoint");
    require("Ice/LocalException");
    
    var Ice = global.Ice || {};
    
    var Base64 = Ice.Base64;
    var Debug = Ice.Debug;
    var HashUtil = Ice.HashUtil;
    var Protocol = Ice.Protocol;
    var StringUtil = Ice.StringUtil;
    
    var Class = Ice.Class;

    var OpaqueEndpointI = Class(Ice.Endpoint, {
        //
        // Marshal the endpoint
        //
        streamWrite: function(s)
        {
            s.writeShort(this._type);
            s.startWriteEncaps(this._rawEncoding, Ice.FormatType.DefaultFormat);
            s.writeBlob(this._rawBytes);
            s.endWriteEncaps();
        },
        //
        // Convert the endpoint to its string form
        //
        toString: function()
        {
            var val = Base64.encode(this._rawBytes);
            return "opaque -t " + this._type + " -e " + Ice.encodingVersionToString(this._rawEncoding) +
                   " -v " + val;
        },
        //
        // Return the endpoint information.
        //
        getInfo: function()
        {
            return new OpaqueEndpointInfoI(-1, false, this._rawEncoding, this._rawBytes, this._type);
        },
        //
        // Return the endpoint type
        //
        type: function()
        {
            return this._type;
        },
        protocol: function()
        {
            return "opaque";
        },
        //
        // Return the timeout for the endpoint in milliseconds. 0 means
        // non-blocking, -1 means no timeout.
        //
        timeout: function()
        {
            return -1;
        },
        //
        // Return a new endpoint with a different timeout value, provided
        // that timeouts are supported by the endpoint. Otherwise the same
        // endpoint is returned.
        //
        changeTimeout: function(t)
        {
            return this;
        },
        //
        // Return a new endpoint with a different connection id.
        //
        changeConnectionId: function(connectionId)
        {
            return this;
        },
        //
        // Return true if the endpoints support bzip2 compress, or false
        // otherwise.
        //
        compress: function()
        {
            return false;
        },
        //
        // Return a new endpoint with a different compression value,
        // provided that compression is supported by the
        // endpoint. Otherwise the same endpoint is returned.
        //
        changeCompress: function(compress)
        {
            return this;
        },
        //
        // Return true if the endpoint is datagram-based.
        //
        datagram: function()
        {
            return false;
        },
        //
        // Return true if the endpoint is secure.
        //
        secure: function()
        {
            return false;
        },
        //
        // Get the encoded endpoint.
        //
        rawBytes: function()
        {
            return this._rawBytes; // Returns a Uint8Array
        },
        //
        // Return a server side transceiver for this endpoint, or null if a
        // transceiver can only be created by an acceptor. In case a
        // transceiver is created, this operation also returns a new
        // "effective" endpoint, which might differ from this endpoint,
        // for example, if a dynamic port number is assigned.
        //
        transceiver: function(endpoint)
        {
            endpoint.value = null;
            return null;
        },
        //
        // Return an acceptor for this endpoint, or null if no acceptors
        // is available. In case an acceptor is created, this operation
        // also returns a new "effective" endpoint, which might differ
        // from this endpoint, for example, if a dynamic port number is
        // assigned.
        //
        acceptor: function(endpoint, adapterName)
        {
            endpoint.value = this;
            return null;
        },
        connect: function()
        {
            return null;
        },
        hashCode: function()
        {
            return this._hashCode;
        },
        //
        // Compare endpoints for sorting purposes
        //
        equals: function(p)
        {
            if(!(p instanceof OpaqueEndpointI))
            {
                return false;
            }

            if(this === p)
            {
                return true;
            }

            if(this._type !== p._type)
            {
                return false;
            }

            if(!this._rawEncoding.equals(p._rawEncoding))
            {
                return false;
            }

            if(this._rawBytes.length !== p._rawBytes.length)
            {
                return false;
            }
            for(var i = 0; i < this._rawBytes.length; i++)
            {
                if(this._rawBytes[i] !== p._rawBytes[i])
                {
                    return false;
                }
            }

            return true;
        },
        compareTo: function(p)
        {
            if(this === p)
            {
                return 0;
            }

            if(p === null)
            {
                return 1;
            }

            if(!(p instanceof OpaqueEndpointI))
            {
                return this.type() < p.type() ? -1 : 1;
            }

            if(this._type < p._type)
            {
                return -1;
            }
            else if(p._type < this._type)
            {
                return 1;
            }

            if(this._rawEncoding.major < p._rawEncoding.major)
            {
                return -1;
            }
            else if(p._rawEncoding.major < this._rawEncoding.major)
            {
                return 1;
            }

            if(this._rawEncoding.minor < p._rawEncoding.minor)
            {
                return -1;
            }
            else if(p._rawEncoding.minor < this._rawEncoding.minor)
            {
                return 1;
            }

            if(this._rawBytes.length < p._rawBytes.length)
            {
                return -1;
            }
            else if(p._rawBytes.length < this._rawBytes.length)
            {
                return 1;
            }
            for(var i = 0; i < this._rawBytes.length; i++)
            {
                if(this._rawBytes[i] < p._rawBytes[i])
                {
                    return -1;
                }
                else if(p._rawBytes[i] < this._rawBytes[i])
                {
                    return 1;
                }
            }

            return 0;
        },
        calcHashValue: function()
        {
            var h = 5381;
            h = HashUtil.addNumber(h, this._type);
            h = HashUtil.addHashable(h, this._rawEncoding);
            h = HashUtil.addArray(h, this._rawBytes, HashUtil.addNumber);
            this._hashCode = h;
        }
    });
    
    OpaqueEndpointI.fromString = function(str)
    {
        var result = new OpaqueEndpointI();

        result._rawEncoding = Protocol.Encoding_1_0;

        var topt = 0;
        var vopt = 0;

        var arr = str.split(/[ \t\n\r]+/);
        var i = 0;
        while(i < arr.length)
        {
            if(arr[i].length === 0)
            {
                i++;
                continue;
            }

            var option = arr[i++];
            if(option.length != 2 && option.charAt(0) != '-')
            {
                throw new Ice.EndpointParseException("expected an endpoint option but found `" + option +
                                                    "' in endpoint `opaque " + str + "'");
            }

            var argument = null;
            if(i < arr.length && arr[i].charAt(0) != '-')
            {
                argument = arr[i++];
            }

            switch(option.charAt(1))
            {
                case 't':
                {
                    if(argument === null)
                    {
                        throw new Ice.EndpointParseException("no argument provided for -t option in endpoint `opaque " +
                                                            str + "'");
                    }

                    var type;

                    try
                    {
                        type = StringUtil.toInt(argument);
                    }
                    catch(ex)
                    {
                        throw new Ice.EndpointParseException("invalid type value `" + argument +
                                                            "' in endpoint `opaque " + str + "'");
                    }

                    if(type < 0 || type > 65535)
                    {
                        throw new Ice.EndpointParseException("type value `" + argument +
                                                            "' out of range in endpoint `opaque " + str + "'");
                    }

                    result._type = type;
                    ++topt;
                    if(topt > 1)
                    {
                        throw new Ice.EndpointParseException("multiple -t options in endpoint `opaque " + str + "'");
                    }
                    break;
                }

                case 'v':
                {
                    if(argument === null || argument.length === 0)
                    {
                        throw new Ice.EndpointParseException("no argument provided for -v option in endpoint `opaque " +
                                                            str + "'");
                    }
                    for(var j = 0; j < argument.length; ++j)
                    {
                        if(!Base64.isBase64(argument.charAt(j)))
                        {
                            throw new Ice.EndpointParseException("invalid base64 character `" + argument.charAt(j) +
                                                                "' (ordinal " + argument.charCodeAt(j) +
                                                                ") in endpoint `opaque " + str + "'");
                        }
                    }
                    result._rawBytes = Base64.decode(argument);
                    ++vopt;
                    if(vopt > 1)
                    {
                        throw new Ice.EndpointParseException("multiple -v options in endpoint `opaque " + str + "'");
                    }
                    break;
                }

                case 'e':
                {
                    if(argument === null)
                    {
                        throw new Ice.EndpointParseException("no argument provided for -e option in endpoint `opaque " +
                                                            str + "'");
                    }
                    try
                    {
                        result._rawEncoding = Ice.stringToEncodingVersion(argument);
                    }
                    catch(e)
                    {
                        throw new Ice.EndpointParseException("invalid encoding version `" + argument +
                                                            "' in endpoint `opaque " + str + "':\n" + e.str);
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
        result.calcHashValue();
        return result;
    };

    OpaqueEndpointI.fromStream = function(type, s)
    {
        var result = new OpaqueEndpointI();
        result._type = type;
        result._rawEncoding = s.startReadEncaps();
        var sz = s.getReadEncapsSize();
        result._rawBytes = s.readBlob(sz);
        s.endReadEncaps();
        result.calcHashValue();
        return result;
    };

    Ice.OpaqueEndpointI = OpaqueEndpointI;
    
    var OpaqueEndpointInfoI = Class(Ice.OpaqueEndpointInfo, {
        __init__: function(timeout, compress, rawEncoding, rawBytes, type)
        {
            Ice.OpaqueEndpointInfo.call(this, -1, false, rawEncoding, rawBytes);
            this._type = type;
        },
        type: function()
        {
            return this._type;
        },
        datagram: function()
        {
            return false;
        },
        secure: function()
        {
            return false;
        }
    });
    
    global.Ice = Ice;
}(typeof (global) === "undefined" ? window : global));
