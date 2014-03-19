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
    require("Ice/TcpEndpointI");
    require("Ice/Endpoint");
    
    var Ice = global.Ice || {};
    
    var TcpEndpointI = Ice.TcpEndpointI;
    var TCPEndpointType = Ice.TCPEndpointType;

    var TcpEndpointFactory = Ice.Class({
        __init__: function(instance)
        {
            this._instance = instance;
        },
        type: function()
        {
            return TCPEndpointType;
        },
        protocol: function()
        {
            return "tcp";
        },
        create: function(str, oaEndpoint)
        {
            return TcpEndpointI.fromString(this._instance, str, oaEndpoint);
        },
        read: function(s)
        {
            return TcpEndpointI.fromStream(s);
        },
        destroy: function()
        {
            this._instance = null;
        }
    });
    
    Ice.TcpEndpointFactory = TcpEndpointFactory;
    global.Ice = Ice;
}(typeof (global) === "undefined" ? window : global));
