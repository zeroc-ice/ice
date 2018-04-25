// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

//
// jshint browser: true
//

/* global WorkerGlobalScope */

const Ice = require("../Ice/ModuleRegistry").Ice;

//
// Create a timer object that uses the default browser methods. Note that we also
// have to use apply with null as the first argument to workaround an issue where
// IE doesn't like these functions to be called with an unknown object (it reports
// an "Invalid calling object" error).
//
function createTimerObject()
{
    const Timer = class
    {
        static setTimeout(cb, ms)
        {
            return setTimeout.apply(null, arguments);
        }

        static clearTimeout(id)
        {
            return clearTimeout.apply(null, arguments);
        }

        static setInterval()
        {
            return setInterval.apply(null, arguments);
        }

        static clearInterval()
        {
            return clearInterval.apply(null, arguments);
        }
    };

    if(typeof(setImmediate) == "function")
    {
        Timer.setImmediate = function()
        {
            return setImmediate.apply(null, arguments);
        };
    }
    else
    {
        Timer.setImmediate = function()
        {
            return setTimeout.apply(null, arguments);
        };
    }

    return Timer;
}

const MAX_SAFE_INTEGER = Number.MAX_SAFE_INTEGER  || 9007199254740991;

const _timers = new Map();

const _SetTimeoutType = 0;
const _SetIntervalType = 1;
const _SetImmediateType = 2;
const _ClearTimeoutType = 3;
const _ClearIntervalType = 4;

let worker;

let _nextId = 0;
const nextId = function()
{
    if(_nextId == MAX_SAFE_INTEGER)
    {
        _nextId = 0;
    }
    return _nextId++;
};

class Timer
{
    static setTimeout(cb, ms)
    {
        const id = nextId();
        _timers.set(id, cb);
        worker.postMessage({type: _SetTimeoutType, id: id, ms: ms});
        return id;
    }

    static clearTimeout(id)
    {
        _timers.delete(id);
        worker.postMessage({type: _ClearTimeoutType, id: id});
    }

    static setInterval(cb, ms)
    {
        const id = nextId();
        _timers.set(id, cb);
        worker.postMessage({type: _SetIntervalType, id: id, ms: ms});
        return id;
    }

    static clearInterval(id)
    {
        _timers.delete(id);
        worker.postMessage({type: _ClearIntervalType, id: id});
    }

    static setImmediate(cb)
    {
        const id = nextId();
        _timers.set(id, cb);
        worker.postMessage({type: _SetImmediateType, id: id});
        return id;
    }

    static onmessage(e)
    {
        const cb = _timers.get(e.data.id);
        if(cb !== undefined)
        {
            cb.call();
            if(e.data.type !== _SetIntervalType)
            {
                _timers.delete(e.data.id);
            }
        }
    }
}

const workerCode = function()
{
    return "(" +
    function()
    {
        //
        // jshint worker: true
        //
        const _wSetTimeoutType = 0;
        const _wSetIntervalType = 1;
        const _wSetImmediateType = 2;
        const _wClearTimeoutType = 3;
        const _wClearIntervalType = 4;

        const timers = {};

        self.onmessage = e =>
        {
            if(e.data.type == _wSetTimeoutType)
            {
                timers[e.data.id] = setTimeout(() => self.postMessage(e.data), e.data.ms);
            }
            else if(e.data.type == _wSetIntervalType)
            {
                timers[e.data.id] = setInterval(() => self.postMessage(e.data), e.data.ms);
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

if(typeof navigator !== "undefined" &&
   (navigator.userAgent.indexOf("MSIE") !== -1 ||
    navigator.userAgent.match(/Trident.*rv:11\./)))
{
    //
    // With IE always use the setInterval/setTimeout browser functions directly
    //
    Ice.Timer = createTimerObject();
}
else if(typeof WorkerGlobalScope !== 'undefined' && this instanceof WorkerGlobalScope)
{
    //
    // If we are running in a worker don't spawn a separate worker for the timer
    //
    Ice.Timer = createTimerObject();
}
else if(worker === undefined)
{
    const url = URL.createObjectURL(new Blob([workerCode()], {type : 'text/javascript'}));
    worker = new Worker(url);
    worker.onmessage = Timer.onmessage;
    Ice.Timer = Timer;
}
