// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

final class RequestHandlerCache {
    RequestHandler getRequestHandler() {
        if (_cacheConnection) {
            synchronized (this) {
                if (_cachedRequestHandler != null) {
                    return _cachedRequestHandler;
                }
            }
        }

        RequestHandler handler = _reference.getRequestHandler();
        if (_cacheConnection) {
            synchronized (this) {
                if (_cachedRequestHandler == null) {
                    _cachedRequestHandler = handler;
                }
                // else ignore handler: another thread cached a handler before we could.
                return _cachedRequestHandler;
            }
        } else {
            return handler;
        }
    }

    public Connection getCachedConnection() {
        if (_cacheConnection) {
            RequestHandler handler;
            synchronized (this) {
                handler = _cachedRequestHandler;
            }
            if (handler != null) {
                return handler.getConnection();
            }
        }
        return null;
    }

    void clearCachedRequestHandler(RequestHandler handler) {
        if (_cacheConnection) {
            synchronized (this) {
                if (handler == _cachedRequestHandler) {
                    _cachedRequestHandler = null;
                }
            }
        }
    }

    public RequestHandlerCache(Reference reference) {
        _reference = reference;
        _cacheConnection = reference.getCacheConnection();
    }

    private final Reference _reference;
    private final boolean _cacheConnection;
    private RequestHandler _cachedRequestHandler;
}
