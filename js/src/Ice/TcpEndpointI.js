// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module,
    [
        "../Ice/Class",
        "../Ice/Debug",
        "../Ice/HashUtil",
        "../Ice/StringUtil",
        "../Ice/IPEndpointI",
        "../Ice/TcpTransceiver",
        "../Ice/LocalException",
        "../Ice/EndpointInfo"
    ]);

var IceSSL = Ice.__M.require(module, ["../Ice/EndpointInfo"]).IceSSL;

var Debug = Ice.Debug;
var HashUtil = Ice.HashUtil;
var StringUtil = Ice.StringUtil;
var TcpTransceiver = typeof(Ice.TcpTransceiver) !== "undefined" ? Ice.TcpTransceiver : null;
var Class = Ice.Class;

var TcpEndpointI = Class(Ice.IPEndpointI, {
    __init__: function(instance, ho, po, sif, ti, conId, co)
    {
        Ice.IPEndpointI.call(this, instance, ho, po, sif, conId);
        this._timeout = ti === undefined ? (instance ? instance.defaultTimeout() : undefined) : ti;
        this._compress = co === undefined ? false : co;
    },
    //
    // Return the endpoint information.
    //
    getInfo: function()
    {
        var info = this.secure() ? new IceSSL.EndpointInfo() : new Ice.TCPEndpointInfo();
        this.fillEndpointInfo(info);
        return info;
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
            return new TcpEndpointI(this._instance, this._host, this._port, this._sourceAddr, timeout,
                                    this._connectionId, this._compress);
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
            return new TcpEndpointI(this._instance, this._host, this._port, this._sourceAddr, this._timeout,
                                    connectionId, this._compress);
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
            return new TcpEndpointI(this._instance, this._host, this._port, this._sourceAddr, this._timeout,
                                    this._connectionId, compress);
        }
    },
    //
    // Return true if the endpoint is datagram-based.
    //
    datagram: function()
    {
        return false;
    },
    connectable: function()
    {
        //
        // TCP endpoints are not connectable when running in a browser, SSL
        // isn't currently supported.
        //
        return TcpTransceiver !== null && !this.secure();
    },
    connect: function()
    {
        Debug.assert(!this.secure());
        return TcpTransceiver.createOutgoing(this._instance, this.getAddress(), this._sourceAddr);
    },
    //
    // Convert the endpoint to its string form
    //
    options: function()
    {
        //
        // WARNING: Certain features, such as proxy validation in Glacier2,
        // depend on the format of proxy strings. Changes to toString() and
        // methods called to generate parts of the reference string could break
        // these features. Please review for all features that depend on the
        // format of proxyToString() before changing this and related code.
        //
        var s = Ice.IPEndpointI.prototype.options.call(this);
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
        return s;
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

        if(this._timeout < p._timeout)
        {
            return -1;
        }
        else if(p._timeout < this._timeout)
        {
            return 1;
        }

        if(!this._compress && p._compress)
        {
            return -1;
        }
        else if(!p._compress && this._compress)
        {
            return 1;
        }

        return Ice.IPEndpointI.prototype.compareTo.call(this, p);
    },
    streamWriteImpl: function(s)
    {
        Ice.IPEndpointI.prototype.streamWriteImpl.call(this, s);
        s.writeInt(this._timeout);
        s.writeBool(this._compress);
    },
    hashInit: function(h)
    {
        h = Ice.IPEndpointI.prototype.hashInit.call(this, h);
        h = HashUtil.addNumber(h, this._timeout);
        h = HashUtil.addBoolean(h, this._compress);
        return h;
    },
    fillEndpointInfo: function(info)
    {
        Ice.IPEndpointI.prototype.fillEndpointInfo.call(this, info);
        info.timeout = this._timeout;
        info.compress = this._compress;
    },
    initWithStream: function(s)
    {
        Ice.IPEndpointI.prototype.initWithStream.call(this, s);
        this._timeout = s.readInt();
        this._compress = s.readBool();
    },
    checkOption: function(option, argument, endpoint)
    {
        if(Ice.IPEndpointI.prototype.checkOption.call(this, option, argument, endpoint))
        {
            return true;
        }

        if(option === "-t")
        {
            if(argument === null)
            {
                throw new Ice.EndpointParseException("no argument provided for -t option in endpoint " + endpoint);
            }

            if(argument == "infinite")
            {
                this._timeout = -1;
            }
            else
            {
                var invalid = false;
                try
                {
                    this._timeout = StringUtil.toInt(argument);
                }
                catch(ex)
                {
                    invalid = true;
                }
                if(invalid || this._timeout < 1)
                {
                    throw new Ice.EndpointParseException("invalid timeout value `" + argument + "' in endpoint " +
                                                         endpoint);
                }
            }
        }
        else if(option === "-z")
        {
            if(argument !== null)
            {
                throw new Ice.EndpointParseException("unexpected argument `" + argument +
                                                     "' provided for -z option in " + endpoint);
            }

            this._compress = true;
        }
        else
        {
            return false;
        }
        return true;
    },
    createEndpoint: function(host, port, conId)
    {
        return new TcpEndpointI(this._instance, host, port, this._sourceAddr, this._timeout, conId, this._compress);
    }
});

Ice.TcpEndpointI = TcpEndpointI;
module.exports.Ice = Ice;
