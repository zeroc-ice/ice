// Copyright (c) ZeroC, Inc.

#nullable enable

using System.Diagnostics;

namespace Ice.Internal;

internal sealed class RequestHandlerCache
{
    internal RequestHandler? requestHandler
    {
        get
        {
            if (_cacheConnection)
            {
                lock (_mutex)
                {
                    if (_cachedRequestHandler != null)
                    {
                        return _cachedRequestHandler;
                    }
                }
            }

            RequestHandler handler = _reference.getRequestHandler();
            if (_cacheConnection)
            {
                lock (_mutex)
                {
                    _cachedRequestHandler ??= handler;
                    // else ignore handler
                    return _cachedRequestHandler;
                }
            }
            else
            {
                return handler;
            }
        }
    }

    internal Connection? cachedConnection
    {
        get
        {
            if (_cacheConnection)
            {
                RequestHandler? handler;
                lock (_mutex)
                {
                    handler = _cachedRequestHandler;
                }
                if (handler is not null)
                {
                    return handler.getConnection();
                }
            }
            return null;
        }
    }

    private readonly Reference _reference;
    private readonly bool _cacheConnection;
    private readonly object _mutex = new();
    private RequestHandler? _cachedRequestHandler;

    internal RequestHandlerCache(Reference reference)
    {
        _reference = reference;
        _cacheConnection = reference.getCacheConnection();
    }

    internal void clearCachedRequestHandler(RequestHandler handler)
    {
        if (_cacheConnection)
        {
            lock (_mutex)
            {
                if (handler == _cachedRequestHandler)
                {
                    _cachedRequestHandler = null;
                }
            }
        }
    }
}
