//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <BlobjectI.h>

using namespace std;

BlobjectI::BlobjectI() :
    _startBatch(false)
{
}

void
BlobjectI::setConnection(const Ice::ConnectionPtr& connection)
{
    Lock sync(*this);
    _connection = connection;
    notifyAll();
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
    _batchProxy = 0;
}

void
BlobjectI::ice_invokeAsync(std::vector<Ice::Byte> inEncaps,
                           std::function<void(bool, const std::vector<Ice::Byte>&)> response,
                           std::function<void(std::exception_ptr)> ex,
                           const Ice::Current& current)
{
    auto connection = getConnection(current);
    const bool twoway = current.requestId > 0;
    auto obj = connection->createProxy(current.id);
    if(!twoway)
    {
        if(_startBatch)
        {
            _startBatch = false;
            _batchProxy = obj->ice_batchOneway();
        }
        if(_batchProxy)
        {
            obj = _batchProxy;
        }

        if(!current.facet.empty())
        {
            obj = obj->ice_facet(current.facet);
        }

        if(_batchProxy)
        {
            vector<Ice::Byte> out;
            obj->ice_invoke(current.operation, current.mode, inEncaps, out, current.ctx);
            response(true, vector<Ice::Byte>());
        }
        else
        {
            obj->ice_oneway()->ice_invokeAsync(current.operation, current.mode, inEncaps,
                                               [](bool, const std::vector<Ice::Byte>&) { assert(0); },
                                               ex,
                                               [&](bool) { response(true, vector<Ice::Byte>()); },
                                               current.ctx);
        }
    }
    else
    {
        if(!current.facet.empty())
        {
            obj = obj->ice_facet(current.facet);
        }

        obj->ice_invokeAsync(current.operation, current.mode, inEncaps, response, ex, nullptr, current.ctx);
    }
}

Ice::ConnectionPtr
BlobjectI::getConnection(const Ice::Current& current)
{
    Lock sync(*this);
    if(!_connection)
    {
        return current.con;
    }

    try
    {
        _connection->throwException();
    }
    catch(const Ice::ConnectionLostException&)
    {
        // If we lost the connection, wait 5 seconds for the server to re-establish it. Some tests,
        // involve connection closure (e.g.: exceptions MemoryLimitException test) and the server
        // automatically re-establishes the connection with the echo server.
        timedWait(IceUtil::Time::seconds(5));
        if(!_connection)
        {
            throw;
        }
    }
    return _connection;
}
