// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Communicator.h>
#include <Properties.h>
#include <ObjectAdapter.h>
#include <ObjectFactory.h>
#include <Proxy.h>
#include <Util.h>
#include <Ice/Initialize.h>
#include <Ice/Communicator.h>
#include <Ice/LocalException.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/Properties.h>

using namespace std;
using namespace IcePy;

namespace IcePy
{

struct CommunicatorObject
{
    PyObject_HEAD
    Ice::CommunicatorPtr* communicator;
};

}

#ifdef WIN32
extern "C"
#endif
static CommunicatorObject*
communicatorNew(PyObject* /*arg*/)
{
    CommunicatorObject* self = PyObject_New(CommunicatorObject, &CommunicatorType);
    if (self == NULL)
    {
        return NULL;
    }
    self->communicator = 0;
    return self;
}

#ifdef WIN32
extern "C"
#endif
static int
communicatorInit(CommunicatorObject* self, PyObject* args, PyObject* /*kwds*/)
{
    PyObject* arglist = NULL;
    PyObject* properties = NULL;
    if(!PyArg_ParseTuple(args, "|O!O!", &PyList_Type, &arglist, &PropertiesType, &properties))
    {
        return -1;
    }

    Ice::StringSeq seq;
    if(arglist && !listToStringSeq(arglist, seq))
    {
        return -1;
    }

    Ice::PropertiesPtr props;
    if(properties)
    {
        props = getProperties(properties);
    }
    else
    {
        props = Ice::getDefaultProperties(seq);
    }

    seq = props->parseIceCommandLineOptions(seq);

    Ice::CommunicatorPtr communicator;
    try
    {
        int argc = 0;
        static char** argv = { 0 };
        communicator = Ice::initializeWithProperties(argc, argv, props);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return -1;
    }

    //
    // Replace the contents of the given argument list with the filtered arguments.
    //
    if(arglist)
    {
        if(PyList_SetSlice(arglist, 0, PyList_Size(arglist), NULL) < 0)
        {
            return -1;
        }
        if(!stringSeqToList(seq, arglist))
        {
            return -1;
        }
    }

    self->communicator = new Ice::CommunicatorPtr(communicator);
    ObjectFactoryPtr factory = new ObjectFactory;
    (*self->communicator)->addObjectFactory(factory, "");

    return 0;
}

#ifdef WIN32
extern "C"
#endif
static void
communicatorDealloc(CommunicatorObject* self)
{
    delete self->communicator;
    PyObject_Del(self);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
communicatorDestroy(CommunicatorObject* self)
{
    assert(self->communicator);
    try
    {
        (*self->communicator)->destroy();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
communicatorShutdown(CommunicatorObject* self)
{
    assert(self->communicator);
    try
    {
        (*self->communicator)->shutdown();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
communicatorWaitForShutdown(CommunicatorObject* self)
{
    assert(self->communicator);
    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during blocking calls.
        (*self->communicator)->waitForShutdown();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
communicatorStringToProxy(CommunicatorObject* self, PyObject* args)
{
    char* str;
    if(!PyArg_ParseTuple(args, "s", &str))
    {
        return NULL;
    }

    assert(self->communicator);
    Ice::ObjectPrx proxy;
    try
    {
        proxy = (*self->communicator)->stringToProxy(str);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    return createProxy(proxy, *self->communicator);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
communicatorProxyToString(CommunicatorObject* self, PyObject* args)
{
    PyObject* obj;
    if(!PyArg_ParseTuple(args, "O!", &ProxyType, &obj))
    {
        return NULL;
    }

    Ice::ObjectPrx proxy = getProxy(obj);
    string str;

    assert(self->communicator);
    try
    {
        str = (*self->communicator)->proxyToString(proxy);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    return PyString_FromString(const_cast<char*>(str.c_str()));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
communicatorFlushBatchRequests(CommunicatorObject* self)
{
    assert(self->communicator);
    try
    {
        (*self->communicator)->flushBatchRequests();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
communicatorGetProperties(CommunicatorObject* self)
{
    assert(self->communicator);
    Ice::PropertiesPtr properties;
    try
    {
        properties = (*self->communicator)->getProperties();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    return createProperties(properties);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
communicatorAddObjectFactory(CommunicatorObject* self, PyObject* args)
{
    PyObject* factoryType = lookupType("Ice.ObjectFactory");
    assert(factoryType != NULL);

    PyObject* factory;
    char* id;
    if(!PyArg_ParseTuple(args, "O!s", factoryType, &factory, &id))
    {
        return NULL;
    }

    ObjectFactoryPtr pof;
    try
    {
        pof = ObjectFactoryPtr::dynamicCast((*self->communicator)->findObjectFactory(""));
        assert(pof);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;

    }

    if(!pof->add(factory, id))
    {
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
communicatorRemoveObjectFactory(CommunicatorObject* self, PyObject* args)
{
    char* id;
    if(!PyArg_ParseTuple(args, "s", &id))
    {
        return NULL;
    }

    ObjectFactoryPtr pof;
    try
    {
        pof = ObjectFactoryPtr::dynamicCast((*self->communicator)->findObjectFactory(""));
        assert(pof);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;

    }

    if(!pof->remove(id))
    {
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
communicatorFindObjectFactory(CommunicatorObject* self, PyObject* args)
{
    char* id;
    if(!PyArg_ParseTuple(args, "s", &id))
    {
        return NULL;
    }

    ObjectFactoryPtr pof;
    try
    {
        pof = ObjectFactoryPtr::dynamicCast((*self->communicator)->findObjectFactory(""));
        assert(pof);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;

    }

    return pof->find(id);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
communicatorCreateObjectAdapter(CommunicatorObject* self, PyObject* args)
{
    char* name;
    if(!PyArg_ParseTuple(args, "s", &name))
    {
        return NULL;
    }

    assert(self->communicator);
    Ice::ObjectAdapterPtr adapter;
    try
    {
        adapter = (*self->communicator)->createObjectAdapter(name);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    PyObject* obj = createObjectAdapter(adapter);
    if(obj == NULL)
    {
        try
        {
            adapter->deactivate();
        }
        catch(const Ice::Exception&)
        {
        }
    }

    return obj;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
communicatorCreateObjectAdapterWithEndpoints(CommunicatorObject* self, PyObject* args)
{
    char* name;
    char* endpoints;
    if(!PyArg_ParseTuple(args, "ss", &name, &endpoints))
    {
        return NULL;
    }

    assert(self->communicator);
    Ice::ObjectAdapterPtr adapter;
    try
    {
        adapter = (*self->communicator)->createObjectAdapterWithEndpoints(name, endpoints);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    PyObject* obj = createObjectAdapter(adapter);
    if(obj == NULL)
    {
        try
        {
            adapter->deactivate();
        }
        catch(const Ice::Exception&)
        {
        }
    }

    return obj;
}

static PyMethodDef CommunicatorMethods[] =
{
    { "destroy", (PyCFunction)communicatorDestroy, METH_NOARGS,
        PyDoc_STR("destroy() -> None") },
    { "shutdown", (PyCFunction)communicatorShutdown, METH_NOARGS,
        PyDoc_STR("shutdown() -> None") },
    { "waitForShutdown", (PyCFunction)communicatorWaitForShutdown, METH_NOARGS,
        PyDoc_STR("waitForShutdown() -> None") },
    { "stringToProxy", (PyCFunction)communicatorStringToProxy, METH_VARARGS,
        PyDoc_STR("stringToProxy(str) -> Ice.ObjectPrx") },
    { "proxyToString", (PyCFunction)communicatorProxyToString, METH_VARARGS,
        PyDoc_STR("proxyToString(Ice.ObjectPrx) -> string") },
    { "flushBatchRequests", (PyCFunction)communicatorFlushBatchRequests, METH_NOARGS,
        PyDoc_STR("flushBatchRequests() -> None") },
    { "getProperties", (PyCFunction)communicatorGetProperties, METH_NOARGS,
        PyDoc_STR("getProperties() -> Ice.Properties") },
    { "addObjectFactory", (PyCFunction)communicatorAddObjectFactory, METH_VARARGS,
        PyDoc_STR("addObjectFactory(factory, id) -> None") },
    { "removeObjectFactory", (PyCFunction)communicatorRemoveObjectFactory, METH_VARARGS,
        PyDoc_STR("removeObjectFactory(id) -> None") },
    { "findObjectFactory", (PyCFunction)communicatorFindObjectFactory, METH_VARARGS,
        PyDoc_STR("findObjectFactory(id) -> Ice.ObjectFactory") },
    { "createObjectAdapter", (PyCFunction)communicatorCreateObjectAdapter, METH_VARARGS,
        PyDoc_STR("createObjectAdapter(name) -> Ice.ObjectAdapter") },
    { "createObjectAdapterWithEndpoints", (PyCFunction)communicatorCreateObjectAdapterWithEndpoints, METH_VARARGS,
        PyDoc_STR("createObjectAdapterWithEndpoints(name, endpoints) -> Ice.ObjectAdapter") },
    { NULL, NULL} /* sentinel */
};

namespace IcePy
{

PyTypeObject CommunicatorType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyObject_HEAD_INIT(NULL)
    0,                               /* ob_size */
    "IcePy.Communicator",            /* tp_name */
    sizeof(CommunicatorObject),      /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    (destructor)communicatorDealloc, /* tp_dealloc */
    0,                               /* tp_print */
    0,                               /* tp_getattr */
    0,                               /* tp_setattr */
    0,                               /* tp_compare */
    0,                               /* tp_repr */
    0,                               /* tp_as_number */
    0,                               /* tp_as_sequence */
    0,                               /* tp_as_mapping */
    0,                               /* tp_hash */
    0,                               /* tp_call */
    0,                               /* tp_str */
    0,                               /* tp_getattro */
    0,                               /* tp_setattro */
    0,                               /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,              /* tp_flags */
    0,                               /* tp_doc */
    0,                               /* tp_traverse */
    0,                               /* tp_clear */
    0,                               /* tp_richcompare */
    0,                               /* tp_weaklistoffset */
    0,                               /* tp_iter */
    0,                               /* tp_iternext */
    CommunicatorMethods,             /* tp_methods */
    0,                               /* tp_members */
    0,                               /* tp_getset */
    0,                               /* tp_base */
    0,                               /* tp_dict */
    0,                               /* tp_descr_get */
    0,                               /* tp_descr_set */
    0,                               /* tp_dictoffset */
    (initproc)communicatorInit,      /* tp_init */
    0,                               /* tp_alloc */
    (newfunc)communicatorNew,        /* tp_new */
    0,                               /* tp_free */
    0,                               /* tp_is_gc */
};

}

bool
IcePy::initCommunicator(PyObject* module)
{
    if(PyType_Ready(&CommunicatorType) < 0)
    {
        return false;
    }
    if(PyModule_AddObject(module, "Communicator", (PyObject*)&CommunicatorType) < 0)
    {
        return false;
    }
    return true;
}

Ice::CommunicatorPtr
IcePy::getCommunicator(PyObject* obj)
{
    assert(PyObject_IsInstance(obj, (PyObject*)&CommunicatorType));
    CommunicatorObject* cobj = (CommunicatorObject*)obj;
    return *cobj->communicator;
}

PyObject*
IcePy::createCommunicator(const Ice::CommunicatorPtr& communicator)
{
    CommunicatorObject* obj = communicatorNew(NULL);
    if(obj != NULL)
    {
        obj->communicator = new Ice::CommunicatorPtr(communicator);
    }
    return (PyObject*)obj;
}

extern "C"
PyObject*
Ice_initialize(PyObject* /*self*/, PyObject* args)
{
    //
    // Currently the same as "c = Ice.Communicator(args)".
    //
    return PyObject_Call((PyObject*)&CommunicatorType, args, NULL);
}

extern "C"
PyObject*
Ice_initializeWithProperties(PyObject* /*self*/, PyObject* args)
{
    //
    // Currently the same as "c = Ice.Communicator(args, properties)".
    //
    return PyObject_Call((PyObject*)&CommunicatorType, args, NULL);
}
