// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const Ice = require("../Ice/ModuleRegistry").Ice;

class Timer
{
}

Timer.setTimeout = setTimeout;
Timer.clearTimeout = clearTimeout;
Timer.setInterval = setInterval;
Timer.clearInterval = clearInterval;
Timer.setImmediate = setImmediate;

Ice.Timer = Timer;
module.exports.Ice = Ice;
