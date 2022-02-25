//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;

require("../Ice/Base64");
require("../Ice/Debug");
require("../Ice/EndpointI");
require("../Ice/FormatType");
require("../Ice/HashUtil");
require("../Ice/LocalException");
require("../Ice/StringUtil");

const Base64 = Ice.Base64;
const Debug = Ice.Debug;
const EndpointParseException = Ice.EndpointParseException;
const HashUtil = Ice.HashUtil;
const StringUtil = Ice.StringUtil;

class OpaqueEndpointI extends Ice.EndpointI
{
    constructor(type)
    {
        super();
        this._rawEncoding = Ice.Encoding_1_0;
        this._type = type === undefined ? -1 : type;
        this._rawBytes = null;
    }

    //
    // Marshal the endpoint
    //
    streamWrite(s)
    {
        s.startEncapsulation(this._rawEncoding, Ice.FormatType.DefaultFormat);
        s.writeBlob(this._rawBytes);
        s.endEncapsulation();
    }

    //
    // Return the endpoint information.
    //
    getInfo()
    {
        return new OpaqueEndpointInfoI(null, -1, false, this._rawEncoding, this._rawBytes, this._type);
    }

    //
    // Return the endpoint type
    //
    type()
    {
        return this._type;
    }

    protocol()
    {
        return "opaque";
    }

    //
    // Return the timeout for the endpoint in milliseconds. 0 means
    // non-blocking, -1 means no timeout.
    //
    timeout()
    {
        return -1;
    }

    //
    // Return a new endpoint with a different timeout value, provided
    // that timeouts are supported by the endpoint. Otherwise the same
    // endpoint is returned.
    //
    changeTimeout(t)
    {
        return this;
    }

    //
    // Return a new endpoint with a different connection id.
    //
    changeConnectionId(connectionId)
    {
        return this;
    }

    //
    // Return true if the endpoints support bzip2 compress, or false
    // otherwise.
    //
    compress()
    {
        return false;
    }

    //
    // Return a new endpoint with a different compression value,
    // provided that compression is supported by the
    // endpoint. Otherwise the same endpoint is returned.
    //
    changeCompress(compress)
    {
        return this;
    }

    //
    // Return true if the endpoint is datagram-based.
    //
    datagram()
    {
        return false;
    }

    //
    // Return true if the endpoint is secure.
    //
    secure()
    {
        return false;
    }

    //
    // Get the encoded endpoint.
    //
    rawBytes()
    {
        return this._rawBytes; // Returns a Uint8Array
    }

    //
    // Return a server side transceiver for this endpoint, or null if a
    // transceiver can only be created by an acceptor. In case a
    // transceiver is created, this operation also returns a new
    // "effective" endpoint, which might differ from this endpoint,
    // for example, if a dynamic port number is assigned.
    //
    transceiver(endpoint)
    {
        endpoint.value = null;
        return null;
    }

    //
    // Return an acceptor for this endpoint, or null if no acceptors
    // is available. In case an acceptor is created, this operation
    // also returns a new "effective" endpoint, which might differ
    // from this endpoint, for example, if a dynamic port number is
    // assigned.
    //
    acceptor(endpoint, adapterName)
    {
        endpoint.value = this;
        return null;
    }

    connect()
    {
        return null;
    }

    hashCode()
    {
        if(this._hashCode === undefined)
        {
            let h = 5381;
            h = HashUtil.addNumber(h, this._type);
            h = HashUtil.addHashable(h, this._rawEncoding);
            h = HashUtil.addArray(h, this._rawBytes, HashUtil.addNumber);
            this._hashCode = h;
        }
        return this._hashCode;
    }

    options()
    {
        let s = "";
        s += " -t " + this._type;
        s += " -e " + Ice.encodingVersionToString(this._rawEncoding);
        s += " -v " + Base64.encode(this._rawBytes);
        return s;
    }

    //
    // Compare endpoints for sorting purposes
    //
    equals(p)
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
        for(let i = 0; i < this._rawBytes.length; i++)
        {
            if(this._rawBytes[i] !== p._rawBytes[i])
            {
                return false;
            }
        }

        return true;
    }

    compareTo(p)
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
        for(let i = 0; i < this._rawBytes.length; i++)
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
    }

    checkOption(option, argument, endpoint)
    {
        switch(option.charAt(1))
        {
            case 't':
            {
                if(this._type > -1)
                {
                    throw new EndpointParseException("multiple -t options in endpoint " + endpoint);
                }
                if(argument === null)
                {
                    throw new EndpointParseException("no argument provided for -t option in endpoint " + endpoint);
                }

                let type;

                try
                {
                    type = StringUtil.toInt(argument);
                }
                catch(ex)
                {
                    throw new EndpointParseException("invalid type value `" + argument + "' in endpoint " + endpoint);
                }

                if(type < 0 || type > 65535)
                {
                    throw new EndpointParseException("type value `" + argument + "' out of range in endpoint " +
                                                     endpoint);
                }

                this._type = type;
                return true;
            }

            case 'v':
            {
                if(this._rawBytes)
                {
                    throw new EndpointParseException("multiple -v options in endpoint " + endpoint);
                }
                if(argument === null || argument.length === 0)
                {
                    throw new EndpointParseException("no argument provided for -v option in endpoint " + endpoint);
                }
                for(let i = 0; i < argument.length; ++i)
                {
                    if(!Base64.isBase64(argument.charAt(i)))
                    {
                        throw new EndpointParseException("invalid base64 character `" + argument.charAt(i) +
                                                         "' (ordinal " + argument.charCodeAt(i) +
                                                         ") in endpoint " + endpoint);
                    }
                }
                this._rawBytes = Base64.decode(argument);
                return true;
            }

            case 'e':
            {
                if(argument === null)
                {
                    throw new EndpointParseException("no argument provided for -e option in endpoint " + endpoint);
                }
                try
                {
                    this._rawEncoding = Ice.stringToEncodingVersion(argument);
                }
                catch(e)
                {
                    throw new EndpointParseException("invalid encoding version `" + argument +
                                                     "' in endpoint " + endpoint + ":\n" + e.str);
                }
                return true;
            }

            default:
            {
                return false;
            }
        }
    }

    initWithOptions(args)
    {
        super.initWithOptions(args);
        Debug.assert(this._rawEncoding);

        if(this._type < 0)
        {
            throw new EndpointParseException("no -t option in endpoint `" + this + "'");
        }
        if(this._rawBytes === null || this._rawBytes.length === 0)
        {
            throw new EndpointParseException("no -v option in endpoint `" + this + "'");
        }
    }

    initWithStream(s)
    {
        this._rawEncoding = s.getEncoding();
        this._rawBytes = s.readBlob(s.getEncapsulationSize());
    }
}

class OpaqueEndpointInfoI extends Ice.OpaqueEndpointInfo
{
    constructor(timeout, compress, rawEncoding, rawBytes, type)
    {
        super(-1, false, rawEncoding, rawBytes);
        this._type = type;
    }

    type()
    {
        return this._type;
    }

    datagram()
    {
        return false;
    }

    secure()
    {
        return false;
    }
}

Ice.OpaqueEndpointI = OpaqueEndpointI;
module.exports.Ice = Ice;
