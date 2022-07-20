//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;

require("../Ice/LocalException");
require("../Ice/TimerUtil");

const CommunicatorDestroyedException = Ice.CommunicatorDestroyedException;

class Timer
{
    constructor(logger)
    {
        this._logger = logger;
        this._destroyed = false;
        this._tokenId = 0;
        this._tokens = new Map();
    }

    destroy()
    {
        this._tokens.forEach((value, key) => this.cancel(key));
        this._destroyed = true;
        this._tokens.clear();
    }

    schedule(callback, delay)
    {
        if(this._destroyed)
        {
            throw new CommunicatorDestroyedException();
        }
        const token = this._tokenId++;
        const id = Timer.setTimeout(() => this.handleTimeout(token), delay);
        this._tokens.set(token, {callback: callback, id: id, isInterval: false});
        return token;
    }

    scheduleRepeated(callback, period)
    {
        if(this._destroyed)
        {
            throw new CommunicatorDestroyedException();
        }
        const token = this._tokenId++;
        const id = Timer.setInterval(() => this.handleInterval(token), period);
        this._tokens.set(token, {callback: callback, id: id, isInterval: true});
        return token;
    }

    cancel(id)
    {
        if(this._destroyed)
        {
            return false;
        }

        const token = this._tokens.get(id);
        if(token === undefined)
        {
            return false;
        }

        this._tokens.delete(id);
        if(token.isInterval)
        {
            Timer.clearInterval(token.id);
        }
        else
        {
            Timer.clearTimeout(token.id);
        }

        return true;
    }

    handleTimeout(id)
    {
        if(this._destroyed)
        {
            return;
        }

        const token = this._tokens.get(id);
        if(token !== undefined)
        {
            this._tokens.delete(id);
            try
            {
                token.callback();
            }
            catch(ex)
            {
                this._logger.warning("uncaught exception while executing timer:\n" + ex);
            }
        }
    }

    handleInterval(id)
    {
        if(this._destroyed)
        {
            return;
        }

        const token = this._tokens.get(id);
        if(token !== undefined)
        {
            try
            {
                token.callback();
            }
            catch(ex)
            {
                this._logger.warning("uncaught exception while executing timer:\n" + ex);
            }
        }
    }
}

Timer.setTimeout = Ice.Timer.setTimeout;
Timer.clearTimeout = Ice.Timer.clearTimeout;
Timer.setInterval = Ice.Timer.setInterval;
Timer.clearInterval = Ice.Timer.clearInterval;
Timer.setImmediate = Ice.Timer.setImmediate;

Ice.Timer = Timer;
module.exports.Ice = Ice;
