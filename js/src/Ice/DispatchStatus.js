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
    var DispatchStatus = {DispatchOK: 0, DispatchUserException: 1, DispatchAsync: 2};
    Ice.DispatchStatus = DispatchStatus;
    global.Ice = Ice;
}(typeof (global) === "undefined" ? window : global));
