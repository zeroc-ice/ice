// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifdef _WIN32
#   include <IceUtil/Config.h>
#endif
#include <Communicator.h>
#include <Logger.h>
#include <ObjectAdapter.h>
#include <ObjectFactory.h>
#include <Properties.h>
#include <Proxy.h>
#include <Util.h>
#include <Ice/Initialize.h>
#include <Ice/Communicator.h>
#include <Ice/LocalException.h>
#include <Ice/Locator.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/Properties.h>
#include <Ice/Router.h>

#include <pythread.h>

using namespace std;
using namespace IcePy;

static long _mainThreadId;

namespace IcePy
{

struct CommunicatorObject;

typedef InvokeThread<Ice::Communicator> WaitForShutdownThread;
typedef IceUtil::Handle<WaitForShutdownThread> WaitForShutdownThreadPtr;

struct CommunicatorObject
{
    PyObject_HEAD
    Ice::CommunicatorPtr* communicator;
    IceUtil::Monitor<IceUtil::Mutex>* shutdownMonitor;
    WaitForShutdownThreadPtr* shutdownThread;
    bool shutdown;
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
    self->shutdownMonitor = new IceUtil::Monitor<IceUtil::Mutex>;
    self->shutdownThread = 0;
    self->shutdown = false;
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
    if(self->shutdownThread)
    {
        (*self->shutdownThread)->getThreadControl().join();
    }
    delete self->communicator;
    delete self->shutdownMonitor;
    delete self->shutdownThread;
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
communicatorWaitForShutdown(CommunicatorObject* self, PyObject* args)
{
    //
    // This method differs somewhat from the standard Ice API because of
    // signal issues. This method expects an integer timeout value, and
    // returns a boolean to indicate whether it was successful. When
    // called from the main thread, the timeout is used to allow control
    // to return to the caller (the Python interpreter) periodically.
    // When called from any other thread, we call waitForShutdown directly
    // and ignore the timeout.
    //
    int timeout = 0;
    if(!PyArg_ParseTuple(args, "i", &timeout))
    {
        return NULL;
    }

    assert(timeout > 0);
    assert(self->communicator);

    //
    // Do not call waitForShutdown from the main thread, because it prevents
    // signals (such as keyboard interrupts) from being delivered to Python.
    //
    if(PyThread_get_thread_ident() == _mainThreadId)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*self->shutdownMonitor);

        if(!self->shutdown)
        {
            if(self->shutdownThread == 0)
            {
                WaitForShutdownThreadPtr t = new WaitForShutdownThread(*self->communicator,
                                                                       &Ice::Communicator::waitForShutdown,
                                                                       *self->shutdownMonitor, self->shutdown);
                self->shutdownThread = new WaitForShutdownThreadPtr(t);
                t->start();
            }

	    while(!self->shutdown)
	    {
		bool done;
		{
		    AllowThreads allowThreads; // Release Python's global interpreter lock during blocking calls.
		    done = (*self->shutdownMonitor).timedWait(IceUtil::Time::milliSeconds(timeout));
		}
		
		if(!done)
		{
		    Py_INCREF(Py_False);
		    return Py_False;
		}
	    }
        }

        assert(self->shutdown);

        Ice::Exception* ex = (*self->shutdownThread)->getException();
        if(ex)
        {
            setPythonException(*ex);
            return NULL;
        }
    }
    else
    {
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
    }

    Py_INCREF(Py_True);
    return Py_True;
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
communicatorGetLogger(CommunicatorObject* self)
{
    assert(self->communicator);
    Ice::LoggerPtr logger;
    try
    {
        logger = (*self->communicator)->getLogger();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    return createLogger(logger);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
communicatorSetLogger(CommunicatorObject* self, PyObject* args)
{
    PyObject* loggerType = lookupType("Ice.Logger");
    assert(loggerType != NULL);

    PyObject* logger;
    if(!PyArg_ParseTuple(args, "O!", loggerType, &logger))
    {
        return NULL;
    }

    Ice::LoggerPtr wrapper = wrapLogger(logger);
    try
    {
        (*self->communicator)->setLogger(wrapper);
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
communicatorSetDefaultContext(CommunicatorObject* self, PyObject* args)
{
    PyObject* dict;
    if(!PyArg_ParseTuple(args, "O!", &PyDict_Type, &dict))
    {
        return NULL;
    }

    Ice::Context ctx;
    if(!dictionaryToContext(dict, ctx))
    {
	return NULL;
    }

    try
    {
        (*self->communicator)->setDefaultContext(ctx);
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
communicatorGetDefaultContext(CommunicatorObject* self)
{
    Ice::Context ctx;
    try
    {
        ctx = (*self->communicator)->getDefaultContext();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;

    }

    PyObjectHandle dict = PyDict_New();
    if(dict.get() == NULL)
    {
	return NULL;
    }

    if(!contextToDictionary(ctx, dict.get()))
    {
	return NULL;
    }

    return dict.release();
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

#ifdef WIN32
extern "C"
#endif
static PyObject*
communicatorGetDefaultRouter(CommunicatorObject* self)
{
    assert(self->communicator);
    Ice::RouterPrx router;
    try
    {
        router = (*self->communicator)->getDefaultRouter();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    PyObject* routerProxyType = lookupType("Ice.RouterPrx");
    assert(routerProxyType != NULL);
    return createProxy(router, *self->communicator, routerProxyType);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
communicatorSetDefaultRouter(CommunicatorObject* self, PyObject* args)
{
    PyObject* routerProxyType = lookupType("Ice.RouterPrx");
    assert(routerProxyType != NULL);
    PyObject* proxy;
    if(!PyArg_ParseTuple(args, "O!", routerProxyType, &proxy))
    {
        return NULL;
    }

    Ice::RouterPrx router = Ice::RouterPrx::uncheckedCast(getProxy(proxy));

    assert(self->communicator);
    try
    {
        (*self->communicator)->setDefaultRouter(router);
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
communicatorGetDefaultLocator(CommunicatorObject* self)
{
    assert(self->communicator);
    Ice::LocatorPrx locator;
    try
    {
        locator = (*self->communicator)->getDefaultLocator();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    PyObject* locatorProxyType = lookupType("Ice.LocatorPrx");
    assert(locatorProxyType != NULL);
    return createProxy(locator, *self->communicator, locatorProxyType);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
communicatorSetDefaultLocator(CommunicatorObject* self, PyObject* args)
{
    PyObject* locatorProxyType = lookupType("Ice.LocatorPrx");
    assert(locatorProxyType != NULL);
    PyObject* proxy;
    if(!PyArg_ParseTuple(args, "O!", locatorProxyType, &proxy))
    {
        return NULL;
    }

    Ice::LocatorPrx locator = Ice::LocatorPrx::uncheckedCast(getProxy(proxy));

    assert(self->communicator);
    try
    {
        (*self->communicator)->setDefaultLocator(locator);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef CommunicatorMethods[] =
{
    { "destroy", (PyCFunction)communicatorDestroy, METH_NOARGS,
        PyDoc_STR("destroy() -> None") },
    { "shutdown", (PyCFunction)communicatorShutdown, METH_NOARGS,
        PyDoc_STR("shutdown() -> None") },
    { "waitForShutdown", (PyCFunction)communicatorWaitForShutdown, METH_VARARGS,
        PyDoc_STR("waitForShutdown() -> None") },
    { "stringToProxy", (PyCFunction)communicatorStringToProxy, METH_VARARGS,
        PyDoc_STR("stringToProxy(str) -> Ice.ObjectPrx") },
    { "proxyToString", (PyCFunction)communicatorProxyToString, METH_VARARGS,
        PyDoc_STR("proxyToString(Ice.ObjectPrx) -> string") },
    { "createObjectAdapter", (PyCFunction)communicatorCreateObjectAdapter, METH_VARARGS,
        PyDoc_STR("createObjectAdapter(name) -> Ice.ObjectAdapter") },
    { "createObjectAdapterWithEndpoints", (PyCFunction)communicatorCreateObjectAdapterWithEndpoints, METH_VARARGS,
        PyDoc_STR("createObjectAdapterWithEndpoints(name, endpoints) -> Ice.ObjectAdapter") },
    { "addObjectFactory", (PyCFunction)communicatorAddObjectFactory, METH_VARARGS,
        PyDoc_STR("addObjectFactory(factory, id) -> None") },
    { "removeObjectFactory", (PyCFunction)communicatorRemoveObjectFactory, METH_VARARGS,
        PyDoc_STR("removeObjectFactory(id) -> None") },
    { "findObjectFactory", (PyCFunction)communicatorFindObjectFactory, METH_VARARGS,
        PyDoc_STR("findObjectFactory(id) -> Ice.ObjectFactory") },
    { "setDefaultContext", (PyCFunction)communicatorSetDefaultContext, METH_VARARGS,
        PyDoc_STR("setDefaultContext(ctx) -> None") },
    { "getDefaultContext", (PyCFunction)communicatorGetDefaultContext, METH_NOARGS,
        PyDoc_STR("getDefaultContext() -> Ice.Context") },
    { "getProperties", (PyCFunction)communicatorGetProperties, METH_NOARGS,
        PyDoc_STR("getProperties() -> Ice.Properties") },
    { "getLogger", (PyCFunction)communicatorGetLogger, METH_NOARGS,
        PyDoc_STR("getLogger() -> Ice.Logger") },
    { "setLogger", (PyCFunction)communicatorSetLogger, METH_VARARGS,
        PyDoc_STR("setLogger(Ice.Logger) -> None") },
    { "getDefaultRouter", (PyCFunction)communicatorGetDefaultRouter, METH_NOARGS,
        PyDoc_STR("getDefaultRouter() -> proxy") },
    { "setDefaultRouter", (PyCFunction)communicatorSetDefaultRouter, METH_VARARGS,
        PyDoc_STR("setDefaultRouter(proxy) -> None") },
    { "getDefaultLocator", (PyCFunction)communicatorGetDefaultLocator, METH_NOARGS,
        PyDoc_STR("getDefaultLocator() -> proxy") },
    { "setDefaultLocator", (PyCFunction)communicatorSetDefaultLocator, METH_VARARGS,
        PyDoc_STR("setDefaultLocator(proxy) -> None") },
    { "flushBatchRequests", (PyCFunction)communicatorFlushBatchRequests, METH_NOARGS,
        PyDoc_STR("flushBatchRequests() -> None") },
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
    _mainThreadId = PyThread_get_thread_ident();

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
IcePy_initialize(PyObject* /*self*/, PyObject* args)
{
    //
    // Currently the same as "c = Ice.Communicator(args)".
    //
    return PyObject_Call((PyObject*)&CommunicatorType, args, NULL);
}

extern "C"
PyObject*
IcePy_initializeWithProperties(PyObject* /*self*/, PyObject* args)
{
    //
    // Currently the same as "c = Ice.Communicator(args, properties)".
    //
    return PyObject_Call((PyObject*)&CommunicatorType, args, NULL);
}
