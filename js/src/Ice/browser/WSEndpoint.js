// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;

Ice.__M.require(module, "Ice", 
    [
        "../Ice/Class", 
        "../Ice/Address",
        "../Ice/HashUtil",
        "../Ice/StringUtil",
        "../Ice/Endpoint",
        "../Ice/LocalException",
        "../Ice/browser/WSTransceiver"
    ]);

var Address = Ice.Address;
var HashUtil = Ice.HashUtil;
var StringUtil = Ice.StringUtil;
var WSTransceiver = Ice.WSTransceiver;

var Class = Ice.Class;

var WSEndpoint = Class(Ice.Endpoint, {
    __init__: function(instance, secure, ho, po, sif, ti, conId, co, re)
    {
        this._instance = instance;
        this._secure = secure;
        this._host = ho;
        this._port = po;
        this._timeout = ti;
        this._sourceAddress = sif;
        this._connectionId = conId;
        this._compress = co;
        this._resource = re;
        this.calcHashValue();
    },
    //
    // Convert the endpoint to its string form
    //
    toString: function()
    {
        //
        // WARNING: Certain features, such as proxy validation in Glacier2,
        // depend on the format of proxy strings. Changes to toString() and
        // methods called to generate parts of the reference string could break
        // these features. Please review for all features that depend on the
        // format of proxyToString() before changing this and related code.
        //
        var s = (this._secure ? "wss" : "ws");

        if(this._host !== null && this._host.length > 0)
        {
            s += " -h ";
            s += (this._host.indexOf(':') !== -1) ? ("\"" + this._host + "\"") : this._host;
        }

        s += " -p " + this._port;

        if(this._sourceAddress.length > 0)
        {
            s += " --sourceAddress " + this._sourceAddress;
        }

        if(this._timeout == -1)
        {
            s += " -t infinite";
        }
        else
        {
            s += " -t " + this._timeout;
        }

        if(this._compress)
        {
            s += " -z";
        }

        if(this._resource !== null && this._resource.length > 0)
        {
            s += " -r ";
            s += (this._resource.indexOf(':') !== -1) ? ("\"" + this._resource + "\"") : this._resource;
        }
        return s;
    },
    //
    // Return the endpoint information.
    //
    getInfo: function()
    {
        return new EndpointInfoI(this._secure, this._timeout, this._compress, this._host, this._port,
                                    this._sourceAddress, this._resource);
    },
    //
    // Marshal the endpoint
    //
    streamWrite: function(s)
    {
        s.writeShort(this._secure ? Ice.WSSEndpointType : Ice.WSEndpointType);
        s.startWriteEncaps();
        s.writeString(this._host);
        s.writeInt(this._port);
        s.writeInt(this._timeout);
        s.writeBool(this._compress);
        s.writeString(this._resource);
        s.endWriteEncaps();
    },
    //
    // Return the endpoint type
    //
    type: function()
    {
        return this._secure ? Ice.WSSEndpointType : Ice.WSEndpointType;
    },
    //
    // Return the timeout for the endpoint in milliseconds. 0 means
    // non-blocking, -1 means no timeout.
    //
    timeout: function()
    {
        return this._timeout;
    },
    //
    // Return a new endpoint with a different timeout value, provided
    // that timeouts are supported by the endpoint. Otherwise the same
    // endpoint is returned.
    //
    changeTimeout: function(timeout)
    {
        if(timeout === this._timeout)
        {
            return this;
        }
        else
        {
            return new WSEndpoint(this._instance, this._secure, this._host, this._port, this._sourceAddress,
                                    timeout, this._connectionId, this._compress, this._resource);
        }
    },
    //
    // Return a new endpoint with a different connection id.
    //
    changeConnectionId: function(connectionId)
    {
        if(connectionId === this._connectionId)
        {
            return this;
        }
        else
        {
            return new WSEndpoint(this._instance, this._secure, this._host, this._port, this._sourceAddress,
                                    this._timeout, connectionId, this._compress, this._resource);
        }
    },
    //
    // Return true if the endpoints support bzip2 compress, or false
    // otherwise.
    //
    compress: function()
    {
        return this._compress;
    },
    //
    // Return a new endpoint with a different compression value,
    // provided that compression is supported by the
    // endpoint. Otherwise the same endpoint is returned.
    //
    changeCompress: function(compress)
    {
        if(compress === this._compress)
        {
            return this;
        }
        else
        {
            return new WSEndpoint(this._instance, this._secure, this._host, this._port, this._sourceAddress,
                                    this._timeout, this._connectionId, compress, this._resource);
        }
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
        return this._secure;
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
        return null;
    },
    connect: function()
    {
        if(this._instance.traceLevels().network >= 2)
        {
            this._instance.initializationData().logger.trace(this._instance.traceLevels().networkCat,
                "trying to establish " + (this._secure ? "wss" : "ws") + " connection to " + this._host + ":" +
                this._port);
        }

        return WSTransceiver.createOutgoing(this._instance, this._secure, new Address(this._host, this._port),
                                            this._resource);
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
        if(!(p instanceof WSEndpoint))
        {
            return false;
        }

        if(this === p)
        {
            return true;
        }

        if(this._host !== p._host)
        {
            return false;
        }

        if(this._port !== p._port)
        {
            return false;
        }

        if(this._timeout !== p._timeout)
        {
            return false;
        }

        if(this._connectionId !== p._connectionId)
        {
            return false;
        }

        if(this._compress !== p._compress)
        {
            return false;
        }

        if(this._resource !== p._resource)
        {
            return false;
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

        if(!(p instanceof WSEndpoint))
        {
            return this.type() < p.type() ? -1 : 1;
        }

        if(this._port < p._port)
        {
            return -1;
        }
        else if(p._port < this._port)
        {
            return 1;
        }

        if(this._timeout < p._timeout)
        {
            return -1;
        }
        else if(p._timeout < this._timeout)
        {
            return 1;
        }

        if(this._connectionId != p._connectionId)
        {
            return this._connectionId < p._connectionId ? -1 : 1;
        }

        if(!this._compress && p._compress)
        {
            return -1;
        }
        else if(!p._compress && this._compress)
        {
            return 1;
        }

        if(this._host < p._host)
        {
            return -1;
        }
        else if(p._host < this._host)
        {
            return 1;
        }

        if(this._resource == p._resource)
        {
            return 0;
        }
        else
        {
            return this._resource < p._resource ? -1 : 1;
        }
    },
    calcHashValue: function()
    {
        var h = 5381;
        h = HashUtil.addNumber(h, this._secure ? Ice.WSSEndpointType : Ice.WSEndpointType);
        h = HashUtil.addString(h, this._host);
        h = HashUtil.addNumber(h, this._port);
        h = HashUtil.addNumber(h, this._timeout);
        h = HashUtil.addString(h, this._connectionId);
        h = HashUtil.addBoolean(h, this._compress);
        h = HashUtil.addString(h, this._resource);
        this._hashCode = h;
    }
});

WSEndpoint.fromString = function(instance, secure, str, oaEndpoint)
{
    var host = null;
    var port = 0;
    var sourceAddress = "";
    var timeout = -2;
    var compress = false;
    var resource = "";

    var protocol = secure ? "wss" : "ws";

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
        var argument = null;
        if(i < arr.length && arr[i].charAt(0) != '-')
        {
            argument = arr[i++];
            if(argument.charAt(0) == '\"' && argument.charAt(argument.length - 1) == '\"')
            {
                argument = argument.substring(1, argument.length - 1);
            }
        }


        if(option === "-h")
        {
            if(argument === null)
            {
                throw new Ice.EndpointParseException(
                    "no argument provided for -h option in endpoint `tcp " + str + "'");
            }

            host = argument;
        }
        else if(option === "-p")
        {
            if(argument === null)
            {
                throw new Ice.EndpointParseException(
                    "no argument provided for -p option in endpoint `tcp " + str + "'");
            }

            try
            {
                port = StringUtil.toInt(argument);
            }
            catch(ex)
            {
                throw new Ice.EndpointParseException("invalid port value `" + argument +
                                                        "' in endpoint `tcp " + str + "'");
            }

            if(port < 0 || port > 65535)
            {
                throw new Ice.EndpointParseException("port value `" + argument +
                                                        "' out of range in endpoint `tcp " + str + "'");
            }
        }
        else if(option === "-r")
        {
            if(argument === null)
            {
                throw new Ice.EndpointParseException("no argument provided for -r option in endpoint `" +
                                                        protocol + " " + str + "'");
            }

            resource = argument;
        }
        else if(option === "-t")
        {
            if(argument === null)
            {
                throw new Ice.EndpointParseException(
                    "no argument provided for -t option in endpoint `tcp " + str + "'");
            }

            if(argument == "infinite")
            {
                timeout = -1;
            }
            else
            {
                var invalid = false;
                try
                {
                    timeout = StringUtil.toInt(argument);
                }
                catch(ex)
                {
                    invalid = true;
                }
                if(invalid || timeout < 1)
                {
                    throw new Ice.EndpointParseException(
                        "invalid timeout value `" + argument + "' in endpoint `tcp " + str + "'");
                }
            }
        }
        else if(option === "-z")
        {
            if(argument !== null)
            {
                throw new Ice.EndpointParseException("unexpected argument `" + argument +
                                                        "' provided for -z option in `tcp " + str + "'");
            }

            compress = true;
        }
        else if(option === "--sourceAddress")
        {
            if(argument === null)
            {
                throw new Ice.EndpointParseException(
                    "no argument provided for --sourceAddress option in endpoint `tcp " + str + "'");
            }

            sourceAddress = argument;
        }
        else
        {
            throw new Ice.EndpointParseException("unknown option `" + option + "' in `tcp " + str + "'");
        }
    }

    if(host === null)
    {
        host = instance.defaultsAndOverrides().defaultHost;
    }
    else if(host == "*")
    {
        if(oaEndpoint)
        {
            host = null;
        }
        else
        {
            throw new Ice.EndpointParseException("`-h *' not valid for proxy endpoint `" + protocol + " " + str +
                                                    "'");
        }
    }

    if(host === null)
    {
        host = "";
    }

    if(timeout == -2)
    {
        timeout = instance.defaultsAndOverrides().defaultTimeout;
    }

    return new WSEndpoint(instance, secure, host, port, sourceAddress, timeout, "", compress, resource);
};

WSEndpoint.fromStream = function(s, secure)
{
    s.startReadEncaps();
    var host = s.readString();
    var port = s.readInt();
    var timeout = s.readInt();
    var compress = s.readBool();
    var resource = s.readString();
    s.endReadEncaps();
    return new WSEndpoint(s.instance, secure, host, port, "", timeout, "", compress, resource);
};

Ice.WSEndpoint = WSEndpoint;

var EndpointInfoI = Class(Ice.WSEndpointInfo, {
    __init__: function(secure, timeout, compress, host, port, sourceAddress, resource)
    {
        Ice.WSEndpointInfo.call(this, timeout, compress, host, port, sourceAddress, resource);
        this.secure = secure;
    },
    type: function()
    {
        return this._secure ? Ice.WSSEndpointType : Ice.WSEndpointType;
    },
    datagram: function()
    {
        return false;
    },
    secure: function()
    {
        return this._secure;
    }
});
