// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <BlobjectI.h>

using namespace std;

#ifndef ICE_CPP11_MAPPING
class Callback : public IceUtil::Shared
{
public:

    Callback(const Ice::AMD_Object_ice_invokePtr& cb, bool twoway) :
        _cb(cb), _twoway(twoway)
    {
    }

    void response(bool ok, const vector<Ice::Byte>& encaps)
    {
        _cb->ice_response(ok, encaps);
    }

    void exception(const Ice::Exception& ex)
    {
        _cb->ice_exception(ex);
    }

    void sent(bool)
    {
        if(!_twoway)
        {
            _cb->ice_response(true, vector<Ice::Byte>());
        }
    }

private:

    Ice::AMD_Object_ice_invokePtr _cb;
    bool _twoway;
};
typedef IceUtil::Handle<Callback> CallbackPtr;
#endif

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

#ifdef ICE_CPP11_MAPPING
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
#else
void
BlobjectI::ice_invoke_async(const Ice::AMD_Object_ice_invokePtr& amdCb, const vector<Ice::Byte>& inEncaps,
                            const Ice::Current& current)
{
    Ice::ConnectionPtr connection = getConnection(current);
    const bool twoway = current.requestId > 0;
    Ice::ObjectPrx obj = connection->createProxy(current.id);
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
            amdCb->ice_response(true, vector<Ice::Byte>());
        }
        else
        {
            CallbackPtr cb = new Callback(amdCb, false);
            Ice::Callback_Object_ice_invokePtr del =
                Ice::newCallback_Object_ice_invoke(cb, &Callback::response, &Callback::exception, &Callback::sent);
            obj->ice_oneway()->begin_ice_invoke(current.operation, current.mode, inEncaps, current.ctx, del);
        }
    }
    else
    {
        if(!current.facet.empty())
        {
            obj = obj->ice_facet(current.facet);
        }

        CallbackPtr cb = new Callback(amdCb, true);
        Ice::Callback_Object_ice_invokePtr del =
            Ice::newCallback_Object_ice_invoke(cb, &Callback::response, &Callback::exception, &Callback::sent);
        obj->begin_ice_invoke(current.operation, current.mode, inEncaps, current.ctx, del);
    }
}
#endif

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
