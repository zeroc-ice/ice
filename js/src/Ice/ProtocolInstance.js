//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;

class ProtocolInstance
{
    constructor(instance, type, protocol, secure)
    {
        this._instance = instance;
        this._traceLevel = instance.traceLevels().network;
        this._traceCategory = instance.traceLevels().networkCat;
        this._logger = instance.initializationData().logger;
        this._properties = instance.initializationData().properties;
        this._type = type;
        this._protocol = protocol;
        this._secure = secure;
    }

    traceLevel()
    {
        return this._traceLevel;
    }

    traceCategory()
    {
        return this._traceCategory;
    }

    logger()
    {
        return this._logger;
    }

    protocol()
    {
        return this._protocol;
    }

    type()
    {
        return this._type;
    }

    secure()
    {
        return this._secure;
    }

    properties()
    {
        return this._properties;
    }

    defaultHost()
    {
        return this._instance.defaultsAndOverrides().defaultHost;
    }

    defaultSourceAddress()
    {
        return this._instance.defaultsAndOverrides().defaultSourceAddress;
    }

    defaultEncoding()
    {
        return this._instance.defaultsAndOverrides().defaultEncoding;
    }

    defaultTimeout()
    {
        return this._instance.defaultsAndOverrides().defaultTimeout;
    }

    messageSizeMax()
    {
        return this._instance.messageSizeMax();
    }
}

Ice.ProtocolInstance = ProtocolInstance;
module.exports.Ice = Ice;
