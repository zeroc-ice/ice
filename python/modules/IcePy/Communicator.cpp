// Copyright (c) ZeroC, Inc.

#include "Communicator.h"
#include "BatchRequestInterceptor.h"
#include "DefaultSliceLoader.h"
#include "Executor.h"
#include "Future.h"
#include "Ice/DisableWarnings.h"
#include "Ice/Ice.h"
#include "IceDiscovery/IceDiscovery.h"
#include "IceLocatorDiscovery/IceLocatorDiscovery.h"
#include "ImplicitContext.h"
#include "Logger.h"
#include "ObjectAdapter.h"
#include "Operation.h"
#include "Properties.h"
#include "PropertiesAdmin.h"
#include "Proxy.h"
#include "PySliceLoader.h"
#include "Thread.h"
#include "Types.h"
#include "Util.h"

#include <pythread.h>

#include <future>
#include <valarray>

using namespace std;
using namespace IcePy;

#if defined(__clang__) && defined(__has_warning)
#    if __has_warning("-Wcast-function-type-mismatch")
#        pragma clang diagnostic ignored "-Wcast-function-type-mismatch"
#    endif
#endif

#if defined(__GNUC__) && ((__GNUC__ >= 8))
#    pragma GCC diagnostic ignored "-Wcast-function-type"
#endif

namespace
{
    using CommunicatorMap = map<Ice::CommunicatorPtr, PyObject*>;
    CommunicatorMap communicatorMap;

    using SliceLoaderMap = map<Ice::CommunicatorPtr, Ice::SliceLoaderPtr>;
    SliceLoaderMap sliceLoaderMap;
}

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

    void removeSliceLoader(const Ice::CommunicatorPtr& communicator);
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
// IcePy.Communicator(initData: Ice.InitializationData|None)

extern "C" int
communicatorInit(CommunicatorObject* self, PyObject* args, PyObject* /*kwds*/)
{
    PyObject* pyInitData{nullptr};
    if (!PyArg_ParseTuple(args, "O", &pyInitData))
    {
        return -1;
    }

    Ice::InitializationData initData;
    ExecutorPtr executorWrapper;

    Ice::SliceLoaderPtr sliceLoader = DefaultSliceLoader::instance();

    try
    {
        if (pyInitData != Py_None)
        {
            PyObjectHandle properties{getAttr(pyInitData, "properties", false)};
            PyObjectHandle logger{getAttr(pyInitData, "logger", false)};
            PyObjectHandle threadStart{getAttr(pyInitData, "threadStart", false)};
            PyObjectHandle threadStop{getAttr(pyInitData, "threadStop", false)};
            PyObjectHandle batchRequestInterceptor{getAttr(pyInitData, "batchRequestInterceptor", false)};
            PyObjectHandle executor{getAttr(pyInitData, "executor", false)};
            PyObjectHandle initDataSliceLoader{getAttr(pyInitData, "sliceLoader", false)};

            if (properties.get())
            {
                //
                // Get the properties implementation.
                //
                PyObjectHandle impl{getAttr(properties.get(), "_impl", false)};
                assert(impl.get());
                initData.properties = getProperties(impl.get());
            }

            if (logger.get())
            {
                initData.logger = make_shared<LoggerWrapper>(logger.get());
            }

            if (threadStart.get() || threadStop.get())
            {
                auto threadHook = make_shared<ThreadHook>(threadStart.get(), threadStop.get());
                initData.threadStart = [threadHook]() { threadHook->start(); };
                initData.threadStop = [threadHook]() { threadHook->stop(); };
            }

            if (executor.get())
            {
                executorWrapper = make_shared<Executor>(executor.get());
                initData.executor =
                    [executorWrapper](function<void()> call, const shared_ptr<Ice::Connection>& connection)
                { executorWrapper->execute(std::move(call), connection); };
            }

            if (batchRequestInterceptor.get())
            {
                auto batchRequestInterceptorWrapper =
                    make_shared<BatchRequestInterceptorWrapper>(batchRequestInterceptor.get());
                initData.batchRequestInterceptor =
                    [batchRequestInterceptorWrapper](const Ice::BatchRequest& req, int count, int size)
                { batchRequestInterceptorWrapper->enqueue(req, count, size); };
            }

            if (initDataSliceLoader.get())
            {
                auto compositeSliceLoader = make_shared<Ice::CompositeSliceLoader>();
                compositeSliceLoader->add(make_shared<PySliceLoader>(initDataSliceLoader.get()));
                compositeSliceLoader->add(std::move(sliceLoader));
                sliceLoader = std::move(compositeSliceLoader);
            }
        }

        // initData.sliceLoader remains null as we don't want to change the Slice loader for the Ice C++ runtime.

        if (!initData.properties)
        {
            initData.properties = Ice::createProperties();
        }
    }
    catch (...)
    {
        setPythonException(current_exception());
        return -1;
    }

    // Set Ice.ProgramName if not already set.
    if (initData.properties->getIceProperty("Ice.ProgramName").empty())
    {
        string programName;
        // Get python program name using python c api
        PyObjectHandle sysModule{PyImport_ImportModule("sys")};
        if (sysModule.get())
        {
            PyObjectHandle argv{PyObject_GetAttrString(sysModule.get(), "argv")};
            if (argv && PyList_Check(argv.get()) && PyList_Size(argv.get()) > 0)
            {
                PyObject* programNameObj = PyList_GetItem(argv.get(), 0);
                if (programNameObj)
                {
                    // Name is empty when Python is executed without a script file. For example, when using the
                    // interactive shell or the -c option.
                    const char* namePtr = PyUnicode_AsUTF8(programNameObj);
                    if (namePtr)
                    {
                        string name = namePtr;
                        if (!name.empty())
                        {
                            programName = name;
                            size_t pos = programName.find_last_of("/\\");
                            if (pos != string::npos)
                            {
                                programName = programName.substr(pos + 1);
                            }
                        }
                    }
                }
            }
        }

        initData.properties->setProperty("Ice.ProgramName", programName);
    }

    // Always accept class cycles during the unmarshaling of Python objects by the C++ code.
    initData.properties->setProperty("Ice.AcceptClassCycles", "1");

    // Add IceDiscovery/IceLocatorDiscovery if these plug-ins are configured via Ice.Plugin.name.
    if (!initData.properties->getIceProperty("Ice.Plugin.IceDiscovery").empty())
    {
        initData.pluginFactories.push_back(IceDiscovery::discoveryPluginFactory());
    }

    if (!initData.properties->getIceProperty("Ice.Plugin.IceLocatorDiscovery").empty())
    {
        initData.pluginFactories.push_back(IceLocatorDiscovery::locatorDiscoveryPluginFactory());
    }

    Ice::CommunicatorPtr communicator;
    try
    {
        AllowThreads allowThreads;
        communicator = Ice::initialize(initData);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return -1;
    }

    self->communicator = new Ice::CommunicatorPtr(communicator);
    communicatorMap.insert(CommunicatorMap::value_type{communicator, reinterpret_cast<PyObject*>(self)});
    sliceLoaderMap[communicator] = sliceLoader;

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

    {
        // Release Python's Global Interpreter Lock (GIL) to prevent potential deadlocks.
        // The communicator destructor waits for destroyAsync callback threads to complete.
        // If the GIL isn't released here, callbacks attempting to acquire it will deadlock.
        AllowThreads allowThreads;
        delete self->communicator;
    }

    delete self->shutdownException;
    delete self->shutdownFuture;
    delete self->executor;
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

extern "C" PyObject*
communicatorDestroy(CommunicatorObject* self, PyObject* /*args*/)
{
    assert(self->communicator);

    {
        AllowThreads allowThreads; // Release Python's global interpreter lock to avoid a potential deadlock.
        (*self->communicator)->destroy();
    }

    removeSliceLoader(*self->communicator);

    if (self->executor)
    {
        (*self->executor)->setCommunicator(nullptr); // Break cyclic reference.
    }

    // Break cyclic reference between this object and its Python wrapper.
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
communicatorDestroyAsync(CommunicatorObject* self, PyObject* args)
{
    assert(self->communicator);

    PyObject* completed;
    if (!PyArg_ParseTuple(args, "O", &completed))
    {
        return nullptr;
    }

    if (!PyCallable_Check(completed))
    {
        PyErr_SetString(PyExc_TypeError, "completed must be callable");
        return nullptr;
    }

    // We create a new reference to `completed` to ensure it remains alive until the callback is invoked.
    // This is necessary in case the user abandons the future, for example by cancelling it.
    (*self->communicator)
        ->destroyAsync(
            [self, completed = Py_NewRef(completed)]()
            {
                // Ensure the current thread can call into Python. We avoid using AdoptThread because it unconditionally
                // releases the GIL when it goes out of scope. Here, we want to avoid releasing the GIL if interpreter
                // finalization starts before the call to PyObject_Call(completed) returns.
                PyGILState_STATE gilState = PyGILState_Ensure();

                removeSliceLoader(*self->communicator);

                if (self->executor)
                {
                    (*self->executor)->setCommunicator(nullptr); // Break cyclic reference.
                }

                // Break cyclic reference between this object and its Python wrapper.
                Py_XDECREF(self->wrapper);
                self->wrapper = nullptr;

                PyObject* emptyArgs = PyTuple_New(0);
                if (!emptyArgs)
                {
                    PyErr_SetString(PyExc_RuntimeError, "failed to create args tuple");
                    return;
                }

                // Calling completed completes the future that Communicator.__aexit__ is awaiting. Note that the
                // future's callbacks can start running before PyObject_Call returns. In a typical Ice application, the
                // main script exits once this future is completed, which can trigger Python interpreter finalization.
                //
                // If the interpreter is already finalizing by the time PyObject_Call returns, we must avoid releasing
                // the references to completed and emptyArgs, as invoking Python API functions (like Py_DECREF)
                // during finalization is not safe. In that case, we intentionally leak these objects to prevent
                // potential crashes.
                PyObject* result = PyObject_Call(completed, emptyArgs, nullptr);
                assert(result == nullptr || result == Py_None);
                if (result == nullptr)
                {
                    PyErr_PrintEx(0);
                }

#if PY_VERSION_HEX >= 0x030D0000
                // With Python 3.13 and later, we use Py_IsFinalizing to conditionally release the references to
                // completed and emptyArgs when the interpreter is not finalizing.
                if (!Py_IsFinalizing())
                {
                    Py_DECREF(completed);
                    Py_DECREF(emptyArgs);
                }
#endif
                // Check whether we still own the GIL. If interpreter finalization has begun, we may no longer hold the
                // GIL and must avoid calling PyGILState_Release unless we do.
                if (PyGILState_Check())
                {
                    PyGILState_Release(gilState);
                }
            });
    return Py_None;
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
    if (isMainThread())
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

    // Create a new reference to prevent premature release of the future object. The reference will be released by
    // the callback.
    PyObject* futureObject = Py_NewRef(future.get());

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
    return (*self->communicator)->isShutdown() ? Py_True : Py_False;
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

    PyObjectHandle v{getAttr(compressBatch, "value", false)};
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

    PyObjectHandle v{getAttr(compressBatch, "value", false)};
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

    PyObjectHandle future{createFuture(asyncInvocationContextObj.get())};
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
        // The facet being found may not be implemented by a Python servant in which case we return None.
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

            // If the facet isn't supported in Python, just return None.
            return Py_None;
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
        PyObjectHandle obj{};

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

        if (obj)
        {
            if (PyDict_SetItemString(result.get(), const_cast<char*>(facet.c_str()), obj.get()) < 0)
            {
                return nullptr;
            }
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
    self->wrapper = Py_NewRef(wrapper);
    return Py_None;
}

extern "C" PyObject*
communicatorGetWrapper(CommunicatorObject* self, PyObject* /*args*/)
{
    assert(self->wrapper);
    return Py_NewRef(self->wrapper);
}

extern "C" PyObject*
communicatorGetProperties(CommunicatorObject* self, PyObject* /*args*/)
{
    assert(self->communicator);
    Ice::PropertiesPtr properties = (*self->communicator)->getProperties();
    return IcePy::createProperties(properties);
}

extern "C" PyObject*
communicatorGetLogger(CommunicatorObject* self, PyObject* /*args*/)
{
    assert(self->communicator);
    Ice::LoggerPtr logger = (*self->communicator)->getLogger();

    // The communicator's logger can either be a C++ object (such as the default logger supplied by the Ice run time),
    // or a C++ wrapper around a Python implementation. If the latter, we return it directly. Otherwise, we create a
    // Python object that delegates to the C++ object.
    LoggerWrapperPtr wrapper = dynamic_pointer_cast<LoggerWrapper>(logger);
    if (wrapper)
    {
        return Py_NewRef(wrapper->getObject());
    }

    return createLogger(logger);
}

extern "C" PyObject*
communicatorGetImplicitContext(CommunicatorObject* self, PyObject* /*args*/)
{
    Ice::ImplicitContextPtr implicitContext = (*self->communicator)->getImplicitContext();
    return implicitContext ? createImplicitContext(implicitContext) : Py_None;
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
    {"destroyAsync",
     reinterpret_cast<PyCFunction>(communicatorDestroyAsync),
     METH_VARARGS,
     PyDoc_STR("destroyAsync(callable: Callable) -> None")},
    {"shutdown", reinterpret_cast<PyCFunction>(communicatorShutdown), METH_NOARGS, PyDoc_STR("shutdown() -> None")},
    {"waitForShutdown",
     reinterpret_cast<PyCFunction>(communicatorWaitForShutdown),
     METH_VARARGS,
     PyDoc_STR("waitForShutdown(timeout: int) -> bool")},
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
     PyDoc_STR("stringToProxy(str: str) -> Ice.ObjectPrx")},
    {"proxyToString",
     reinterpret_cast<PyCFunction>(communicatorProxyToString),
     METH_VARARGS,
     PyDoc_STR("proxyToString(proxy: Ice.ObjectPrx | None) -> str")},
    {"propertyToProxy",
     reinterpret_cast<PyCFunction>(communicatorPropertyToProxy),
     METH_VARARGS,
     PyDoc_STR("propertyToProxy(property: str) -> Ice.ObjectPrx")},
    {"proxyToProperty",
     reinterpret_cast<PyCFunction>(communicatorProxyToProperty),
     METH_VARARGS,
     PyDoc_STR("proxyToProperty(proxy: Ice.ObjectPrx, property: str) -> dict[str, str]")},
    {"identityToString",
     reinterpret_cast<PyCFunction>(communicatorIdentityToString),
     METH_VARARGS,
     PyDoc_STR("identityToString(identity: Ice.Identity) -> str")},
    {"createObjectAdapter",
     reinterpret_cast<PyCFunction>(communicatorCreateObjectAdapter),
     METH_VARARGS,
     PyDoc_STR("createObjectAdapter(name: str) -> ObjectAdapter")},
    {"createObjectAdapterWithEndpoints",
     reinterpret_cast<PyCFunction>(communicatorCreateObjectAdapterWithEndpoints),
     METH_VARARGS,
     PyDoc_STR("createObjectAdapterWithEndpoints(name: str, endpoints: str) -> ObjectAdapter")},
    {"createObjectAdapterWithRouter",
     reinterpret_cast<PyCFunction>(communicatorCreateObjectAdapterWithRouter),
     METH_VARARGS,
     PyDoc_STR("createObjectAdapterWithRouter(name: str, router: Ice.RouterPrx) -> ObjectAdapter")},
    {"getDefaultObjectAdapter",
     reinterpret_cast<PyCFunction>(communicatorGetDefaultObjectAdapter),
     METH_NOARGS,
     PyDoc_STR("getDefaultObjectAdapter() -> Ice.ObjectAdapter | None")},
    {"setDefaultObjectAdapter",
     reinterpret_cast<PyCFunction>(communicatorSetDefaultObjectAdapter),
     METH_VARARGS,
     PyDoc_STR("setDefaultObjectAdapter(adapter: Ice.ObjectAdapter | None) -> None")},
    {"getImplicitContext",
     reinterpret_cast<PyCFunction>(communicatorGetImplicitContext),
     METH_NOARGS,
     PyDoc_STR("getImplicitContext() -> ImplicitContext | None")},
    {"getProperties",
     reinterpret_cast<PyCFunction>(communicatorGetProperties),
     METH_NOARGS,
     PyDoc_STR("getProperties() -> Properties")},
    {"getLogger",
     reinterpret_cast<PyCFunction>(communicatorGetLogger),
     METH_NOARGS,
     PyDoc_STR("getLogger() -> Ice.Logger | Logger")},
    {"getDefaultRouter",
     reinterpret_cast<PyCFunction>(communicatorGetDefaultRouter),
     METH_NOARGS,
     PyDoc_STR("getDefaultRouter() -> Ice.RouterPrx | None")},
    {"setDefaultRouter",
     reinterpret_cast<PyCFunction>(communicatorSetDefaultRouter),
     METH_VARARGS,
     PyDoc_STR("setDefaultRouter(router: Ice.RouterPrx | None) -> None")},
    {"getDefaultLocator",
     reinterpret_cast<PyCFunction>(communicatorGetDefaultLocator),
     METH_NOARGS,
     PyDoc_STR("getDefaultLocator() -> Ice.LocatorPrx | None")},
    {"setDefaultLocator",
     reinterpret_cast<PyCFunction>(communicatorSetDefaultLocator),
     METH_VARARGS,
     PyDoc_STR("setDefaultLocator(locator: Ice.LocatorPrx | None) -> None")},
    {"flushBatchRequests",
     reinterpret_cast<PyCFunction>(communicatorFlushBatchRequests),
     METH_VARARGS,
     PyDoc_STR("flushBatchRequests(compress: Ice.CompressBatch) -> None")},
    {"flushBatchRequestsAsync",
     reinterpret_cast<PyCFunction>(communicatorFlushBatchRequestsAsync),
     METH_VARARGS,
     PyDoc_STR("flushBatchRequestsAsync(compress: Ice.CompressBatch) -> Awaitable[None]")},
    {"createAdmin",
     reinterpret_cast<PyCFunction>(communicatorCreateAdmin),
     METH_VARARGS,
     PyDoc_STR("createAdmin(adminAdapter: Ice.ObjectAdapter | None, adminIdentity: Ice.Identity) -> Ice.ObjectPrx")},
    {"getAdmin",
     reinterpret_cast<PyCFunction>(communicatorGetAdmin),
     METH_NOARGS,
     PyDoc_STR("getAdmin() -> Ice.ObjectPrx | None")},
    {"addAdminFacet",
     reinterpret_cast<PyCFunction>(communicatorAddAdminFacet),
     METH_VARARGS,
     PyDoc_STR("addAdminFacet(servant: Ice.Object | None, facet: str) -> None")},
    {"findAdminFacet",
     reinterpret_cast<PyCFunction>(communicatorFindAdminFacet),
     METH_VARARGS,
     PyDoc_STR("findAdminFacet(facet: str) -> Ice.Object")},
    {"findAllAdminFacets",
     reinterpret_cast<PyCFunction>(communicatorFindAllAdminFacets),
     METH_NOARGS,
     PyDoc_STR("findAllAdminFacets() -> dict")},
    {"removeAdminFacet",
     reinterpret_cast<PyCFunction>(communicatorRemoveAdminFacet),
     METH_VARARGS,
     PyDoc_STR("removeAdminFacet(facet: str) -> Ice.Object")},
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
        .tp_doc = PyDoc_STR("IcePy.Communicator"),
        .tp_methods = CommunicatorMethods,
        .tp_init = reinterpret_cast<initproc>(communicatorInit),
        .tp_new = reinterpret_cast<newfunc>(communicatorNew)};
    // clang-format on
}

bool
IcePy::initCommunicator(PyObject* module)
{
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
        return Py_NewRef(p->second);
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
        return Py_NewRef(obj->wrapper);
    }
    else
    {
        // Communicator must have been destroyed already.
        return Py_None;
    }
}

Ice::SliceLoaderPtr
IcePy::getSliceLoader(const Ice::CommunicatorPtr& communicator)
{
    // It's in the map until the communicator is destroyed.
    auto p = sliceLoaderMap.find(communicator);
    assert(p != sliceLoaderMap.end());
    return p->second;
}

void
IcePy::removeSliceLoader(const Ice::CommunicatorPtr& communicator)
{
    // Access to sliceLoaderMap is protected by the GIL.

    // Called by destroy[Async], which can be called multiple times.
    auto p = sliceLoaderMap.find(communicator);
    if (p != sliceLoaderMap.end())
    {
        // Make sure we don't release the last refcount of a Python object in erase, since this releasing may release
        // the GIL temporarily and let another thread access the map.
        Ice::SliceLoaderPtr sliceLoader = p->second;
        sliceLoaderMap.erase(p);
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
