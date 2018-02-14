// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module, ["../Ice/Class"]);

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
var ProtocolInstance = Ice.Class({
    __init__: function(instance, type, protocol, secure)
    {
        this._instance = instance;
        this._traceLevel = instance.traceLevels().network;
        this._traceCategory = instance.traceLevels().networkCat;
        this._logger = instance.initializationData().logger;
        this._properties = instance.initializationData().properties;
        this._type = type;
        this._protocol = protocol;
        this._secure = secure;
    },
    traceLevel: function()
    {
        return this._traceLevel;
    },
    traceCategory: function()
    {
        return this._traceCategory;
    },
    logger: function()
    {
        return this._logger;
    },
    protocol: function()
    {
        return this._protocol;
    },
    type: function()
    {
        return this._type;
    },
    secure: function()
    {
        return this._secure;
    },
    properties: function()
    {
        return this._properties;
    },
    defaultHost: function()
    {
        return this._instance.defaultsAndOverrides().defaultHost;
    },
    defaultSourceAddress: function()
    {
        return this._instance.defaultsAndOverrides().defaultSourceAddress;
    },
    defaultEncoding: function()
    {
        return this._instance.defaultsAndOverrides().defaultEncoding;
    },
    defaultTimeout: function()
    {
        return this._instance.defaultsAndOverrides().defaultTimeout;
    },
    messageSizeMax: function()
    {
        return this._instance.messageSizeMax();
    }
});

Ice.ProtocolInstance = ProtocolInstance;
module.exports.Ice = Ice;


