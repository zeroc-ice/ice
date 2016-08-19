// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const Ice = require("../Ice/ModuleRegistry").Ice;
Ice.DispatchStatus = { DispatchOK: 0, DispatchUserException: 1, DispatchAsync: 2 };
module.exports.Ice = Ice;
