//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;

let Timer = {};

if (typeof process != 'undefined')
{
    Timer = class
    {
    }

    Timer.setTimeout = setTimeout;
    Timer.clearTimeout = clearTimeout;
    Timer.setInterval = setInterval;
    Timer.clearInterval = clearInterval;
    Timer.setImmediate = setImmediate;
    Ice.Timer = Timer;
}
else
{
    function isIE()
    {
        return (navigator.userAgent.indexOf("MSIE") !== -1 || navigator.userAgent.match(/Trident.*rv:11\./));
    }

    function isEdge()
    {
        return (/Edge/).test(navigator.userAgent);
    }

    function isWorker()
    {
        return typeof WorkerGlobalScope !== 'undefined' && self instanceof WorkerGlobalScope;
    }

    const MAX_SAFE_INTEGER = Number.MAX_SAFE_INTEGER || 9007199254740991;
    let _nextId = 0;
    const nextId = function()
    {
        if(_nextId == MAX_SAFE_INTEGER)
        {
            _nextId = 0;
        }
        return _nextId++;
    };
    const _timers = new Map();

    //
    // Create a timer object that uses the default browser methods.
    //
    function createTimerObject()
    {
        const Timer = class
        {
            static setTimeout(cb, ms)
            {
                return setTimeout(cb, ms);
            }

            static clearTimeout(id)
            {
                return clearTimeout(id);
            }

            static setInterval(cb, ms)
            {
                return setInterval(cb, ms);
            }

            static clearInterval(id)
            {
                return clearInterval(id);
            }
        };

        //
        // For Browsers that support setImmediate prefer that,
        // otherwise implement it using MessageChannel
        //
        if(isEdge() || isIE())
        {
            Timer.setImmediate = function(cb)
            {
                setImmediate(cb);
            };
        }
        else
        {
            //
            // Should be only call for workers
            //
            const channel = new MessageChannel();
            channel.port1.onmessage = event =>
            {
                const id = event.data;
                const cb = _timers.get(id);
                if(cb !== undefined)
                {
                    cb.call();
                    _timers.delete(id);
                }
            };

            Timer.setImmediate = function(cb)
            {
                const id = nextId();
                _timers.set(id, cb);
                channel.port2.postMessage(id);
            };
        }

        return Timer;
    }

    const _SetTimeoutType = 0;
    const _SetIntervalType = 1;
    const _SetImmediateType = 2;
    const _ClearTimeoutType = 3;
    const _ClearIntervalType = 4;

    let worker;

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

    if(isIE())
    {
        //
        // With IE always use the setInterval/setTimeout browser functions directly
        //
        Ice.Timer = createTimerObject();
    }
    else if(isWorker())
    {
        //
        // If we are running in a worker don't spawn a separate worker for the timer
        //
        Ice.Timer = createTimerObject();
    }
    else if(worker === undefined)
    {
        const url = URL.createObjectURL(new Blob([workerCode()], {type: 'text/javascript'}));
        worker = new Worker(url);
        worker.onmessage = Timer.onmessage;
        Ice.Timer = Timer;
    }
}

module.exports.Ice = Ice;
