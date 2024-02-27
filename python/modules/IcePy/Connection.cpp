//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Connection.h>
#include <Communicator.h>
#include <ConnectionInfo.h>
#include <Endpoint.h>
#include <ObjectAdapter.h>
#include <Operation.h>
#include <Proxy.h>
#include <Thread.h>
#include <Types.h>
#include <Util.h>

using namespace std;
using namespace IcePy;

#if defined(__GNUC__) && ((__GNUC__ >= 8))
#    pragma GCC diagnostic ignored "-Wcast-function-type"
#endif

namespace
{

    // P = sizeof(void*), L = sizeof(long)
    template <int P, int L> struct Hasher;

#ifndef _WIN64
    //
    // COMPILERFIX: With Windows 64 the templates bellow will produce trucation warnings,
    // we ifdef them out as they are never used with Windows 64.
    //
    template <> struct Hasher<4, 4>
    {
        long operator()(void* ptr) const { return reinterpret_cast<long>(ptr); }
    };

    template <> struct Hasher<8, 8>
    {
        long operator()(void* ptr) const { return reinterpret_cast<long>(ptr); }
    };
#endif

    template <> struct Hasher<8, 4>
    {
        long operator()(void* ptr) const
        {
            intptr_t v = reinterpret_cast<intptr_t>(ptr);

            // Eliminate lower 4 bits as objecs are usually aligned on 16 bytes boundaries,
            // then eliminate upper bits
            return (v >> 4) & 0xFFFFFFFF;
        }
    };

    long hashPointer(void* ptr) { return Hasher<sizeof(void*), sizeof(long)>()(ptr); }

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
        CloseCallbackWrapper(PyObject* cb, PyObject* con) : _cb(cb), _con(con)
        {
            Py_INCREF(cb);
            Py_INCREF(con);
        }

        ~CloseCallbackWrapper()
        {
            AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

            Py_DECREF(_cb);
            Py_DECREF(_con);
        }

        void closed()
        {
            AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

            PyObjectHandle args = Py_BuildValue(STRCAST("(O)"), _con);
            assert(_cb);
            PyObjectHandle tmp = PyObject_Call(_cb, args.get(), 0);
            if (PyErr_Occurred())
            {
                PyException ex; // Retrieve it before another Python API call clears it.

                //
                // A callback that calls sys.exit() will raise the SystemExit exception.
                // This is normally caught by the interpreter, causing it to exit.
                // However, we have no way to pass this exception to the interpreter,
                // so we act on it directly.
                //
                ex.checkSystemExit();

                ex.raise();
            }
        }

    private:
        PyObject* _cb;
        PyObject* _con;
    };
    using CloseCallbackWrapperPtr = shared_ptr<CloseCallbackWrapper>;

    class HeartbeatCallbackWrapper final
    {
    public:
        HeartbeatCallbackWrapper(PyObject* cb, PyObject* con) : _cb(cb), _con(con)
        {
            Py_INCREF(cb);
            Py_INCREF(con);
        }

        ~HeartbeatCallbackWrapper()
        {
            AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

            Py_DECREF(_cb);
            Py_DECREF(_con);
        }

        void heartbeat()
        {
            AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

            PyObjectHandle args = Py_BuildValue(STRCAST("(O)"), _con);
            assert(_cb);
            PyObjectHandle tmp = PyObject_Call(_cb, args.get(), 0);
            if (PyErr_Occurred())
            {
                PyException ex; // Retrieve it before another Python API call clears it.

                //
                // A callback that calls sys.exit() will raise the SystemExit exception.
                // This is normally caught by the interpreter, causing it to exit.
                // However, we have no way to pass this exception to the interpreter,
                // so we act on it directly.
                //
                ex.checkSystemExit();

                ex.raise();
            }
        }

    private:
        PyObject* _cb;
        PyObject* _con;
    };
    using HeartbeatCallbackWrapperPtr = shared_ptr<HeartbeatCallbackWrapper>;

}

#ifdef WIN32
extern "C"
#endif
    static ConnectionObject*
    connectionNew(PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwds*/)
{
    assert(type && type->tp_alloc);
    ConnectionObject* self = reinterpret_cast<ConnectionObject*>(type->tp_alloc(type, 0));
    if (!self)
    {
        return 0;
    }
    self->connection = 0;
    self->communicator = 0;
    return self;
}

#ifdef WIN32
extern "C"
#endif
    static void
    connectionDealloc(ConnectionObject* self)
{
    delete self->connection;
    delete self->communicator;
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

#ifdef WIN32
extern "C"
#endif
    static PyObject*
    connectionCompare(ConnectionObject* c1, PyObject* other, int op)
{
    bool result = false;

    if (PyObject_TypeCheck(other, &ConnectionType))
    {
        ConnectionObject* c2 = reinterpret_cast<ConnectionObject*>(other);

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
            return 0;
        }
    }

    return result ? incTrue() : incFalse();
}

#ifdef WIN32
extern "C"
#endif
    static long
    connectionHash(ConnectionObject* self)
{
    return hashPointer((*self->connection).get());
}

#ifdef WIN32
extern "C"
#endif
    static PyObject*
    connectionClose(ConnectionObject* self, PyObject* args)
{
    PyObject* closeType = lookupType("Ice.ConnectionClose");
    PyObject* mode;
    if (!PyArg_ParseTuple(args, STRCAST("O!"), closeType, &mode))
    {
        return 0;
    }

    PyObjectHandle v = getAttr(mode, "_value", true);
    assert(v.get());
    Ice::ConnectionClose cc = static_cast<Ice::ConnectionClose>(PyLong_AsLong(v.get()));

    assert(self->connection);
    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during blocking invocations.
        (*self->connection)->close(cc);
    }
    catch (const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

#ifdef WIN32
extern "C"
#endif
    static PyObject*
    connectionCreateProxy(ConnectionObject* self, PyObject* args)
{
    PyObject* identityType = lookupType("Ice.Identity");
    PyObject* id;
    if (!PyArg_ParseTuple(args, STRCAST("O!"), identityType, &id))
    {
        return 0;
    }

    Ice::Identity ident;
    if (!getIdentity(id, ident))
    {
        return 0;
    }

    assert(self->connection);
    assert(self->communicator);
    optional<Ice::ObjectPrx> proxy;
    try
    {
        proxy = (*self->connection)->createProxy(ident);
    }
    catch (const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(proxy.value(), (*self->communicator));
}

#ifdef WIN32
extern "C"
#endif
    static PyObject*
    connectionSetAdapter(ConnectionObject* self, PyObject* args)
{
    PyObject* adapter = Py_None;
    if (!PyArg_ParseTuple(args, STRCAST("O"), &adapter))
    {
        return 0;
    }

    PyObject* adapterType = lookupType("Ice.ObjectAdapter");
    if (adapter != Py_None && !PyObject_IsInstance(adapter, adapterType))
    {
        PyErr_Format(PyExc_TypeError, "value for 'adapter' argument must be None or an Ice.ObjectAdapter instance");
        return 0;
    }

    Ice::ObjectAdapterPtr oa = adapter != Py_None ? unwrapObjectAdapter(adapter) : Ice::ObjectAdapterPtr();

    assert(self->connection);
    assert(self->communicator);
    try
    {
        (*self->connection)->setAdapter(oa);
    }
    catch (const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

#ifdef WIN32
extern "C"
#endif
    static PyObject*
    connectionGetAdapter(ConnectionObject* self, PyObject* /*args*/)
{
    Ice::ObjectAdapterPtr adapter;

    assert(self->connection);
    assert(self->communicator);
    try
    {
        adapter = (*self->connection)->getAdapter();
    }
    catch (const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    if (adapter)
    {
        return wrapObjectAdapter(adapter);
    }
    else
    {
        Py_INCREF(Py_None);
        return Py_None;
    }
}

#ifdef WIN32
extern "C"
#endif
    static PyObject*
    connectionFlushBatchRequests(ConnectionObject* self, PyObject* args)
{
    PyObject* compressBatchType = lookupType("Ice.CompressBatch");
    PyObject* compressBatch;
    if (!PyArg_ParseTuple(args, STRCAST("O!"), compressBatchType, &compressBatch))
    {
        return 0;
    }

    PyObjectHandle v = getAttr(compressBatch, "_value", true);
    assert(v.get());
    Ice::CompressBatch cb = static_cast<Ice::CompressBatch>(PyLong_AsLong(v.get()));

    assert(self->connection);
    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.
        (*self->connection)->flushBatchRequests(cb);
    }
    catch (const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

#ifdef WIN32
extern "C"
#endif
    static PyObject*
    connectionFlushBatchRequestsAsync(ConnectionObject* self, PyObject* args)
{
    PyObject* compressBatchType = lookupType("Ice.CompressBatch");
    PyObject* compressBatch;
    if (!PyArg_ParseTuple(args, STRCAST("O!"), compressBatchType, &compressBatch))
    {
        return 0;
    }

    PyObjectHandle v = getAttr(compressBatch, "_value", true);
    assert(v.get());
    Ice::CompressBatch compress = static_cast<Ice::CompressBatch>(PyLong_AsLong(v.get()));

    assert(self->connection);
    const string op = "flushBatchRequests";

    auto callback = make_shared<FlushAsyncCallback>(op);
    function<void()> cancel;
    try
    {
        cancel = (*self->connection)
                     ->flushBatchRequestsAsync(
                         compress,
                         [callback](exception_ptr exptr)
                         {
                             try
                             {
                                 rethrow_exception(exptr);
                             }
                             catch (const Ice::Exception& ex)
                             {
                                 callback->exception(ex);
                             }
                             catch (...)
                             {
                                 assert(false);
                             }
                         },
                         [callback](bool sentSynchronously) { callback->sent(sentSynchronously); });
    }
    catch (const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    PyObjectHandle asyncInvocationContextObj = createAsyncInvocationContext(std::move(cancel), *self->communicator);
    if (!asyncInvocationContextObj.get())
    {
        return 0;
    }

    PyObjectHandle future = createFuture(op, asyncInvocationContextObj.get());
    if (!future.get())
    {
        return 0;
    }
    callback->setFuture(future.get());
    return future.release();
}

#ifdef WIN32
extern "C"
#endif
    static PyObject*
    connectionSetCloseCallback(ConnectionObject* self, PyObject* args)
{
    assert(self->connection);

    PyObject* cb;
    if (!PyArg_ParseTuple(args, STRCAST("O"), &cb))
    {
        return 0;
    }

    PyObject* callbackType = lookupType("types.FunctionType");
    if (cb != Py_None && !PyObject_IsInstance(cb, callbackType))
    {
        PyErr_Format(PyExc_ValueError, STRCAST("callback must be None or a function"));
        return 0;
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
    catch (const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

#ifdef WIN32
extern "C"
#endif
    static PyObject*
    connectionSetHeartbeatCallback(ConnectionObject* self, PyObject* args)
{
    assert(self->connection);

    PyObject* cb;
    if (!PyArg_ParseTuple(args, STRCAST("O"), &cb))
    {
        return 0;
    }

    PyObject* callbackType = lookupType("types.FunctionType");
    if (cb != Py_None && !PyObject_IsInstance(cb, callbackType))
    {
        PyErr_Format(PyExc_ValueError, STRCAST("callback must be None or a function"));
        return 0;
    }

    HeartbeatCallbackWrapperPtr wrapper;
    if (cb != Py_None)
    {
        wrapper = make_shared<HeartbeatCallbackWrapper>(cb, reinterpret_cast<PyObject*>(self));
    }

    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during blocking invocations.
        if (wrapper)
        {
            (*self->connection)->setHeartbeatCallback([wrapper](const Ice::ConnectionPtr&) { wrapper->heartbeat(); });
        }
        else
        {
            (*self->connection)->setHeartbeatCallback(nullptr);
        }
    }
    catch (const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

#ifdef WIN32
extern "C"
#endif
    static PyObject*
    connectionHeartbeat(ConnectionObject* self, PyObject* /*args*/)
{
    assert(self->connection);
    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.
        (*self->connection)->heartbeat();
    }
    catch (const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

#ifdef WIN32
extern "C"
#endif
    static PyObject*
    connectionSetACM(ConnectionObject* self, PyObject* args)
{
    assert(self->connection);

    optional<int32_t> timeout;
    optional<Ice::ACMClose> close;
    optional<Ice::ACMHeartbeat> heartbeat;

    PyObject* acmCloseType = lookupType("Ice.ACMClose");
    PyObject* acmHeartbeatType = lookupType("Ice.ACMHeartbeat");
    PyObject* t;
    PyObject* c;
    PyObject* h;
    if (!PyArg_ParseTuple(args, STRCAST("OOO"), &t, &c, &h))
    {
        return 0;
    }

    if (t != Unset)
    {
        timeout = static_cast<int32_t>(PyLong_AsLong(t));
        if (PyErr_Occurred())
        {
            return 0;
        }
    }

    if (c != Unset)
    {
        if (PyObject_IsInstance(c, acmCloseType) == 0)
        {
            PyErr_Format(PyExc_TypeError, "value for 'close' argument must be Unset or an enumerator of Ice.ACMClose");
            return 0;
        }
        PyObjectHandle v = getAttr(c, "_value", true);
        assert(v.get());
        close = static_cast<Ice::ACMClose>(PyLong_AsLong(v.get()));
    }

    if (h != Unset)
    {
        if (PyObject_IsInstance(h, acmHeartbeatType) == 0)
        {
            PyErr_Format(PyExc_TypeError,
                         "value for 'heartbeat' argument must be Unset or an enumerator of Ice.ACMHeartbeat");
            return 0;
        }
        PyObjectHandle v = getAttr(h, "_value", true);
        assert(v.get());
        heartbeat = static_cast<Ice::ACMHeartbeat>(PyLong_AsLong(v.get()));
    }

    try
    {
        (*self->connection)->setACM(timeout, close, heartbeat);
    }
    catch (const invalid_argument& ex)
    {
        PyErr_Format(PyExc_RuntimeError, "%s", ex.what());
        return 0;
    }
    catch (const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

#ifdef WIN32
extern "C"
#endif
    static PyObject*
    connectionGetACM(ConnectionObject* self, PyObject* /*args*/)
{
    assert(self->connection);

    PyObject* acmType = lookupType("Ice.ACM");
    PyObject* acmCloseType = lookupType("Ice._t_ACMClose");
    PyObject* acmHeartbeatType = lookupType("Ice._t_ACMHeartbeat");
    Ice::ACM acm;

    try
    {
        acm = (*self->connection)->getACM();
    }
    catch (const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    PyObjectHandle r = StructInfo::instantiate(acmType);
    if (!r.get())
    {
        return 0;
    }

    PyObjectHandle timeout = PyLong_FromLong(acm.timeout);
    if (!timeout.get())
    {
        assert(PyErr_Occurred());
        return 0;
    }

    if (PyObject_SetAttrString(r.get(), STRCAST("timeout"), timeout.get()) < 0)
    {
        assert(PyErr_Occurred());
        return 0;
    }

    EnumInfoPtr acmCloseEnum = dynamic_pointer_cast<EnumInfo>(getType(acmCloseType));
    assert(acmCloseEnum);
    PyObjectHandle close = acmCloseEnum->enumeratorForValue(static_cast<int32_t>(acm.close));
    if (!close.get())
    {
        PyErr_Format(PyExc_ValueError, "unexpected value for 'close' member of Ice.ACM");
        return 0;
    }
    if (PyObject_SetAttrString(r.get(), STRCAST("close"), close.get()) < 0)
    {
        assert(PyErr_Occurred());
        return 0;
    }

    EnumInfoPtr acmHeartbeatEnum = dynamic_pointer_cast<EnumInfo>(getType(acmHeartbeatType));
    assert(acmHeartbeatEnum);
    PyObjectHandle heartbeat = acmHeartbeatEnum->enumeratorForValue(static_cast<int32_t>(acm.heartbeat));
    if (!heartbeat.get())
    {
        PyErr_Format(PyExc_ValueError, "unexpected value for 'heartbeat' member of Ice.ACM");
        return 0;
    }
    if (PyObject_SetAttrString(r.get(), STRCAST("heartbeat"), heartbeat.get()) < 0)
    {
        assert(PyErr_Occurred());
        return 0;
    }

    return r.release();
}

#ifdef WIN32
extern "C"
#endif
    static PyObject*
    connectionType(ConnectionObject* self, PyObject* /*args*/)
{
    assert(self->connection);
    string type;
    try
    {
        type = (*self->connection)->type();
    }
    catch (const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createString(type);
}

#ifdef WIN32
extern "C"
#endif
    static PyObject*
    connectionTimeout(ConnectionObject* self, PyObject* /*args*/)
{
    assert(self->connection);
    int timeout;
    try
    {
        timeout = (*self->connection)->timeout();
    }
    catch (const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return PyLong_FromLong(timeout);
}

#ifdef WIN32
extern "C"
#endif
    static PyObject*
    connectionToString(ConnectionObject* self, PyObject* /*args*/)
{
    assert(self->connection);
    string str;
    try
    {
        str = (*self->connection)->toString();
    }
    catch (const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createString(str);
}

#ifdef WIN32
extern "C"
#endif
    static PyObject*
    connectionGetInfo(ConnectionObject* self, PyObject* /*args*/)
{
    assert(self->connection);
    try
    {
        Ice::ConnectionInfoPtr info = (*self->connection)->getInfo();
        return createConnectionInfo(info);
    }
    catch (const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }
}

#ifdef WIN32
extern "C"
#endif
    static PyObject*
    connectionGetEndpoint(ConnectionObject* self, PyObject* /*args*/)
{
    assert(self->connection);
    try
    {
        Ice::EndpointPtr endpoint = (*self->connection)->getEndpoint();
        return createEndpoint(endpoint);
    }
    catch (const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }
}

#ifdef WIN32
extern "C"
#endif
    static PyObject*
    connectionSetBufferSize(ConnectionObject* self, PyObject* args)
{
    int rcvSize;
    int sndSize;
    if (!PyArg_ParseTuple(args, STRCAST("ii"), &rcvSize, &sndSize))
    {
        return 0;
    }

    assert(self->connection);
    try
    {
        (*self->connection)->setBufferSize(rcvSize, sndSize);
    }
    catch (const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

#ifdef WIN32
extern "C"
#endif
    static PyObject*
    connectionThrowException(ConnectionObject* self, PyObject* /*args*/)
{
    assert(self->connection);
    try
    {
        (*self->connection)->throwException();
    }
    catch (const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef ConnectionMethods[] = {
    {STRCAST("close"), reinterpret_cast<PyCFunction>(connectionClose), METH_VARARGS,
     PyDoc_STR(STRCAST("close(Ice.ConnectionClose) -> None"))},
    {STRCAST("createProxy"), reinterpret_cast<PyCFunction>(connectionCreateProxy), METH_VARARGS,
     PyDoc_STR(STRCAST("createProxy(Ice.Identity) -> Ice.ObjectPrx"))},
    {STRCAST("setAdapter"), reinterpret_cast<PyCFunction>(connectionSetAdapter), METH_VARARGS,
     PyDoc_STR(STRCAST("setAdapter(Ice.ObjectAdapter) -> None"))},
    {STRCAST("getAdapter"), reinterpret_cast<PyCFunction>(connectionGetAdapter), METH_NOARGS,
     PyDoc_STR(STRCAST("getAdapter() -> Ice.ObjectAdapter"))},
    {STRCAST("flushBatchRequests"), reinterpret_cast<PyCFunction>(connectionFlushBatchRequests), METH_VARARGS,
     PyDoc_STR(STRCAST("flushBatchRequests(Ice.CompressBatch) -> None"))},
    {STRCAST("flushBatchRequestsAsync"), reinterpret_cast<PyCFunction>(connectionFlushBatchRequestsAsync), METH_VARARGS,
     PyDoc_STR(STRCAST("flushBatchRequestsAsync(Ice.CompressBatch) -> Ice.Future"))},
    {STRCAST("setCloseCallback"), reinterpret_cast<PyCFunction>(connectionSetCloseCallback), METH_VARARGS,
     PyDoc_STR(STRCAST("setCloseCallback(Ice.CloseCallback) -> None"))},
    {STRCAST("setHeartbeatCallback"), reinterpret_cast<PyCFunction>(connectionSetHeartbeatCallback), METH_VARARGS,
     PyDoc_STR(STRCAST("setHeartbeatCallback(Ice.HeartbeatCallback) -> None"))},
    {STRCAST("heartbeat"), reinterpret_cast<PyCFunction>(connectionHeartbeat), METH_NOARGS,
     PyDoc_STR(STRCAST("heartbeat() -> None"))},
    {STRCAST("setACM"), reinterpret_cast<PyCFunction>(connectionSetACM), METH_VARARGS,
     PyDoc_STR(STRCAST("setACM(int, Ice.ACMClose, Ice.ACMHeartbeat) -> None"))},
    {STRCAST("getACM"), reinterpret_cast<PyCFunction>(connectionGetACM), METH_NOARGS,
     PyDoc_STR(STRCAST("getACM() -> Ice.ACM"))},
    {STRCAST("type"), reinterpret_cast<PyCFunction>(connectionType), METH_NOARGS,
     PyDoc_STR(STRCAST("type() -> string"))},
    {STRCAST("timeout"), reinterpret_cast<PyCFunction>(connectionTimeout), METH_NOARGS,
     PyDoc_STR(STRCAST("timeout() -> int"))},
    {STRCAST("toString"), reinterpret_cast<PyCFunction>(connectionToString), METH_NOARGS,
     PyDoc_STR(STRCAST("toString() -> string"))},
    {STRCAST("getInfo"), reinterpret_cast<PyCFunction>(connectionGetInfo), METH_NOARGS,
     PyDoc_STR(STRCAST("getInfo() -> Ice.ConnectionInfo"))},
    {STRCAST("getEndpoint"), reinterpret_cast<PyCFunction>(connectionGetEndpoint), METH_NOARGS,
     PyDoc_STR(STRCAST("getEndpoint() -> Ice.Endpoint"))},
    {STRCAST("setBufferSize"), reinterpret_cast<PyCFunction>(connectionSetBufferSize), METH_VARARGS,
     PyDoc_STR(STRCAST("setBufferSize(int, int) -> None"))},
    {STRCAST("throwException"), reinterpret_cast<PyCFunction>(connectionThrowException), METH_NOARGS,
     PyDoc_STR(STRCAST("throwException() -> None"))},
    {0, 0} /* sentinel */
};

namespace IcePy
{

    PyTypeObject ConnectionType = {
        /* The ob_type field must be initialized in the module init function
         * to be portable to Windows without using C++. */
        PyVarObject_HEAD_INIT(0, 0) STRCAST("IcePy.Connection"), /* tp_name */
        sizeof(ConnectionObject),                                /* tp_basicsize */
        0,                                                       /* tp_itemsize */
        /* methods */
        reinterpret_cast<destructor>(connectionDealloc),  /* tp_dealloc */
        0,                                                /* tp_print */
        0,                                                /* tp_getattr */
        0,                                                /* tp_setattr */
        0,                                                /* tp_reserved */
        0,                                                /* tp_repr */
        0,                                                /* tp_as_number */
        0,                                                /* tp_as_sequence */
        0,                                                /* tp_as_mapping */
        reinterpret_cast<hashfunc>(connectionHash),       /* tp_hash */
        0,                                                /* tp_call */
        0,                                                /* tp_str */
        0,                                                /* tp_getattro */
        0,                                                /* tp_setattro */
        0,                                                /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT,                               /* tp_flags */
        0,                                                /* tp_doc */
        0,                                                /* tp_traverse */
        0,                                                /* tp_clear */
        reinterpret_cast<richcmpfunc>(connectionCompare), /* tp_richcompare */
        0,                                                /* tp_weaklistoffset */
        0,                                                /* tp_iter */
        0,                                                /* tp_iternext */
        ConnectionMethods,                                /* tp_methods */
        0,                                                /* tp_members */
        0,                                                /* tp_getset */
        0,                                                /* tp_base */
        0,                                                /* tp_dict */
        0,                                                /* tp_descr_get */
        0,                                                /* tp_descr_set */
        0,                                                /* tp_dictoffset */
        0,                                                /* tp_init */
        0,                                                /* tp_alloc */
        reinterpret_cast<newfunc>(connectionNew),         /* tp_new */
        0,                                                /* tp_free */
        0,                                                /* tp_is_gc */
    };

}

bool
IcePy::initConnection(PyObject* module)
{
    if (PyType_Ready(&ConnectionType) < 0)
    {
        return false;
    }
    PyTypeObject* type = &ConnectionType; // Necessary to prevent GCC's strict-alias warnings.
    if (PyModule_AddObject(module, STRCAST("Connection"), reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    return true;
}

PyObject*
IcePy::createConnection(const Ice::ConnectionPtr& connection, const Ice::CommunicatorPtr& communicator)
{
    ConnectionObject* obj = connectionNew(&ConnectionType, 0, 0);
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
        con = 0;
        return true;
    }
    else if (!checkConnection(p))
    {
        PyErr_Format(PyExc_ValueError, STRCAST("%s expects an Ice.Connection object or None for argument '%s'"),
                     func.c_str(), arg.c_str());
        return false;
    }
    else
    {
        ConnectionObject* obj = reinterpret_cast<ConnectionObject*>(p);
        con = *obj->connection;
        return true;
    }
}
