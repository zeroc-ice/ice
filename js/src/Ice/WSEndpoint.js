// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;

Ice.__M.require(module,
    [
        "../Ice/Class",
        "../Ice/HashUtil",
        "../Ice/StringUtil",
        "../Ice/EndpointI",
        "../Ice/LocalException",
        "../Ice/WSTransceiver",
        "../Ice/EndpointInfo"
    ]);
var IceSSL = Ice.__M.module("IceSSL");

var HashUtil = Ice.HashUtil;
var StringUtil = Ice.StringUtil;
var EndpointI = Ice.EndpointI;
var Class = Ice.Class;

var WSEndpoint = Class(EndpointI, {
    __init__: function(instance, del, re)
    {
        this._instance = instance;
        this._delegate = del;
        this._resource = re || "/";
    },
    getInfo: function()
    {
        var info = new Ice.WSEndpointInfo();
        info.resource = this._resource;
        info.underlying = this._delegate.getInfo();
        info.timeout = info.underlying.timeout;
        info.compress = info.underlying.compress;
        return info;
    },
    type: function()
    {
        return this._delegate.type();
    },
    protocol: function()
    {
        return this._delegate.protocol();
    },
    streamWrite: function(s)
    {
        s.startEncapsulation();
        this._delegate.streamWriteImpl(s);
        s.writeString(this._resource);
        s.endEncapsulation();
    },
    timeout: function()
    {
        return this._delegate.timeout();
    },
    changeTimeout: function(timeout)
    {
        if(timeout === this._delegate.timeout())
        {
            return this;
        }
        else
        {
            return new WSEndpoint(this._instance, this._delegate.changeTimeout(timeout), this._resource);
        }
    },
    changeConnectionId: function(connectionId)
    {
        if(connectionId === this._delegate.connectionId())
        {
            return this;
        }
        else
        {
            return new WSEndpoint(this._instance, this._delegate.changeConnectionId(connectionId), this._resource);
        }
    },
    compress: function()
    {
        return this._delegate.compress();
    },
    changeCompress: function(compress)
    {
        if(compress === this._delegate.compress())
        {
            return this;
        }
        else
        {
            return new WSEndpoint(this._instance, this._delegate.changeCompress(compress), this._resource);
        }
    },
    datagram: function()
    {
        return this._delegate.datagram();
    },
    secure: function()
    {
        return this._delegate.secure();
    },
    connect: function()
    {
        return Ice.WSTransceiver.createOutgoing(this._instance,
                                                this._delegate.secure(),
                                                this._delegate.getAddress(),
                                                this._resource);
    },
    hashCode: function()
    {
        if(this._hashCode === undefined)
        {
            this._hashCode = this._delegate.hashCode();
            this._hashCode = HashUtil.addString(this._hashCode, this._resource);
        }
        return this._hashCode;
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

        var r = this._delegate.compareTo(p._delegate);
        if(r !== 0)
        {
            return r;
        }

        if(this._resource !== p._resource)
        {
            return this._resource < p._resource ? -1 : 1;
        }

        return 0;
    },
    options: function()
    {
        //
        // WARNING: Certain features, such as proxy validation in Glacier2,
        // depend on the format of proxy strings. Changes to toString() and
        // methods called to generate parts of the reference string could break
        // these features. Please review for all features that depend on the
        // format of proxyToString() before changing this and related code.
        //
        var s = this._delegate.options();

        if(this._resource !== null && this._resource.length > 0)
        {
            s += " -r ";
            s += (this._resource.indexOf(':') !== -1) ? ("\"" + this._resource + "\"") : this._resource;
        }

        return s;
    },
    toConnectorString: function()
    {
        return this._delegate.toConnectorString();
    },
    initWithStream: function(s)
    {
        this._resource = s.readString();
    },
    checkOption: function(option, argument, endpoint)
    {
        if(option === "-r")
        {
            if(argument === null)
            {
                throw new Ice.EndpointParseException("no argument provided for -r option in endpoint " + endpoint);
            }
            this._resource = argument;
        }
        else
        {
            return false;
        }
        return true;
    },
});

if(typeof(Ice.WSTransceiver) !== "undefined")
{
    WSEndpoint.prototype.connectable = function()
    {
        return true;
    };
}
else
{
    WSEndpoint.prototype.connectable = function()
    {
        return false;
    };
}

Ice.WSEndpoint = WSEndpoint;
exports.Ice = Ice;
