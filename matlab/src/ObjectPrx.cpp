//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Future.h"
#include "Ice/Ice.h"
#include "Util.h"
#include "ice.h"

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

        void finished(
            const std::shared_ptr<Ice::Communicator>&,
            const Ice::EncodingVersion&,
            bool,
            std::pair<const byte*, const byte*>);
        void getResults(bool&, pair<const byte*, const byte*>&);

    protected:
        virtual State stateImpl() const;

    private:
        const bool _twoway;
        State _state;
        bool _ok; // True for success, false for user exception.
        vector<byte> _data;
    };

    InvocationFuture::InvocationFuture(bool twoway, bool batch)
        : _twoway(twoway),
          _state(batch ? State::Finished : State::Running),
          _ok(false)
    {
    }

    void InvocationFuture::sent()
    {
        lock_guard<mutex> lock(_mutex);
        if (_state == State::Running)
        {
            _state = _twoway ? State::Sent : State::Finished;
        }
    }

    void InvocationFuture::exception(exception_ptr e)
    {
        lock_guard<mutex> lock(_mutex);
        _state = State::Finished;
        _token = nullptr;
        _exception = e;
        _cond.notify_all();
    }

    void InvocationFuture::finished(
        const std::shared_ptr<Ice::Communicator>& /*communicator*/,
        const Ice::EncodingVersion& /*encoding*/,
        bool b,
        pair<const byte*, const byte*> p)
    {
        lock_guard<mutex> lock(_mutex);
        _ok = b;
        _state = State::Finished;
        _token = nullptr;
        if (p.second > p.first)
        {
            vector<byte> data(p.first, p.second); // Makes a copy.
            _data.swap(data);                     // Avoids another copy.
        }
        _cond.notify_all();
    }

    void InvocationFuture::getResults(bool& ok, pair<const byte*, const byte*>& p)
    {
        lock_guard<mutex> lock(_mutex);
        assert(_twoway);
        ok = _ok;
        if (!_data.empty())
        {
            p.first = &_data[0];
            p.second = p.first + _data.size();
        }
        else
        {
            p.first = p.second = 0;
        }
    }

    Future::State InvocationFuture::stateImpl() const { return _state; }

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

    void GetConnectionFuture::finished(shared_ptr<Ice::Connection> con)
    {
        lock_guard<mutex> lock(_mutex);
        _token = nullptr;
        _connection = con;
        _cond.notify_all();
    }

    shared_ptr<Ice::Connection> GetConnectionFuture::getConnection() const
    {
        lock_guard<mutex> lock(const_cast<mutex&>(_mutex));
        return _connection;
    }

    Future::State GetConnectionFuture::stateImpl() const
    {
        if (_exception || _connection)
        {
            return State::Finished;
        }
        else
        {
            return State::Running;
        }
    }

    static const char* invokeResultFields[] = {"ok", "params"};

    mxArray* createInvokeResultValue(mxArray* ok, mxArray* params)
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
    mxArray* Ice_ObjectPrx_unref(void* self)
    {
        delete reinterpret_cast<Ice::ObjectPrx*>(self);
        return 0;
    }

    mxArray* Ice_ObjectPrx_equals(void* self, void* other)
    {
        assert(other); // Wrapper only calls this function for non-nil arguments.
        try
        {
            return createResultValue(createBool(restoreProxy(self) == restoreProxy(other)));
        }
        catch (...)
        {
            return createResultException(convertException(std::current_exception()));
        }
    }

    mxArray* Ice_ObjectPrx_read(void* communicator, mxArray* encoding, mxArray* buf, int size, void** r, int* bytesRead)
    {
        assert(!mxIsEmpty(buf));

        pair<const byte*, const byte*> p;
        p.first = reinterpret_cast<byte*>(mxGetData(buf));
        p.second = p.first + size;

        try
        {
            Ice::EncodingVersion ev;
            getEncodingVersion(encoding, ev);

            Ice::InputStream in(deref<Ice::Communicator>(communicator), ev, p);
            std::optional<Ice::ObjectPrx> proxy;
            in.read(proxy);
            *r = createProxy(std::move(proxy)); // can return nullptr for a null proxy
            *bytesRead = static_cast<int>(in.pos());
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_ObjectPrx_write(void* proxy, void* communicator, mxArray* encoding)
    {
        //
        // Marshal a proxy into a stream and return the encoded bytes.
        //
        try
        {
            optional<Ice::ObjectPrx> prx = restoreNullableProxy(proxy);

            assert(communicator);
            auto comm = deref<Ice::Communicator>(communicator);

            Ice::EncodingVersion enc;
            getEncodingVersion(encoding, enc);

            Ice::OutputStream out(comm, enc);
            out.write(prx);
            pair<const byte*, const byte*> p = out.finished();

            assert(p.second > p.first);
            return createResultValue(createByteArray(p.first, p.second));
        }
        catch (...)
        {
            return createResultException(convertException(std::current_exception()));
        }
    }

    mxArray*
    Ice_ObjectPrx_ice_invoke(void* self, const char* op, int m, mxArray* inParams, unsigned int size, mxArray* context)
    {
        pair<const byte*, const byte*> params(0, 0);
        if (!mxIsEmpty(inParams))
        {
            params.first = reinterpret_cast<byte*>(mxGetData(inParams));
            params.second = params.first + size;
        }
        auto mode = static_cast<Ice::OperationMode>(m);
        vector<byte> v;

        try
        {
            Ice::Context ctx;
            getContext(context, ctx);
            auto ok = restoreProxy(self)->ice_invoke(op, mode, params, v, ctx);
            mxArray* results = 0;
            if (!v.empty())
            {
                results = createByteArray(&v[0], &v[0] + v.size());
            }
            return createResultValue(createInvokeResultValue(createBool(ok), results));
        }
        catch (...)
        {
            return createResultException(convertException(std::current_exception()));
        }
        return 0;
    }

    mxArray* Ice_ObjectPrx_ice_invokeNC(void* self, const char* op, int m, mxArray* inParams, unsigned int size)
    {
        pair<const byte*, const byte*> params(0, 0);
        if (!mxIsEmpty(inParams))
        {
            params.first = reinterpret_cast<byte*>(mxGetData(inParams));
            params.second = params.first + size;
        }
        auto mode = static_cast<Ice::OperationMode>(m);
        vector<byte> v;

        try
        {
            auto ok = restoreProxy(self)->ice_invoke(op, mode, params, v);
            mxArray* results = 0;
            if (!v.empty())
            {
                results = createByteArray(&v[0], &v[0] + v.size());
            }
            return createResultValue(createInvokeResultValue(createBool(ok), results));
        }
        catch (...)
        {
            return createResultException(convertException(std::current_exception()));
        }
        return 0;
    }

    mxArray* Ice_ObjectPrx_ice_invokeAsync(
        void* self,
        const char* op,
        int m,
        mxArray* inParams,
        unsigned int size,
        mxArray* context,
        void** future)
    {
        const auto proxy = restoreProxy(self);
        pair<const byte*, const byte*> params(0, 0);
        if (!mxIsEmpty(inParams))
        {
            params.first = reinterpret_cast<byte*>(mxGetData(inParams));
            params.second = params.first + size;
        }
        auto mode = static_cast<Ice::OperationMode>(m);

        *future = 0;
        auto f = make_shared<InvocationFuture>(
            proxy->ice_isTwoway(),
            proxy->ice_isBatchOneway() || proxy->ice_isBatchDatagram());

        try
        {
            Ice::Context ctx;
            getContext(context, ctx);
            function<void()> token = proxy->ice_invokeAsync(
                op,
                mode,
                params,
                [proxy, f](bool ok, pair<const byte*, const byte*> outParams)
                { f->finished(proxy->ice_getCommunicator(), proxy->ice_getEncodingVersion(), ok, outParams); },
                [f](exception_ptr e) { f->exception(e); },
                [f](bool /*sentSynchronously*/) { f->sent(); },
                ctx);
            f->token(token);
            *future = new shared_ptr<InvocationFuture>(move(f));
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_ObjectPrx_ice_invokeAsyncNC(
        void* self,
        const char* op,
        int m,
        mxArray* inParams,
        unsigned int size,
        void** future)
    {
        const auto proxy = restoreProxy(self);
        pair<const byte*, const byte*> params(0, 0);
        if (!mxIsEmpty(inParams))
        {
            params.first = reinterpret_cast<byte*>(mxGetData(inParams));
            params.second = params.first + size;
        }
        auto mode = static_cast<Ice::OperationMode>(m);

        *future = 0;
        auto f = make_shared<InvocationFuture>(
            proxy->ice_isTwoway(),
            proxy->ice_isBatchOneway() || proxy->ice_isBatchDatagram());

        try
        {
            function<void()> token = proxy->ice_invokeAsync(
                op,
                mode,
                params,
                [proxy, f](bool ok, pair<const byte*, const byte*> outParams)
                { f->finished(proxy->ice_getCommunicator(), proxy->ice_getEncodingVersion(), ok, outParams); },
                [f](exception_ptr e) { f->exception(e); },
                [f](bool /*sentSynchronously*/) { f->sent(); });
            f->token(token);
            *future = new shared_ptr<InvocationFuture>(move(f));
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_ObjectPrx_ice_toString(void* self)
    {
        return createResultValue(createStringFromUTF8(restoreProxy(self)->ice_toString()));
    }

    mxArray* Ice_ObjectPrx_ice_getIdentity(void* self)
    {
        return createResultValue(createIdentity(restoreProxy(self)->ice_getIdentity()));
    }

    mxArray* Ice_ObjectPrx_ice_identity(void* self, void** r, mxArray* id)
    {
        try
        {
            Ice::Identity ident;
            getIdentity(id, ident);
            auto proxy = restoreProxy(self);
            auto newProxy = proxy->ice_identity(ident);
            *r = newProxy == proxy ? nullptr : new Ice::ObjectPrx(std::move(newProxy));
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_ObjectPrx_ice_getContext(void* self)
    {
        return createResultValue(createContext(restoreProxy(self)->ice_getContext()));
    }

    mxArray* Ice_ObjectPrx_ice_context(void* self, void** r, mxArray* c)
    {
        try
        {
            Ice::Context ctx;
            getContext(c, ctx);
            auto proxy = restoreProxy(self);
            auto newProxy = proxy->ice_context(ctx);
            *r = newProxy == proxy ? nullptr : new Ice::ObjectPrx(std::move(newProxy));
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_ObjectPrx_ice_getFacet(void* self)
    {
        return createResultValue(createStringFromUTF8(restoreProxy(self)->ice_getFacet()));
    }

    mxArray* Ice_ObjectPrx_ice_facet(void* self, void** r, const char* f)
    {
        try
        {
            auto proxy = restoreProxy(self);
            auto newProxy = proxy->ice_facet(f);
            *r = newProxy == proxy ? nullptr : new Ice::ObjectPrx(std::move(newProxy));
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_ObjectPrx_ice_getAdapterId(void* self)
    {
        return createResultValue(createStringFromUTF8(restoreProxy(self)->ice_getAdapterId()));
    }

    mxArray* Ice_ObjectPrx_ice_adapterId(void* self, void** r, const char* id)
    {
        try
        {
            auto proxy = restoreProxy(self);
            auto newProxy = proxy->ice_adapterId(id);
            *r = newProxy == proxy ? nullptr : new Ice::ObjectPrx(std::move(newProxy));
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_ObjectPrx_ice_getNumEndpoints(void* self)
    {
        try
        {
            return createResultValue(createInt(static_cast<int>(restoreProxy(self)->ice_getEndpoints().size())));
        }
        catch (...)
        {
            return createResultException(convertException(std::current_exception()));
        }
    }

    mxArray* Ice_ObjectPrx_ice_getEndpoint(void* self, unsigned int idx, void** r)
    {
        try
        {
            auto endpoints = restoreProxy(self)->ice_getEndpoints();
            if (idx > endpoints.size())
            {
                throw std::invalid_argument("index outside range");
            }
            *r = createShared<Ice::Endpoint>(endpoints[idx]);
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_ObjectPrx_ice_createEndpointList(void* /*self*/, unsigned int num, void** r)
    {
        *r = new vector<shared_ptr<Ice::Endpoint>>(num);
        return 0;
    }

    mxArray* Ice_ObjectPrx_ice_setEndpoint(void* /*self*/, void* arr, unsigned int idx, void* e)
    {
        //
        // arr must hold a pointer returned by createEndpointList.
        //

        try
        {
            vector<shared_ptr<Ice::Endpoint>>* v = reinterpret_cast<vector<shared_ptr<Ice::Endpoint>>*>(arr);
            if (idx > v->size())
            {
                throw std::invalid_argument("index outside range");
            }
            (*v)[idx] = deref<Ice::Endpoint>(e);
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_ObjectPrx_ice_endpoints(void* self, void** r, void* arr)
    {
        //
        // arr must hold a pointer returned by createEndpointList and populated with setEndpoint.
        //

        try
        {
            vector<shared_ptr<Ice::Endpoint>>* v = reinterpret_cast<vector<shared_ptr<Ice::Endpoint>>*>(arr);
            vector<shared_ptr<Ice::Endpoint>> tmp = *v;
            delete v;
            auto proxy = restoreProxy(self);
            auto newProxy = proxy->ice_endpoints(tmp);
            *r = newProxy == proxy ? nullptr : new Ice::ObjectPrx(std::move(newProxy));
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_ObjectPrx_ice_getLocatorCacheTimeout(void* self)
    {
        chrono::seconds timeout =
            chrono::duration_cast<chrono::seconds>(restoreProxy(self)->ice_getLocatorCacheTimeout());
        return createResultValue(createInt(static_cast<int32_t>(timeout.count())));
    }

    mxArray* Ice_ObjectPrx_ice_locatorCacheTimeout(void* self, void** r, int t)
    {
        try
        {
            auto proxy = restoreProxy(self);
            auto newProxy = proxy->ice_locatorCacheTimeout(t);
            *r = newProxy == proxy ? nullptr : new Ice::ObjectPrx(std::move(newProxy));
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_ObjectPrx_ice_getInvocationTimeout(void* self)
    {
        chrono::milliseconds timeout = restoreProxy(self)->ice_getInvocationTimeout();
        return createResultValue(createInt(static_cast<int32_t>(timeout.count())));
    }

    mxArray* Ice_ObjectPrx_ice_invocationTimeout(void* self, void** r, int t)
    {
        try
        {
            auto proxy = restoreProxy(self);
            auto newProxy = proxy->ice_invocationTimeout(t);
            *r = newProxy == proxy ? nullptr : new Ice::ObjectPrx(std::move(newProxy));
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_ObjectPrx_ice_getConnectionId(void* self)
    {
        auto s = restoreProxy(self)->ice_getConnectionId();
        return createResultValue(createStringFromUTF8(s));
    }

    mxArray* Ice_ObjectPrx_ice_connectionId(void* self, void** r, const char* id)
    {
        try
        {
            auto proxy = restoreProxy(self);
            auto newProxy = proxy->ice_connectionId(id);
            *r = newProxy == proxy ? nullptr : new Ice::ObjectPrx(std::move(newProxy));
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_ObjectPrx_ice_isConnectionCached(void* self)
    {
        return createResultValue(createBool(restoreProxy(self)->ice_isConnectionCached()));
    }

    mxArray* Ice_ObjectPrx_ice_connectionCached(void* self, void** r, unsigned char v)
    {
        try
        {
            auto proxy = restoreProxy(self);
            auto newProxy = proxy->ice_connectionCached(v == 1);
            *r = newProxy == proxy ? nullptr : new Ice::ObjectPrx(std::move(newProxy));
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_ObjectPrx_ice_getEndpointSelection(void* self)
    {
        try
        {
            return createResultValue(createInt(static_cast<int>(restoreProxy(self)->ice_getEndpointSelection())));
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
    }

    mxArray* Ice_ObjectPrx_ice_endpointSelection(void* self, void** r, mxArray* type)
    {
        try
        {
            auto proxy = restoreProxy(self);
            auto newProxy = proxy->ice_endpointSelection(
                static_cast<Ice::EndpointSelectionType>(getEnumerator(type, "Ice.EndpointSelectionType")));
            *r = newProxy == proxy ? nullptr : new Ice::ObjectPrx(std::move(newProxy));
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_ObjectPrx_ice_getEncodingVersion(void* self)
    {
        return createResultValue(createEncodingVersion(restoreProxy(self)->ice_getEncodingVersion()));
    }

    mxArray* Ice_ObjectPrx_ice_encodingVersion(void* self, void** r, mxArray* v)
    {
        try
        {
            Ice::EncodingVersion ev;
            getEncodingVersion(v, ev);
            auto proxy = restoreProxy(self);
            auto newProxy = proxy->ice_encodingVersion(ev);
            *r = newProxy == proxy ? nullptr : new Ice::ObjectPrx(std::move(newProxy));
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_ObjectPrx_ice_getRouter(void* self, void** r)
    {
        auto router = restoreProxy(self)->ice_getRouter();
        *r = createProxy(std::move(router));
        return 0;
    }

    mxArray* Ice_ObjectPrx_ice_router(void* self, void** r, void* rtr)
    {
        try
        {
            optional<Ice::ObjectPrx> router = restoreNullableProxy(rtr);
            auto proxy = restoreProxy(self);
            auto newProxy = proxy->ice_router(Ice::uncheckedCast<Ice::RouterPrx>(router));
            *r = newProxy == proxy ? nullptr : new Ice::ObjectPrx(std::move(newProxy));
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_ObjectPrx_ice_getLocator(void* self, void** r)
    {
        auto locator = restoreProxy(self)->ice_getLocator();
        *r = createProxy(std::move(locator));
        return 0;
    }

    mxArray* Ice_ObjectPrx_ice_locator(void* self, void** r, void* loc)
    {
        try
        {
            optional<Ice::ObjectPrx> locator = restoreNullableProxy(loc);
            auto proxy = restoreProxy(self);
            auto newProxy = proxy->ice_locator(Ice::uncheckedCast<Ice::LocatorPrx>(locator));
            *r = newProxy == proxy ? nullptr : new Ice::ObjectPrx(std::move(newProxy));
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_ObjectPrx_ice_isSecure(void* self)
    {
        return createResultValue(createBool(restoreProxy(self)->ice_isSecure()));
    }

    mxArray* Ice_ObjectPrx_ice_secure(void* self, void** r, unsigned char b)
    {
        try
        {
            auto proxy = restoreProxy(self);
            auto newProxy = proxy->ice_secure(b == 1);
            *r = newProxy == proxy ? nullptr : new Ice::ObjectPrx(std::move(newProxy));
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_ObjectPrx_ice_isPreferSecure(void* self)
    {
        return createResultValue(createBool(restoreProxy(self)->ice_isPreferSecure()));
    }

    mxArray* Ice_ObjectPrx_ice_preferSecure(void* self, void** r, unsigned char b)
    {
        try
        {
            auto proxy = restoreProxy(self);
            auto newProxy = proxy->ice_preferSecure(b == 1);
            *r = newProxy == proxy ? nullptr : new Ice::ObjectPrx(std::move(newProxy));
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_ObjectPrx_ice_isTwoway(void* self)
    {
        return createResultValue(createBool(restoreProxy(self)->ice_isTwoway()));
    }

    mxArray* Ice_ObjectPrx_ice_twoway(void* self, void** r)
    {
        try
        {
            auto proxy = restoreProxy(self);
            auto newProxy = proxy->ice_twoway();
            *r = newProxy == proxy ? nullptr : new Ice::ObjectPrx(std::move(newProxy));
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_ObjectPrx_ice_isOneway(void* self)
    {
        return createResultValue(createBool(restoreProxy(self)->ice_isOneway()));
    }

    mxArray* Ice_ObjectPrx_ice_oneway(void* self, void** r)
    {
        try
        {
            auto proxy = restoreProxy(self);
            auto newProxy = proxy->ice_oneway();
            *r = newProxy == proxy ? nullptr : new Ice::ObjectPrx(std::move(newProxy));
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_ObjectPrx_ice_isBatchOneway(void* self)
    {
        return createResultValue(createBool(restoreProxy(self)->ice_isBatchOneway()));
    }

    mxArray* Ice_ObjectPrx_ice_batchOneway(void* self, void** r)
    {
        try
        {
            auto proxy = restoreProxy(self);
            auto newProxy = proxy->ice_batchOneway();
            *r = newProxy == proxy ? nullptr : new Ice::ObjectPrx(std::move(newProxy));
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_ObjectPrx_ice_isDatagram(void* self)
    {
        return createResultValue(createBool(restoreProxy(self)->ice_isDatagram()));
    }

    mxArray* Ice_ObjectPrx_ice_datagram(void* self, void** r)
    {
        try
        {
            auto proxy = restoreProxy(self);
            auto newProxy = proxy->ice_datagram();
            *r = newProxy == proxy ? nullptr : new Ice::ObjectPrx(std::move(newProxy));
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_ObjectPrx_ice_isBatchDatagram(void* self)
    {
        return createResultValue(createBool(restoreProxy(self)->ice_isBatchDatagram()));
    }

    mxArray* Ice_ObjectPrx_ice_batchDatagram(void* self, void** r)
    {
        try
        {
            auto proxy = restoreProxy(self);
            auto newProxy = proxy->ice_batchDatagram();
            *r = newProxy == proxy ? nullptr : new Ice::ObjectPrx(std::move(newProxy));
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_ObjectPrx_ice_compress(void* self, void** r, unsigned char b)
    {
        try
        {
            auto proxy = restoreProxy(self);
            auto newProxy = proxy->ice_compress(b == 1);
            *r = newProxy == proxy ? nullptr : new Ice::ObjectPrx(std::move(newProxy));
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_ObjectPrx_ice_getCompress(void* self)
    {
        auto v = restoreProxy(self)->ice_getCompress();
        if (v.has_value())
        {
            return createResultValue(createOptionalValue(true, createBool(v.value())));
        }
        else
        {
            return createResultValue(createOptionalValue(false, 0));
        }
    }

    mxArray* Ice_ObjectPrx_ice_fixed(void* self, void** r, void* connection)
    {
        assert(connection); // Wrapper only calls this function for non-nil arguments.
        try
        {
            auto proxy = restoreProxy(self);
            auto newProxy = proxy->ice_fixed(deref<Ice::Connection>(connection));
            *r = newProxy == proxy ? nullptr : new Ice::ObjectPrx(std::move(newProxy));
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_ObjectPrx_ice_isFixed(void* self)
    {
        return createResultValue(createBool(restoreProxy(self)->ice_isFixed()));
    }

    mxArray* Ice_ObjectPrx_ice_getConnection(void* self, void** r)
    {
        *r = 0;
        try
        {
            auto conn = restoreProxy(self)->ice_getConnection();
            if (conn)
            {
                *r = new shared_ptr<Ice::Connection>(move(conn));
            }
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_ObjectPrx_ice_getConnectionAsync(void* self, void** future)
    {
        *future = 0;
        auto f = make_shared<GetConnectionFuture>();

        try
        {
            function<void()> token = restoreProxy(self)->ice_getConnectionAsync(
                [f](shared_ptr<Ice::Connection> con) { f->finished(con); },
                [f](exception_ptr e) { f->exception(e); },
                nullptr);
            f->token(token);
            *future = new shared_ptr<GetConnectionFuture>(move(f));
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_ObjectPrx_ice_getCachedConnection(void* self, void** r)
    {
        *r = 0;
        try
        {
            auto conn = restoreProxy(self)->ice_getCachedConnection();
            if (conn)
            {
                *r = new shared_ptr<Ice::Connection>(move(conn));
            }
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_ObjectPrx_ice_flushBatchRequests(void* self)
    {
        try
        {
            restoreProxy(self)->ice_flushBatchRequests();
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_ObjectPrx_ice_flushBatchRequestsAsync(void* self, void** future)
    {
        *future = 0;
        auto f = make_shared<SimpleFuture>();

        try
        {
            function<void()> token = restoreProxy(self)->ice_flushBatchRequestsAsync(
                [f](exception_ptr e) { f->exception(e); },
                [f](bool /*sentSynchronously*/) { f->done(); });
            f->token(token);
            *future = new shared_ptr<SimpleFuture>(move(f));
        }
        catch (...)
        {
            return convertException(std::current_exception());
        }
        return 0;
    }

    mxArray* Ice_ObjectPrx_clone(void* self, void** r)
    {
        *r = createProxy(restoreProxy(self));
        return 0;
    }

    mxArray* Ice_InvocationFuture_unref(void* self)
    {
        delete reinterpret_cast<shared_ptr<InvocationFuture>*>(self);
        return 0;
    }

    mxArray* Ice_InvocationFuture_id(void* self, unsigned long long* id)
    {
        *id = reinterpret_cast<unsigned long long>(self);
        return 0;
    }

    mxArray* Ice_InvocationFuture_wait(void* self)
    {
        bool b = deref<InvocationFuture>(self)->waitForState(Future::State::Finished, -1);
        return createResultValue(createBool(b));
    }

    mxArray* Ice_InvocationFuture_waitState(void* self, mxArray* st, double timeout)
    {
        try
        {
            string state = getStringFromUTF16(st);
            bool b = deref<InvocationFuture>(self)->waitForState(state, timeout);
            return createResultValue(createBool(b));
        }
        catch (...)
        {
            return createResultException(convertException(std::current_exception()));
        }
    }

    mxArray* Ice_InvocationFuture_results(void* self)
    {
        auto f = deref<InvocationFuture>(self);
        if (!f->waitForState(Future::State::Finished, -1))
        {
            assert(f->getException());
            delete reinterpret_cast<shared_ptr<InvocationFuture>*>(self);
            return createResultException(convertException(f->getException()));
        }

        bool ok;
        pair<const byte*, const byte*> p;
        f->getResults(ok, p);
        mxArray* params = 0;
        if (p.second > p.first)
        {
            params = createByteArray(p.first, p.second);
        }

        //
        // The C++ object won't be used after this.
        //
        delete reinterpret_cast<shared_ptr<InvocationFuture>*>(self);

        return createResultValue(createInvokeResultValue(createBool(ok), params));
    }

    mxArray* Ice_InvocationFuture_state(void* self)
    {
        return createResultValue(createStringFromUTF8(deref<InvocationFuture>(self)->state()));
    }

    mxArray* Ice_InvocationFuture_cancel(void* self)
    {
        deref<InvocationFuture>(self)->cancel();
        return 0;
    }

    mxArray* Ice_InvocationFuture_check(void* self)
    {
        auto f = deref<InvocationFuture>(self);
        if (!f->waitForState(Future::State::Finished, -1))
        {
            assert(f->getException());
            //
            // The C++ object won't be used after this.
            //
            delete reinterpret_cast<shared_ptr<InvocationFuture>*>(self);
            return convertException(f->getException());
        }

        //
        // The C++ object won't be used after this.
        //
        delete reinterpret_cast<shared_ptr<InvocationFuture>*>(self);

        return 0;
    }

    mxArray* Ice_GetConnectionFuture_unref(void* self)
    {
        delete reinterpret_cast<shared_ptr<GetConnectionFuture>*>(self);
        return 0;
    }

    mxArray* Ice_GetConnectionFuture_id(void* self, unsigned long long* id)
    {
        *id = reinterpret_cast<unsigned long long>(self);
        return 0;
    }

    mxArray* Ice_GetConnectionFuture_wait(void* self)
    {
        bool b = deref<GetConnectionFuture>(self)->waitForState(Future::State::Finished, -1);
        return createResultValue(createBool(b));
    }

    mxArray* Ice_GetConnectionFuture_waitState(void* self, mxArray* st, double timeout)
    {
        try
        {
            string state = getStringFromUTF16(st);
            bool b = deref<GetConnectionFuture>(self)->waitForState(state, timeout);
            return createResultValue(createBool(b));
        }
        catch (...)
        {
            return createResultException(convertException(std::current_exception()));
        }
    }

    mxArray* Ice_GetConnectionFuture_fetch(void* self, void** con)
    {
        auto f = deref<GetConnectionFuture>(self);
        if (!f->waitForState(Future::State::Finished, -1))
        {
            assert(f->getException());
            // The C++ object won't be used after this.
            delete reinterpret_cast<shared_ptr<GetConnectionFuture>*>(self);
            return convertException(f->getException());
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

    mxArray* Ice_GetConnectionFuture_state(void* self)
    {
        return createResultValue(createStringFromUTF8(deref<GetConnectionFuture>(self)->state()));
    }

    mxArray* Ice_GetConnectionFuture_cancel(void* self)
    {
        deref<GetConnectionFuture>(self)->cancel();
        return 0;
    }
}
