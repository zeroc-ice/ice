// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(global){
    require("Ice/HashMap");
    var Ice = global.Ice || {};
    Ice.CompactIdRegistry = new Ice.HashMap();
    global.Ice = Ice;
}(typeof (global) === "undefined" ? window : global));
