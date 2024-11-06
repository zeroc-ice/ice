//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { TimerUtil } from "./TimerUtil.js";
import { CommunicatorDestroyedException } from "./LocalExceptions.js";

export class Timer {
    constructor(logger) {
        this._logger = logger;
        this._destroyed = false;
        this._tokenId = 0;
        this._tokens = new Map();
    }

    destroy() {
        this._tokens.forEach((_, key) => this.cancel(key));
        this._destroyed = true;
        this._tokens.clear();
    }

    schedule(callback, delay) {
        if (this._destroyed) {
            throw new CommunicatorDestroyedException();
        }
        const token = this._tokenId++;
        const id = Timer.setTimeout(() => this.handleTimeout(token), delay);
        this._tokens.set(token, {
            callback: callback,
            id: id,
            isInterval: false,
        });
        return token;
    }

    scheduleRepeated(callback, period) {
        if (this._destroyed) {
            throw new CommunicatorDestroyedException();
        }
        const token = this._tokenId++;
        const id = Timer.setInterval(() => this.handleInterval(token), period);
        this._tokens.set(token, {
            callback: callback,
            id: id,
            isInterval: true,
        });
        return token;
    }

    cancel(id) {
        if (this._destroyed) {
            return false;
        }

        const token = this._tokens.get(id);
        if (token === undefined) {
            return false;
        }

        this._tokens.delete(id);
        if (token.isInterval) {
            Timer.clearInterval(token.id);
        } else {
            Timer.clearTimeout(token.id);
        }

        return true;
    }

    handleTimeout(id) {
        if (this._destroyed) {
            return;
        }

        const token = this._tokens.get(id);
        if (token !== undefined) {
            this._tokens.delete(id);
            try {
                token.callback();
            } catch (ex) {
                this._logger.warning("uncaught exception while executing timer:\n" + ex);
            }
        }
    }

    handleInterval(id) {
        if (this._destroyed) {
            return;
        }

        const token = this._tokens.get(id);
        if (token !== undefined) {
            try {
                token.callback();
            } catch (ex) {
                this._logger.warning("uncaught exception while executing timer:\n" + ex);
            }
        }
    }
}

Timer.setTimeout = TimerUtil.setTimeout;
Timer.clearTimeout = TimerUtil.clearTimeout;
Timer.setInterval = TimerUtil.setInterval;
Timer.clearInterval = TimerUtil.clearInterval;
Timer.setImmediate = TimerUtil.setImmediate;
