// Copyright (c) ZeroC, Inc.

#include "Communicator.h"
#include "BatchRequestInterceptor.h"
#include "Executor.h"
#include "Future.h"
#include "Ice/DisableWarnings.h"
#include "Ice/Initialize.h"
#include "Ice/LocalExceptions.h"
#include "Ice/Locator.h"
#include "Ice/ObjectAdapter.h"
#include "Ice/Properties.h"
#include "Ice/Router.h"
#include "Ice/ValueFactory.h"
#include "ImplicitContext.h"
#include "Logger.h"
#include "ObjectAdapter.h"
#include "Operation.h"
#include "Properties.h"
#include "PropertiesAdmin.h"
#include "Proxy.h"
#include "Thread.h"
#include "Types.h"
#include "Util.h"
#include "ValueFactoryManager.h"

#include <pythread.h>

#include <future>

using namespace std;
using namespace IcePy;

#if defined(__GNUC__) && ((__GNUC__ >= 8))
#    pragma GCC diagnostic ignored "-Wcast-function-type"
#endif

static unsigned long mainThreadId;

using CommunicatorMap = map<Ice::CommunicatorPtr, PyObject*>;
static CommunicatorMap communicatorMap;

namespace IcePy
{
    struct CommunicatorObject
    {
        PyObject_HEAD Ice::CommunicatorPtr* communicator;
        PyObject* wrapper;
        std::future<void>* shutdownFuture;
        std::exception_ptr* shutdownException;
        bool shutdown;
        ExecutorPtr* executor;
    };
}

extern "C" CommunicatorObject*
communicatorNew(PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwds*/)
{
    assert(type && type->tp_alloc);
    auto* self = reinterpret_cast<CommunicatorObject*>(type->tp_alloc(type, 0));
    if (!self)
    {
        return nullptr;
    }
    self->communicator = nullptr;
    self->wrapper = nullptr;
    self->shutdownFuture = nullptr;
    self->shutdownException = nullptr;
    self->shutdown = false;
    self->executor = nullptr;
    return self;
}

// Only called by Ice.Initialize
// IcePy.Communicator(args: [str]|None, initData: Ice.InitializationData|None, configFile: str|None)

extern "C" int
communicatorInit(CommunicatorObject* self, PyObject* args, PyObject* /*kwds*/)
{
    PyObject* argList{nullptr};
    PyObject* initData{nullptr};
    PyObject* configFile{nullptr};
    if (!PyArg_ParseTuple(args, "OOO", &argList, &initData, &configFile))
    {
        return -1;
    }

    Ice::StringSeq seq;
    if (argList != Py_None && !listToStringSeq(argList, seq))
    {
        return -1;
    }

    Ice::InitializationData data;
    ExecutorPtr executorWrapper;

    try
    {
        if (initData != Py_None)
        {
            PyObjectHandle properties{getAttr(initData, "properties", false)};
            PyObjectHandle logger{getAttr(initData, "logger", false)};
            PyObjectHandle threadStart{getAttr(initData, "threadStart", false)};
            PyObjectHandle threadStop{getAttr(initData, "threadStop", false)};
            PyObjectHandle batchRequestInterceptor{getAttr(initData, "batchRequestInterceptor", false)};
            PyObjectHandle executor{getAttr(initData, "executor", false)};

            if (properties.get())
            {
                //
                // Get the properties implementation.
                //
                PyObjectHandle impl{getAttr(properties.get(), "_impl", false)};
                assert(impl.get());
                data.properties = getProperties(impl.get());
            }

            if (logger.get())
            {
                data.logger = make_shared<LoggerWrapper>(logger.get());
            }

            if (threadStart.get() || threadStop.get())
            {
                auto threadHook = make_shared<ThreadHook>(threadStart.get(), threadStop.get());
                data.threadStart = [threadHook]() { threadHook->start(); };
                data.threadStop = [threadHook]() { threadHook->stop(); };
            }

            if (executor.get())
            {
                executorWrapper = make_shared<Executor>(executor.get());
                data.executor = [executorWrapper](function<void()> call, const shared_ptr<Ice::Connection>& connection)
                { executorWrapper->execute(std::move(call), connection); };
            }

            if (batchRequestInterceptor.get())
            {
                auto batchRequestInterceptorWrapper =
                    make_shared<BatchRequestInterceptorWrapper>(batchRequestInterceptor.get());
                data.batchRequestInterceptor =
                    [batchRequestInterceptorWrapper](const Ice::BatchRequest& req, int count, int size)
                { batchRequestInterceptorWrapper->enqueue(req, count, size); };
            }
        }

        // We always supply our own implementation of ValueFactoryManager.
        data.valueFactoryManager = ValueFactoryManager::create();

        if (!data.properties)
        {
            data.properties = Ice::createProperties();
        }

        if (configFile != Py_None)
        {
            data.properties->load(getString(configFile));
        }

        if (argList != Py_None)
        {
            data.properties = Ice::createProperties(seq, data.properties);
        }
    }
    catch (...)
    {
        setPythonException(current_exception());
        return -1;
    }

    data.compactIdResolver = resolveCompactId;

    // Always accept cycles in Python
    data.properties->setProperty("Ice.AcceptClassCycles", "1");

    Ice::CommunicatorPtr communicator;
    try
    {
        AllowThreads allowThreads;
        if (argList != Py_None)
        {
            // Remaining command line options are passed to the communicator as an argument vector in case they
            // contain plug-in properties.
            communicator = Ice::initialize(seq, data);
        }
        else
        {
            communicator = Ice::initialize(data);
        }
    }
    catch (...)
    {
        setPythonException(current_exception());
        return -1;
    }

    //
    // Replace the contents of the given argument list with the filtered arguments.
    //
    if (argList != Py_None)
    {
        // Clear the list.
        if (PyList_SetSlice(argList, 0, PY_SSIZE_T_MAX, nullptr) == -1)
        {
            return -1;
        }

        for (const string& arg : seq)
        {
            PyObjectHandle str{Py_BuildValue("s", arg.c_str())};
            PyList_Append(argList, str.get());
        }
    }

    self->communicator = new Ice::CommunicatorPtr(communicator);
    communicatorMap.insert(CommunicatorMap::value_type(communicator, reinterpret_cast<PyObject*>(self)));

    if (executorWrapper)
    {
        self->executor = new ExecutorPtr(executorWrapper);
        executorWrapper->setCommunicator(communicator);
    }

    return 0;
}

extern "C" void
communicatorDealloc(CommunicatorObject* self)
{
    if (self->communicator)
    {
        auto p = communicatorMap.find(*self->communicator);
        //
        // find() can fail if an error occurred during communicator initialization.
        //
        if (p != communicatorMap.end())
        {
            communicatorMap.erase(p);
        }
    }

    delete self->communicator;
    delete self->shutdownException;
    delete self->shutdownFuture;
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

extern "C" PyObject*
communicatorDestroy(CommunicatorObject* self, PyObject* /*args*/)
{
    assert(self->communicator);

    auto vfm = dynamic_pointer_cast<ValueFactoryManager>((*self->communicator)->getValueFactoryManager());
    assert(vfm);

    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock to avoid a potential deadlock.
        (*self->communicator)->destroy();
    }
    catch (...)
    {
        setPythonException(current_exception());
    }

    vfm->destroy();

    if (self->executor)
    {
        (*self->executor)->setCommunicator(nullptr); // Break cyclic reference.
    }

    //
    // Break cyclic reference between this object and its Python wrapper.
    //
    Py_XDECREF(self->wrapper);
    self->wrapper = nullptr;

    if (PyErr_Occurred())
    {
        return nullptr;
    }
    else
    {
        return Py_None;
    }
}

extern "C" PyObject*
communicatorShutdown(CommunicatorObject* self, PyObject* /*args*/)
{
    assert(self->communicator);
    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock to avoid a potential deadlock.
        (*self->communicator)->shutdown();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return Py_None;
}

extern "C" PyObject*
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
    if (!PyArg_ParseTuple(args, "i", &timeout))
    {
        return nullptr;
    }

    assert(timeout > 0);
    assert(self->communicator);

    //
    // Do not call waitForShutdown from the main thread, because it prevents
    // signals (such as keyboard interrupts) from being delivered to Python.
    //
    if (PyThread_get_thread_ident() == mainThreadId)
    {
        if (!self->shutdown)
        {
            if (self->shutdownFuture == nullptr)
            {
                self->shutdownFuture = new std::future<void>();
                *self->shutdownFuture =
                    std::async(std::launch::async, [&self] { (*self->communicator)->waitForShutdown(); });
            }

            {
                AllowThreads allowThreads; // Release Python's global interpreter lock during blocking calls.
                if (self->shutdownFuture->wait_for(std::chrono::milliseconds(timeout)) == std::future_status::timeout)
                {
                    return Py_False;
                }
            }

            self->shutdown = true;
            try
            {
                self->shutdownFuture->get();
            }
            catch (...)
            {
                self->shutdownException = new std::exception_ptr(std::current_exception());
            }
        }

        assert(self->shutdown);
        if (self->shutdownException)
        {
            setPythonException(*self->shutdownException);
            return nullptr;
        }
    }
    else
    {
        try
        {
            AllowThreads allowThreads; // Release Python's global interpreter lock during blocking calls.
            (*self->communicator)->waitForShutdown();
        }
        catch (...)
        {
            setPythonException(current_exception());
            return nullptr;
        }
    }

    return Py_True;
}

extern "C" PyObject*
communicatorShutdownCompleted(CommunicatorObject* self, PyObject* /*args*/)
{
    assert(self->communicator);

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

    // Create a strong reference to prevent premature release of the future object. The reference will be released by
    // the callback.
    PyObject* futureObject = future.get();
    Py_INCREF(futureObject);

    (*self->communicator)
        ->waitForShutdownAsync(
            [futureObject]()
            {
                AdoptThread adoptThread; // Ensure the current thread is able to call into Python.
                // Adopt the future object so it is released within this scope.
                PyObjectHandle futureGuard{futureObject};
                PyObjectHandle discard{callMethod(futureGuard.get(), "set_result", Py_None)};
            });

    return IcePy::wrapFuture(*self->communicator, future.get());
}

extern "C" PyObject*
communicatorIsShutdown(CommunicatorObject* self, PyObject* /*args*/)
{
    assert(self->communicator);
    bool isShutdown;
    try
    {
        isShutdown = (*self->communicator)->isShutdown();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return isShutdown ? Py_True : Py_False;
}

extern "C" PyObject*
communicatorStringToProxy(CommunicatorObject* self, PyObject* args)
{
    PyObject* strObj;
    if (!PyArg_ParseTuple(args, "O", &strObj))
    {
        return nullptr;
    }

    string str;
    if (!getStringArg(strObj, "str", str))
    {
        return nullptr;
    }

    assert(self->communicator);
    optional<Ice::ObjectPrx> proxy;
    try
    {
        proxy = (*self->communicator)->stringToProxy(str);
        if (proxy)
        {
            return createProxy(proxy.value(), *self->communicator);
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
communicatorProxyToString(CommunicatorObject* self, PyObject* args)
{
    PyObject* obj;
    if (!PyArg_ParseTuple(args, "O", &obj))
    {
        return nullptr;
    }

    optional<Ice::ObjectPrx> proxy;
    if (!getProxyArg(obj, "proxyToString", "obj", proxy))
    {
        return nullptr;
    }

    string str;

    assert(self->communicator);
    try
    {
        str = (*self->communicator)->proxyToString(proxy);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createString(str);
}

extern "C" PyObject*
communicatorPropertyToProxy(CommunicatorObject* self, PyObject* args)
{
    PyObject* strObj;
    if (!PyArg_ParseTuple(args, "O", &strObj))
    {
        return nullptr;
    }

    string str;
    if (!getStringArg(strObj, "property", str))
    {
        return nullptr;
    }

    assert(self->communicator);
    optional<Ice::ObjectPrx> proxy;
    try
    {
        proxy = (*self->communicator)->propertyToProxy(str);
        if (proxy)
        {
            return createProxy(proxy.value(), *self->communicator);
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
communicatorProxyToProperty(CommunicatorObject* self, PyObject* args)
{
    //
    // We don't want to accept None here, so we can specify ProxyType and force
    // the caller to supply a proxy object.
    //
    PyObject* proxyObj;
    PyObject* strObj;
    if (!PyArg_ParseTuple(args, "O!O", &ProxyType, &proxyObj, &strObj))
    {
        return nullptr;
    }

    Ice::ObjectPrx proxy = getProxy(proxyObj);
    string str;
    if (!getStringArg(strObj, "property", str))
    {
        return nullptr;
    }

    assert(self->communicator);
    Ice::PropertyDict dict;
    try
    {
        dict = (*self->communicator)->proxyToProperty(proxy, str);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    PyObjectHandle result{PyDict_New()};
    if (result.get())
    {
        for (const auto& [key, value] : dict)
        {
            PyObjectHandle pyKey{createString(key)};
            PyObjectHandle pyValue{createString(value)};
            if (!pyValue.get() || PyDict_SetItem(result.get(), pyKey.get(), pyValue.get()) < 0)
            {
                return nullptr;
            }
        }
    }

    return result.release();
}

extern "C" PyObject*
communicatorIdentityToString(CommunicatorObject* self, PyObject* args)
{
    PyObject* identityType = lookupType("Ice.Identity");
    PyObject* obj;
    if (!PyArg_ParseTuple(args, "O!", identityType, &obj))
    {
        return nullptr;
    }

    Ice::Identity id;
    if (!getIdentity(obj, id))
    {
        return nullptr;
    }
    string str;

    assert(self->communicator);
    try
    {
        str = (*self->communicator)->identityToString(id);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createString(str);
}

extern "C" PyObject*
communicatorFlushBatchRequests(CommunicatorObject* self, PyObject* args)
{
    PyObject* compressBatchType = lookupType("Ice.CompressBatch");
    PyObject* compressBatch;
    if (!PyArg_ParseTuple(args, "O!", compressBatchType, &compressBatch))
    {
        return nullptr;
    }

    PyObjectHandle v{getAttr(compressBatch, "_value", false)};
    assert(v.get());
    auto cb = static_cast<Ice::CompressBatch>(PyLong_AsLong(v.get()));

    assert(self->communicator);
    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock to avoid a potential deadlock.
        (*self->communicator)->flushBatchRequests(cb);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return Py_None;
}

extern "C" PyObject*
communicatorFlushBatchRequestsAsync(CommunicatorObject* self, PyObject* args, PyObject* /*kwds*/)
{
    PyObject* compressBatchType = lookupType("Ice.CompressBatch");
    PyObject* compressBatch;
    if (!PyArg_ParseTuple(args, "O!", compressBatchType, &compressBatch))
    {
        return nullptr;
    }

    PyObjectHandle v{getAttr(compressBatch, "_value", false)};
    assert(v.get());
    auto compress = static_cast<Ice::CompressBatch>(PyLong_AsLong(v.get()));

    assert(self->communicator);
    const string op = "flushBatchRequests";

    auto callback = make_shared<FlushAsyncCallback>(op);
    function<void()> cancel;
    try
    {
        cancel = (*self->communicator)
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

    PyObjectHandle future{createFuture(op, asyncInvocationContextObj.get())};
    if (!future.get())
    {
        return nullptr;
    }
    callback->setFuture(future.get());
    return IcePy::wrapFuture(*self->communicator, future.get());
}

extern "C" PyObject*
communicatorCreateAdmin(CommunicatorObject* self, PyObject* args)
{
    PyObject* adapter;
    PyObject* identityType = lookupType("Ice.Identity");
    PyObject* id;
    if (!PyArg_ParseTuple(args, "OO!", &adapter, identityType, &id))
    {
        return nullptr;
    }

    Ice::ObjectAdapterPtr oa;

    PyObject* adapterType = lookupType("Ice.ObjectAdapter");
    if (adapter != Py_None && !PyObject_IsInstance(adapter, adapterType))
    {
        PyErr_Format(PyExc_ValueError, "expected ObjectAdapter or None");
        return nullptr;
    }

    if (adapter != Py_None)
    {
        oa = unwrapObjectAdapter(adapter);
    }

    Ice::Identity identity;
    if (!getIdentity(id, identity))
    {
        return nullptr;
    }

    assert(self->communicator);
    optional<Ice::ObjectPrx> proxy;
    try
    {
        proxy = (*self->communicator)->createAdmin(oa, identity);
        assert(proxy);

        return createProxy(proxy.value(), *self->communicator);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }
}

extern "C" PyObject*
communicatorGetAdmin(CommunicatorObject* self, PyObject* /*args*/)
{
    assert(self->communicator);
    optional<Ice::ObjectPrx> proxy;
    try
    {
        proxy = (*self->communicator)->getAdmin();
        if (proxy)
        {
            return createProxy(proxy.value(), *self->communicator);
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
communicatorAddAdminFacet(CommunicatorObject* self, PyObject* args)
{
    PyObject* objectType = lookupType("Ice.Object");
    PyObject* servant;
    PyObject* facetObj;
    if (!PyArg_ParseTuple(args, "O!O", objectType, &servant, &facetObj))
    {
        return nullptr;
    }

    string facet;
    if (!getStringArg(facetObj, "facet", facet))
    {
        return nullptr;
    }

    ServantWrapperPtr wrapper = createServantWrapper(servant);
    if (PyErr_Occurred())
    {
        return nullptr;
    }

    assert(self->communicator);
    try
    {
        (*self->communicator)->addAdminFacet(wrapper, facet);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return Py_None;
}

extern "C" PyObject*
communicatorFindAdminFacet(CommunicatorObject* self, PyObject* args)
{
    PyObject* facetObj;
    if (!PyArg_ParseTuple(args, "O", &facetObj))
    {
        return nullptr;
    }

    string facet;
    if (!getStringArg(facetObj, "facet", facet))
    {
        return nullptr;
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
        if (obj)
        {
            ServantWrapperPtr wrapper = dynamic_pointer_cast<ServantWrapper>(obj);
            if (wrapper)
            {
                return wrapper->getObject();
            }

            Ice::NativePropertiesAdminPtr props = dynamic_pointer_cast<Ice::NativePropertiesAdmin>(obj);
            if (props)
            {
                return createNativePropertiesAdmin(props);
            }

            // If the facet isn't supported in Python, just return an Ice.Object.
            PyTypeObject* objectType = reinterpret_cast<PyTypeObject*>(lookupType("Ice.Object"));
            return objectType->tp_alloc(objectType, 0);
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
communicatorFindAllAdminFacets(CommunicatorObject* self, PyObject* /*args*/)
{
    assert(self->communicator);
    Ice::FacetMap facetMap;
    try
    {
        facetMap = (*self->communicator)->findAllAdminFacets();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    PyObjectHandle result{PyDict_New()};
    if (!result.get())
    {
        return nullptr;
    }

    PyTypeObject* objectType = reinterpret_cast<PyTypeObject*>(lookupType("Ice.Object"));
    PyObjectHandle plainObject{objectType->tp_alloc(objectType, 0)};

    for (const auto& [facet, servant] : facetMap)
    {
        PyObjectHandle obj = plainObject;

        ServantWrapperPtr wrapper = dynamic_pointer_cast<ServantWrapper>(servant);
        if (wrapper)
        {
            obj = wrapper->getObject();
        }
        else
        {
            Ice::NativePropertiesAdminPtr props = dynamic_pointer_cast<Ice::NativePropertiesAdmin>(servant);
            if (props)
            {
                obj = createNativePropertiesAdmin(props);
            }
        }

        if (PyDict_SetItemString(result.get(), const_cast<char*>(facet.c_str()), obj.get()) < 0)
        {
            return nullptr;
        }
    }

    return result.release();
}

extern "C" PyObject*
communicatorRemoveAdminFacet(CommunicatorObject* self, PyObject* args)
{
    PyObject* facetObj;
    if (!PyArg_ParseTuple(args, "O", &facetObj))
    {
        return nullptr;
    }

    string facet;
    if (!getStringArg(facetObj, "facet", facet))
    {
        return nullptr;
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
        ServantWrapperPtr wrapper = dynamic_pointer_cast<ServantWrapper>(obj);
        if (wrapper)
        {
            return wrapper->getObject();
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
communicatorSetWrapper(CommunicatorObject* self, PyObject* args)
{
    PyObject* wrapper;
    if (!PyArg_ParseTuple(args, "O", &wrapper))
    {
        return nullptr;
    }

    assert(!self->wrapper);
    self->wrapper = wrapper;
    Py_INCREF(self->wrapper);

    return Py_None;
}

extern "C" PyObject*
communicatorGetWrapper(CommunicatorObject* self, PyObject* /*args*/)
{
    assert(self->wrapper);
    Py_INCREF(self->wrapper);
    return self->wrapper;
}

extern "C" PyObject*
communicatorGetProperties(CommunicatorObject* self, PyObject* /*args*/)
{
    assert(self->communicator);
    Ice::PropertiesPtr properties;
    try
    {
        properties = (*self->communicator)->getProperties();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createProperties(properties);
}

extern "C" PyObject*
communicatorGetLogger(CommunicatorObject* self, PyObject* /*args*/)
{
    assert(self->communicator);
    Ice::LoggerPtr logger;
    try
    {
        logger = (*self->communicator)->getLogger();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    //
    // The communicator's logger can either be a C++ object (such as
    // the default logger supplied by the Ice run time), or a C++
    // wrapper around a Python implementation. If the latter, we
    // return it directly. Otherwise, we create a Python object
    // that delegates to the C++ object.
    //
    LoggerWrapperPtr wrapper = dynamic_pointer_cast<LoggerWrapper>(logger);
    if (wrapper)
    {
        PyObject* obj = wrapper->getObject();
        Py_INCREF(obj);
        return obj;
    }

    return createLogger(logger);
}

extern "C" PyObject*
communicatorGetValueFactoryManager(CommunicatorObject* self, PyObject* /*args*/)
{
    auto vfm = dynamic_pointer_cast<ValueFactoryManager>((*self->communicator)->getValueFactoryManager());
    return vfm->getObject();
}

extern "C" PyObject*
communicatorGetImplicitContext(CommunicatorObject* self, PyObject* /*args*/)
{
    Ice::ImplicitContextPtr implicitContext = (*self->communicator)->getImplicitContext();

    if (!implicitContext)
    {
        return Py_None;
    }

    return createImplicitContext(implicitContext);
}

extern "C" PyObject*
communicatorCreateObjectAdapter(CommunicatorObject* self, PyObject* args)
{
    PyObject* strObj;
    if (!PyArg_ParseTuple(args, "O", &strObj))
    {
        return nullptr;
    }

    string name;
    if (!getStringArg(strObj, "name", name))
    {
        return nullptr;
    }

    assert(self->communicator);
    Ice::ObjectAdapterPtr adapter;
    try
    {
        adapter = (*self->communicator)->createObjectAdapter(name);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    PyObject* obj = createObjectAdapter(adapter);
    if (!obj)
    {
        try
        {
            adapter->deactivate();
        }
        catch (const Ice::Exception&)
        {
        }
    }

    return obj;
}

extern "C" PyObject*
communicatorCreateObjectAdapterWithEndpoints(CommunicatorObject* self, PyObject* args)
{
    PyObject* nameObj;
    PyObject* endpointsObj;
    if (!PyArg_ParseTuple(args, "OO", &nameObj, &endpointsObj))
    {
        return nullptr;
    }

    string name;
    string endpoints;
    if (!getStringArg(nameObj, "name", name))
    {
        return nullptr;
    }
    if (!getStringArg(endpointsObj, "endpoints", endpoints))
    {
        return nullptr;
    }

    assert(self->communicator);
    Ice::ObjectAdapterPtr adapter;
    try
    {
        adapter = (*self->communicator)->createObjectAdapterWithEndpoints(name, endpoints);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    PyObject* obj = createObjectAdapter(adapter);
    if (!obj)
    {
        try
        {
            adapter->deactivate();
        }
        catch (const Ice::Exception&)
        {
        }
    }

    return obj;
}

extern "C" PyObject*
communicatorCreateObjectAdapterWithRouter(CommunicatorObject* self, PyObject* args)
{
    PyObject* nameObj;
    PyObject* p;
    if (!PyArg_ParseTuple(args, "OO", &nameObj, &p))
    {
        return nullptr;
    }

    string name;
    if (!getStringArg(nameObj, "name", name))
    {
        return nullptr;
    }

    optional<Ice::ObjectPrx> proxy;
    if (!getProxyArg(p, "createObjectAdapterWithRouter", "rtr", proxy, "Ice.RouterPrx"))
    {
        return nullptr;
    }

    optional<Ice::RouterPrx> router = Ice::uncheckedCast<Ice::RouterPrx>(proxy);

    assert(self->communicator);
    Ice::ObjectAdapterPtr adapter;
    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock to avoid a potential deadlock.
        adapter = (*self->communicator)->createObjectAdapterWithRouter(name, router.value());
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    PyObject* obj = createObjectAdapter(adapter);
    if (!obj)
    {
        try
        {
            adapter->deactivate();
        }
        catch (const Ice::Exception&)
        {
        }
    }

    return obj;
}

extern "C" PyObject*
communicatorGetDefaultObjectAdapter(CommunicatorObject* self, PyObject* /*args*/)
{
    assert(self->communicator);
    Ice::ObjectAdapterPtr adapter = (*self->communicator)->getDefaultObjectAdapter();

    if (adapter)
    {
        // Returns an Ice.ObjectAdapter
        return wrapObjectAdapter(adapter);
    }
    else
    {
        return Py_None;
    }
}

extern "C" PyObject*
communicatorSetDefaultObjectAdapter(CommunicatorObject* self, PyObject* args)
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

    assert(self->communicator);
    (*self->communicator)->setDefaultObjectAdapter(oa);
    return Py_None;
}

extern "C" PyObject*
communicatorGetDefaultRouter(CommunicatorObject* self, PyObject* /*args*/)
{
    assert(self->communicator);
    optional<Ice::RouterPrx> router;
    try
    {
        router = (*self->communicator)->getDefaultRouter();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    if (!router)
    {
        return Py_None;
    }

    PyObject* routerProxyType = lookupType("Ice.RouterPrx");
    assert(routerProxyType);
    return createProxy(router.value(), *self->communicator, routerProxyType);
}

extern "C" PyObject*
communicatorSetDefaultRouter(CommunicatorObject* self, PyObject* args)
{
    PyObject* p;
    if (!PyArg_ParseTuple(args, "O", &p))
    {
        return nullptr;
    }

    optional<Ice::ObjectPrx> proxy;
    if (!getProxyArg(p, "setDefaultRouter", "rtr", proxy, "Ice.RouterPrx"))
    {
        return nullptr;
    }

    optional<Ice::RouterPrx> router = Ice::uncheckedCast<Ice::RouterPrx>(proxy);

    assert(self->communicator);
    try
    {
        (*self->communicator)->setDefaultRouter(router);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return Py_None;
}

extern "C" PyObject*
communicatorGetDefaultLocator(CommunicatorObject* self, PyObject* /*args*/)
{
    assert(self->communicator);
    optional<Ice::LocatorPrx> locator;
    try
    {
        locator = (*self->communicator)->getDefaultLocator();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    if (!locator)
    {
        return Py_None;
    }

    PyObject* locatorProxyType = lookupType("Ice.LocatorPrx");
    assert(locatorProxyType);
    return createProxy(locator.value(), *self->communicator, locatorProxyType);
}

extern "C" PyObject*
communicatorSetDefaultLocator(CommunicatorObject* self, PyObject* args)
{
    PyObject* p;
    if (!PyArg_ParseTuple(args, "O", &p))
    {
        return nullptr;
    }

    optional<Ice::ObjectPrx> proxy;
    if (!getProxyArg(p, "setDefaultLocator", "loc", proxy, "Ice.LocatorPrx"))
    {
        return nullptr;
    }

    optional<Ice::LocatorPrx> locator = Ice::uncheckedCast<Ice::LocatorPrx>(proxy);

    assert(self->communicator);
    try
    {
        (*self->communicator)->setDefaultLocator(locator);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return Py_None;
}

static PyMethodDef CommunicatorMethods[] = {
    {"destroy", reinterpret_cast<PyCFunction>(communicatorDestroy), METH_NOARGS, PyDoc_STR("destroy() -> None")},
    {"shutdown", reinterpret_cast<PyCFunction>(communicatorShutdown), METH_NOARGS, PyDoc_STR("shutdown() -> None")},
    {"waitForShutdown",
     reinterpret_cast<PyCFunction>(communicatorWaitForShutdown),
     METH_VARARGS,
     PyDoc_STR("waitForShutdown() -> None")},
    {"shutdownCompleted",
     reinterpret_cast<PyCFunction>(communicatorShutdownCompleted),
     METH_NOARGS,
     PyDoc_STR("shutdownCompleted() -> Ice.Future")},
    {"isShutdown",
     reinterpret_cast<PyCFunction>(communicatorIsShutdown),
     METH_NOARGS,
     PyDoc_STR("isShutdown() -> bool")},
    {"stringToProxy",
     reinterpret_cast<PyCFunction>(communicatorStringToProxy),
     METH_VARARGS,
     PyDoc_STR("stringToProxy(str) -> Ice.ObjectPrx")},
    {"proxyToString",
     reinterpret_cast<PyCFunction>(communicatorProxyToString),
     METH_VARARGS,
     PyDoc_STR("proxyToString(Ice.ObjectPrx) -> string")},
    {"propertyToProxy",
     reinterpret_cast<PyCFunction>(communicatorPropertyToProxy),
     METH_VARARGS,
     PyDoc_STR("propertyToProxy(str) -> Ice.ObjectPrx")},
    {"proxyToProperty",
     reinterpret_cast<PyCFunction>(communicatorProxyToProperty),
     METH_VARARGS,
     PyDoc_STR("proxyToProperty(Ice.ObjectPrx, str) -> dict")},
    {"identityToString",
     reinterpret_cast<PyCFunction>(communicatorIdentityToString),
     METH_VARARGS,
     PyDoc_STR("identityToString(Ice.Identity) -> string")},
    {"createObjectAdapter",
     reinterpret_cast<PyCFunction>(communicatorCreateObjectAdapter),
     METH_VARARGS,
     PyDoc_STR("createObjectAdapter(name) -> Ice.ObjectAdapter")},
    {"createObjectAdapterWithEndpoints",
     reinterpret_cast<PyCFunction>(communicatorCreateObjectAdapterWithEndpoints),
     METH_VARARGS,
     PyDoc_STR("createObjectAdapterWithEndpoints(name, endpoints) -> Ice.ObjectAdapter")},
    {"createObjectAdapterWithRouter",
     reinterpret_cast<PyCFunction>(communicatorCreateObjectAdapterWithRouter),
     METH_VARARGS,
     PyDoc_STR("createObjectAdapterWithRouter(name, router) -> Ice.ObjectAdapter")},
    {"getDefaultObjectAdapter",
     reinterpret_cast<PyCFunction>(communicatorGetDefaultObjectAdapter),
     METH_NOARGS,
     PyDoc_STR("getDefaultObjectAdapter() -> Ice.ObjectAdapter")},
    {"setDefaultObjectAdapter",
     reinterpret_cast<PyCFunction>(communicatorSetDefaultObjectAdapter),
     METH_VARARGS,
     PyDoc_STR("setDefaultObjectAdapter(adapter) -> None")},
    {"getValueFactoryManager",
     reinterpret_cast<PyCFunction>(communicatorGetValueFactoryManager),
     METH_NOARGS,
     PyDoc_STR("getValueFactoryManager() -> Ice.ValueFactoryManager")},
    {"getImplicitContext",
     reinterpret_cast<PyCFunction>(communicatorGetImplicitContext),
     METH_NOARGS,
     PyDoc_STR("getImplicitContext() -> Ice.ImplicitContext")},
    {"getProperties",
     reinterpret_cast<PyCFunction>(communicatorGetProperties),
     METH_NOARGS,
     PyDoc_STR("getProperties() -> Ice.Properties")},
    {"getLogger",
     reinterpret_cast<PyCFunction>(communicatorGetLogger),
     METH_NOARGS,
     PyDoc_STR("getLogger() -> Ice.Logger")},
    {"getDefaultRouter",
     reinterpret_cast<PyCFunction>(communicatorGetDefaultRouter),
     METH_NOARGS,
     PyDoc_STR("getDefaultRouter() -> proxy")},
    {"setDefaultRouter",
     reinterpret_cast<PyCFunction>(communicatorSetDefaultRouter),
     METH_VARARGS,
     "setDefaultRouter(proxy) -> None"},
    {"getDefaultLocator",
     reinterpret_cast<PyCFunction>(communicatorGetDefaultLocator),
     METH_NOARGS,
     PyDoc_STR("getDefaultLocator() -> proxy")},
    {"setDefaultLocator",
     reinterpret_cast<PyCFunction>(communicatorSetDefaultLocator),
     METH_VARARGS,
     PyDoc_STR("setDefaultLocator(proxy) -> None")},
    {"flushBatchRequests",
     reinterpret_cast<PyCFunction>(communicatorFlushBatchRequests),
     METH_VARARGS,
     PyDoc_STR("flushBatchRequests(compress) -> None")},
    {"flushBatchRequestsAsync",
     reinterpret_cast<PyCFunction>(communicatorFlushBatchRequestsAsync),
     METH_VARARGS,
     PyDoc_STR("flushBatchRequestsAsync(compress) -> Ice.Future")},
    {"createAdmin",
     reinterpret_cast<PyCFunction>(communicatorCreateAdmin),
     METH_VARARGS,
     PyDoc_STR("createAdmin(adminAdapter, adminIdentity) -> Ice.ObjectPrx")},
    {"getAdmin",
     reinterpret_cast<PyCFunction>(communicatorGetAdmin),
     METH_NOARGS,
     PyDoc_STR("getAdmin() -> Ice.ObjectPrx")},
    {"addAdminFacet",
     reinterpret_cast<PyCFunction>(communicatorAddAdminFacet),
     METH_VARARGS,
     PyDoc_STR("addAdminFacet(servant, facet) -> None")},
    {"findAdminFacet",
     reinterpret_cast<PyCFunction>(communicatorFindAdminFacet),
     METH_VARARGS,
     PyDoc_STR("findAdminFacet(facet) -> Ice.Object")},
    {"findAllAdminFacets",
     reinterpret_cast<PyCFunction>(communicatorFindAllAdminFacets),
     METH_NOARGS,
     PyDoc_STR("findAllAdminFacets() -> dictionary")},
    {"removeAdminFacet",
     reinterpret_cast<PyCFunction>(communicatorRemoveAdminFacet),
     METH_VARARGS,
     PyDoc_STR("removeAdminFacet(facet) -> Ice.Object")},
    {"_setWrapper",
     reinterpret_cast<PyCFunction>(communicatorSetWrapper),
     METH_VARARGS,
     PyDoc_STR("internal function")},
    {"_getWrapper", reinterpret_cast<PyCFunction>(communicatorGetWrapper), METH_NOARGS, PyDoc_STR("internal function")},
    {} /* sentinel */
};

namespace IcePy
{
    // clang-format off
    PyTypeObject CommunicatorType = {
        .ob_base = PyVarObject_HEAD_INIT(nullptr, 0)
        .tp_name = "IcePy.Communicator",
        .tp_basicsize = sizeof(CommunicatorObject),
        .tp_dealloc = reinterpret_cast<destructor>(communicatorDealloc),
        .tp_flags = Py_TPFLAGS_DEFAULT,
        .tp_methods = CommunicatorMethods,
        .tp_init = reinterpret_cast<initproc>(communicatorInit),
        .tp_new = reinterpret_cast<newfunc>(communicatorNew)};
    // clang-format on
}

bool
IcePy::initCommunicator(PyObject* module)
{
    mainThreadId = PyThread_get_thread_ident();

    if (PyType_Ready(&CommunicatorType) < 0)
    {
        return false;
    }
    PyTypeObject* type = &CommunicatorType; // Necessary to prevent GCC's strict-alias warnings.
    if (PyModule_AddObject(module, "Communicator", reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    return true;
}

Ice::CommunicatorPtr
IcePy::getCommunicator(PyObject* obj)
{
    assert(PyObject_IsInstance(obj, reinterpret_cast<PyObject*>(&CommunicatorType)));
    auto* cobj = reinterpret_cast<CommunicatorObject*>(obj);
    return *cobj->communicator;
}

PyObject*
IcePy::createCommunicator(const Ice::CommunicatorPtr& communicator)
{
    auto p = communicatorMap.find(communicator);
    if (p != communicatorMap.end())
    {
        Py_INCREF(p->second);
        return p->second;
    }

    CommunicatorObject* obj = communicatorNew(&CommunicatorType, nullptr, nullptr);
    if (obj)
    {
        obj->communicator = new Ice::CommunicatorPtr(communicator);
    }
    return (PyObject*)obj;
}

PyObject*
IcePy::getCommunicatorWrapper(const Ice::CommunicatorPtr& communicator)
{
    auto p = communicatorMap.find(communicator);
    assert(p != communicatorMap.end());
    auto* obj = reinterpret_cast<CommunicatorObject*>(p->second);
    if (obj->wrapper)
    {
        Py_INCREF(obj->wrapper);
        return obj->wrapper;
    }
    else
    {
        // Communicator must have been destroyed already.
        return Py_None;
    }
}

extern "C" PyObject*
IcePy_identityToString(PyObject* /*self*/, PyObject* args)
{
    PyObject* identityType = lookupType("Ice.Identity");
    PyObject* obj{nullptr};
    PyObject* mode{nullptr};
    if (!PyArg_ParseTuple(args, "O!O", identityType, &obj, &mode))
    {
        return nullptr;
    }

    Ice::Identity id;
    if (!getIdentity(obj, id))
    {
        return nullptr;
    }

    Ice::ToStringMode toStringMode = Ice::ToStringMode::Unicode;
    if (mode != Py_None && PyObject_HasAttrString(mode, "value"))
    {
        PyObjectHandle modeValue{getAttr(mode, "value", true)};
        toStringMode = static_cast<Ice::ToStringMode>(PyLong_AsLong(modeValue.get()));
    }

    string str;

    try
    {
        str = identityToString(id, toStringMode);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createString(str);
}

extern "C" PyObject*
IcePy_stringToIdentity(PyObject* /*self*/, PyObject* obj)
{
    string str;
    if (!getStringArg(obj, "str", str))
    {
        return nullptr;
    }

    Ice::Identity id;
    try
    {
        id = Ice::stringToIdentity(str);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createIdentity(id);
}
