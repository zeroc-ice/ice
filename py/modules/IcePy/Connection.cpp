// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifdef _WIN32
#   include <IceUtil/Config.h>
#endif
#include <Connection.h>
#include <Communicator.h>
#include <ConnectionInfo.h>
#include <Endpoint.h>
#include <ObjectAdapter.h>
#include <Operation.h>
#include <Proxy.h>
#include <Thread.h>
#include <Util.h>
#include <Ice/ConnectionAsync.h>

using namespace std;
using namespace IcePy;

namespace IcePy
{

extern PyTypeObject ConnectionType;

struct ConnectionObject
{
    PyObject_HEAD
    Ice::ConnectionPtr* connection;
    Ice::CommunicatorPtr* communicator;
};

}

#ifdef WIN32
extern "C"
#endif
static ConnectionObject*
connectionNew(PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwds*/)
{
    assert(type && type->tp_alloc);
    ConnectionObject* self = reinterpret_cast<ConnectionObject*>(type->tp_alloc(type, 0));
    if(!self)
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

    if(PyObject_TypeCheck(other, &ConnectionType))
    {
        ConnectionObject* c2 = reinterpret_cast<ConnectionObject*>(other);

        switch(op)
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
        if(op == Py_EQ)
        {
            result = false;
        }
        else if(op == Py_NE)
        {
            result = true;
        }
        else
        {
            PyErr_Format(PyExc_TypeError, "can't compare %s to %s", Py_TYPE(c1)->tp_name, Py_TYPE(other)->tp_name);
            return 0;
        }
    }

    PyObject* r = result ? getTrue() : getFalse();
    Py_INCREF(r);
    return r;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
connectionClose(ConnectionObject* self, PyObject* args)
{
    int force;
    if(!PyArg_ParseTuple(args, STRCAST("i"), &force))
    {
        return 0;
    }

    assert(self->connection);
    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during blocking invocations.
        (*self->connection)->close(force > 0);
    }
    catch(const Ice::Exception& ex)
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
    if(!PyArg_ParseTuple(args, STRCAST("O!"), identityType, &id))
    {
        return 0;
    }

    Ice::Identity ident;
    if(!getIdentity(id, ident))
    {
        return 0;
    }

    assert(self->connection);
    assert(self->communicator);
    Ice::ObjectPrx proxy;
    try
    {
        proxy = (*self->connection)->createProxy(ident);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(proxy, (*self->communicator));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
connectionSetAdapter(ConnectionObject* self, PyObject* args)
{
    PyObject* adapterType = lookupType("Ice.ObjectAdapter");
    PyObject* adapter;
    if(!PyArg_ParseTuple(args, STRCAST("O!"), adapterType, &adapter))
    {
        return 0;
    }

    Ice::ObjectAdapterPtr oa = unwrapObjectAdapter(adapter);
    assert(oa);

    assert(self->connection);
    assert(self->communicator);
    try
    {
        (*self->connection)->setAdapter(oa);
    }
    catch(const Ice::Exception& ex)
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
connectionGetAdapter(ConnectionObject* self)
{
    Ice::ObjectAdapterPtr adapter;

    assert(self->connection);
    assert(self->communicator);
    try
    {
        adapter = (*self->connection)->getAdapter();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return wrapObjectAdapter(adapter);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
connectionFlushBatchRequests(ConnectionObject* self)
{
    assert(self->connection);
    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.
        (*self->connection)->flushBatchRequests();
    }
    catch(const Ice::Exception& ex)
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
connectionBeginFlushBatchRequests(ConnectionObject* self, PyObject* args, PyObject* kwds)
{
    assert(self->connection);

    static char* argNames[] =
    {
        const_cast<char*>("_ex"),
        const_cast<char*>("_sent"),
        0
    };
    PyObject* ex = Py_None;
    PyObject* sent = Py_None;
    if(!PyArg_ParseTupleAndKeywords(args, kwds, STRCAST("|OO"), argNames, &ex, &sent))
    {
        return 0;
    }

    if(ex == Py_None)
    {
        ex = 0;
    }
    if(sent == Py_None)
    {
        sent = 0;
    }

    if(!ex && sent)
    {
        PyErr_Format(PyExc_RuntimeError,
            STRCAST("exception callback must also be provided when sent callback is used"));
        return 0;
    }

    Ice::Callback_Connection_flushBatchRequestsPtr cb;
    if(ex || sent)
    {
        FlushCallbackPtr d = new FlushCallback(ex, sent, "flushBatchRequests");
        cb = Ice::newCallback_Connection_flushBatchRequests(d, &FlushCallback::exception, &FlushCallback::sent);
    }

    Ice::AsyncResultPtr result;
    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.

        if(cb)
        {
            result = (*self->connection)->begin_flushBatchRequests(cb);
        }
        else
        {
            result = (*self->connection)->begin_flushBatchRequests();
        }
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    PyObjectHandle communicator = getCommunicatorWrapper(*self->communicator);
    return createAsyncResult(result, 0, reinterpret_cast<PyObject*>(self), communicator.get());
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
connectionEndFlushBatchRequests(ConnectionObject* self, PyObject* args)
{
    assert(self->connection);

    PyObject* result;
    if(!PyArg_ParseTuple(args, STRCAST("O!"), &AsyncResultType, &result))
    {
        return 0;
    }

    Ice::AsyncResultPtr r = getAsyncResult(result);
    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during blocking invocations.
        (*self->connection)->end_flushBatchRequests(r);
    }
    catch(const Ice::Exception& ex)
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
connectionType(ConnectionObject* self)
{
    assert(self->connection);
    string type;
    try
    {
        type = (*self->connection)->type();
    }
    catch(const Ice::Exception& ex)
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
connectionTimeout(ConnectionObject* self)
{
    assert(self->connection);
    int timeout;
    try
    {
        timeout = (*self->connection)->timeout();
    }
    catch(const Ice::Exception& ex)
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
connectionToString(ConnectionObject* self)
{
    assert(self->connection);
    string str;
    try
    {
        str = (*self->connection)->toString();
    }
    catch(const Ice::Exception& ex)
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
connectionGetInfo(ConnectionObject* self)
{
    assert(self->connection);
    try
    {
        Ice::ConnectionInfoPtr info = (*self->connection)->getInfo();
        return createConnectionInfo(info);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
connectionGetEndpoint(ConnectionObject* self)
{
    assert(self->connection);
    try
    {
        Ice::EndpointPtr endpoint = (*self->connection)->getEndpoint();
        return createEndpoint(endpoint);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }
}

static PyMethodDef ConnectionMethods[] =
{
    { STRCAST("close"), reinterpret_cast<PyCFunction>(connectionClose), METH_VARARGS,
        PyDoc_STR(STRCAST("close(bool) -> None")) },
    { STRCAST("createProxy"), reinterpret_cast<PyCFunction>(connectionCreateProxy), METH_VARARGS,
        PyDoc_STR(STRCAST("createProxy(Ice.Identity) -> Ice.ObjectPrx")) },
    { STRCAST("setAdapter"), reinterpret_cast<PyCFunction>(connectionSetAdapter), METH_VARARGS,
        PyDoc_STR(STRCAST("setAdapter(Ice.ObjectAdapter) -> None")) },
    { STRCAST("getAdapter"), reinterpret_cast<PyCFunction>(connectionGetAdapter), METH_NOARGS,
        PyDoc_STR(STRCAST("getAdapter() -> Ice.ObjectAdapter")) },
    { STRCAST("flushBatchRequests"), reinterpret_cast<PyCFunction>(connectionFlushBatchRequests), METH_NOARGS,
        PyDoc_STR(STRCAST("flushBatchRequests() -> None")) },
    { STRCAST("begin_flushBatchRequests"), reinterpret_cast<PyCFunction>(connectionBeginFlushBatchRequests),
        METH_VARARGS | METH_KEYWORDS, PyDoc_STR(STRCAST("begin_flushBatchRequests([_ex][, _sent]) -> Ice.AsyncResult")) },
    { STRCAST("end_flushBatchRequests"), reinterpret_cast<PyCFunction>(connectionEndFlushBatchRequests), METH_VARARGS,
        PyDoc_STR(STRCAST("end_flushBatchRequests(Ice.AsyncResult) -> None")) },
    { STRCAST("type"), reinterpret_cast<PyCFunction>(connectionType), METH_NOARGS,
        PyDoc_STR(STRCAST("type() -> string")) },
    { STRCAST("timeout"), reinterpret_cast<PyCFunction>(connectionTimeout), METH_NOARGS,
        PyDoc_STR(STRCAST("timeout() -> int")) },
    { STRCAST("toString"), reinterpret_cast<PyCFunction>(connectionToString), METH_NOARGS,
        PyDoc_STR(STRCAST("toString() -> string")) },
    { STRCAST("getInfo"), reinterpret_cast<PyCFunction>(connectionGetInfo), METH_NOARGS,
        PyDoc_STR(STRCAST("getInfo() -> Ice.ConnectionInfo")) },
    { STRCAST("getEndpoint"), reinterpret_cast<PyCFunction>(connectionGetEndpoint), METH_NOARGS,
        PyDoc_STR(STRCAST("getEndpoint() -> Ice.Endpoint")) },
    { 0, 0 } /* sentinel */
};

namespace IcePy
{

PyTypeObject ConnectionType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyVarObject_HEAD_INIT(0, 0)
    STRCAST("IcePy.Connection"),    /* tp_name */
    sizeof(ConnectionObject),       /* tp_basicsize */
    0,                              /* tp_itemsize */
    /* methods */
    reinterpret_cast<destructor>(connectionDealloc), /* tp_dealloc */
    0,                              /* tp_print */
    0,                              /* tp_getattr */
    0,                              /* tp_setattr */
    0,                              /* tp_reserved */
    0,                              /* tp_repr */
    0,                              /* tp_as_number */
    0,                              /* tp_as_sequence */
    0,                              /* tp_as_mapping */
    0,                              /* tp_hash */
    0,                              /* tp_call */
    0,                              /* tp_str */
    0,                              /* tp_getattro */
    0,                              /* tp_setattro */
    0,                              /* tp_as_buffer */
#if PY_VERSION_HEX >= 0x03000000
    Py_TPFLAGS_DEFAULT,             /* tp_flags */
#else
    Py_TPFLAGS_DEFAULT |
    Py_TPFLAGS_HAVE_RICHCOMPARE,    /* tp_flags */
#endif
    0,                              /* tp_doc */
    0,                              /* tp_traverse */
    0,                              /* tp_clear */
    reinterpret_cast<richcmpfunc>(connectionCompare), /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    0,                              /* tp_iter */
    0,                              /* tp_iternext */
    ConnectionMethods,              /* tp_methods */
    0,                              /* tp_members */
    0,                              /* tp_getset */
    0,                              /* tp_base */
    0,                              /* tp_dict */
    0,                              /* tp_descr_get */
    0,                              /* tp_descr_set */
    0,                              /* tp_dictoffset */
    0,                              /* tp_init */
    0,                              /* tp_alloc */
    reinterpret_cast<newfunc>(connectionNew), /* tp_new */
    0,                              /* tp_free */
    0,                              /* tp_is_gc */
};

}

bool
IcePy::initConnection(PyObject* module)
{
    if(PyType_Ready(&ConnectionType) < 0)
    {
        return false;
    }
    PyTypeObject* type = &ConnectionType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("Connection"), reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    return true;
}

PyObject*
IcePy::createConnection(const Ice::ConnectionPtr& connection, const Ice::CommunicatorPtr& communicator)
{
    ConnectionObject* obj = connectionNew(&ConnectionType, 0, 0);
    if(obj)
    {
        obj->connection = new Ice::ConnectionPtr(connection);
        obj->communicator = new Ice::CommunicatorPtr(communicator);
    }
    return reinterpret_cast<PyObject*>(obj);
}
