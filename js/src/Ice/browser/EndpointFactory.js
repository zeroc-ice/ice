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

    require("Ice/browser/EndpointInfo");
    require("Ice/browser/EndpointI");
    
    var Ice = global.Ice || {};
    var IceWS = global.IceWS || {};
    
    var EndpointI = IceWS.EndpointI;

    var EndpointFactory = Ice.Class({
        __init__:function(instance, secure)
        {
            this._instance = instance;
            this._secure = secure;
        },
        type: function()
        {
            return this._secure ? IceWS.WSSEndpointType : IceWS.WSEndpointType;
        },
        protocol: function()
        {
            return this._secure ? "wss" : "ws";
        },
        create: function(str, oaEndpoint)
        {
            return EndpointI.fromString(this._instance, this._secure, str, oaEndpoint);
        },
        read: function(s)
        {
            return EndpointI.fromStream(s, this._secure);
        },
        destroy: function()
        {
            this._instance = null;
        }
    });
    IceWS.EndpointFactory = EndpointFactory;
    global.IceWS = IceWS;
}(typeof (global) === "undefined" ? window : global));
