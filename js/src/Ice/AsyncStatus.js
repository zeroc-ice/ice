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
    var AsyncStatus = {Queued: 0, Sent: 1};
    Ice.AsyncStatus = AsyncStatus;
    global.Ice = Ice;
}(typeof (global) === "undefined" ? window : global));
