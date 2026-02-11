// Copyright (c) ZeroC, Inc.

let Timer = {};

function isWorker() {
    return typeof WorkerGlobalScope !== "undefined" && self instanceof WorkerGlobalScope;
}

const MAX_SAFE_INTEGER = Number.MAX_SAFE_INTEGER || 9007199254740991;
let _nextId = 0;
const nextId = function () {
    if (_nextId == MAX_SAFE_INTEGER) {
        _nextId = 0;
    }
    return _nextId++;
};
const _timers = new Map();

//
// Create a timer object that uses the default browser methods.
//
function createTimerObject() {
    const Timer = class {
        static setTimeout(cb, ms) {
            return setTimeout(cb, ms);
        }

        static clearTimeout(id) {
            return clearTimeout(id);
        }
    };

    // Only called for workers
    const channel = new MessageChannel();
    channel.port1.onmessage = event => {
        const id = event.data;
        const cb = _timers.get(id);
        if (cb !== undefined) {
            cb.call();
            _timers.delete(id);
        }
    };

    Timer.setImmediate = function (cb) {
        const id = nextId();
        _timers.set(id, cb);
        channel.port2.postMessage(id);
    };

    return Timer;
}

const _SetTimeoutType = 0;
const _SetImmediateType = 1;
const _ClearTimeoutType = 2;

let worker;

Timer = class {
    static setTimeout(cb, ms) {
        const id = nextId();
        _timers.set(id, cb);
        worker.postMessage({ type: _SetTimeoutType, id: id, ms: ms });
        return id;
    }

    static clearTimeout(id) {
        _timers.delete(id);
        worker.postMessage({ type: _ClearTimeoutType, id: id });
    }

    static setImmediate(cb) {
        const id = nextId();
        _timers.set(id, cb);
        worker.postMessage({ type: _SetImmediateType, id: id });
        return id;
    }

    static onmessage(e) {
        const cb = _timers.get(e.data.id);
        if (cb !== undefined) {
            cb.call();
            _timers.delete(e.data.id);
        }
    }
};

const workerCode = function () {
    return (
        "(" +
        function () {
            //
            // jshint worker: true
            //
            const _wSetTimeoutType = 0;
            const _wSetImmediateType = 1;
            const _wClearTimeoutType = 2;

            const timers = {};

            self.onmessage = e => {
                if (e.data.type == _wSetTimeoutType) {
                    timers[e.data.id] = setTimeout(() => self.postMessage(e.data), e.data.ms);
                } else if (e.data.type == _wSetImmediateType) {
                    self.postMessage(e.data);
                } else if (e.data.type == _wClearTimeoutType) {
                    clearTimeout(timers[e.data.id]);
                    delete timers[e.data.id];
                }
            };

            //
            // jshint worker: false
            //
        }.toString() +
        "());"
    );
};

if (isWorker()) {
    //
    // If we are running in a worker don't spawn a separate worker for the timer
    //
    Timer = createTimerObject();
} else if (worker === undefined) {
    const url = URL.createObjectURL(new Blob([workerCode()], { type: "text/javascript" }));
    worker = new Worker(url);
    worker.onmessage = Timer.onmessage;
}

export { Timer as TimerUtil };
