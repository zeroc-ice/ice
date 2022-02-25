//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;

require("../Ice/Debug");
require("../Ice/LocalException");
require("../Ice/OpaqueEndpointI");
require("../Ice/Protocol");
require("../Ice/Stream");
require("../Ice/StringUtil");

//
// Local aliases.
//
const Debug = Ice.Debug;
const InputStream = Ice.InputStream;
const OutputStream = Ice.OutputStream;
const EndpointParseException = Ice.EndpointParseException;
const OpaqueEndpointI = Ice.OpaqueEndpointI;
const Protocol = Ice.Protocol;
const StringUtil = Ice.StringUtil;

class EndpointFactoryManager
{
    constructor(instance)
    {
        this._instance = instance;
        this._factories = [];
    }

    add(factory)
    {
        Debug.assert(this._factories.find(f => factory.type() == f.type()) === undefined);
        this._factories.push(factory);
    }

    get(type)
    {
        return this._factories.find(f => type == f.type()) || null;
    }

    create(str, oaEndpoint)
    {
        const s = str.trim();
        if(s.length === 0)
        {
            throw new EndpointParseException("value has no non-whitespace characters");
        }

        const arr = StringUtil.splitString(s, " \t\n\r");
        if(arr.length === 0)
        {
            throw new EndpointParseException("value has no non-whitespace characters");
        }

        let protocol = arr[0];
        arr.splice(0, 1);

        if(protocol === "default")
        {
            protocol = this._instance.defaultsAndOverrides().defaultProtocol;
        }
        for(let i = 0, length = this._factories.length; i < length; ++i)
        {
            if(this._factories[i].protocol() === protocol)
            {
                const e = this._factories[i].create(arr, oaEndpoint);
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
            const ue = new OpaqueEndpointI();
            ue.initWithOptions(arr);
            if(arr.length > 0)
            {
                throw new EndpointParseException("unrecognized argument `" + arr[0] + "' in endpoint `" + str + "'");
            }

            for(let i = 0, length = this._factories.length; i < length; ++i)
            {
                if(this._factories[i].type() == ue.type())
                {
                    //
                    // Make a temporary stream, write the opaque endpoint data into the stream,
                    // and ask the factory to read the endpoint data from that stream to create
                    // the actual endpoint.
                    //
                    const os = new OutputStream(this._instance, Protocol.currentProtocolEncoding);
                    os.writeShort(ue.type());
                    ue.streamWrite(os);
                    const is = new InputStream(this._instance, Protocol.currentProtocolEncoding, os.buffer);
                    is.pos = 0;
                    is.readShort(); // type
                    is.startEncapsulation();
                    const e = this._factories[i].read(is);
                    is.endEncapsulation();
                    return e;
                }
            }
            return ue; // Endpoint is opaque, but we don't have a factory for its type.
        }

        return null;
    }

    read(s)
    {
        const type = s.readShort();

        const factory = this.get(type);
        let e = null;
        s.startEncapsulation();
        if(factory)
        {
            e = factory.read(s);
        }
        //
        // If the factory failed to read the endpoint, return an opaque endpoint. This can
        // occur if for example the factory delegates to another factory and this factory
        // isn't available. In this case, the factory needs to make sure the stream position
        // is preserved for reading the opaque endpoint.
        //
        if(!e)
        {
            e = new OpaqueEndpointI(type);
            e.initWithStream(s);
        }
        s.endEncapsulation();
        return e;
    }

    destroy()
    {
        this._factories.forEach(factory => factory.destroy());
        this._factories = [];
    }
}

Ice.EndpointFactoryManager = EndpointFactoryManager;
module.exports.Ice = Ice;
