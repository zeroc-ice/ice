// Copyright (c) ZeroC, Inc.

#include "RequestHandlerCache.h"
#include "ConnectionI.h"
#include "Reference.h"
#include "RequestHandler.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

RequestHandlerCache::RequestHandlerCache(const ReferencePtr& reference)
    : _reference(reference),
      _cacheConnection(reference->getCacheConnection())
{
}

IceInternal::RequestHandlerPtr
RequestHandlerCache::getRequestHandler()
{
    if (_cacheConnection)
    {
        lock_guard<mutex> lock(_mutex);
        if (_cachedRequestHandler)
        {
            return _cachedRequestHandler;
        }
    }

    auto handler = _reference->getRequestHandler();
    if (_cacheConnection)
    {
        lock_guard<mutex> lock(_mutex);
        if (!_cachedRequestHandler)
        {
            _cachedRequestHandler = handler;
        }
        // else discard handler
        return _cachedRequestHandler;
    }
    else
    {
        return handler;
    }
}

ConnectionPtr
RequestHandlerCache::getCachedConnection()
{
    if (_cacheConnection)
    {
        RequestHandlerPtr handler;
        {
            lock_guard<mutex> lock(_mutex);
            handler = _cachedRequestHandler;
        }
        if (handler)
        {
            return handler->getConnection();
        }
    }
    return nullptr;
}

void
RequestHandlerCache::clearCachedRequestHandler(const RequestHandlerPtr& handler)
{
    if (_cacheConnection)
    {
        lock_guard<mutex> lock(_mutex);
        if (handler == _cachedRequestHandler)
        {
            _cachedRequestHandler = nullptr;
        }
    }
}
