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
    require("Ice/Address");
    require("Ice/HashUtil");
    require("Ice/StringUtil");
    require("Ice/TcpTransceiver");
    require("Ice/Endpoint");
    require("Ice/LocalException");

    var Ice = global.Ice || {};

    var Address = Ice.Address;
    var HashUtil = Ice.HashUtil;
    var StringUtil = Ice.StringUtil;
    var TcpTransceiver = Ice.TcpTransceiver;

    var Class = Ice.Class;
    var TcpEndpointI = Class(Ice.Endpoint, {
        __init__: function(instance, ho, po, ti, conId, co)
        {
            this._instance = instance;
            this._host = ho;
            this._port = po;
            this._timeout = ti;
            this._connectionId = conId;
            this._compress = co;
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
            var s = "tcp";

            if(this._host !== null && this._host.length > 0)
            {
                s += " -h ";
                var addQuote = this._host.indexOf(':') != -1;
                if(addQuote)
                {
                    s += "\"";
                }
                s += this._host;
                if(addQuote)
                {
                    s += "\"";
                }
            }

            s += " -p " + this._port;

            if(this._timeout != -1)
            {
                s += " -t " + this._timeout;
            }
            if(this._compress)
            {
                s += " -z";
            }
            return s;
        },
        //
        // Return the endpoint information.
        //
        getInfo: function()
        {
            return new TCPEndpointInfoI(this._timeout, this._compress, this._host, this._port);
        },
        //
        // Marshal the endpoint
        //
        streamWrite: function(s)
        {
            s.writeShort(Ice.TCPEndpointType);
            s.startWriteEncaps();
            s.writeString(this._host);
            s.writeInt(this._port);
            s.writeInt(this._timeout);
            s.writeBool(this._compress);
            s.endWriteEncaps();
        },
        //
        // Return the endpoint type
        //
        type: function()
        {
            return Ice.TCPEndpointType;
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
                return new TcpEndpointI(this._instance, this._host, this._port, timeout, this._connectionId,
                                        this._compress);
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
                return new TcpEndpointI(this._instance, this._host, this._port, this._timeout, connectionId,
                                        this._compress);
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
                return new TcpEndpointI(this._instance, this._host, this._port, this._timeout, this._connectionId,
                                        compress);
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
            return false;
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
            endpoint.value = this;
            return null;
        },
        connect: function()
        {
            if(this._instance.traceLevels().network >= 2)
            {
                var msg = "trying to establish tcp connection to " + this._host + ":" + this._port;
                this._instance.initializationData().logger.trace(this._instance.traceLevels().networkCat, msg);
            }

            return TcpTransceiver.createOutgoing(this._instance, new Address(this._host, this._port));
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
            if(!(p instanceof TcpEndpointI))
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

            if(!(p instanceof TcpEndpointI))
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

            if(this._host == p._host)
            {
                return 0;
            }
            else
            {
                return this._host < p._host ? -1 : 1;
            }
        },
        calcHashValue: function()
        {
            var h = 5381;
            h = HashUtil.addNumber(h, Ice.TCPEndpointType);
            h = HashUtil.addString(h, this._host);
            h = HashUtil.addNumber(h, this._port);
            h = HashUtil.addNumber(h, this._timeout);
            h = HashUtil.addString(h, this._connectionId);
            h = HashUtil.addBoolean(h, this._compress);
            this._hashCode = h;
        }
    });
    
    TcpEndpointI.fromString = function(instance, str, oaEndpoint)
    {
        var host = null;
        var port = 0;
        var timeout = -1;
        var compress = false;

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
                                                        "' in endpoint `tcp " + str + "'");
            }

            var argument = null;
            if(i < arr.length && arr[i].charAt(0) != '-')
            {
                argument = arr[i++];
                if(argument.charAt(0) == '\"' && argument.charAt(argument.length - 1) == '\"')
                {
                    argument = argument.substring(1, argument.length - 1);
                }
            }

            switch(option.charAt(1))
            {
                case 'h':
                {
                    if(argument === null)
                    {
                        throw new Ice.EndpointParseException(
                            "no argument provided for -h option in endpoint `tcp " + str + "'");
                    }

                    host = argument;
                    break;
                }

                case 'p':
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

                    break;
                }

                case 't':
                {
                    if(argument === null)
                    {
                        throw new Ice.EndpointParseException(
                            "no argument provided for -t option in endpoint `tcp " + str + "'");
                    }

                    try
                    {
                        timeout = StringUtil.toInt(argument);
                    }
                    catch(ex)
                    {
                        throw new Ice.EndpointParseException(
                            "invalid timeout value `" + argument + "' in endpoint `tcp " + str + "'");
                    }

                    break;
                }

                case 'z':
                {
                    if(argument !== null)
                    {
                        throw new Ice.EndpointParseException("unexpected argument `" + argument +
                                                                "' provided for -z option in `tcp " + str + "'");
                    }

                    compress = true;
                    break;
                }

                default:
                {
                    throw new Ice.EndpointParseException("unknown option `" + option + "' in `tcp " + str + "'");
                }
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
                throw new Ice.EndpointParseException("`-h *' not valid for proxy endpoint `tcp " + str + "'");
            }
        }

        if(host === null)
        {
            host = "";
        }

        return new TcpEndpointI(instance, host, port, timeout, "", compress);
    };

    TcpEndpointI.fromStream = function(s)
    {
        s.startReadEncaps();
        var host = s.readString();
        var port = s.readInt();
        var timeout = s.readInt();
        var compress = s.readBool();
        s.endReadEncaps();
        return new TcpEndpointI(s.instance, host, port, timeout, "", compress);
    };
    
    Ice.TcpEndpointI = TcpEndpointI;
    global.Ice = Ice;

    var TCPEndpointInfoI = Class(Ice.TCPEndpointInfo, {
        __init__: function(timeout, compress, host, port)
        {
            Ice.TCPEndpointInfo.call(this, timeout, compress, host, port);
        },
        type: function()
        {
            return Ice.TCPEndpointType;
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
}(typeof (global) === "undefined" ? window : global));
