// Copyright (c) ZeroC, Inc.

#include "BlobjectI.h"
#include "Ice/Ice.h"

using namespace std;

BlobjectI::BlobjectI() = default;

void
BlobjectI::setConnection(const Ice::ConnectionPtr& connection)
{
    lock_guard lock(_mutex);
    _connection = connection;
    _condition.notify_all();
}

void
BlobjectI::startBatch()
{
    assert(!_batchProxy);
    _startBatch = true;
}

void
BlobjectI::flushBatch()
{
    assert(_batchProxy);
    _batchProxy->ice_flushBatchRequests();
    _batchProxy = nullopt;
}

void
BlobjectI::ice_invokeAsync(
    std::vector<byte> inEncaps,
    std::function<void(bool, const std::vector<byte>&)> response,
    std::function<void(std::exception_ptr)> ex,
    const Ice::Current& current)
{
    auto connection = getConnection(current);
    const bool twoway = current.requestId > 0;
    auto obj = connection->createProxy(current.id);
    if (!twoway)
    {
        if (_startBatch)
        {
            _startBatch = false;
            _batchProxy = obj->ice_batchOneway();
        }
        if (_batchProxy)
        {
            obj = _batchProxy.value();
        }

        if (!current.facet.empty())
        {
            obj = obj->ice_facet(current.facet);
        }

        if (_batchProxy)
        {
            vector<byte> out;
            obj->ice_invoke(current.operation, current.mode, inEncaps, out, current.ctx);
            response(true, vector<byte>());
        }
        else
        {
            obj->ice_oneway()->ice_invokeAsync(
                current.operation,
                current.mode,
                inEncaps,
                [](bool, const std::vector<byte>&) { assert(false); },
                ex,
                [response = std::move(response)](bool) { response(true, vector<byte>()); },
                current.ctx);
        }
    }
    else
    {
        if (!current.facet.empty())
        {
            obj = obj->ice_facet(current.facet);
        }

        obj->ice_invokeAsync(
            current.operation,
            current.mode,
            inEncaps,
            std::move(response),
            std::move(ex),
            nullptr,
            current.ctx);
    }
}

Ice::ConnectionPtr
BlobjectI::getConnection(const Ice::Current& current)
{
    unique_lock lock(_mutex);
    if (!_connection)
    {
        return current.con;
    }

    try
    {
        _connection->throwException();
    }
    catch (const Ice::ConnectionLostException&)
    {
        // If we lost the connection, wait 5 seconds for the server to re-establish it. Some tests,
        // involve connection closure and the server automatically re-establishes the connection with the echo server.
        _condition.wait_for(lock, chrono::seconds(5));
        if (!_connection)
        {
            throw;
        }
    }
    return _connection;
}
