// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#define EXPORT_FCNS

#include <Ice/Ice.h>
#include "icematlab.h"
#include "Future.h"
#include "InputStream.h"
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
    ~InvocationFuture();

    virtual void exception(exception_ptr);
    virtual void sent();
    virtual string state() const;

    void finished(const std::shared_ptr<Ice::Communicator>&, const Ice::EncodingVersion&, bool,
                  std::pair<const Ice::Byte*, const Ice::Byte*>);
    void getResults(bool&, void*&);

protected:

    virtual bool isFinished() const;

private:

    const bool _twoway;
    enum State { Running, Sent, Finished };
    State _state;
    bool _ok; // True for success, false for user exception.
    void* _stream; // The InputStream for reading the results.
};

#define IFSELF (*(reinterpret_cast<shared_ptr<InvocationFuture>*>(self)))

InvocationFuture::InvocationFuture(bool twoway, bool batch) :
    _twoway(twoway),
    _state(batch ? State::Finished : State::Running),
    _ok(false),
    _stream(0)
{
}

InvocationFuture::~InvocationFuture()
{
    if(_stream)
    {
        Ice_InputStream__release(_stream);
    }
}

void
InvocationFuture::sent()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    if(_state == State::Running)
    {
        _state = _twoway ? State::Sent : State::Finished;
    }
}

void
InvocationFuture::exception(exception_ptr e)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    _state = State::Finished;
    _token = nullptr;
    _exception = e;
    notifyAll();
}

void
InvocationFuture::finished(const std::shared_ptr<Ice::Communicator>& communicator,
                           const Ice::EncodingVersion& encoding, bool b, pair<const Ice::Byte*, const Ice::Byte*> p)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    _ok = b;
    _state = State::Finished;
    _token = nullptr;
    if(p.second > p.first)
    {
        vector<Ice::Byte> data(p.first, p.second);
        _stream = createInputStream(communicator, encoding, data);
    }
    notifyAll();
}

string
InvocationFuture::state() const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
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
InvocationFuture::getResults(bool& ok, void*& stream)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    assert(_twoway);
    ok = _ok;
    assert(_stream);
    stream = _stream;
    _stream = 0;
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
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
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
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    _token = nullptr;
    _connection = con;
    notifyAll();
}

shared_ptr<Ice::Connection>
GetConnectionFuture::getConnection() const
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    return _connection;
}

bool
GetConnectionFuture::isFinished() const
{
    return _connection || _exception;
}

}

extern "C"
{

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx__release(void* self)
{
    delete &SELF;
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_equals(void* self, void* other, unsigned char* r)
{
    assert(other); // Wrapper only calls this function for non-nil arguments.
    try
    {
        *r = Ice::targetEqualTo(SELF, DEREF(other)) ? 1 : 0;
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_createOutputStream(void* self, void** stream)
{
    *stream = new Ice::OutputStream(SELF->ice_getCommunicator(), SELF->ice_getEncodingVersion());
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_invoke(void* self, const char* op, Ice_OperationMode m, void* inParams, mxArray* context,
                         unsigned char* result, void** outParams)
{
    Ice::OutputStream* out = reinterpret_cast<Ice::OutputStream*>(inParams);
    Ice::OperationMode mode = static_cast<Ice::OperationMode>(m);
    pair<const Ice::Byte*, const Ice::Byte*> params(0, 0);
    if(out)
    {
        params = out->finished();
    }
    vector<Ice::Byte> v;

    *outParams = 0;

    try
    {
        Ice::Context ctx;
        getStringMap(context, ctx);
        *result = SELF->ice_invoke(op, mode, params, v, ctx) ? 1 : 0;
        if(!v.empty())
        {
            *outParams = createInputStream(SELF->ice_getCommunicator(), SELF->ice_getEncodingVersion(), v);
        }
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_invokeNC(void* self, const char* op, Ice_OperationMode m, void* inParams, unsigned char* result,
                           void** outParams)
{
    Ice::OutputStream* out = reinterpret_cast<Ice::OutputStream*>(inParams);
    Ice::OperationMode mode = static_cast<Ice::OperationMode>(m);
    pair<const Ice::Byte*, const Ice::Byte*> params(0, 0);
    if(out)
    {
        params = out->finished();
    }
    vector<Ice::Byte> v;

    *outParams = 0;

    try
    {
        *result = SELF->ice_invoke(op, mode, params, v) ? 1 : 0;
        if(!v.empty())
        {
            *outParams = createInputStream(SELF->ice_getCommunicator(), SELF->ice_getEncodingVersion(), v);
        }
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_invokeAsync(void* self, const char* op, Ice_OperationMode m, void* inParams, mxArray* context,
                              void** future)
{
    const shared_ptr<Ice::ObjectPrx> proxy = SELF;
    Ice::OutputStream* out = reinterpret_cast<Ice::OutputStream*>(inParams);
    Ice::OperationMode mode = static_cast<Ice::OperationMode>(m);
    pair<const Ice::Byte*, const Ice::Byte*> params(0, 0);
    if(out)
    {
        params = out->finished();
    }

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
        *future = new shared_ptr<InvocationFuture>(f);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_invokeAsyncNC(void* self, const char* op, Ice_OperationMode m, void* inParams, void** future)
{
    const shared_ptr<Ice::ObjectPrx> proxy = SELF;
    Ice::OutputStream* out = reinterpret_cast<Ice::OutputStream*>(inParams);
    Ice::OperationMode mode = static_cast<Ice::OperationMode>(m);
    pair<const Ice::Byte*, const Ice::Byte*> params(0, 0);
    if(out)
    {
        params = out->finished();
    }

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
        *future = new shared_ptr<InvocationFuture>(f);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_toString(void* self)
{
    return createResultValue(createStringFromUTF8(SELF->ice_toString()));
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_getCommunicator(void* self, void** r)
{
    *r = new shared_ptr<Ice::Communicator>(SELF->ice_getCommunicator());
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_getIdentity(void* self)
{
    return createResultValue(createIdentity(SELF->ice_getIdentity()));
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_identity(void* self, mxArray* id, void** r)
{
    try
    {
        Ice::Identity ident;
        getIdentity(id, ident);
        shared_ptr<Ice::ObjectPrx> newProxy = SELF->ice_identity(ident);
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(newProxy);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_getContext(void* self)
{
    return createResultValue(createStringMap(SELF->ice_getContext()));
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_context(void* self, mxArray* c, void** r)
{
    try
    {
        Ice::Context ctx;
        getStringMap(c, ctx);
        shared_ptr<Ice::ObjectPrx> newProxy = SELF->ice_context(ctx);
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(newProxy);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_getFacet(void* self)
{
    const string f = SELF->ice_getFacet();
    return createResultValue(createStringFromUTF8(f));
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_facet(void* self, const char* f, void** r)
{
    try
    {
        shared_ptr<Ice::ObjectPrx> newProxy = SELF->ice_facet(f);
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(newProxy);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_getAdapterId(void* self)
{
    const string f = SELF->ice_getAdapterId();
    return createResultValue(createStringFromUTF8(f));
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_adapterId(void* self, const char* id, void** r)
{
    try
    {
        shared_ptr<Ice::ObjectPrx> newProxy = SELF->ice_adapterId(id);
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(newProxy);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_getEndpoints(void* self)
{
    // TBD
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_endpoints(void* self, mxArray* endpts, void** r)
{
    // TBD
#if 0
    try
    {
        shared_ptr<Ice::ObjectPrx> newProxy = SELF->ice_endpoints(id);
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(newProxy);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
#endif
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_getLocatorCacheTimeout(void* self, int* t)
{
    *t = SELF->ice_getLocatorCacheTimeout();
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_locatorCacheTimeout(void* self, int t, void** r)
{
    try
    {
        shared_ptr<Ice::ObjectPrx> newProxy = SELF->ice_locatorCacheTimeout(t);
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(newProxy);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_getInvocationTimeout(void* self, int* t)
{
    *t = SELF->ice_getInvocationTimeout();
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_invocationTimeout(void* self, int t, void** r)
{
    try
    {
        shared_ptr<Ice::ObjectPrx> newProxy = SELF->ice_invocationTimeout(t);
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(newProxy);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_getConnectionId(void* self)
{
    const string f = SELF->ice_getConnectionId();
    return createResultValue(createStringFromUTF8(f));
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_connectionId(void* self, const char* id, void** r)
{
    try
    {
        shared_ptr<Ice::ObjectPrx> newProxy = SELF->ice_connectionId(id);
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(newProxy);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_isConnectionCached(void* self, unsigned char* r)
{
    *r = SELF->ice_isConnectionCached() ? 1 : 0;
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_connectionCached(void* self, unsigned char v, void** r)
{
    try
    {
        shared_ptr<Ice::ObjectPrx> newProxy = SELF->ice_connectionCached(v == 1);
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(newProxy);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_getEndpointSelection(void* self, Ice_EndpointSelectionType* r)
{
    *r = SELF->ice_getEndpointSelection() == Ice::EndpointSelectionType::Random ? Random : Ordered;
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_endpointSelection(void* self, Ice_EndpointSelectionType v, void** r)
{
    try
    {
        Ice::EndpointSelectionType type = static_cast<Ice::EndpointSelectionType>(v);
        shared_ptr<Ice::ObjectPrx> newProxy = SELF->ice_endpointSelection(type);
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(newProxy);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_getEncodingVersion(void* self)
{
    return createResultValue(createEncodingVersion(SELF->ice_getEncodingVersion()));
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_encodingVersion(void* self, mxArray* v, void** r)
{
    try
    {
        Ice::EncodingVersion ev;
        getEncodingVersion(v, ev);
        shared_ptr<Ice::ObjectPrx> newProxy = SELF->ice_encodingVersion(ev);
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(newProxy);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_getRouter(void* self, void** r)
{
    shared_ptr<Ice::ObjectPrx> router = SELF->ice_getRouter();
    *r = router ? new shared_ptr<Ice::ObjectPrx>(router) : 0;
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_router(void* self, void* rtr, void** r)
{
    try
    {
        shared_ptr<Ice::ObjectPrx> router;
        if(rtr)
        {
            router = *(reinterpret_cast<shared_ptr<Ice::ObjectPrx>*>(rtr));
        }
        shared_ptr<Ice::ObjectPrx> newProxy = SELF->ice_router(Ice::uncheckedCast<Ice::RouterPrx>(router));
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(newProxy);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_getLocator(void* self, void** r)
{
    shared_ptr<Ice::ObjectPrx> locator = SELF->ice_getLocator();
    *r = locator ? new shared_ptr<Ice::ObjectPrx>(locator) : 0;
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_locator(void* self, void* loc, void** r)
{
    try
    {
        shared_ptr<Ice::ObjectPrx> locator;
        if(loc)
        {
            locator = *(reinterpret_cast<shared_ptr<Ice::ObjectPrx>*>(loc));
        }
        shared_ptr<Ice::ObjectPrx> newProxy = SELF->ice_locator(Ice::uncheckedCast<Ice::LocatorPrx>(locator));
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(newProxy);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_isSecure(void* self, unsigned char* r)
{
    *r = SELF->ice_isSecure() ? 1 : 0;
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_secure(void* self, unsigned char b, void** r)
{
    try
    {
        shared_ptr<Ice::ObjectPrx> newProxy = SELF->ice_secure(b == 1);
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(newProxy);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_isPreferSecure(void* self, unsigned char* r)
{
    *r = SELF->ice_isPreferSecure() ? 1 : 0;
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_preferSecure(void* self, unsigned char b, void** r)
{
    try
    {
        shared_ptr<Ice::ObjectPrx> newProxy = SELF->ice_preferSecure(b == 1);
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(newProxy);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_isTwoway(void* self, unsigned char* r)
{
    *r = SELF->ice_isTwoway() ? 1 : 0;
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_twoway(void* self, void** r)
{
    try
    {
        shared_ptr<Ice::ObjectPrx> newProxy = SELF->ice_twoway();
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(newProxy);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_isOneway(void* self, unsigned char* r)
{
    *r = SELF->ice_isOneway() ? 1 : 0;
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_oneway(void* self, void** r)
{
    try
    {
        shared_ptr<Ice::ObjectPrx> newProxy = SELF->ice_oneway();
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(newProxy);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_isBatchOneway(void* self, unsigned char* r)
{
    *r = SELF->ice_isBatchOneway() ? 1 : 0;
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_batchOneway(void* self, void** r)
{
    try
    {
        shared_ptr<Ice::ObjectPrx> newProxy = SELF->ice_batchOneway();
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(newProxy);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_isDatagram(void* self, unsigned char* r)
{
    *r = SELF->ice_isDatagram() ? 1 : 0;
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_datagram(void* self, void** r)
{
    try
    {
        shared_ptr<Ice::ObjectPrx> newProxy = SELF->ice_datagram();
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(newProxy);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_isBatchDatagram(void* self, unsigned char* r)
{
    *r = SELF->ice_isBatchDatagram() ? 1 : 0;
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_batchDatagram(void* self, void** r)
{
    try
    {
        shared_ptr<Ice::ObjectPrx> newProxy = SELF->ice_batchDatagram();
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(newProxy);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_compress(void* self, unsigned char b, void** r)
{
    try
    {
        shared_ptr<Ice::ObjectPrx> newProxy = SELF->ice_compress(b == 1);
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(newProxy);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_timeout(void* self, int t, void** r)
{
    try
    {
        shared_ptr<Ice::ObjectPrx> newProxy = SELF->ice_timeout(t);
        *r = newProxy.get() == SELF.get() ? 0 : new shared_ptr<Ice::ObjectPrx>(newProxy);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_getConnection(void* self, void** r)
{
    *r = 0;
    try
    {
        shared_ptr<Ice::Connection> conn = SELF->ice_getConnection();
        if(conn)
        {
            *r = new shared_ptr<Ice::Connection>(conn);
        }
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
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
        *future = new shared_ptr<GetConnectionFuture>(f);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_getCachedConnection(void* self, void** r)
{
    *r = 0;
    try
    {
        shared_ptr<Ice::Connection> conn = SELF->ice_getCachedConnection();
        if(conn)
        {
            *r = new shared_ptr<Ice::Connection>(conn);
        }
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
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

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_ice_flushBatchRequestsAsync(void* self, void** future)
{
    *future = 0;
    auto f = make_shared<SentFuture>();

    try
    {
        function<void()> token = SELF->ice_flushBatchRequestsAsync(
            [f](exception_ptr e)
            {
                f->exception(e);
            },
            [f](bool /*sentSynchronously*/)
            {
                f->sent();
            });
        f->token(token);
        *future = new shared_ptr<SentFuture>(f);
    }
    catch(const std::exception& ex)
    {
        return convertException(ex);
    }
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_ObjectPrx_clone(void* self, void** r)
{
    shared_ptr<Ice::ObjectPrx> p = SELF;
    *r = new shared_ptr<Ice::ObjectPrx>(p);
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_InvocationFuture__release(void* self)
{
    delete &IFSELF;
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_InvocationFuture_id(void* self, unsigned long long* id)
{
    *id = reinterpret_cast<unsigned long long>(self);
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_InvocationFuture_wait(void* self, unsigned char* ok)
{
    // TBD: Timeout?

    bool b = IFSELF->waitUntilFinished();
    *ok = b ? 1 : 0;
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_InvocationFuture_stream(void* self, unsigned char* ok, void** stream)
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
            return convertException(ex);
        }
    }

    bool b;
    IFSELF->getResults(b, *stream);
    *ok = b ? 1 : 0;
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_InvocationFuture_state(void* self)
{
    return createResultValue(createStringFromUTF8(IFSELF->state()));
}

EXPORTED_FUNCTION mxArray*
Ice_InvocationFuture_cancel(void* self)
{
    IFSELF->cancel();
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_InvocationFuture_check(void* self)
{
    if(!IFSELF->waitUntilFinished())
    {
        assert(IFSELF->getException());
        try
        {
            rethrow_exception(IFSELF->getException());
        }
        catch(const std::exception& ex)
        {
            return convertException(ex);
        }
    }

    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_GetConnectionFuture__release(void* self)
{
    delete &GCFSELF;
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_GetConnectionFuture_id(void* self, unsigned long long* id)
{
    *id = reinterpret_cast<unsigned long long>(self);
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_GetConnectionFuture_wait(void* self, unsigned char* ok)
{
    // TBD: Timeout?

    bool b = GCFSELF->waitUntilFinished();
    *ok = b ? 1 : 0;
    return 0;
}

EXPORTED_FUNCTION mxArray*
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
            return convertException(ex);
        }
    }

    assert(GCFSELF->getConnection());
    *con = new shared_ptr<Ice::Connection>(GCFSELF->getConnection());
    return 0;
}

EXPORTED_FUNCTION mxArray*
Ice_GetConnectionFuture_state(void* self)
{
    return createResultValue(createStringFromUTF8(GCFSELF->state()));
}

EXPORTED_FUNCTION mxArray*
Ice_GetConnectionFuture_cancel(void* self)
{
    GCFSELF->cancel();
    return 0;
}

}
