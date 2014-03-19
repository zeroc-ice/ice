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
    
    var Ice = global.Ice || {};
    
    var ConnectionReaper = Ice.Class({
        __init__: function()
        {
            this._connections = [];
        },
        add: function(connection)
        {
            this._connections.push(connection);
        },
        swapConnections: function()
        {
            if(this._connections.length === 0)
            {
                return null;
            }
            var connections = this._connections;
            this._connections = [];
            return connections;
        }
    });
    
    Ice.ConnectionReaper = ConnectionReaper;
    global.Ice = Ice;
}(typeof (global) === "undefined" ? window : global));
