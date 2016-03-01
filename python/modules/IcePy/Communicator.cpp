// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifdef _WIN32
#   include <IceUtil/Config.h>
#endif
#include <IceUtil/DisableWarnings.h>
#include <Communicator.h>
#include <BatchRequestInterceptor.h>
#include <ImplicitContext.h>
#include <Logger.h>
#include <ObjectAdapter.h>
#include <ObjectFactory.h>
#include <Operation.h>
#include <Properties.h>
#include <PropertiesAdmin.h>
#include <Proxy.h>
#include <Thread.h>
#include <Types.h>
#include <Util.h>
#include <Ice/Initialize.h>
#include <Ice/CommunicatorAsync.h>
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
communicatorNew(PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwds*/)
{
    assert(type && type->tp_alloc);
    CommunicatorObject* self = reinterpret_cast<CommunicatorObject*>(type->tp_alloc(type, 0));
    if(!self)
    {
        return 0;
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
    PyObject* argList = 0;
    PyObject* initData = 0;
    if(!PyArg_ParseTuple(args, STRCAST("|OO"), &argList, &initData))
    {
        return -1;
    }

    if(argList == Py_None)
    {
        argList = 0;
    }

    if(initData == Py_None)
    {
        initData = 0;
    }

    PyObject* initDataType = lookupType("Ice.InitializationData");

    if(argList && !initData)
    {
        if(PyObject_IsInstance(argList, initDataType))
        {
            initData = argList;
            argList = 0;
        }
        else if(!PyList_Check(argList))
        {
            PyErr_Format(PyExc_ValueError, STRCAST("initialize expects an argument list or Ice.InitializationData"));
            return -1;
        }
    }
    else if(argList && initData)
    {
        if(!PyList_Check(argList) || !PyObject_IsInstance(initData, initDataType))
        {
            PyErr_Format(PyExc_ValueError, STRCAST("initialize expects an argument list and Ice.InitializationData"));
            return -1;
        }
    }

    Ice::StringSeq seq;
    if(argList && !listToStringSeq(argList, seq))
    {
        return -1;
    }

    //
    // Use the with-args or the without-args version of initialize()?
    //
    bool hasArgs = argList != 0;

    Ice::InitializationData data;
    if(initData)
    {
        PyObjectHandle properties = PyObject_GetAttrString(initData, STRCAST("properties"));
        PyObjectHandle logger = PyObject_GetAttrString(initData, STRCAST("logger"));
        PyObjectHandle threadHook = PyObject_GetAttrString(initData, STRCAST("threadHook"));
        PyObjectHandle batchRequestInterceptor = PyObject_GetAttrString(initData, STRCAST("batchRequestInterceptor"));
        PyErr_Clear(); // PyObject_GetAttrString sets an error on failure.

        if(properties.get() && properties.get() != Py_None)
        {
            //
            // Get the properties implementation.
            //
            PyObjectHandle impl = PyObject_GetAttrString(properties.get(), STRCAST("_impl"));
            assert(impl.get());
            data.properties = getProperties(impl.get());
        }

        if(logger.get() && logger.get() != Py_None)
        {
            data.logger = new LoggerWrapper(logger.get());
        }

        if(threadHook.get() && threadHook.get() != Py_None)
        {
            data.threadHook = new ThreadHook(threadHook.get());
        }

        if(batchRequestInterceptor.get() && batchRequestInterceptor.get() != Py_None)
        {
            data.batchRequestInterceptor = new BatchRequestInterceptor(batchRequestInterceptor.get());
        }
    }

    try
    {
        if(argList)
        {
            data.properties = Ice::createProperties(seq, data.properties);
        }
        else if(!data.properties)
        {
            data.properties = Ice::createProperties();
        }
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return -1;
    }

    //
    // Remaining command line options are passed to the communicator
    // as an argument vector in case they contain plug-in properties.
    //
    int argc = static_cast<int>(seq.size());
    char** argv = new char*[argc + 1];
    int i = 0;
    for(Ice::StringSeq::const_iterator s = seq.begin(); s != seq.end(); ++s, ++i)
    {
        argv[i] = strdup(s->c_str());
    }
    argv[argc] = 0;

    data.compactIdResolver = new IdResolver;

    Ice::CommunicatorPtr communicator;
    try
    {
        AllowThreads allowThreads;
        if(hasArgs)
        {
            communicator = Ice::initialize(argc, argv, data);
        }
        else
        {
            communicator = Ice::initialize(data);
        }
    }
    catch(const Ice::Exception& ex)
    {
        for(i = 0; i < argc; ++i)
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
    if(argList)
    {
        PyList_SetSlice(argList, 0, PyList_Size(argList), 0); // Clear the list.

        for(i = 0; i < argc; ++i)
        {
            PyObjectHandle str = Py_BuildValue(STRCAST("s"), argv[i]);
            PyList_Append(argList, str.get());
        }
    }

    for(i = 0; i < argc; ++i)
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
    _communicatorMap.insert(CommunicatorMap::value_type(communicator, reinterpret_cast<PyObject*>(self)));

    return 0;
}

#ifdef WIN32
extern "C"
#endif
static void
communicatorDealloc(CommunicatorObject* self)
{
    if(self->communicator)
    {
        CommunicatorMap::iterator p = _communicatorMap.find(*self->communicator);
        //
        // find() can fail if an error occurred during communicator initialization.
        //
        if(p != _communicatorMap.end())
        {
            _communicatorMap.erase(p);
        }
    }

    if(self->shutdownThread)
    {
        (*self->shutdownThread)->getThreadControl().join();
    }
    delete self->communicator;
    delete self->shutdownMonitor;
    delete self->shutdownThread;
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
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
        return 0;
    }

    //
    // Break cyclic reference between this object and its Python wrapper.
    //
    Py_XDECREF(self->wrapper);
    self->wrapper = 0;

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
        AllowThreads allowThreads; // Release Python's global interpreter lock to avoid a potential deadlock.
        (*self->communicator)->shutdown();
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
        return 0;
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
                    PyRETURN_FALSE;
                }
            }
        }

        assert(self->shutdown);

        Ice::Exception* ex = (*self->shutdownThread)->getException();
        if(ex)
        {
            setPythonException(*ex);
            return 0;
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
            return 0;
        }
    }

    PyRETURN_TRUE;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
communicatorIsShutdown(CommunicatorObject* self)
{
    assert(self->communicator);
    bool isShutdown;
    try
    {
        isShutdown = (*self->communicator)->isShutdown();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    PyRETURN_BOOL(isShutdown);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
communicatorStringToProxy(CommunicatorObject* self, PyObject* args)
{
    PyObject* strObj;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &strObj))
    {
        return 0;
    }

    string str;
    if(!getStringArg(strObj, "str", str))
    {
        return 0;
    }

    assert(self->communicator);
    Ice::ObjectPrx proxy;
    try
    {
        proxy = (*self->communicator)->stringToProxy(str);
        if(proxy)
        {
            return createProxy(proxy, *self->communicator);
        }
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
communicatorProxyToString(CommunicatorObject* self, PyObject* args)
{
    PyObject* obj;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &obj))
    {
        return 0;
    }

    Ice::ObjectPrx proxy;
    if(!getProxyArg(obj, "proxyToString", "obj", proxy))
    {
        return 0;
    }

    string str;

    assert(self->communicator);
    try
    {
        str = (*self->communicator)->proxyToString(proxy);
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
communicatorPropertyToProxy(CommunicatorObject* self, PyObject* args)
{
    PyObject* strObj;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &strObj))
    {
        return 0;
    }

    string str;
    if(!getStringArg(strObj, "property", str))
    {
        return 0;
    }

    assert(self->communicator);
    Ice::ObjectPrx proxy;
    try
    {
        proxy = (*self->communicator)->propertyToProxy(str);
        if(proxy)
        {
            return createProxy(proxy, *self->communicator);
        }
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
communicatorProxyToProperty(CommunicatorObject* self, PyObject* args)
{
    //
    // We don't want to accept None here, so we can specify ProxyType and force
    // the caller to supply a proxy object.
    //
    PyObject* proxyObj;
    PyObject* strObj;
    if(!PyArg_ParseTuple(args, STRCAST("O!O"), &ProxyType, &proxyObj, &strObj))
    {
        return 0;
    }

    Ice::ObjectPrx proxy = getProxy(proxyObj);
    string str;
    if(!getStringArg(strObj, "property", str))
    {
        return 0;
    }

    assert(self->communicator);
    Ice::PropertyDict dict;
    try
    {
        dict = (*self->communicator)->proxyToProperty(proxy, str);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    PyObjectHandle result = PyDict_New();
    if(result.get())
    {
        for(Ice::PropertyDict::iterator p = dict.begin(); p != dict.end(); ++p)
        {
            PyObjectHandle key = createString(p->first);
            PyObjectHandle val = createString(p->second);
            if(!val.get() || PyDict_SetItem(result.get(), key.get(), val.get()) < 0)
            {
                return 0;
            }
        }
    }

    return result.release();
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
communicatorStringToIdentity(CommunicatorObject* self, PyObject* args)
{
    PyObject* strObj;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &strObj))
    {
        return 0;
    }

    string str;
    if(!getStringArg(strObj, "str", str))
    {
        return 0;
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
        return 0;
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
        return 0;
    }

    Ice::Identity id;
    if(!getIdentity(obj, id))
    {
        return 0;
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
        return 0;
    }

    return createString(str);
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
        AllowThreads allowThreads; // Release Python's global interpreter lock to avoid a potential deadlock.
        (*self->communicator)->flushBatchRequests();
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
communicatorBeginFlushBatchRequests(CommunicatorObject* self, PyObject* args, PyObject* kwds)
{
    assert(self->communicator);

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

    Ice::Callback_Communicator_flushBatchRequestsPtr cb;
    if(ex || sent)
    {
        FlushCallbackPtr d = new FlushCallback(ex, sent, "flushBatchRequests");
        cb = Ice::newCallback_Communicator_flushBatchRequests(d, &FlushCallback::exception, &FlushCallback::sent);
    }

    Ice::AsyncResultPtr result;
    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.

        if(cb)
        {
            result = (*self->communicator)->begin_flushBatchRequests(cb);
        }
        else
        {
            result = (*self->communicator)->begin_flushBatchRequests();
        }
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createAsyncResult(result, 0, 0, self->wrapper);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
communicatorEndFlushBatchRequests(CommunicatorObject* self, PyObject* args)
{
    assert(self->communicator);

    PyObject* result;
    if(!PyArg_ParseTuple(args, STRCAST("O!"), &AsyncResultType, &result))
    {
        return 0;
    }

    Ice::AsyncResultPtr r = getAsyncResult(result);
    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during blocking invocations.
        (*self->communicator)->end_flushBatchRequests(r);
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
communicatorCreateAdmin(CommunicatorObject* self, PyObject* args)
{
    PyObject* adapter;
    PyObject* identityType = lookupType("Ice.Identity");
    PyObject* id;
    if(!PyArg_ParseTuple(args, STRCAST("OO!"), &adapter, identityType, &id))
    {
        return 0;
    }

    Ice::ObjectAdapterPtr oa;

    PyObject* adapterType = lookupType("Ice.ObjectAdapter");
    if(adapter != Py_None && !PyObject_IsInstance(adapter, adapterType))
    {
        PyErr_Format(PyExc_ValueError, STRCAST("expected ObjectAdapter or None"));
        return 0;
    }

    if(adapter != Py_None)
    {
        oa = unwrapObjectAdapter(adapter);
    }

    Ice::Identity identity;
    if(!getIdentity(id, identity))
    {
        return 0;
    }

    assert(self->communicator);
    Ice::ObjectPrx proxy;
    try
    {
        proxy = (*self->communicator)->createAdmin(oa, identity);
        assert(proxy);

        return createProxy(proxy, *self->communicator);
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
communicatorGetAdmin(CommunicatorObject* self)
{
    assert(self->communicator);
    Ice::ObjectPrx proxy;
    try
    {
        proxy = (*self->communicator)->getAdmin();
        if(proxy)
        {
            return createProxy(proxy, *self->communicator);
        }
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
communicatorAddAdminFacet(CommunicatorObject* self, PyObject* args)
{
    PyObject* objectType = lookupType("Ice.Object");
    PyObject* servant;
    PyObject* facetObj;
    if(!PyArg_ParseTuple(args, STRCAST("O!O"), objectType, &servant, &facetObj))
    {
        return 0;
    }

    string facet;
    if(!getStringArg(facetObj, "facet", facet))
    {
        return 0;
    }

    ServantWrapperPtr wrapper = createServantWrapper(servant);
    if(PyErr_Occurred())
    {
        return 0;
    }

    assert(self->communicator);
    try
    {
        (*self->communicator)->addAdminFacet(wrapper, facet);
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
communicatorFindAdminFacet(CommunicatorObject* self, PyObject* args)
{
    PyObject* facetObj;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &facetObj))
    {
        return 0;
    }

    string facet;
    if(!getStringArg(facetObj, "facet", facet))
    {
        return 0;
    }

    assert(self->communicator);
    try
    {
        //
        // The facet being found may not be implemented by a Python servant
        // (e.g., it could be the Process or Properties facet), in which case
        // we return None.
        //
        Ice::ObjectPtr obj = (*self->communicator)->findAdminFacet(facet);
        if(obj)
        {
            ServantWrapperPtr wrapper = ServantWrapperPtr::dynamicCast(obj);
            if(wrapper)
            {
                return wrapper->getObject();
            }

            Ice::NativePropertiesAdminPtr props = Ice::NativePropertiesAdminPtr::dynamicCast(obj);
            if(props)
            {
                return createNativePropertiesAdmin(props);
            }

            // If the facet isn't supported in Python, just return an Ice.Object.
            PyTypeObject* objectType = reinterpret_cast<PyTypeObject*>(lookupType("Ice.Object"));
            return objectType->tp_alloc(objectType, 0);
        }
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
communicatorFindAllAdminFacets(CommunicatorObject* self)
{
    assert(self->communicator);
    Ice::FacetMap facetMap;
    try
    {
        facetMap = (*self->communicator)->findAllAdminFacets();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    PyObjectHandle result = PyDict_New();
    if(!result.get())
    {
        return 0;
    }

    PyTypeObject* objectType = reinterpret_cast<PyTypeObject*>(lookupType("Ice.Object"));
    PyObjectHandle plainObject = objectType->tp_alloc(objectType, 0);


    for(Ice::FacetMap::const_iterator p = facetMap.begin(); p != facetMap.end(); ++p)
    {

        PyObjectHandle obj = plainObject;

        ServantWrapperPtr wrapper = ServantWrapperPtr::dynamicCast(p->second);
        if(wrapper)
        {
            obj = wrapper->getObject();
        }
        else
        {
            Ice::NativePropertiesAdminPtr props = Ice::NativePropertiesAdminPtr::dynamicCast(p->second);
            if(props)
            {
                obj = createNativePropertiesAdmin(props);
            }
        }

        if(PyDict_SetItemString(result.get(), const_cast<char*>(p->first.c_str()), obj.get()) < 0)
        {
            return 0;
        }
    }

    return result.release();
}



#ifdef WIN32
extern "C"
#endif
static PyObject*
communicatorRemoveAdminFacet(CommunicatorObject* self, PyObject* args)
{
    PyObject* facetObj;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &facetObj))
    {
        return 0;
    }

    string facet;
    if(!getStringArg(facetObj, "facet", facet))
    {
        return 0;
    }

    assert(self->communicator);
    try
    {
        //
        // The facet being removed may not be implemented by a Python servant
        // (e.g., it could be the Process or Properties facet), in which case
        // we return None.
        //
        Ice::ObjectPtr obj = (*self->communicator)->removeAdminFacet(facet);
        assert(obj);
        ServantWrapperPtr wrapper = ServantWrapperPtr::dynamicCast(obj);
        if(wrapper)
        {
            return wrapper->getObject();
        }
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
communicatorSetWrapper(CommunicatorObject* self, PyObject* args)
{
    PyObject* wrapper;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &wrapper))
    {
        return 0;
    }

    assert(!self->wrapper);
    self->wrapper = wrapper;
    Py_INCREF(self->wrapper);

    Py_INCREF(Py_None);
    return Py_None;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
communicatorGetWrapper(CommunicatorObject* self)
{
    assert(self->wrapper);
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
        return 0;
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
        return 0;
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
    assert(factoryType);

    PyObject* factory;
    PyObject* strObj;
    if(!PyArg_ParseTuple(args, STRCAST("O!O"), factoryType, &factory, &strObj))
    {
        return 0;
    }

    string id;
    if(!getStringArg(strObj, "id", id))
    {
        return 0;
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
        return 0;

    }

    if(!pof->add(factory, id))
    {
        return 0;
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
    PyObject* strObj;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &strObj))
    {
        return 0;
    }

    string id;
    if(!getStringArg(strObj, "id", id))
    {
        return 0;
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
        return 0;
    }

    return pof->find(id);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
communicatorGetImplicitContext(CommunicatorObject* self)
{
    Ice::ImplicitContextPtr implicitContext = (*self->communicator)->getImplicitContext();

    if(implicitContext == 0)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    return createImplicitContext(implicitContext);
}


#ifdef WIN32
extern "C"
#endif
static PyObject*
communicatorCreateObjectAdapter(CommunicatorObject* self, PyObject* args)
{
    PyObject* strObj;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &strObj))
    {
        return 0;
    }

    string name;
    if(!getStringArg(strObj, "name", name))
    {
        return 0;
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
        return 0;
    }

    PyObject* obj = createObjectAdapter(adapter);
    if(!obj)
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
    PyObject* nameObj;
    PyObject* endpointsObj;
    if(!PyArg_ParseTuple(args, STRCAST("OO"), &nameObj, &endpointsObj))
    {
        return 0;
    }

    string name;
    string endpoints;
    if(!getStringArg(nameObj, "name", name))
    {
        return 0;
    }
    if(!getStringArg(endpointsObj, "endpoints", endpoints))
    {
        return 0;
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
        return 0;
    }

    PyObject* obj = createObjectAdapter(adapter);
    if(!obj)
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
    PyObject* nameObj;
    PyObject* p;
    if(!PyArg_ParseTuple(args, STRCAST("OO"), &nameObj, &p))
    {
        return 0;
    }

    string name;
    if(!getStringArg(nameObj, "name", name))
    {
        return 0;
    }

    Ice::ObjectPrx proxy;
    if(!getProxyArg(p, "createObjectAdapterWithRouter", "rtr", proxy, "Ice.RouterPrx"))
    {
        return 0;
    }

    Ice::RouterPrx router = Ice::RouterPrx::uncheckedCast(proxy);

    assert(self->communicator);
    Ice::ObjectAdapterPtr adapter;
    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock to avoid a potential deadlock.
        adapter = (*self->communicator)->createObjectAdapterWithRouter(name, router);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    PyObject* obj = createObjectAdapter(adapter);
    if(!obj)
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
        return 0;
    }

    if(!router)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    PyObject* routerProxyType = lookupType("Ice.RouterPrx");
    assert(routerProxyType);
    return createProxy(router, *self->communicator, routerProxyType);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
communicatorSetDefaultRouter(CommunicatorObject* self, PyObject* args)
{
    PyObject* p;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &p))
    {
        return 0;
    }

    Ice::ObjectPrx proxy;
    if(!getProxyArg(p, "setDefaultRouter", "rtr", proxy, "Ice.RouterPrx"))
    {
        return 0;
    }

    Ice::RouterPrx router = Ice::RouterPrx::uncheckedCast(proxy);

    assert(self->communicator);
    try
    {
        (*self->communicator)->setDefaultRouter(router);
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
        return 0;
    }

    if(!locator)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    PyObject* locatorProxyType = lookupType("Ice.LocatorPrx");
    assert(locatorProxyType);
    return createProxy(locator, *self->communicator, locatorProxyType);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
communicatorSetDefaultLocator(CommunicatorObject* self, PyObject* args)
{
    PyObject* p;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &p))
    {
        return 0;
    }

    Ice::ObjectPrx proxy;
    if(!getProxyArg(p, "setDefaultLocator", "loc", proxy, "Ice.LocatorPrx"))
    {
        return 0;
    }

    Ice::LocatorPrx locator = Ice::LocatorPrx::uncheckedCast(proxy);

    assert(self->communicator);
    try
    {
        (*self->communicator)->setDefaultLocator(locator);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef CommunicatorMethods[] =
{
    { STRCAST("destroy"), reinterpret_cast<PyCFunction>(communicatorDestroy), METH_NOARGS,
        PyDoc_STR(STRCAST("destroy() -> None")) },
    { STRCAST("shutdown"), reinterpret_cast<PyCFunction>(communicatorShutdown), METH_NOARGS,
        PyDoc_STR(STRCAST("shutdown() -> None")) },
    { STRCAST("waitForShutdown"), reinterpret_cast<PyCFunction>(communicatorWaitForShutdown), METH_VARARGS,
        PyDoc_STR(STRCAST("waitForShutdown() -> None")) },
    { STRCAST("isShutdown"), reinterpret_cast<PyCFunction>(communicatorIsShutdown), METH_NOARGS,
        PyDoc_STR(STRCAST("isShutdown() -> bool")) },
    { STRCAST("stringToProxy"), reinterpret_cast<PyCFunction>(communicatorStringToProxy), METH_VARARGS,
        PyDoc_STR(STRCAST("stringToProxy(str) -> Ice.ObjectPrx")) },
    { STRCAST("proxyToString"), reinterpret_cast<PyCFunction>(communicatorProxyToString), METH_VARARGS,
        PyDoc_STR(STRCAST("proxyToString(Ice.ObjectPrx) -> string")) },
    { STRCAST("propertyToProxy"), reinterpret_cast<PyCFunction>(communicatorPropertyToProxy), METH_VARARGS,
        PyDoc_STR(STRCAST("propertyToProxy(str) -> Ice.ObjectPrx")) },
    { STRCAST("proxyToProperty"), reinterpret_cast<PyCFunction>(communicatorProxyToProperty), METH_VARARGS,
        PyDoc_STR(STRCAST("proxyToProperty(Ice.ObjectPrx, str) -> dict")) },
    { STRCAST("stringToIdentity"), reinterpret_cast<PyCFunction>(communicatorStringToIdentity), METH_VARARGS,
        PyDoc_STR(STRCAST("stringToIdentity(str) -> Ice.Identity")) },
    { STRCAST("identityToString"), reinterpret_cast<PyCFunction>(communicatorIdentityToString), METH_VARARGS,
        PyDoc_STR(STRCAST("identityToString(Ice.Identity) -> string")) },
    { STRCAST("createObjectAdapter"), reinterpret_cast<PyCFunction>(communicatorCreateObjectAdapter), METH_VARARGS,
        PyDoc_STR(STRCAST("createObjectAdapter(name) -> Ice.ObjectAdapter")) },
    { STRCAST("createObjectAdapterWithEndpoints"),
        reinterpret_cast<PyCFunction>(communicatorCreateObjectAdapterWithEndpoints), METH_VARARGS,
        PyDoc_STR(STRCAST("createObjectAdapterWithEndpoints(name, endpoints) -> Ice.ObjectAdapter")) },
    { STRCAST("createObjectAdapterWithRouter"),
        reinterpret_cast<PyCFunction>(communicatorCreateObjectAdapterWithRouter), METH_VARARGS,
        PyDoc_STR(STRCAST("createObjectAdapterWithRouter(name, router) -> Ice.ObjectAdapter")) },
    { STRCAST("addObjectFactory"), reinterpret_cast<PyCFunction>(communicatorAddObjectFactory), METH_VARARGS,
        PyDoc_STR(STRCAST("addObjectFactory(factory, id) -> None")) },
    { STRCAST("findObjectFactory"), reinterpret_cast<PyCFunction>(communicatorFindObjectFactory), METH_VARARGS,
        PyDoc_STR(STRCAST("findObjectFactory(id) -> Ice.ObjectFactory")) },
    { STRCAST("getImplicitContext"), reinterpret_cast<PyCFunction>(communicatorGetImplicitContext), METH_NOARGS,
      PyDoc_STR(STRCAST("getImplicitContext() -> Ice.ImplicitContext")) },
    { STRCAST("getProperties"), reinterpret_cast<PyCFunction>(communicatorGetProperties), METH_NOARGS,
        PyDoc_STR(STRCAST("getProperties() -> Ice.Properties")) },
    { STRCAST("getLogger"), reinterpret_cast<PyCFunction>(communicatorGetLogger), METH_NOARGS,
        PyDoc_STR(STRCAST("getLogger() -> Ice.Logger")) },
    { STRCAST("getDefaultRouter"), reinterpret_cast<PyCFunction>(communicatorGetDefaultRouter), METH_NOARGS,
        PyDoc_STR(STRCAST("getDefaultRouter() -> proxy")) },
    { STRCAST("setDefaultRouter"), reinterpret_cast<PyCFunction>(communicatorSetDefaultRouter), METH_VARARGS,
        PyDoc_STR(STRCAST("setDefaultRouter(proxy) -> None")) },
    { STRCAST("getDefaultLocator"), reinterpret_cast<PyCFunction>(communicatorGetDefaultLocator), METH_NOARGS,
        PyDoc_STR(STRCAST("getDefaultLocator() -> proxy")) },
    { STRCAST("setDefaultLocator"), reinterpret_cast<PyCFunction>(communicatorSetDefaultLocator), METH_VARARGS,
        PyDoc_STR(STRCAST("setDefaultLocator(proxy) -> None")) },
    { STRCAST("flushBatchRequests"), reinterpret_cast<PyCFunction>(communicatorFlushBatchRequests), METH_NOARGS,
        PyDoc_STR(STRCAST("flushBatchRequests() -> None")) },
    { STRCAST("begin_flushBatchRequests"), reinterpret_cast<PyCFunction>(communicatorBeginFlushBatchRequests),
        METH_VARARGS | METH_KEYWORDS,
        PyDoc_STR(STRCAST("begin_flushBatchRequests([_ex][, _sent]) -> Ice.AsyncResult")) },
    { STRCAST("end_flushBatchRequests"), reinterpret_cast<PyCFunction>(communicatorEndFlushBatchRequests),
        METH_VARARGS, PyDoc_STR(STRCAST("end_flushBatchRequests(Ice.AsyncResult) -> None")) },
    { STRCAST("createAdmin"), reinterpret_cast<PyCFunction>(communicatorCreateAdmin), METH_VARARGS,
        PyDoc_STR(STRCAST("createAdmin(adminAdapter, adminIdentity) -> Ice.ObjectPrx")) },
    { STRCAST("getAdmin"), reinterpret_cast<PyCFunction>(communicatorGetAdmin), METH_NOARGS,
        PyDoc_STR(STRCAST("getAdmin() -> Ice.ObjectPrx")) },
    { STRCAST("addAdminFacet"), reinterpret_cast<PyCFunction>(communicatorAddAdminFacet), METH_VARARGS,
        PyDoc_STR(STRCAST("addAdminFacet(servant, facet) -> None")) },
    { STRCAST("findAdminFacet"), reinterpret_cast<PyCFunction>(communicatorFindAdminFacet), METH_VARARGS,
        PyDoc_STR(STRCAST("findAdminFacet(facet) -> Ice.Object")) },
    { STRCAST("findAllAdminFacets"), reinterpret_cast<PyCFunction>(communicatorFindAllAdminFacets), METH_NOARGS,
      PyDoc_STR(STRCAST("findAllAdminFacets() -> dictionary")) },
    { STRCAST("removeAdminFacet"), reinterpret_cast<PyCFunction>(communicatorRemoveAdminFacet), METH_VARARGS,
        PyDoc_STR(STRCAST("removeAdminFacet(facet) -> Ice.Object")) },
    { STRCAST("_setWrapper"), reinterpret_cast<PyCFunction>(communicatorSetWrapper), METH_VARARGS,
        PyDoc_STR(STRCAST("internal function")) },
    { STRCAST("_getWrapper"), reinterpret_cast<PyCFunction>(communicatorGetWrapper), METH_NOARGS,
        PyDoc_STR(STRCAST("internal function")) },
    { 0, 0 } /* sentinel */
};

namespace IcePy
{

PyTypeObject CommunicatorType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyVarObject_HEAD_INIT(0, 0)
    STRCAST("IcePy.Communicator"),   /* tp_name */
    sizeof(CommunicatorObject),      /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    reinterpret_cast<destructor>(communicatorDealloc), /* tp_dealloc */
    0,                               /* tp_print */
    0,                               /* tp_getattr */
    0,                               /* tp_setattr */
    0,                               /* tp_reserved */
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
    reinterpret_cast<initproc>(communicatorInit), /* tp_init */
    0,                               /* tp_alloc */
    reinterpret_cast<newfunc>(communicatorNew), /* tp_new */
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
    PyTypeObject* type = &CommunicatorType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("Communicator"), reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    return true;
}

Ice::CommunicatorPtr
IcePy::getCommunicator(PyObject* obj)
{
    assert(PyObject_IsInstance(obj, reinterpret_cast<PyObject*>(&CommunicatorType)));
    CommunicatorObject* cobj = reinterpret_cast<CommunicatorObject*>(obj);
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

    CommunicatorObject* obj = communicatorNew(&CommunicatorType, 0, 0);
    if(obj)
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
    CommunicatorObject* obj = reinterpret_cast<CommunicatorObject*>(p->second);
    Py_INCREF(obj->wrapper);
    return obj->wrapper;
}

extern "C"
PyObject*
IcePy_identityToString(PyObject* /*self*/, PyObject* obj)
{
    Ice::Identity id;
    if(!getIdentity(obj, id))
    {
        return 0;
    }
    
    string str;

    try
    {
        str = Ice::identityToString(id);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createString(str);
}

extern "C"
PyObject*
IcePy_stringToIdentity(PyObject* /*self*/, PyObject* obj)
{
    string str;
    if(!getStringArg(obj, "str", str))
    {
        return 0;
    }

    Ice::Identity id;
    try
    {
        id = Ice::stringToIdentity(str);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createIdentity(id);
}
