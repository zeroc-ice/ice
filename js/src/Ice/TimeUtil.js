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
    var TimeUtil = {};
    TimeUtil.now = function()
    {
        return new Date().getTime();
    };
    Ice.TimeUtil = TimeUtil;
    global.Ice = Ice;
}(typeof (global) === "undefined" ? window : global));
