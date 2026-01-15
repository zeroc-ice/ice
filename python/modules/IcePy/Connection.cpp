// Copyright (c) ZeroC, Inc.

#include "Connection.h"
#include "Communicator.h"
#include "ConnectionInfo.h"
#include "Endpoint.h"
#include "Future.h"
#include "ObjectAdapter.h"
#include "Operation.h"
#include "Proxy.h"
#include "Thread.h"
#include "Types.h"
#include "Util.h"

using namespace std;
using namespace IcePy;

#if defined(__GNUC__) && ((__GNUC__ >= 8))
#    pragma GCC diagnostic ignored "-Wcast-function-type"
#endif

namespace
{
    // P = sizeof(void*), L = sizeof(long)
    template<int P, int L> struct Hasher;

#ifndef _WIN64
    //
    // COMPILERFIX: With Windows 64 the templates bellow will produce trucation warnings,
    // we ifdef them out as they are never used with Windows 64.
    //
    template<> struct Hasher<4, 4>
    {
        long operator()(void* ptr) const { return reinterpret_cast<long>(ptr); }
    };

    template<> struct Hasher<8, 8>
    {
        long operator()(void* ptr) const { return reinterpret_cast<long>(ptr); }
    };
#endif

    template<> struct Hasher<8, 4>
    {
        long operator()(void* ptr) const
        {
            auto v = reinterpret_cast<intptr_t>(ptr);

            // Eliminate lower 4 bits as objecs are usually aligned on 16 bytes boundaries,
            // then eliminate upper bits
            return (v >> 4) & 0xFFFFFFFF;
        }
    };

    long hashPointer(void* ptr) { return Hasher<sizeof(void*), sizeof(long)>()(ptr); }

    constexpr const char* connectionAbort_doc = R"(abort() -> None

Aborts this connection.)";

    constexpr const char* connectionClose_doc = R"(close() -> Awaitable[None]

Starts a graceful closure of this connection once all outstanding invocations have completed.

Returns
-------
Awaitable[None]
    A future that becomes available when the connection is closed.)";

    constexpr const char* connectionCreateProxy_doc = R"(createProxy(identity: Ice.Identity) -> Ice.ObjectPrx

Creates a special proxy (a 'fixed proxy') that always uses this connection.

Parameters
----------
identity : Ice.Identity
    The identity of the target object.

Returns
-------
Ice.ObjectPrx
    A fixed proxy with the provided identity.)";

    constexpr const char* connectionDisableInactivityCheck_doc = R"(disableInactivityCheck() -> None

Disables the inactivity check on this connection.

By default, Ice will close connections that remain inactive for a certain period.
This function disables that behavior for this connection.)";

    constexpr const char* connectionSetAdapter_doc = R"(setAdapter(adapter: Ice.ObjectAdapter | None) -> None

Associates an object adapter with this connection.

When a connection receives a request, it dispatches this request using its associated object adapter.
If the associated object adapter is ``None``, the connection rejects any incoming request with an
:class:`ObjectNotExistException`.

The default object adapter of an incoming connection is the object adapter that created this connection;
the default object adapter of an outgoing connection is the communicator's default object adapter.

Parameters
----------
adapter : Ice.ObjectAdapter | None
    The object adapter to associate with this connection.)";

    constexpr const char* connectionGetAdapter_doc = R"(getAdapter() -> Ice.ObjectAdapter | None

Gets the object adapter associated with this connection.

Returns
-------
Ice.ObjectAdapter | None
    The object adapter associated with this connection.)";

    constexpr const char* connectionFlushBatchRequests_doc = R"(flushBatchRequests(compress: Ice.CompressBatch) -> None

Flushes any pending batch requests for this connection.

This corresponds to all batch requests invoked on fixed proxies associated with the connection.

Parameters
----------
compress : Ice.CompressBatch
    Specifies whether or not the queued batch requests should be compressed before being sent over the wire.)";

    constexpr const char* connectionFlushBatchRequestsAsync_doc =
        R"(flushBatchRequestsAsync(compress: Ice.CompressBatch) -> Awaitable[None]

Flushes any pending batch requests for this connection asynchronously.

This corresponds to all batch requests invoked on fixed proxies associated with the connection.

Parameters
----------
compress : Ice.CompressBatch
    Specifies whether or not the queued batch requests should be compressed before being sent over the wire.

Returns
-------
Awaitable[None]
    A future that becomes available when the flush completes.)";

    constexpr const char* connectionSetCloseCallback_doc =
        R"(setCloseCallback(callback: Callable[[Connection], None]) -> None

Sets a close callback on the connection. The callback is called by the connection when it's closed.
The callback is called from the Ice thread pool associated with the connection.

Parameters
----------
callback : Callable[[Connection], None]
    The close callback callable.)";

    constexpr const char* connectionType_doc = R"(type() -> str

Returns the connection type. This corresponds to the endpoint type, such as 'tcp', 'udp', etc.

Returns
-------
str
    The type of the connection.)";

    constexpr const char* connectionToString_doc = R"(toString() -> str

Returns a description of the connection as human readable text, suitable for logging or error messages.

Notes
-----
This function remains usable after the connection is closed or aborted.

Returns
-------
str
    The description of the connection as human readable text.)";

    constexpr const char* connectionGetInfo_doc = R"(getInfo() -> Ice.ConnectionInfo

Returns the connection information.

Returns
-------
Ice.ConnectionInfo
    The connection information.)";

    constexpr const char* connectionGetEndpoint_doc = R"(getEndpoint() -> Endpoint

Gets the endpoint from which the connection was created.

Returns
-------
Ice.Endpoint
    The endpoint from which the connection was created.)";

    constexpr const char* connectionSetBufferSize_doc = R"(setBufferSize(rcvSize: int, sndSize: int) -> None

Sets the size of the receive and send buffers.

Parameters
----------
rcvSize : int
    The size of the receive buffer.
sndSize : int
    The size of the send buffer.)";

    constexpr const char* connectionThrowException_doc = R"(throwException() -> None

Raises an exception that provides the reason for the closure of this connection. For example,
this function raises :class:`CloseConnectionException` when the connection was closed gracefully by the peer;
it raises :class:`ConnectionAbortedException` when the connection is aborted with :func:`abort`.
This function does nothing if the connection is not yet closed.)";
}

namespace IcePy
{
    extern PyTypeObject ConnectionType;

    struct ConnectionObject
    {
        PyObject_HEAD Ice::ConnectionPtr* connection;
        Ice::CommunicatorPtr* communicator;
    };

    class CloseCallbackWrapper final
    {
    public:
        CloseCallbackWrapper(PyObject* cb, PyObject* con) : _cb(Py_NewRef(cb)), _con(Py_NewRef(con)) {}

        ~CloseCallbackWrapper()
        {
            AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

            Py_DECREF(_cb);
            Py_DECREF(_con);
        }

        void closed()
        {
            AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

            PyObjectHandle args{Py_BuildValue("(O)", _con)};
            assert(_cb);
            PyObjectHandle tmp{PyObject_Call(_cb, args.get(), nullptr)};
            if (PyErr_Occurred())
            {
                PyException ex; // Retrieve it before another Python API call clears it.

                // A callback that calls sys.exit() will raise the SystemExit exception.
                //
                // This is normally caught by the interpreter, causing it to exit. However, we have no way to pass this
                // exception to the interpreter, so we act on it directly.
                ex.checkSystemExit();

                ex.raise();
            }
        }

    private:
        PyObject* _cb;
        PyObject* _con;
    };
    using CloseCallbackWrapperPtr = shared_ptr<CloseCallbackWrapper>;
}

extern "C" ConnectionObject*
connectionNew(PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwds*/)
{
    assert(type && type->tp_alloc);
    auto* self = reinterpret_cast<ConnectionObject*>(type->tp_alloc(type, 0));
    if (!self)
    {
        return nullptr;
    }
    self->connection = nullptr;
    self->communicator = nullptr;
    return self;
}

extern "C" void
connectionDealloc(ConnectionObject* self)
{
    delete self->connection;
    delete self->communicator;
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

extern "C" PyObject*
connectionCompare(ConnectionObject* c1, PyObject* other, int op)
{
    bool result = false;

    if (PyObject_TypeCheck(other, &ConnectionType))
    {
        auto* c2 = reinterpret_cast<ConnectionObject*>(other);

        switch (op)
        {
            case Py_EQ:
                result = *c1->connection == *c2->connection;
                break;
            case Py_NE:
                result = *c1->connection != *c2->connection;
                break;
            case Py_LE:
                result = *c1->connection <= *c2->connection;
                break;
            case Py_GE:
                result = *c1->connection >= *c2->connection;
                break;
            case Py_LT:
                result = *c1->connection < *c2->connection;
                break;
            case Py_GT:
                result = *c1->connection > *c2->connection;
                break;
        }
    }
    else
    {
        if (op == Py_EQ)
        {
            result = false;
        }
        else if (op == Py_NE)
        {
            result = true;
        }
        else
        {
            PyErr_Format(PyExc_TypeError, "can't compare %s to %s", Py_TYPE(c1)->tp_name, Py_TYPE(other)->tp_name);
            return nullptr;
        }
    }

    return result ? Py_True : Py_False;
}

extern "C" long
connectionHash(ConnectionObject* self)
{
    return hashPointer((*self->connection).get());
}

extern "C" PyObject*
connectionAbort(ConnectionObject* self, PyObject* /* args */)
{
    assert(self->connection);
    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during blocking invocations.
        (*self->connection)->abort();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return Py_None;
}

extern "C" PyObject*
connectionClose(ConnectionObject* self, PyObject* /* args */)
{
    assert(self->connection);

    auto type = reinterpret_cast<PyTypeObject*>(lookupType("Ice.Future"));
    assert(type);

    PyObjectHandle emptyArgs{PyTuple_New(0)};
    PyObjectHandle future{type->tp_new(type, emptyArgs.get(), nullptr)};
    if (!future.get())
    {
        return nullptr;
    }

    // Call Ice.Future.__init__
    type->tp_init(future.get(), emptyArgs.get(), nullptr);

    // Create a new reference to prevent premature release of the future object. The reference will be released by
    // either the success or exception callback, depending on the outcome of the close operation.
    PyObject* futureObject = Py_NewRef(future.get());

    try
    {
        (*self->connection)
            ->close(
                [futureObject]()
                {
                    // Ensure the current thread is able to call into Python.
                    AdoptThread adoptThread;
                    // Adopt the future object so it is released within this scope.
                    PyObjectHandle futureGuard{futureObject};
                    PyObjectHandle discard{callMethod(futureGuard.get(), "set_result", Py_None)};
                },
                [futureObject](exception_ptr ex)
                {
                    // Ensure the current thread is able to call into Python.
                    AdoptThread adoptThread;
                    // Adopt the future object so it is released within this scope.
                    PyObjectHandle futureGuard{futureObject};
                    PyObjectHandle pythonException{convertException(ex)};
                    PyObjectHandle discard{callMethod(futureGuard.get(), "set_exception", pythonException.get())};
                });
    }
    catch (...)
    {
        // Ensure the future is released if an exception is thrown synchronously, since neither the success nor
        // exception callback will be invoked.
        PyObjectHandle futureGuard{futureObject};
        setPythonException(current_exception());
        return nullptr;
    }

    return IcePy::wrapFuture(*self->communicator, future.get());
}

extern "C" PyObject*
connectionCreateProxy(ConnectionObject* self, PyObject* args)
{
    PyObject* identityType = lookupType("Ice.Identity");
    PyObject* id;
    if (!PyArg_ParseTuple(args, "O!", identityType, &id))
    {
        return nullptr;
    }

    Ice::Identity ident;
    if (!getIdentity(id, ident))
    {
        return nullptr;
    }

    assert(self->connection);
    assert(self->communicator);
    optional<Ice::ObjectPrx> proxy;
    try
    {
        proxy = (*self->connection)->createProxy(ident);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createProxy(proxy.value(), (*self->communicator));
}

extern "C" PyObject*
connectionSetAdapter(ConnectionObject* self, PyObject* args)
{
    PyObject* adapter = Py_None;
    if (!PyArg_ParseTuple(args, "O", &adapter))
    {
        return nullptr;
    }

    PyObject* adapterType = lookupType("Ice.ObjectAdapter");
    if (adapter != Py_None && !PyObject_IsInstance(adapter, adapterType))
    {
        PyErr_Format(PyExc_TypeError, "value for 'adapter' argument must be None or an Ice.ObjectAdapter instance");
        return nullptr;
    }

    Ice::ObjectAdapterPtr oa = adapter != Py_None ? unwrapObjectAdapter(adapter) : nullptr;

    assert(self->connection);
    assert(self->communicator);
    try
    {
        (*self->connection)->setAdapter(oa);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return Py_None;
}

extern "C" PyObject*
connectionGetAdapter(ConnectionObject* self, PyObject* /*args*/)
{
    Ice::ObjectAdapterPtr adapter;

    assert(self->connection);
    assert(self->communicator);
    try
    {
        adapter = (*self->connection)->getAdapter();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    if (adapter)
    {
        return wrapObjectAdapter(adapter);
    }
    else
    {
        return Py_None;
    }
}

extern "C" PyObject*
connectionFlushBatchRequests(ConnectionObject* self, PyObject* args)
{
    PyObject* compressBatchType = lookupType("Ice.CompressBatch");
    PyObject* compressBatch;
    if (!PyArg_ParseTuple(args, "O!", compressBatchType, &compressBatch))
    {
        return nullptr;
    }

    PyObjectHandle v{getAttr(compressBatch, "value", true)};
    assert(v.get());
    auto cb = static_cast<Ice::CompressBatch>(PyLong_AsLong(v.get()));

    assert(self->connection);
    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.
        (*self->connection)->flushBatchRequests(cb);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return Py_None;
}

extern "C" PyObject*
connectionFlushBatchRequestsAsync(ConnectionObject* self, PyObject* args)
{
    PyObject* compressBatchType = lookupType("Ice.CompressBatch");
    PyObject* compressBatch;
    if (!PyArg_ParseTuple(args, "O!", compressBatchType, &compressBatch))
    {
        return nullptr;
    }

    PyObjectHandle v{getAttr(compressBatch, "value", true)};
    assert(v.get());
    auto compress = static_cast<Ice::CompressBatch>(PyLong_AsLong(v.get()));

    assert(self->connection);
    const string op = "flushBatchRequests";

    auto callback = make_shared<FlushAsyncCallback>(op);
    function<void()> cancel;
    try
    {
        cancel = (*self->connection)
                     ->flushBatchRequestsAsync(
                         compress,
                         [callback](exception_ptr ex) { callback->exception(ex); },
                         [callback](bool sentSynchronously) { callback->sent(sentSynchronously); });
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    PyObjectHandle asyncInvocationContextObj{createAsyncInvocationContext(std::move(cancel), *self->communicator)};
    if (!asyncInvocationContextObj.get())
    {
        return nullptr;
    }

    PyObjectHandle future{createFuture(asyncInvocationContextObj.get())};
    if (!future.get())
    {
        return nullptr;
    }
    callback->setFuture(future.get());
    return IcePy::wrapFuture(*self->communicator, future.get());
}

extern "C" PyObject*
connectionSetCloseCallback(ConnectionObject* self, PyObject* args)
{
    assert(self->connection);

    PyObject* cb;
    if (!PyArg_ParseTuple(args, "O", &cb))
    {
        return nullptr;
    }

    PyObject* callbackType = lookupType("types.FunctionType");
    if (cb != Py_None && !PyObject_IsInstance(cb, callbackType))
    {
        PyErr_Format(PyExc_ValueError, "callback must be None or a function");
        return nullptr;
    }

    CloseCallbackWrapperPtr wrapper;
    if (cb != Py_None)
    {
        wrapper = make_shared<CloseCallbackWrapper>(cb, reinterpret_cast<PyObject*>(self));
    }

    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during blocking invocations.
        if (wrapper)
        {
            (*self->connection)->setCloseCallback([wrapper](const Ice::ConnectionPtr&) { wrapper->closed(); });
        }
        else
        {
            (*self->connection)->setCloseCallback(nullptr);
        }
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return Py_None;
}

extern "C" PyObject*
connectionDisableInactivityCheck(ConnectionObject* self, PyObject* /* args */)
{
    assert(self->connection);
    (*self->connection)->disableInactivityCheck();
    return Py_None;
}

extern "C" PyObject*
connectionType(ConnectionObject* self, PyObject* /*args*/)
{
    assert(self->connection);
    string type;
    try
    {
        type = (*self->connection)->type();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createString(type);
}

extern "C" PyObject*
connectionToString(ConnectionObject* self, PyObject* /*args*/)
{
    assert(self->connection);
    string str;
    try
    {
        str = (*self->connection)->toString();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createString(str);
}

extern "C" PyObject*
connectionGetInfo(ConnectionObject* self, PyObject* /*args*/)
{
    assert(self->connection);
    try
    {
        Ice::ConnectionInfoPtr info = (*self->connection)->getInfo();
        return createConnectionInfo(info);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }
}

extern "C" PyObject*
connectionGetEndpoint(ConnectionObject* self, PyObject* /*args*/)
{
    assert(self->connection);
    try
    {
        Ice::EndpointPtr endpoint = (*self->connection)->getEndpoint();
        return createEndpoint(endpoint);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }
}

extern "C" PyObject*
connectionSetBufferSize(ConnectionObject* self, PyObject* args)
{
    int rcvSize;
    int sndSize;
    if (!PyArg_ParseTuple(args, "ii", &rcvSize, &sndSize))
    {
        return nullptr;
    }

    assert(self->connection);
    try
    {
        (*self->connection)->setBufferSize(rcvSize, sndSize);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return Py_None;
}

extern "C" PyObject*
connectionThrowException(ConnectionObject* self, PyObject* /*args*/)
{
    assert(self->connection);
    try
    {
        (*self->connection)->throwException();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return Py_None;
}

static PyMethodDef ConnectionMethods[] = {
    {"abort", reinterpret_cast<PyCFunction>(connectionAbort), METH_NOARGS, PyDoc_STR(connectionAbort_doc)},
    {"close", reinterpret_cast<PyCFunction>(connectionClose), METH_NOARGS, PyDoc_STR(connectionClose_doc)},
    {"createProxy",
     reinterpret_cast<PyCFunction>(connectionCreateProxy),
     METH_VARARGS,
     PyDoc_STR(connectionCreateProxy_doc)},
    {"disableInactivityCheck",
     reinterpret_cast<PyCFunction>(connectionDisableInactivityCheck),
     METH_NOARGS,
     PyDoc_STR(connectionDisableInactivityCheck_doc)},
    {"setAdapter",
     reinterpret_cast<PyCFunction>(connectionSetAdapter),
     METH_VARARGS,
     PyDoc_STR(connectionSetAdapter_doc)},
    {"getAdapter",
     reinterpret_cast<PyCFunction>(connectionGetAdapter),
     METH_NOARGS,
     PyDoc_STR(connectionGetAdapter_doc)},
    {"flushBatchRequests",
     reinterpret_cast<PyCFunction>(connectionFlushBatchRequests),
     METH_VARARGS,
     PyDoc_STR(connectionFlushBatchRequests_doc)},
    {"flushBatchRequestsAsync",
     reinterpret_cast<PyCFunction>(connectionFlushBatchRequestsAsync),
     METH_VARARGS,
     PyDoc_STR(connectionFlushBatchRequestsAsync_doc)},
    {"setCloseCallback",
     reinterpret_cast<PyCFunction>(connectionSetCloseCallback),
     METH_VARARGS,
     PyDoc_STR(connectionSetCloseCallback_doc)},
    {"type", reinterpret_cast<PyCFunction>(connectionType), METH_NOARGS, PyDoc_STR(connectionType_doc)},
    {"toString", reinterpret_cast<PyCFunction>(connectionToString), METH_NOARGS, PyDoc_STR(connectionToString_doc)},
    {"getInfo", reinterpret_cast<PyCFunction>(connectionGetInfo), METH_NOARGS, PyDoc_STR(connectionGetInfo_doc)},
    {"getEndpoint",
     reinterpret_cast<PyCFunction>(connectionGetEndpoint),
     METH_NOARGS,
     PyDoc_STR(connectionGetEndpoint_doc)},
    {"setBufferSize",
     reinterpret_cast<PyCFunction>(connectionSetBufferSize),
     METH_VARARGS,
     PyDoc_STR(connectionSetBufferSize_doc)},
    {"throwException",
     reinterpret_cast<PyCFunction>(connectionThrowException),
     METH_NOARGS,
     PyDoc_STR(connectionThrowException_doc)},
    {} /* sentinel */
};

namespace IcePy
{
    // clang-format off
    PyTypeObject ConnectionType = {
        .ob_base = PyVarObject_HEAD_INIT(nullptr, 0)
        .tp_name = "IcePy.Connection",
        .tp_basicsize = sizeof(ConnectionObject),
        .tp_dealloc = reinterpret_cast<destructor>(connectionDealloc),
        .tp_hash = reinterpret_cast<hashfunc>(connectionHash),
        .tp_flags = Py_TPFLAGS_DEFAULT,
        .tp_doc = PyDoc_STR("Represents a connection that uses the Ice protocol."),
        .tp_richcompare = reinterpret_cast<richcmpfunc>(connectionCompare),
        .tp_methods = ConnectionMethods,
        .tp_new = reinterpret_cast<newfunc>(connectionNew),
    };
    // clang-format on
}

bool
IcePy::initConnection(PyObject* module)
{
    if (PyType_Ready(&ConnectionType) < 0)
    {
        return false;
    }
    PyTypeObject* type = &ConnectionType; // Necessary to prevent GCC's strict-alias warnings.
    if (PyModule_AddObject(module, "Connection", reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    return true;
}

PyObject*
IcePy::createConnection(const Ice::ConnectionPtr& connection, const Ice::CommunicatorPtr& communicator)
{
    ConnectionObject* obj = connectionNew(&ConnectionType, nullptr, nullptr);
    if (obj)
    {
        obj->connection = new Ice::ConnectionPtr(connection);
        obj->communicator = new Ice::CommunicatorPtr(communicator);
    }
    return reinterpret_cast<PyObject*>(obj);
}

bool
IcePy::checkConnection(PyObject* p)
{
    PyTypeObject* type = &ConnectionType; // Necessary to prevent GCC's strict-alias warnings.
    return PyObject_IsInstance(p, reinterpret_cast<PyObject*>(type)) == 1;
}

bool
IcePy::getConnectionArg(PyObject* p, const string& func, const string& arg, Ice::ConnectionPtr& con)
{
    if (p == Py_None)
    {
        con = nullptr;
        return true;
    }
    else if (!checkConnection(p))
    {
        PyErr_Format(
            PyExc_ValueError,
            "%s expects an Ice.Connection object or None for argument '%s'",
            func.c_str(),
            arg.c_str());
        return false;
    }
    else
    {
        auto* obj = reinterpret_cast<ConnectionObject*>(p);
        con = *obj->connection;
        return true;
    }
}
