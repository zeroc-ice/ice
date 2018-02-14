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
        "../Ice/StringUtil",
        "../Ice/BasicStream",
        "../Ice/Debug",
        "../Ice/OpaqueEndpointI",
        "../Ice/Protocol",
        "../Ice/LocalException"
    ]);

//
// Local aliases.
//
var Debug = Ice.Debug;
var BasicStream = Ice.BasicStream;
var EndpointParseException = Ice.EndpointParseException;
var OpaqueEndpointI = Ice.OpaqueEndpointI;
var Protocol = Ice.Protocol;
var StringUtil = Ice.StringUtil;

var EndpointFactoryManager = Ice.Class({
    __init__: function(instance)
    {
        this._instance = instance;
        this._factories = [];
    },
    add: function(factory)
    {
        for(var i = 0; i < this._factories.length; ++i)
        {
            Debug.assert(this._factories[i].type() != factory.type());
        }

        this._factories.push(factory);
    },
    get: function(type)
    {
        for(var i = 0; i < this._factories.length; ++i)
        {
            if(this._factories[i].type() === type)
            {
                return this._factories[i];
            }
        }
        return null;
    },
    create: function(str, oaEndpoint)
    {
        var s = str.trim();
        if(s.length === 0)
        {
            throw new EndpointParseException("value has no non-whitespace characters");
        }

        var arr = StringUtil.splitString(s, " \t\n\r");
        if(arr.length === 0)
        {
            throw new EndpointParseException("value has no non-whitespace characters");
        }

        var protocol = arr[0];
        arr.splice(0, 1);

        if(protocol === "default")
        {
            protocol = this._instance.defaultsAndOverrides().defaultProtocol;
        }

        for(var i = 0, length = this._factories.length; i < length; ++i)
        {
            if(this._factories[i].protocol() === protocol)
            {
                var e = this._factories[i].create(arr, oaEndpoint);
                if(arr.length > 0)
                {
                    throw new EndpointParseException("unrecognized argument `" + arr[0] + "' in endpoint `" +
                                                     str + "'");
                }
                return e;
            }
        }

        //
        // If the stringified endpoint is opaque, create an unknown endpoint,
        // then see whether the type matches one of the known endpoints.
        //
        if(protocol === "opaque")
        {
            var ue = new OpaqueEndpointI();
            ue.initWithOptions(arr);
            if(arr.length > 0)
            {
                throw new EndpointParseException("unrecognized argument `" + arr[0] + "' in endpoint `" + str + "'");
            }

            for(i = 0, length =  this._factories.length; i < length; ++i)
            {
                if(this._factories[i].type() == ue.type())
                {
                    //
                    // Make a temporary stream, write the opaque endpoint data into the stream,
                    // and ask the factory to read the endpoint data from that stream to create
                    // the actual endpoint.
                    //
                    var bs = new BasicStream(this._instance, Protocol.currentProtocolEncoding);
                    bs.writeShort(ue.type());
                    ue.streamWrite(bs);
                    bs.pos = 0;
                    bs.readShort(); // type
                    bs.startReadEncaps();
                    var endpoint = this._factories[i].read(bs);
                    bs.endReadEncaps();
                    return endpoint;
                }
            }
            return ue; // Endpoint is opaque, but we don't have a factory for its type.
        }

        return null;
    },
    read: function(s)
    {
        var e;
        var type = s.readShort();
        for(var i = 0; i < this._factories.length; ++i)
        {
            if(this._factories[i].type() == type)
            {
                s.startReadEncaps();
                e = this._factories[i].read(s);
                s.endReadEncaps();
                return e;
            }
        }
        s.startReadEncaps();
        e = new OpaqueEndpointI(type);
        e.initWithStream(s);
        s.endReadEncaps();
        return e;
    },
    destroy: function()
    {
        for(var i = 0; i < this._factories.length; ++i)
        {
            this._factories[i].destroy();
        }
        this._factories = [];
    }
});

Ice.EndpointFactoryManager = EndpointFactoryManager;
module.exports.Ice = Ice;
