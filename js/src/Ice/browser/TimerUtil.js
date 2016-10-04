// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

//
// jshint browser: true
//

/* global WorkerGlobalScope */


var Ice = require("../Ice/ModuleRegistry").Ice;

//
// Create a timer object that uses the default browser methods. Note that we also
// have to use apply with null as the first argument to workaround an issue where
// IE doesn't like these functions to be called with an unknown object (it reports
// an "Invalid calling object" error).
//
function createTimerObject()
{
    var Timer = {};
    Timer.setTimeout = function () { setTimeout.apply(null, arguments); };
    Timer.clearTimeout = function () { clearTimeout.apply(null, arguments); };
    Timer.setInterval = function () { setInterval.apply(null, arguments); };
    Timer.clearInterval = function () { clearInterval.apply(null, arguments); };
    Timer.setImmediate = typeof(setImmediate) == "function" ?
        function () { setImmediate.apply(null, arguments); } :
        function () { setTimeout.apply(null, arguments); };
    return Timer;
}


Ice.__M.require(module,
    [
        "../Ice/HashMap",
    ]);

var HashMap = Ice.HashMap;

var MAX_SAFE_INTEGER = Number.MAX_SAFE_INTEGER  || 9007199254740991;

var _timers = new HashMap();

var _SetTimeoutType = 0,
    _SetIntervalType = 1,
    _SetImmediateType = 2,
    _ClearTimeoutType = 3,
    _ClearIntervalType = 4;

var Timer = {};
var worker;

var _nextId = 0;

var nextId = function()
{
    if(_nextId == MAX_SAFE_INTEGER)
    {
        _nextId = 0;
    }
    return _nextId++;
};

Timer.setTimeout = function(cb, ms)
{
    var id = nextId();
    _timers.set(id, cb);
    worker.postMessage({type: _SetTimeoutType, id: id, ms: ms});
    return id;
};

Timer.clearTimeout = function(id)
{
    _timers.delete(id);
    worker.postMessage({type: _ClearTimeoutType, id: id});
};

Timer.setInterval = function(cb, ms)
{
    var id = nextId();
    _timers.set(id, cb);
    worker.postMessage({type: _SetIntervalType, id: id, ms: ms});
    return id;
};

Timer.clearInterval = function(id)
{
    _timers.delete(id);
    worker.postMessage({type: _ClearIntervalType, id: id});
};

Timer.setImmediate = function(cb)
{
    var id = nextId();
    _timers.set(id, cb);
    worker.postMessage({type: _SetImmediateType, id: id});
    return id;
};

Timer.onmessage = function(e)
{
    var cb;
    if(e.data.type === _SetIntervalType)
    {
        cb = _timers.get(e.data.id);
    }
    else
    {
        cb = _timers.delete(e.data.id);
    }

    if(cb !== undefined)
    {
        cb.call();
    }
};

var workerCode = function()
{
    return "(" +
    function()
    {
        //
        // jshint worker: true
        //
        var _wSetTimeoutType = 0,
            _wSetIntervalType = 1,
            _wSetImmediateType = 2,
            _wClearTimeoutType = 3,
            _wClearIntervalType = 4;

        var timers = {};

        self.onmessage = function(e)
        {
            if(e.data.type == _wSetTimeoutType)
            {
                timers[e.data.id] = setTimeout(function()
                    {
                        self.postMessage(e.data);
                    },
                    e.data.ms);
            }
            else if(e.data.type == _wSetIntervalType)
            {
                timers[e.data.id] = setInterval(function()
                    {
                        self.postMessage(e.data);
                    },
                    e.data.ms);
            }
            else if(e.data.type == _wSetImmediateType)
            {
                self.postMessage(e.data);
            }
            else if(e.data.type == _wClearTimeoutType)
            {
                clearTimeout(timers[e.data.id]);
                delete timers[e.data.id];
            }
            else if(e.data.type == _wClearIntervalType)
            {
                clearInterval(timers[e.data.id]);
                delete timers[e.data.id];
            }
        };

        //
        // jshint worker: false
        //
    }.toString() + "());";
};

if(worker === undefined)
{
    var url;
    try
    {
        url = URL.createObjectURL(new Blob([workerCode()], {type : 'text/javascript'}));
        worker = new Worker(url);
        worker.onmessage = Timer.onmessage;
        Ice.Timer = Timer;
    }
    catch(ex)
    {
        URL.revokeObjectURL(url);

        //
        // Fallback on setInterval/setTimeout if the worker creating failed. Some IE10 and IE11 don't
        // support creating workers from blob URLs for instance.
        //
        Ice.Timer = createTimerObject();
    }
}
