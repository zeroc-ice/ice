// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.DispatchStatus = {DispatchOK: 0, DispatchUserException: 1, DispatchAsync: 2};
module.exports.Ice = Ice;
