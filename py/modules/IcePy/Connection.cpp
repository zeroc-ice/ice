// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifdef _WIN32
#   include <IceUtil/Config.h>
#endif
#include <Connection.h>
#include <ObjectAdapter.h>
#include <Proxy.h>
#include <Util.h>
#include <Ice/Connection.h>

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
connectionNew(PyObject* /*arg*/)
{
    ConnectionObject* self = PyObject_New(ConnectionObject, &ConnectionType);
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
    PyObject_Del(self);
}

#ifdef WIN32
extern "C"
#endif
static int
connectionCompare(ConnectionObject* c1, ConnectionObject* c2)
{
    if(*c1->connection < *c2->connection)
    {
        return -1;
    }
    else if(*c1->connection == *c2->connection)
    {
        return 0;
    }
    else
    {
        return 1;
    }
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

    return PyString_FromString(const_cast<char*>(type.c_str()));
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

    return PyInt_FromLong(timeout);
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

    return PyString_FromString(const_cast<char*>(str.c_str()));
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
    { STRCAST("type"), reinterpret_cast<PyCFunction>(connectionType), METH_NOARGS,
        PyDoc_STR(STRCAST("type() -> string")) },
    { STRCAST("timeout"), reinterpret_cast<PyCFunction>(connectionTimeout), METH_NOARGS,
        PyDoc_STR(STRCAST("timeout() -> int")) },
    { STRCAST("toString"), reinterpret_cast<PyCFunction>(connectionToString), METH_NOARGS,
        PyDoc_STR(STRCAST("toString() -> string")) },
    { 0, 0 } /* sentinel */
};

namespace IcePy
{

PyTypeObject ConnectionType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyObject_HEAD_INIT(0)
    0,                              /* ob_size */
    STRCAST("IcePy.Connection"),    /* tp_name */
    sizeof(ConnectionObject),       /* tp_basicsize */
    0,                              /* tp_itemsize */
    /* methods */
    reinterpret_cast<destructor>(connectionDealloc), /* tp_dealloc */
    0,                              /* tp_print */
    0,                              /* tp_getattr */
    0,                              /* tp_setattr */
    reinterpret_cast<cmpfunc>(connectionCompare), /* tp_compare */
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
    Py_TPFLAGS_DEFAULT,             /* tp_flags */
    0,                              /* tp_doc */
    0,                              /* tp_traverse */
    0,                              /* tp_clear */
    0,                              /* tp_richcompare */
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
    ConnectionObject* obj = connectionNew(0);
    if(obj)
    {
        obj->connection = new Ice::ConnectionPtr(connection);
        obj->communicator = new Ice::CommunicatorPtr(communicator);
    }
    return reinterpret_cast<PyObject*>(obj);
}
