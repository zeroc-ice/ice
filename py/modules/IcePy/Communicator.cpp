// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
#include <ThreadNotification.h>
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

typedef map<Ice::CommunicatorPtr, PyObject*> CommunicatorMap;
static CommunicatorMap _communicatorMap;

namespace IcePy
{

struct CommunicatorObject;

typedef InvokeThread<Ice::Communicator> WaitForShutdownThread;
typedef IceUtil::Handle<WaitForShutdownThread> WaitForShutdownThreadPtr;

struct CommunicatorObject
{
    PyObject_HEAD
    Ice::CommunicatorPtr* communicator;
    PyObject* wrapper;
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
    self->wrapper = 0;
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
    PyObject* initDataType = lookupType("Ice.InitializationData");
    PyObject* initData = NULL;
    if(!PyArg_ParseTuple(args, STRCAST("|O!O!"), &PyList_Type, &arglist, initDataType, &initData))
    {
        return -1;
    }

    Ice::StringSeq seq;
    if(arglist && !listToStringSeq(arglist, seq))
    {
        return -1;
    }

    Ice::InitializationData data;
    if(initData)
    {
	PyObjectHandle properties = PyObject_GetAttrString(initData, STRCAST("properties"));
	PyObjectHandle logger = PyObject_GetAttrString(initData, STRCAST("logger"));
	PyObjectHandle defaultContext = PyObject_GetAttrString(initData, STRCAST("defaultContext"));
	PyObjectHandle threadHook = PyObject_GetAttrString(initData, STRCAST("threadHook"));

	if(properties.get() && properties.get() != Py_None)
	{
	    //
	    // Get the properties implementation.
	    //
	    PyObjectHandle impl = PyObject_GetAttrString(properties.get(), STRCAST("_impl"));
	    assert(impl.get() != NULL);
	    data.properties = getProperties(impl.get());
	}

	if(logger.get() && logger.get() != Py_None)
	{
	    data.logger = new LoggerWrapper(logger.get());
	}

	if(defaultContext.get() && defaultContext.get() != Py_None)
	{
	    if(!dictionaryToContext(defaultContext.get(), data.defaultContext))
	    {
		return -1;
	    }
	}

	if(threadHook.get() && threadHook.get() != Py_None)
	{
	    data.threadHook = new ThreadNotificationWrapper(threadHook.get());
	}

    }

    data.properties = Ice::createProperties(seq, data.properties);
  
    //
    // Disable collocation optimization, otherwise a Python invocation on
    // a collocated servant results in a CollocationOptimizationException
    // (because Python uses the blobject API).
    //
    data.properties->setProperty("Ice.Default.CollocationOptimization", "0");
   
    //
    // Remaining command line options are passed to the communicator
    // as an argument vector in case they contain plugin properties.
    //
    int argc = static_cast<int>(seq.size());
    char** argv = new char*[argc + 1];
    int i = 0;
    for(Ice::StringSeq::const_iterator s = seq.begin(); s != seq.end(); ++s, ++i)
    {
	argv[i] = strdup(s->c_str());
    }
    argv[argc] = 0;

    Ice::CommunicatorPtr communicator;
    try
    {
	communicator = Ice::initialize(argc, argv, data);
    }
    catch(const Ice::Exception& ex)
    {
	for(i = 0; i < argc + 1; ++i)
	{
	    free(argv[i]);
	}
	delete[] argv;

	setPythonException(ex);
	return -1;
    }
    
    //
    // Replace the contents of the given argument list with the filtered arguments.
    //
    if(arglist)
    {
	PyList_SetSlice(arglist, 0, PyList_Size(arglist), NULL); // Clear the list.

	for(i = 0; i < argc; ++i)
	{
	    PyObjectHandle str = Py_BuildValue(STRCAST("s"), argv[i]);
	    PyList_Append(arglist, str.get());
	}
    }

    for(i = 0; i < argc + 1; ++i)
    {
	free(argv[i]);
    }
    delete[] argv;

    self->communicator = new Ice::CommunicatorPtr(communicator);
    ObjectFactoryPtr factory = new ObjectFactory;
    (*self->communicator)->addObjectFactory(factory, "");

    CommunicatorMap::iterator p = _communicatorMap.find(communicator);
    if(p != _communicatorMap.end())
    {
	_communicatorMap.erase(p);
    }
    _communicatorMap.insert(CommunicatorMap::value_type(communicator, (PyObject*)self));

    return 0;
}

#ifdef WIN32
extern "C"
#endif
static void
communicatorDealloc(CommunicatorObject* self)
{
    CommunicatorMap::iterator p = _communicatorMap.find(*self->communicator);
    //
    // find() can fail if an error occurred during communicator initialization.
    //
    if(p != _communicatorMap.end())
    {
	_communicatorMap.erase(p);
    }

    if(self->shutdownThread)
    {
        (*self->shutdownThread)->getThreadControl().join();
    }
    delete self->communicator;
    Py_XDECREF(self->wrapper);
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
	AllowThreads allowThreads; // Release Python's global interpreter lock to avoid a potential deadlock.
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
    if(!PyArg_ParseTuple(args, STRCAST("i"), &timeout))
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
    if(!PyArg_ParseTuple(args, STRCAST("s"), &str))
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
    if(!PyArg_ParseTuple(args, STRCAST("O!"), &ProxyType, &obj))
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
communicatorStringToIdentity(CommunicatorObject* self, PyObject* args)
{
    char* str;
    if(!PyArg_ParseTuple(args, STRCAST("s"), &str))
    {
        return NULL;
    }

    assert(self->communicator);
    Ice::Identity id;
    try
    {
        id = (*self->communicator)->stringToIdentity(str);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    return createIdentity(id);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
communicatorIdentityToString(CommunicatorObject* self, PyObject* args)
{
    PyObject* identityType = lookupType("Ice.Identity");
    PyObject* obj;
    if(!PyArg_ParseTuple(args, STRCAST("O!"), identityType, &obj))
    {
        return NULL;
    }

    Ice::Identity id;
    if(!getIdentity(obj, id))
    {
        return NULL;
    }
    string str;

    assert(self->communicator);
    try
    {
        str = (*self->communicator)->identityToString(id);
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
communicatorSetWrapper(CommunicatorObject* self, PyObject* args)
{
    PyObject* wrapper;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &wrapper))
    {
        return NULL;
    }

    assert(self->wrapper == NULL);
    self->wrapper = wrapper;

    Py_INCREF(Py_None);
    return Py_None;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
communicatorGetWrapper(CommunicatorObject* self)
{
    assert(self->wrapper != NULL);
    Py_INCREF(self->wrapper);
    return self->wrapper;
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

    //
    // The communicator's logger can either be a C++ object (such as
    // the default logger supplied by the Ice run time), or a C++
    // wrapper around a Python implementation. If the latter, we
    // return it directly. Otherwise, we create a Python object
    // that delegates to the C++ object.
    //
    LoggerWrapperPtr wrapper = LoggerWrapperPtr::dynamicCast(logger);
    if(wrapper)
    {
	PyObject* obj = wrapper->getObject();
	Py_INCREF(obj);
	return obj;
    }

    return createLogger(logger);
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
    if(!PyArg_ParseTuple(args, STRCAST("O!s"), factoryType, &factory, &id))
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
    if(!PyArg_ParseTuple(args, STRCAST("s"), &id))
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
    if(!PyArg_ParseTuple(args, STRCAST("s"), &id))
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
    if(!PyArg_ParseTuple(args, STRCAST("s"), &name))
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
    if(!PyArg_ParseTuple(args, STRCAST("ss"), &name, &endpoints))
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
communicatorCreateObjectAdapterWithRouter(CommunicatorObject* self, PyObject* args)
{
    char* name;
    PyObject* proxy;
    if(!PyArg_ParseTuple(args, STRCAST("sO"), &name, &proxy))
    {
	return NULL;
    }

    PyObject* routerProxyType = lookupType("Ice.RouterPrx");
    assert(routerProxyType != NULL);
    Ice::RouterPrx router;
    if(PyObject_IsInstance(proxy, routerProxyType))
    {
	router = Ice::RouterPrx::uncheckedCast(getProxy(proxy));
    }
    else if(proxy != Py_None)
    {
	PyErr_Format(PyExc_ValueError, STRCAST("ice_createObjectAdapterWithRouter requires None or Ice.RouterPrx"));
	return NULL;
    }

    assert(self->communicator);
    Ice::ObjectAdapterPtr adapter;
    try
    {
        adapter = (*self->communicator)->createObjectAdapterWithRouter(name, router);
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

    if(!router)
    {
	Py_INCREF(Py_None);
	return Py_None;
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
    PyObject* proxy;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &proxy))
    {
	return NULL;
    }

    PyObject* routerProxyType = lookupType("Ice.RouterPrx");
    assert(routerProxyType != NULL);
    Ice::RouterPrx router;
    if(PyObject_IsInstance(proxy, routerProxyType))
    {
	router = Ice::RouterPrx::uncheckedCast(getProxy(proxy));
    }
    else if(proxy != Py_None)
    {
	PyErr_Format(PyExc_ValueError, STRCAST("ice_setDefaultRouter requires None or Ice.RouterPrx"));
	return NULL;
    }

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

    if(!locator)
    {
	Py_INCREF(Py_None);
	return Py_None;
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
    PyObject* proxy;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &proxy))
    {
	return NULL;
    }

    PyObject* locatorProxyType = lookupType("Ice.LocatorPrx");
    assert(locatorProxyType != NULL);
    Ice::LocatorPrx locator;
    if(PyObject_IsInstance(proxy, locatorProxyType))
    {
	locator = Ice::LocatorPrx::uncheckedCast(getProxy(proxy));
    }
    else if(proxy != Py_None)
    {
	PyErr_Format(PyExc_ValueError, STRCAST("ice_setDefaultLocator requires None or Ice.LocatorPrx"));
	return NULL;
    }

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
    { STRCAST("destroy"), (PyCFunction)communicatorDestroy, METH_NOARGS,
        PyDoc_STR(STRCAST("destroy() -> None")) },
    { STRCAST("shutdown"), (PyCFunction)communicatorShutdown, METH_NOARGS,
        PyDoc_STR(STRCAST("shutdown() -> None")) },
    { STRCAST("waitForShutdown"), (PyCFunction)communicatorWaitForShutdown, METH_VARARGS,
        PyDoc_STR(STRCAST("waitForShutdown() -> None")) },
    { STRCAST("stringToProxy"), (PyCFunction)communicatorStringToProxy, METH_VARARGS,
        PyDoc_STR(STRCAST("stringToProxy(str) -> Ice.ObjectPrx")) },
    { STRCAST("proxyToString"), (PyCFunction)communicatorProxyToString, METH_VARARGS,
        PyDoc_STR(STRCAST("proxyToString(Ice.ObjectPrx) -> string")) },
    { STRCAST("stringToIdentity"), (PyCFunction)communicatorStringToIdentity, METH_VARARGS,
        PyDoc_STR(STRCAST("stringToIdentity(str) -> Ice.Identity")) },
    { STRCAST("identityToString"), (PyCFunction)communicatorIdentityToString, METH_VARARGS,
        PyDoc_STR(STRCAST("identityToString(Ice.Identity) -> string")) },
    { STRCAST("createObjectAdapter"), (PyCFunction)communicatorCreateObjectAdapter, METH_VARARGS,
        PyDoc_STR(STRCAST("createObjectAdapter(name) -> Ice.ObjectAdapter")) },
    { STRCAST("createObjectAdapterWithEndpoints"), (PyCFunction)communicatorCreateObjectAdapterWithEndpoints,
	METH_VARARGS,
	PyDoc_STR(STRCAST("createObjectAdapterWithEndpoints(name, endpoints) -> Ice.ObjectAdapter")) },
    { STRCAST("createObjectAdapterWithRouter"), (PyCFunction)communicatorCreateObjectAdapterWithRouter,
	METH_VARARGS,
	PyDoc_STR(STRCAST("createObjectAdapterWithRouter(name, router) -> Ice.ObjectAdapter")) },
    { STRCAST("addObjectFactory"), (PyCFunction)communicatorAddObjectFactory, METH_VARARGS,
        PyDoc_STR(STRCAST("addObjectFactory(factory, id) -> None")) },
    { STRCAST("removeObjectFactory"), (PyCFunction)communicatorRemoveObjectFactory, METH_VARARGS,
        PyDoc_STR(STRCAST("removeObjectFactory(id) -> None")) },
    { STRCAST("findObjectFactory"), (PyCFunction)communicatorFindObjectFactory, METH_VARARGS,
        PyDoc_STR(STRCAST("findObjectFactory(id) -> Ice.ObjectFactory")) },
    { STRCAST("getDefaultContext"), (PyCFunction)communicatorGetDefaultContext, METH_NOARGS,
        PyDoc_STR(STRCAST("getDefaultContext() -> Ice.Context")) },
    { STRCAST("getProperties"), (PyCFunction)communicatorGetProperties, METH_NOARGS,
        PyDoc_STR(STRCAST("getProperties() -> Ice.Properties")) },
    { STRCAST("getLogger"), (PyCFunction)communicatorGetLogger, METH_NOARGS,
        PyDoc_STR(STRCAST("getLogger() -> Ice.Logger")) },
    { STRCAST("getDefaultRouter"), (PyCFunction)communicatorGetDefaultRouter, METH_NOARGS,
        PyDoc_STR(STRCAST("getDefaultRouter() -> proxy")) },
    { STRCAST("setDefaultRouter"), (PyCFunction)communicatorSetDefaultRouter, METH_VARARGS,
        PyDoc_STR(STRCAST("setDefaultRouter(proxy) -> None")) },
    { STRCAST("getDefaultLocator"), (PyCFunction)communicatorGetDefaultLocator, METH_NOARGS,
        PyDoc_STR(STRCAST("getDefaultLocator() -> proxy")) },
    { STRCAST("setDefaultLocator"), (PyCFunction)communicatorSetDefaultLocator, METH_VARARGS,
        PyDoc_STR(STRCAST("setDefaultLocator(proxy) -> None")) },
    { STRCAST("flushBatchRequests"), (PyCFunction)communicatorFlushBatchRequests, METH_NOARGS,
        PyDoc_STR(STRCAST("flushBatchRequests() -> None")) },
    { STRCAST("_setWrapper"), (PyCFunction)communicatorSetWrapper, METH_VARARGS,
        PyDoc_STR(STRCAST("internal function")) },
    { STRCAST("_getWrapper"), (PyCFunction)communicatorGetWrapper, METH_NOARGS,
        PyDoc_STR(STRCAST("internal function")) },
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
    STRCAST("IcePy.Communicator"),   /* tp_name */
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
    if(PyModule_AddObject(module, STRCAST("Communicator"), (PyObject*)&CommunicatorType) < 0)
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
    CommunicatorMap::iterator p = _communicatorMap.find(communicator);
    if(p != _communicatorMap.end())
    {
	Py_INCREF(p->second);
	return p->second;
    }

    CommunicatorObject* obj = communicatorNew(NULL);
    if(obj != NULL)
    {
        obj->communicator = new Ice::CommunicatorPtr(communicator);
    }
    return (PyObject*)obj;
}

PyObject*
IcePy::getCommunicatorWrapper(const Ice::CommunicatorPtr& communicator)
{
    CommunicatorMap::iterator p = _communicatorMap.find(communicator);
    assert(p != _communicatorMap.end());
    CommunicatorObject* obj = (CommunicatorObject*)p->second;
    Py_INCREF(obj->wrapper);
    return obj->wrapper;
}
