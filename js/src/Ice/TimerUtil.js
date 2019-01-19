//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
