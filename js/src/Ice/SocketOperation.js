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
    
    var SocketOperation = 
    {
        None: 0,
        Read: 1,
        Write: 2,
        Connect: 2 // Same as Write
    };

    Ice.SocketOperation = SocketOperation;
    global.Ice = Ice;
}(typeof (global) === "undefined" ? window : global));
