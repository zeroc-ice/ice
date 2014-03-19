// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(global){
    var Ice = global.Ice || {};
    
    var Address = function(host, port)
    {
        this.host = host;
        this.port = port;
    };
    
    Ice.Address = Address;
    
    global.Ice = Ice;
}(typeof (global) === "undefined" ? window : global));
