// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include "ice.h"
#include "Communicator.h"
#include "Endpoint.h"
#include "Future.h"
#include "ObjectPrx.h"
#include "Util.h"

#define DEREF(x) (*(reinterpret_cast<shared_ptr<Ice::ObjectPrx>*>(x)))
#define SELF DEREF(self)

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
    virtual string state() const;

    void finished(const std::shared_ptr<Ice::Communicator>&, const Ice::EncodingVersion&, bool,
                  std::pair<const Ice::Byte*, const Ice::Byte*>);
    void getResults(bool&, pair<const Ice::Byte*, const Ice::Byte*>&);

protected:

    virtual bool isFinished() const;

private:

    const bool _twoway;
    enum State { Running, Sent, Finished };
    State _state;
    bool _ok; // True for success, false for user exception.
    vector<Ice::Byte> _data;
};

#define IFSELF (*(reinterpret_cast<shared_ptr<InvocationFuture>*>(self)))

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
InvocationFuture::finished(const std::shared_ptr<Ice::Communicator>& communicator,
                           const Ice::EncodingVersion& encoding, bool b, pair<const Ice::Byte*, const Ice::Byte*> p)
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

string
InvocationFuture::state() const
{
    lock_guard<mutex> lock(const_cast<mutex&>(_mutex));
    string st;
    switch(_state)
    {
        case State::Running:
            st = "running";
            break;
        case State::Sent:
            st = "sent";
            break;
        case State::Finished:
            st = "finished";
            break;
    }
    return st;
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

bool
InvocationFuture::isFinished() const
{
    return _state == State::Finished;
}

class GetConnectionFuture : public Future
{
public:

    virtual string state() const;

    void finished(shared_ptr<Ice::Connection>);
    shared_ptr<Ice::Connection> getConnection() const;

protected:

    virtual bool isFinished() const;

private:

    shared_ptr<Ice::Connection> _connection;
};

#define GCFSELF (*(reinterpret_cast<shared_ptr<GetConnectionFuture>*>(self)))

string
GetConnectionFuture::state() const
{
    lock_guard<mutex> lock(const_cast<mutex&>(_mutex));
    if(_exception || _connection)
    {
        return "finished";
    }
    else
    {
        return "running";
    }
}

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

bool
GetConnectionFuture::isFinished() const
{
    return _connection || _exception;
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

void*
IceMatlab::createProxy(shared_ptr<Ice::ObjectPrx> p)
{
    return new shared_ptr<Ice::ObjectPrx>(move(p));
}

shared_ptr<Ice::ObjectPrx>
IceMatlab::getProxy(void* p)
{
    return DEREF(p);
}

extern "C"
{

mxArray*
Ice_ObjectPrx_unref(void* self)
{
    delete &SELF;
    return 0;
}

mxArray*
Ice_ObjectPrx_equals(void* self, void* other)
{
    assert(other); // Wrapper only calls this function for non-nil arguments.
    try
    {
        return createResultValue(createBool(Ice::targetEqualTo(SELF, DEREF(other))));
    }
    catch(const std::exception& ex)
    {
        return createResultException(convertException(ex));
    }
    return 0;
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

        Ice::InputStream in(getCommunicator(communicator), ev, p);
        shared_ptr<Ice::ObjectPrx> proxy;
        in.read(proxy);
        if(proxy)
        {
            *r = createProxy(proxy);
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
            prx = DEREF(proxy);
        }

        assert(communicator);
        shared_ptr<Ice::Communicator> comm = getCommunicator(communicator);

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
    return 0;
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
        auto ok = SELF->ice_invoke(op, mode, params, v, ctx);
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
        auto ok = SELF->ice_invoke(op, mode, params, v);
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
    const shared_ptr<Ice::ObjectPrx> proxy = SELF;
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
    const shared_ptr<Ice::ObjectPrx> proxy = SELF;
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
    return createResultValue(createStringFromUTF8(SELF->ice_toString()));
}

mxArray*
Ice_ObjectPrx_ice_getIdentity(void* self)
{
    return createResultValue(createIdentity(SELF->ice_getIdentity()));
}

mxArray*
Ice_ObjectPrx_ice_identity(void* self, void** r, mxArray* id)
{
    try
    {
        Ice::Identity ident;
        getIdentity(id, ident);
        auto newProxy = SELF->ice_identity(ident);
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
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
    return createResultValue(createStringMap(SELF->ice_getContext()));
}

mxArray*
Ice_ObjectPrx_ice_context(void* self, void** r, mxArray* c)
{
    try
    {
        Ice::Context ctx;
        getStringMap(c, ctx);
        auto newProxy = SELF->ice_context(ctx);
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
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
    auto s = SELF->ice_getFacet();
    return createResultValue(createStringFromUTF8(s));
}

mxArray*
Ice_ObjectPrx_ice_facet(void* self, void** r, const char* f)
{
    try
    {
        auto newProxy = SELF->ice_facet(f);
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
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
    auto s = SELF->ice_getAdapterId();
    return createResultValue(createStringFromUTF8(s));
}

mxArray*
Ice_ObjectPrx_ice_adapterId(void* self, void** r, const char* id)
{
    try
    {
        auto newProxy = SELF->ice_adapterId(id);
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
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
        return createResultValue(createInt(static_cast<int>(SELF->ice_getEndpoints().size())));
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
        auto endpoints = SELF->ice_getEndpoints();
        if(idx > endpoints.size())
        {
            throw std::invalid_argument("index outside range");
        }
        *r = createEndpoint(endpoints[idx]);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_createEndpointList(void* self, unsigned int num, void** r)
{
    *r = new vector<shared_ptr<Ice::Endpoint>>(num);
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_setEndpoint(void* self, void* arr, unsigned int idx, void* e)
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
        (*v)[idx] = getEndpoint(e);
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
        auto newProxy = SELF->ice_endpoints(tmp);
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
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
    return createResultValue(createInt(SELF->ice_getLocatorCacheTimeout()));
}

mxArray*
Ice_ObjectPrx_ice_locatorCacheTimeout(void* self, void** r, int t)
{
    try
    {
        auto newProxy = SELF->ice_locatorCacheTimeout(t);
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
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
    return createResultValue(createInt(SELF->ice_getInvocationTimeout()));
}

mxArray*
Ice_ObjectPrx_ice_invocationTimeout(void* self, void** r, int t)
{
    try
    {
        auto newProxy = SELF->ice_invocationTimeout(t);
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
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
    auto s = SELF->ice_getConnectionId();
    return createResultValue(createStringFromUTF8(s));
}

mxArray*
Ice_ObjectPrx_ice_connectionId(void* self, void** r, const char* id)
{
    try
    {
        auto newProxy = SELF->ice_connectionId(id);
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
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
    return createResultValue(createBool(SELF->ice_isConnectionCached()));
}

mxArray*
Ice_ObjectPrx_ice_connectionCached(void* self, void** r, unsigned char v)
{
    try
    {
        auto newProxy = SELF->ice_connectionCached(v == 1);
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
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
        auto type = SELF->ice_getEndpointSelection();
        return createResultValue(createInt(static_cast<int>(type)));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_endpointSelection(void* self, void** r, mxArray* type)
{
    try
    {
        auto t = static_cast<Ice::EndpointSelectionType>(getEnumerator(type, "Ice.EndpointSelectionType"));
        auto newProxy = SELF->ice_endpointSelection(t);
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
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
    return createResultValue(createEncodingVersion(SELF->ice_getEncodingVersion()));
}

mxArray*
Ice_ObjectPrx_ice_encodingVersion(void* self, void** r, mxArray* v)
{
    try
    {
        Ice::EncodingVersion ev;
        getEncodingVersion(v, ev);
        auto newProxy = SELF->ice_encodingVersion(ev);
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
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
    auto router = SELF->ice_getRouter();
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
            router = *(reinterpret_cast<shared_ptr<Ice::ObjectPrx>*>(rtr));
        }
        auto newProxy = SELF->ice_router(Ice::uncheckedCast<Ice::RouterPrx>(router));
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
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
    auto locator = SELF->ice_getLocator();
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
            locator = *(reinterpret_cast<shared_ptr<Ice::ObjectPrx>*>(loc));
        }
        auto newProxy = SELF->ice_locator(Ice::uncheckedCast<Ice::LocatorPrx>(locator));
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
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
    return createResultValue(createBool(SELF->ice_isSecure()));
}

mxArray*
Ice_ObjectPrx_ice_secure(void* self, void** r, unsigned char b)
{
    try
    {
        auto newProxy = SELF->ice_secure(b == 1);
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
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
    return createResultValue(createBool(SELF->ice_isPreferSecure()));
}

mxArray*
Ice_ObjectPrx_ice_preferSecure(void* self, void** r, unsigned char b)
{
    try
    {
        auto newProxy = SELF->ice_preferSecure(b == 1);
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
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
    return createResultValue(createBool(SELF->ice_isTwoway()));
}

mxArray*
Ice_ObjectPrx_ice_twoway(void* self, void** r)
{
    try
    {
        auto newProxy = SELF->ice_twoway();
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
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
    return createResultValue(createBool(SELF->ice_isOneway()));
}

mxArray*
Ice_ObjectPrx_ice_oneway(void* self, void** r)
{
    try
    {
        auto newProxy = SELF->ice_oneway();
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
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
    return createResultValue(createBool(SELF->ice_isBatchOneway()));
}

mxArray*
Ice_ObjectPrx_ice_batchOneway(void* self, void** r)
{
    try
    {
        auto newProxy = SELF->ice_batchOneway();
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
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
    return createResultValue(createBool(SELF->ice_isDatagram()));
}

mxArray*
Ice_ObjectPrx_ice_datagram(void* self, void** r)
{
    try
    {
        auto newProxy = SELF->ice_datagram();
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
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
    return createResultValue(createBool(SELF->ice_isBatchDatagram()));
}

mxArray*
Ice_ObjectPrx_ice_batchDatagram(void* self, void** r)
{
    try
    {
        auto newProxy = SELF->ice_batchDatagram();
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
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
        auto newProxy = SELF->ice_compress(b == 1);
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

mxArray*
Ice_ObjectPrx_ice_timeout(void* self, void** r, int t)
{
    try
    {
        auto newProxy = SELF->ice_timeout(t);
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(move(newProxy));
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
        auto conn = SELF->ice_getConnection();
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
        function<void()> token = SELF->ice_getConnectionAsync(
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
        auto conn = SELF->ice_getCachedConnection();
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
        SELF->ice_flushBatchRequests();
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
        function<void()> token = SELF->ice_flushBatchRequestsAsync(
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
    auto p = SELF;
    *r = new shared_ptr<Ice::ObjectPrx>(move(p));
    return 0;
}

mxArray*
Ice_InvocationFuture_unref(void* self)
{
    delete &IFSELF;
    return 0;
}

mxArray*
Ice_InvocationFuture_id(void* self, unsigned long long* id)
{
    *id = reinterpret_cast<unsigned long long>(self);
    return 0;
}

mxArray*
Ice_InvocationFuture_wait(void* self, unsigned char* ok)
{
    // TBD: Timeout?

    bool b = IFSELF->waitUntilFinished();
    *ok = b ? 1 : 0;
    return 0;
}

mxArray*
Ice_InvocationFuture_results(void* self)
{
    // TBD: Timeout?

    if(!IFSELF->waitUntilFinished())
    {
        assert(IFSELF->getException());
        try
        {
            rethrow_exception(IFSELF->getException());
        }
        catch(const std::exception& ex)
        {
            //
            // The C++ object won't be used after this.
            //
            delete &IFSELF;
            return createResultException(convertException(ex));
        }
    }

    bool ok;
    pair<const Ice::Byte*, const Ice::Byte*> p;
    IFSELF->getResults(ok, p);
    mxArray* params = 0;
    if(p.second > p.first)
    {
        params = createByteArray(p.first, p.second);
    }

    //
    // The C++ object won't be used after this.
    //
    delete &IFSELF;

    return createResultValue(createInvokeResultValue(createBool(ok), params));
}

mxArray*
Ice_InvocationFuture_state(void* self)
{
    return createResultValue(createStringFromUTF8(IFSELF->state()));
}

mxArray*
Ice_InvocationFuture_cancel(void* self)
{
    IFSELF->cancel();
    return 0;
}

mxArray*
Ice_InvocationFuture_check(void* self)
{
    if(!IFSELF->waitUntilFinished())
    {
        assert(IFSELF->getException());
        try
        {
            //
            // The C++ object won't be used after this.
            //
            delete &IFSELF;
            rethrow_exception(IFSELF->getException());
        }
        catch(const std::exception& ex)
        {
            return convertException(ex);
        }
    }

    //
    // The C++ object won't be used after this.
    //
    delete &IFSELF;

    return 0;
}

mxArray*
Ice_GetConnectionFuture_unref(void* self)
{
    delete &GCFSELF;
    return 0;
}

mxArray*
Ice_GetConnectionFuture_id(void* self, unsigned long long* id)
{
    *id = reinterpret_cast<unsigned long long>(self);
    return 0;
}

mxArray*
Ice_GetConnectionFuture_wait(void* self, unsigned char* ok)
{
    // TBD: Timeout?

    bool b = GCFSELF->waitUntilFinished();
    *ok = b ? 1 : 0;
    return 0;
}

mxArray*
Ice_GetConnectionFuture_fetch(void* self, void** con)
{
    // TBD: Timeout?

    if(!GCFSELF->waitUntilFinished())
    {
        assert(GCFSELF->getException());
        try
        {
            rethrow_exception(GCFSELF->getException());
        }
        catch(const std::exception& ex)
        {
            //
            // The C++ object won't be used after this.
            //
            delete &IFSELF;
            return convertException(ex);
        }
    }

    auto c = GCFSELF->getConnection();
    assert(c);
    *con = new shared_ptr<Ice::Connection>(move(c));

    //
    // The C++ object won't be used after this.
    //
    delete &IFSELF;

    return 0;
}

mxArray*
Ice_GetConnectionFuture_state(void* self)
{
    return createResultValue(createStringFromUTF8(GCFSELF->state()));
}

mxArray*
Ice_GetConnectionFuture_cancel(void* self)
{
    GCFSELF->cancel();
    return 0;
}

}
