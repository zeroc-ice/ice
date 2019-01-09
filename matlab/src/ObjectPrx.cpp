// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/Ice.h>
#include "ice.h"
#include "Future.h"
#include "Util.h"

using namespace std;
using namespace IceMatlab;

namespace
{

class InvocationFuture : public Future
{
public:

    InvocationFuture(bool, bool);

    virtual void exception(exception_ptr);
    virtual void sent();

    void finished(const std::shared_ptr<Ice::Communicator>&, const Ice::EncodingVersion&, bool,
                  std::pair<const Ice::Byte*, const Ice::Byte*>);
    void getResults(bool&, pair<const Ice::Byte*, const Ice::Byte*>&);

protected:

    virtual State stateImpl() const;

private:

    const bool _twoway;
    State _state;
    bool _ok; // True for success, false for user exception.
    vector<Ice::Byte> _data;
};

InvocationFuture::InvocationFuture(bool twoway, bool batch) :
    _twoway(twoway),
    _state(batch ? State::Finished : State::Running),
    _ok(false)
{
}

void
InvocationFuture::sent()
{
    lock_guard<mutex> lock(_mutex);
    if(_state == State::Running)
    {
        _state = _twoway ? State::Sent : State::Finished;
    }
}

void
InvocationFuture::exception(exception_ptr e)
{
    lock_guard<mutex> lock(_mutex);
    _state = State::Finished;
    _token = nullptr;
    _exception = e;
    _cond.notify_all();
}

void
InvocationFuture::finished(const std::shared_ptr<Ice::Communicator>& /*communicator*/,
                           const Ice::EncodingVersion& /*encoding*/, bool b, pair<const Ice::Byte*, const Ice::Byte*> p)
{
    lock_guard<mutex> lock(_mutex);
    _ok = b;
    _state = State::Finished;
    _token = nullptr;
    if(p.second > p.first)
    {
        vector<Ice::Byte> data(p.first, p.second); // Makes a copy.
        _data.swap(data); // Avoids another copy.
    }
    _cond.notify_all();
}

void
InvocationFuture::getResults(bool& ok, pair<const Ice::Byte*, const Ice::Byte*>& p)
{
    lock_guard<mutex> lock(_mutex);
    assert(_twoway);
    ok = _ok;
    if(!_data.empty())
    {
        p.first = &_data[0];
        p.second = p.first + _data.size();
    }
    else
    {
        p.first = p.second = 0;
    }
}

Future::State
InvocationFuture::stateImpl() const
{
    return _state;
}

class GetConnectionFuture : public Future
{
public:

    void finished(shared_ptr<Ice::Connection>);
    shared_ptr<Ice::Connection> getConnection() const;

protected:

    virtual State stateImpl() const;

private:

    shared_ptr<Ice::Connection> _connection;
};

void
GetConnectionFuture::finished(shared_ptr<Ice::Connection> con)
{
    lock_guard<mutex> lock(_mutex);
    _token = nullptr;
    _connection = con;
    _cond.notify_all();
}

shared_ptr<Ice::Connection>
GetConnectionFuture::getConnection() const
{
    lock_guard<mutex> lock(const_cast<mutex&>(_mutex));
    return _connection;
}

Future::State
GetConnectionFuture::stateImpl() const
{
    if(_exception || _connection)
    {
        return State::Finished;
    }
    else
    {
        return State::Running;
    }
}

static const char* invokeResultFields[] = {"ok", "params"};

mxArray*
createInvokeResultValue(mxArray* ok, mxArray* params)
{
    mwSize dims[2] = {1, 1};
    auto r = mxCreateStructArray(2, dims, 2, invokeResultFields);
    mxSetFieldByNumber(r, 0, 0, ok);
    mxSetFieldByNumber(r, 0, 1, params);
    return r;
}

}

extern "C"
{

mxArray*
Ice_ObjectPrx_unref(void* self)
{
    delete reinterpret_cast<shared_ptr<Ice::ObjectPrx>*>(self);
    return 0;
}

mxArray*
Ice_ObjectPrx_equals(void* self, void* other)
{
    assert(other); // Wrapper only calls this function for non-nil arguments.
    try
    {
        return createResultValue(createBool(Ice::targetEqualTo(deref<Ice::ObjectPrx>(self),
                                                               deref<Ice::ObjectPrx>(other))));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
}

mxArray*
Ice_ObjectPrx_read(void* communicator, mxArray* encoding, mxArray* buf, int start, int size, void** r)
{
    assert(!mxIsEmpty(buf));

    pair<const Ice::Byte*, const Ice::Byte*> p;
    p.first = reinterpret_cast<Ice::Byte*>(mxGetData(buf)) + start;
    p.second = p.first + size;

    try
    {
        Ice::EncodingVersion ev;
        getEncodingVersion(encoding, ev);

        Ice::InputStream in(deref<Ice::Communicator>(communicator), ev, p);
        shared_ptr<Ice::ObjectPrx> proxy;
        in.read(proxy);
        if(proxy)
        {
            *r = createShared<Ice::ObjectPrx>(proxy);
        }
        else
        {
            *r = 0;
        }
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_write(void* proxy, void* communicator, mxArray* encoding)
{
    //
    // Marshal a proxy into a stream and return the encoded bytes.
    //
    try
    {
        shared_ptr<Ice::ObjectPrx> prx;
        if(proxy)
        {
            prx = deref<Ice::ObjectPrx>(proxy);
        }

        assert(communicator);
        auto comm = deref<Ice::Communicator>(communicator);

        Ice::EncodingVersion enc;
        getEncodingVersion(encoding, enc);

        Ice::OutputStream out(comm, enc);
        out.write(prx);
        pair<const Ice::Byte*, const Ice::Byte*> p = out.finished();

        assert(p.second > p.first);
        return createResultValue(createByteArray(p.first, p.second));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
}

mxArray*
Ice_ObjectPrx_ice_invoke(void* self, const char* op, int m, mxArray* inParams, unsigned int size, mxArray* context)
{
    pair<const Ice::Byte*, const Ice::Byte*> params(0, 0);
    if(!mxIsEmpty(inParams))
    {
        params.first = reinterpret_cast<Ice::Byte*>(mxGetData(inParams));
        params.second = params.first + size;
    }
    auto mode = static_cast<Ice::OperationMode>(m);
    vector<Ice::Byte> v;

    try
    {
        Ice::Context ctx;
        getStringMap(context, ctx);
        auto ok = deref<Ice::ObjectPrx>(self)->ice_invoke(op, mode, params, v, ctx);
        mxArray* results = 0;
        if(!v.empty())
        {
            results = createByteArray(&v[0], &v[0] + v.size());
        }
        return createResultValue(createInvokeResultValue(createBool(ok), results));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_invokeNC(void* self, const char* op, int m, mxArray* inParams, unsigned int size)
{
    pair<const Ice::Byte*, const Ice::Byte*> params(0, 0);
    if(!mxIsEmpty(inParams))
    {
        params.first = reinterpret_cast<Ice::Byte*>(mxGetData(inParams));
        params.second = params.first + size;
    }
    auto mode = static_cast<Ice::OperationMode>(m);
    vector<Ice::Byte> v;

    try
    {
        auto ok = deref<Ice::ObjectPrx>(self)->ice_invoke(op, mode, params, v);
        mxArray* results = 0;
        if(!v.empty())
        {
            results = createByteArray(&v[0], &v[0] + v.size());
        }
        return createResultValue(createInvokeResultValue(createBool(ok), results));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_invokeAsync(void* self, const char* op, int m, mxArray* inParams, unsigned int size,
                              mxArray* context, void** future)
{
    const auto proxy = deref<Ice::ObjectPrx>(self);
    pair<const Ice::Byte*, const Ice::Byte*> params(0, 0);
    if(!mxIsEmpty(inParams))
    {
        params.first = reinterpret_cast<Ice::Byte*>(mxGetData(inParams));
        params.second = params.first + size;
    }
    auto mode = static_cast<Ice::OperationMode>(m);

    *future = 0;
    auto f = make_shared<InvocationFuture>(proxy->ice_isTwoway(),
                                           proxy->ice_isBatchOneway() || proxy->ice_isBatchDatagram());

    try
    {
        Ice::Context ctx;
        getStringMap(context, ctx);
        function<void()> token = proxy->ice_invokeAsync(
            op, mode, params,
            [proxy, f](bool ok, pair<const Ice::Byte*, const Ice::Byte*> outParams)
            {
                f->finished(proxy->ice_getCommunicator(), proxy->ice_getEncodingVersion(), ok, outParams);
            },
            [f](exception_ptr e)
            {
                f->exception(e);
            },
            [f](bool /*sentSynchronously*/)
            {
                f->sent();
            },
            ctx);
        f->token(token);
        *future = new shared_ptr<InvocationFuture>(move(f));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_invokeAsyncNC(void* self, const char* op, int m, mxArray* inParams, unsigned int size, void** future)
{
    const auto proxy = deref<Ice::ObjectPrx>(self);
    pair<const Ice::Byte*, const Ice::Byte*> params(0, 0);
    if(!mxIsEmpty(inParams))
    {
        params.first = reinterpret_cast<Ice::Byte*>(mxGetData(inParams));
        params.second = params.first + size;
    }
    auto mode = static_cast<Ice::OperationMode>(m);

    *future = 0;
    auto f = make_shared<InvocationFuture>(proxy->ice_isTwoway(),
                                           proxy->ice_isBatchOneway() || proxy->ice_isBatchDatagram());

    try
    {
        function<void()> token = proxy->ice_invokeAsync(
            op, mode, params,
            [proxy, f](bool ok, pair<const Ice::Byte*, const Ice::Byte*> outParams)
            {
                f->finished(proxy->ice_getCommunicator(),
                            proxy->ice_getEncodingVersion(), ok, outParams);
            },
            [f](exception_ptr e)
            {
                f->exception(e);
            },
            [f](bool /*sentSynchronously*/)
            {
                f->sent();
            });
        f->token(token);
        *future = new shared_ptr<InvocationFuture>(move(f));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_toString(void* self)
{
    return createResultValue(createStringFromUTF8(deref<Ice::ObjectPrx>(self)->ice_toString()));
}

mxArray*
Ice_ObjectPrx_ice_getIdentity(void* self)
{
    return createResultValue(createIdentity(deref<Ice::ObjectPrx>(self)->ice_getIdentity()));
}

mxArray*
Ice_ObjectPrx_ice_identity(void* self, void** r, mxArray* id)
{
    try
    {
        Ice::Identity ident;
        getIdentity(id, ident);
        auto proxy = deref<Ice::ObjectPrx>(self);
        auto newProxy = proxy->ice_identity(ident);
        *r = newProxy == proxy ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_getContext(void* self)
{
    return createResultValue(createStringMap(deref<Ice::ObjectPrx>(self)->ice_getContext()));
}

mxArray*
Ice_ObjectPrx_ice_context(void* self, void** r, mxArray* c)
{
    try
    {
        Ice::Context ctx;
        getStringMap(c, ctx);
        auto proxy = deref<Ice::ObjectPrx>(self);
        auto newProxy = proxy->ice_context(ctx);
        *r = newProxy == proxy ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_getFacet(void* self)
{
    return createResultValue(createStringFromUTF8(deref<Ice::ObjectPrx>(self)->ice_getFacet()));
}

mxArray*
Ice_ObjectPrx_ice_facet(void* self, void** r, const char* f)
{
    try
    {
        auto proxy = deref<Ice::ObjectPrx>(self);
        auto newProxy = proxy->ice_facet(f);
        *r = newProxy == proxy ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_getAdapterId(void* self)
{
    return createResultValue(createStringFromUTF8(deref<Ice::ObjectPrx>(self)->ice_getAdapterId()));
}

mxArray*
Ice_ObjectPrx_ice_adapterId(void* self, void** r, const char* id)
{
    try
    {
        auto proxy = deref<Ice::ObjectPrx>(self);
        auto newProxy = proxy->ice_adapterId(id);
        *r = newProxy == proxy ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_getNumEndpoints(void* self)
{
    try
    {
        return createResultValue(createInt(static_cast<int>(deref<Ice::ObjectPrx>(self)->ice_getEndpoints().size())));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
}

mxArray*
Ice_ObjectPrx_ice_getEndpoint(void* self, unsigned int idx, void** r)
{
    try
    {
        auto endpoints = deref<Ice::ObjectPrx>(self)->ice_getEndpoints();
        if(idx > endpoints.size())
        {
            throw std::invalid_argument("index outside range");
        }
        *r = createShared<Ice::Endpoint>(endpoints[idx]);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_createEndpointList(void* /*self*/, unsigned int num, void** r)
{
    *r = new vector<shared_ptr<Ice::Endpoint>>(num);
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_setEndpoint(void* /*self*/, void* arr, unsigned int idx, void* e)
{
    //
    // arr must hold a pointer returned by createEndpointList.
    //

    try
    {
        vector<shared_ptr<Ice::Endpoint>>* v = reinterpret_cast<vector<shared_ptr<Ice::Endpoint>>*>(arr);
        if(idx > v->size())
        {
            throw std::invalid_argument("index outside range");
        }
        (*v)[idx] = deref<Ice::Endpoint>(e);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_endpoints(void* self, void** r, void* arr)
{
    //
    // arr must hold a pointer returned by createEndpointList and populated with setEndpoint.
    //

    try
    {
        vector<shared_ptr<Ice::Endpoint>>* v = reinterpret_cast<vector<shared_ptr<Ice::Endpoint>>*>(arr);
        vector<shared_ptr<Ice::Endpoint>> tmp = *v;
        delete v;
        auto proxy = deref<Ice::ObjectPrx>(self);
        auto newProxy = proxy->ice_endpoints(tmp);
        *r = newProxy == proxy ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_getLocatorCacheTimeout(void* self)
{
    return createResultValue(createInt(deref<Ice::ObjectPrx>(self)->ice_getLocatorCacheTimeout()));
}

mxArray*
Ice_ObjectPrx_ice_locatorCacheTimeout(void* self, void** r, int t)
{
    try
    {
        auto proxy = deref<Ice::ObjectPrx>(self);
        auto newProxy = proxy->ice_locatorCacheTimeout(t);
        *r = newProxy == proxy ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_getInvocationTimeout(void* self)
{
    return createResultValue(createInt(deref<Ice::ObjectPrx>(self)->ice_getInvocationTimeout()));
}

mxArray*
Ice_ObjectPrx_ice_invocationTimeout(void* self, void** r, int t)
{
    try
    {
        auto proxy = deref<Ice::ObjectPrx>(self);
        auto newProxy = proxy->ice_invocationTimeout(t);
        *r = newProxy == proxy ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_getConnectionId(void* self)
{
    auto s = deref<Ice::ObjectPrx>(self)->ice_getConnectionId();
    return createResultValue(createStringFromUTF8(s));
}

mxArray*
Ice_ObjectPrx_ice_connectionId(void* self, void** r, const char* id)
{
    try
    {
        auto proxy = deref<Ice::ObjectPrx>(self);
        auto newProxy = proxy->ice_connectionId(id);
        *r = newProxy == proxy ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_isConnectionCached(void* self)
{
    return createResultValue(createBool(deref<Ice::ObjectPrx>(self)->ice_isConnectionCached()));
}

mxArray*
Ice_ObjectPrx_ice_connectionCached(void* self, void** r, unsigned char v)
{
    try
    {
        auto proxy = deref<Ice::ObjectPrx>(self);
        auto newProxy = proxy->ice_connectionCached(v == 1);
        *r = newProxy == proxy ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_getEndpointSelection(void* self)
{
    try
    {
        return createResultValue(createInt(static_cast<int>(deref<Ice::ObjectPrx>(self)->ice_getEndpointSelection())));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
}

mxArray*
Ice_ObjectPrx_ice_endpointSelection(void* self, void** r, mxArray* type)
{
    try
    {
        auto proxy = deref<Ice::ObjectPrx>(self);
        auto newProxy = proxy->ice_endpointSelection(
            static_cast<Ice::EndpointSelectionType>(getEnumerator(type, "Ice.EndpointSelectionType")));
        *r = newProxy == proxy ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_getEncodingVersion(void* self)
{
    return createResultValue(createEncodingVersion(deref<Ice::ObjectPrx>(self)->ice_getEncodingVersion()));
}

mxArray*
Ice_ObjectPrx_ice_encodingVersion(void* self, void** r, mxArray* v)
{
    try
    {
        Ice::EncodingVersion ev;
        getEncodingVersion(v, ev);
        auto proxy = deref<Ice::ObjectPrx>(self);
        auto newProxy = proxy->ice_encodingVersion(ev);
        *r = newProxy == proxy ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_getRouter(void* self, void** r)
{
    auto router = deref<Ice::ObjectPrx>(self)->ice_getRouter();
    *r = router ? new shared_ptr<Ice::ObjectPrx>(move(router)) : 0;
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_router(void* self, void** r, void* rtr)
{
    try
    {
        shared_ptr<Ice::ObjectPrx> router;
        if(rtr)
        {
            router = deref<Ice::ObjectPrx>(rtr);
        }
        auto proxy = deref<Ice::ObjectPrx>(self);
        auto newProxy = proxy->ice_router(Ice::uncheckedCast<Ice::RouterPrx>(router));
        *r = newProxy == proxy ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_getLocator(void* self, void** r)
{
    auto locator = deref<Ice::ObjectPrx>(self)->ice_getLocator();
    *r = locator ? new shared_ptr<Ice::ObjectPrx>(move(locator)) : 0;
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_locator(void* self, void** r, void* loc)
{
    try
    {
        shared_ptr<Ice::ObjectPrx> locator;
        if(loc)
        {
            locator = deref<Ice::ObjectPrx>(loc);
        }
        auto proxy = deref<Ice::ObjectPrx>(self);
        auto newProxy = proxy->ice_locator(Ice::uncheckedCast<Ice::LocatorPrx>(locator));
        *r = newProxy == proxy ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_isSecure(void* self)
{
    return createResultValue(createBool(deref<Ice::ObjectPrx>(self)->ice_isSecure()));
}

mxArray*
Ice_ObjectPrx_ice_secure(void* self, void** r, unsigned char b)
{
    try
    {
        auto proxy = deref<Ice::ObjectPrx>(self);
        auto newProxy = proxy->ice_secure(b == 1);
        *r = newProxy == proxy ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_isPreferSecure(void* self)
{
    return createResultValue(createBool(deref<Ice::ObjectPrx>(self)->ice_isPreferSecure()));
}

mxArray*
Ice_ObjectPrx_ice_preferSecure(void* self, void** r, unsigned char b)
{
    try
    {
        auto proxy = deref<Ice::ObjectPrx>(self);
        auto newProxy = proxy->ice_preferSecure(b == 1);
        *r = newProxy == proxy ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_isTwoway(void* self)
{
    return createResultValue(createBool(deref<Ice::ObjectPrx>(self)->ice_isTwoway()));
}

mxArray*
Ice_ObjectPrx_ice_twoway(void* self, void** r)
{
    try
    {
        auto proxy = deref<Ice::ObjectPrx>(self);
        auto newProxy = proxy->ice_twoway();
        *r = newProxy == proxy ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_isOneway(void* self)
{
    return createResultValue(createBool(deref<Ice::ObjectPrx>(self)->ice_isOneway()));
}

mxArray*
Ice_ObjectPrx_ice_oneway(void* self, void** r)
{
    try
    {
        auto proxy = deref<Ice::ObjectPrx>(self);
        auto newProxy = proxy->ice_oneway();
        *r = newProxy == proxy ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_isBatchOneway(void* self)
{
    return createResultValue(createBool(deref<Ice::ObjectPrx>(self)->ice_isBatchOneway()));
}

mxArray*
Ice_ObjectPrx_ice_batchOneway(void* self, void** r)
{
    try
    {
        auto proxy = deref<Ice::ObjectPrx>(self);
        auto newProxy = proxy->ice_batchOneway();
        *r = newProxy == proxy ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_isDatagram(void* self)
{
    return createResultValue(createBool(deref<Ice::ObjectPrx>(self)->ice_isDatagram()));
}

mxArray*
Ice_ObjectPrx_ice_datagram(void* self, void** r)
{
    try
    {
        auto proxy = deref<Ice::ObjectPrx>(self);
        auto newProxy = proxy->ice_datagram();
        *r = newProxy == proxy ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_isBatchDatagram(void* self)
{
    return createResultValue(createBool(deref<Ice::ObjectPrx>(self)->ice_isBatchDatagram()));
}

mxArray*
Ice_ObjectPrx_ice_batchDatagram(void* self, void** r)
{
    try
    {
        auto proxy = deref<Ice::ObjectPrx>(self);
        auto newProxy = proxy->ice_batchDatagram();
        *r = newProxy == proxy ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_compress(void* self, void** r, unsigned char b)
{
    try
    {
        auto proxy = deref<Ice::ObjectPrx>(self);
        auto newProxy = proxy->ice_compress(b == 1);
        *r = newProxy == proxy ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_getCompress(void* self)
{
    auto v = deref<Ice::ObjectPrx>(self)->ice_getCompress();
    if(v.has_value())
    {
        return createResultValue(createOptionalValue(true, createBool(v.value())));
    }
    else
    {
        return createResultValue(createOptionalValue(false, 0));
    }
}

mxArray*
Ice_ObjectPrx_ice_timeout(void* self, void** r, int t)
{
    try
    {
        auto proxy = deref<Ice::ObjectPrx>(self);
        auto newProxy = proxy->ice_timeout(t);
        *r = newProxy == proxy ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_getTimeout(void* self)
{
    auto v = deref<Ice::ObjectPrx>(self)->ice_getTimeout();
    if(v.has_value())
    {
        return createResultValue(createOptionalValue(true, createInt(v.value())));
    }
    else
    {
        return createResultValue(createOptionalValue(false, 0));
    }
}

mxArray*
Ice_ObjectPrx_ice_fixed(void* self, void** r, void* connection)
{
    assert(connection); // Wrapper only calls this function for non-nil arguments.
    try
    {
        auto proxy = deref<Ice::ObjectPrx>(self);
        auto newProxy = proxy->ice_fixed(deref<Ice::Connection>(connection));
        *r = newProxy == proxy ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_getConnection(void* self, void** r)
{
    *r = 0;
    try
    {
        auto conn = deref<Ice::ObjectPrx>(self)->ice_getConnection();
        if(conn)
        {
            *r = new shared_ptr<Ice::Connection>(move(conn));
        }
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_getConnectionAsync(void* self, void** future)
{
    *future = 0;
    auto f = make_shared<GetConnectionFuture>();

    try
    {
        function<void()> token = deref<Ice::ObjectPrx>(self)->ice_getConnectionAsync(
            [f](shared_ptr<Ice::Connection> con)
            {
                f->finished(con);
            },
            [f](exception_ptr e)
            {
                f->exception(e);
            },
            nullptr);
        f->token(token);
        *future = new shared_ptr<GetConnectionFuture>(move(f));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_getCachedConnection(void* self, void** r)
{
    *r = 0;
    try
    {
        auto conn = deref<Ice::ObjectPrx>(self)->ice_getCachedConnection();
        if(conn)
        {
            *r = new shared_ptr<Ice::Connection>(move(conn));
        }
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_flushBatchRequests(void* self)
{
    try
    {
        deref<Ice::ObjectPrx>(self)->ice_flushBatchRequests();
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_flushBatchRequestsAsync(void* self, void** future)
{
    *future = 0;
    auto f = make_shared<SimpleFuture>();

    try
    {
        function<void()> token = deref<Ice::ObjectPrx>(self)->ice_flushBatchRequestsAsync(
            [f](exception_ptr e)
            {
                f->exception(e);
            },
            [f](bool /*sentSynchronously*/)
            {
                f->done();
            });
        f->token(token);
        *future = new shared_ptr<SimpleFuture>(move(f));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_clone(void* self, void** r)
{
    *r = new shared_ptr<Ice::ObjectPrx>(deref<Ice::ObjectPrx>(self));
    return 0;
}

mxArray*
Ice_InvocationFuture_unref(void* self)
{
    delete reinterpret_cast<shared_ptr<InvocationFuture>*>(self);
    return 0;
}

mxArray*
Ice_InvocationFuture_id(void* self, unsigned long long* id)
{
    *id = reinterpret_cast<unsigned long long>(self);
    return 0;
}

mxArray*
Ice_InvocationFuture_wait(void* self)
{
    bool b = deref<InvocationFuture>(self)->waitForState(Future::State::Finished, -1);
    return createResultValue(createBool(b));
}

mxArray*
Ice_InvocationFuture_waitState(void* self, mxArray* st, double timeout)
{
    try
    {
        string state = getStringFromUTF16(st);
        bool b = deref<InvocationFuture>(self)->waitForState(state, timeout);
        return createResultValue(createBool(b));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
}

mxArray*
Ice_InvocationFuture_results(void* self)
{
    auto f = deref<InvocationFuture>(self);
    if(!f->waitForState(Future::State::Finished, -1))
    {
        assert(f->getException());
        try
        {
            rethrow_exception(f->getException());
        }
        catch(const std::exception& ex)
        {
            //
            // The C++ object won't be used after this.
            //
            delete reinterpret_cast<shared_ptr<InvocationFuture>*>(self);
            return createResultException(convertException(ex));
        }
    }

    bool ok;
    pair<const Ice::Byte*, const Ice::Byte*> p;
    f->getResults(ok, p);
    mxArray* params = 0;
    if(p.second > p.first)
    {
        params = createByteArray(p.first, p.second);
    }

    //
    // The C++ object won't be used after this.
    //
    delete reinterpret_cast<shared_ptr<InvocationFuture>*>(self);

    return createResultValue(createInvokeResultValue(createBool(ok), params));
}

mxArray*
Ice_InvocationFuture_state(void* self)
{
    return createResultValue(createStringFromUTF8(deref<InvocationFuture>(self)->state()));
}

mxArray*
Ice_InvocationFuture_cancel(void* self)
{
    deref<InvocationFuture>(self)->cancel();
    return 0;
}

mxArray*
Ice_InvocationFuture_check(void* self)
{
    auto f = deref<InvocationFuture>(self);
    if(!f->waitForState(Future::State::Finished, -1))
    {
        assert(f->getException());
        try
        {
            rethrow_exception(f->getException());
        }
        catch(const std::exception& ex)
        {
            //
            // The C++ object won't be used after this.
            //
            delete reinterpret_cast<shared_ptr<InvocationFuture>*>(self);
            return convertException(ex);
        }
    }

    //
    // The C++ object won't be used after this.
    //
    delete reinterpret_cast<shared_ptr<InvocationFuture>*>(self);

    return 0;
}

mxArray*
Ice_GetConnectionFuture_unref(void* self)
{
    delete reinterpret_cast<shared_ptr<GetConnectionFuture>*>(self);
    return 0;
}

mxArray*
Ice_GetConnectionFuture_id(void* self, unsigned long long* id)
{
    *id = reinterpret_cast<unsigned long long>(self);
    return 0;
}

mxArray*
Ice_GetConnectionFuture_wait(void* self)
{
    bool b = deref<GetConnectionFuture>(self)->waitForState(Future::State::Finished, -1);
    return createResultValue(createBool(b));
}

mxArray*
Ice_GetConnectionFuture_waitState(void* self, mxArray* st, double timeout)
{
    try
    {
        string state = getStringFromUTF16(st);
        bool b = deref<GetConnectionFuture>(self)->waitForState(state, timeout);
        return createResultValue(createBool(b));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
}

mxArray*
Ice_GetConnectionFuture_fetch(void* self, void** con)
{
    auto f = deref<GetConnectionFuture>(self);
    if(!f->waitForState(Future::State::Finished, -1))
    {
        assert(f->getException());
        try
        {
            rethrow_exception(f->getException());
        }
        catch(const std::exception& ex)
        {
            //
            // The C++ object won't be used after this.
            //
            delete reinterpret_cast<shared_ptr<GetConnectionFuture>*>(self);
            return convertException(ex);
        }
    }

    auto c = f->getConnection();
    assert(c);
    *con = new shared_ptr<Ice::Connection>(move(c));

    //
    // The C++ object won't be used after this.
    //
    delete reinterpret_cast<shared_ptr<GetConnectionFuture>*>(self);

    return 0;
}

mxArray*
Ice_GetConnectionFuture_state(void* self)
{
    return createResultValue(createStringFromUTF8(deref<GetConnectionFuture>(self)->state()));
}

mxArray*
Ice_GetConnectionFuture_cancel(void* self)
{
    deref<GetConnectionFuture>(self)->cancel();
    return 0;
}

}
