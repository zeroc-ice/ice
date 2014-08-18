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
    require("Ice/Endpoint");
    require("Ice/browser/WSEndpoint");
    
    var Ice = global.Ice || {};
    
    var WSEndpoint = Ice.WSEndpoint;

    var WSEndpointFactory = Ice.Class({
        __init__:function(instance, secure)
        {
            this._instance = instance;
            this._secure = secure;
        },
        type: function()
        {
            return this._secure ? Ice.WSSEndpointType : Ice.WSEndpointType;
        },
        protocol: function()
        {
            return this._secure ? "wss" : "ws";
        },
        create: function(str, oaEndpoint)
        {
            return WSEndpoint.fromString(this._instance, this._secure, str, oaEndpoint);
        },
        read: function(s)
        {
            return WSEndpoint.fromStream(s, this._secure);
        },
        destroy: function()
        {
            this._instance = null;
        }
    });
    Ice.WSEndpointFactory = WSEndpointFactory;
    global.Ice = Ice;
}(typeof (global) === "undefined" ? window : global));
