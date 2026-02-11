// Copyright (c) ZeroC, Inc.

export class RequestHandlerCache {
    get requestHandler() {
        if (this._cacheConnection) {
            if (this._cachedRequestHandler !== null) {
                return this._cachedRequestHandler;
            }
        }

        let handler = this._reference.getRequestHandler();
        if (this._cacheConnection) {
            if (this._cachedRequestHandler === null) {
                this._cachedRequestHandler = handler;
            }
            // else ignore handler
            return this._cachedRequestHandler;
        } else {
            return handler;
        }
    }

    get cachedConnection() {
        if (this._cacheConnection) {
            let handler = this._cachedRequestHandler;
            if (handler !== null) {
                return handler.getConnection();
            }
        }
        return null;
    }

    constructor(reference) {
        this._reference = reference;
        this._cacheConnection = reference.getCacheConnection();
        this._cachedRequestHandler = null;
    }

    clearCachedRequestHandler(handler) {
        if (this._cacheConnection) {
            if (handler == this._cachedRequestHandler) {
                this._cachedRequestHandler = null;
            }
        }
    }
}
