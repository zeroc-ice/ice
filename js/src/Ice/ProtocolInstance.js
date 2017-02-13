// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const Ice = require("../Ice/ModuleRegistry").Ice;

//
// NOTE: the protocol instance class is a bit different from other
// language mappinps since it also provides the secure property for
// the tranport. Since we don't support SSL but still want to be able
// to parse SSL endpoints, we simply re-use the TCP endpoint with a
// different protocol instance to support SSL endpoints.
//
// If SSL endpoints attributes were to diverge from TCP endpoints or
// if we want to support SSL, we'd have to change this and instead, do
// like in other mappings: have a separate implementation for the SSL
// endpoint and suppress the secure member of the protocol instance
// class bellow.
//
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
