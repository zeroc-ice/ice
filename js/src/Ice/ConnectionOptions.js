// Copyright (c) ZeroC, Inc.

export class ConnectionOptions {
    constructor(connectTimeout, closeTimeout, idleTimeout, enableIdleCheck, inactivityTimeout) {
        this._connectTimeout = connectTimeout;
        this._closeTimeout = closeTimeout;
        this._idleTimeout = idleTimeout;
        this._enableIdleCheck = enableIdleCheck;
        this._inactivityTimeout = inactivityTimeout;
    }

    get connectTimeout() {
        return this._connectTimeout;
    }

    get closeTimeout() {
        return this._closeTimeout;
    }

    get idleTimeout() {
        return this._idleTimeout;
    }

    get enableIdleCheck() {
        return this._enableIdleCheck;
    }

    get inactivityTimeout() {
        return this._inactivityTimeout;
    }
}
