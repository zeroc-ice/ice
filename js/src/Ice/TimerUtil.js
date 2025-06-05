// Copyright (c) ZeroC, Inc.

let Timer = class {};
Timer.setTimeout = setTimeout;
Timer.clearTimeout = clearTimeout;
Timer.setImmediate = setImmediate;

export { Timer as TimerUtil };
