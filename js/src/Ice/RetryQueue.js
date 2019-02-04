//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/LocalException").Ice;

class RetryTask
{
    constructor(instance, queue, outAsync, interval)
    {
        this._instance = instance;
        this._queue = queue;
        this._outAsync = outAsync;
    }

    run()
    {
        this._outAsync.retry();
        this._queue.remove(this);
    }

    destroy()
    {
        try
        {
            this._outAsync.abort(new Ice.CommunicatorDestroyedException());
        }
        catch(ex)
        {
            // Abort shouldn't throw if there's no callback, ignore.
        }
    }

    asyncRequestCanceled(outAsync, ex)
    {
        if(this._queue.cancel(this))
        {
            if(this._instance.traceLevels().retry >= 1)
            {
                this._instance.initializationData().logger.trace(this._instance.traceLevels().retryCat,
                                                                 "operation retry canceled\n" + ex.toString());
            }
            this._outAsync.completedEx(ex);
        }
    }
}

class RetryQueue
{
    constructor(instance)
    {
        this._instance = instance;
        this._requests = [];
    }

    add(outAsync, interval)
    {
        if(this._instance === null)
        {
            throw new Ice.CommunicatorDestroyedException();
        }
        const task = new RetryTask(this._instance, this, outAsync);
        outAsync.cancelable(task); // This will throw if the request is canceled
        task.token = this._instance.timer().schedule(() => task.run(), interval);
        this._requests.push(task);
    }

    destroy()
    {
        this._requests.forEach(request =>
            {
                this._instance.timer().cancel(request.token);
                request.destroy();
            });

        this._requests = [];
        this._instance = null;
    }

    remove(task)
    {
        const idx = this._requests.indexOf(task);
        if(idx >= 0)
        {
            this._requests.splice(idx, 1);
        }
    }

    cancel(task)
    {
        const idx = this._requests.indexOf(task);
        if(idx >= 0)
        {
            this._requests.splice(idx, 1);
            return this._instance.timer().cancel(task.token);
        }
        return false;
    }
}
Ice.RetryQueue = RetryQueue;

module.exports.Ice = Ice;
