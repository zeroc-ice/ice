// Copyright (c) ZeroC, Inc.

#include "ObjectAdapter.h"
#include "Communicator.h"
#include "Current.h"
#include "Endpoint.h"
#include "Ice/Communicator.h"
#include "Ice/LocalExceptions.h"
#include "Ice/Locator.h"
#include "Ice/Logger.h"
#include "Ice/ObjectAdapter.h"
#include "Ice/Router.h"
#include "Ice/ServantLocator.h"
#include "Operation.h"
#include "Proxy.h"
#include "Thread.h"
#include "Types.h"
#include "Util.h"

#include <pythread.h>

#include <future>

using namespace std;
using namespace IcePy;

namespace IcePy
{
    struct ObjectAdapterObject
    {
        PyObject_HEAD Ice::ObjectAdapterPtr* adapter;

        std::future<void>* deactivateFuture;
        std::exception_ptr* deactivateException;
        bool deactivated;

        // This mutex protects holdFuture, holdException, and held from concurrent access in activate and waitForHold.
        std::mutex* holdMutex;
        std::future<void>* holdFuture;
        std::exception_ptr* holdException;
        bool held;
    };

    class ServantLocatorWrapper final : public Ice::ServantLocator
    {
    public:
        ServantLocatorWrapper(PyObject*);
        ~ServantLocatorWrapper() final;

        Ice::ObjectPtr locate(const Ice::Current&, shared_ptr<void>&) final;

        void finished(const Ice::Current&, const Ice::ObjectPtr&, const shared_ptr<void>&) final;

        void deactivate(string_view) final;

        PyObject* getObject();

    private:
        //
        // This object is created in locate() and destroyed after finished().
        //
        struct Cookie
        {
            ~Cookie();

            PyObject* current{nullptr};
            ServantWrapperPtr servant{nullptr};
            PyObject* cookie{nullptr};
        };
        using CookiePtr = shared_ptr<Cookie>;

        PyObject* _locator;
        PyObject* _objectType;
    };

    using ServantLocatorWrapperPtr = shared_ptr<ServantLocatorWrapper>;
}

namespace
{
    bool getServantWrapper(PyObject* servant, ServantWrapperPtr& wrapper)
    {
        PyObject* objectType{lookupType("Ice.Object")};
        if (servant != Py_None && !PyObject_IsInstance(servant, objectType))
        {
            PyErr_Format(PyExc_ValueError, "expected Ice object or None");
            return false;
        }

        if (servant != Py_None)
        {
            wrapper = createServantWrapper(servant);
            if (PyErr_Occurred())
            {
                return false;
            }
        }

        return true;
    }
}

//
// ServantLocatorWrapper implementation.
//
IcePy::ServantLocatorWrapper::ServantLocatorWrapper(PyObject* locator) : _locator(Py_NewRef(locator))
{
    _objectType = lookupType("Ice.Object");
}

IcePy::ServantLocatorWrapper::~ServantLocatorWrapper()
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.
    Py_DECREF(_locator);
}

Ice::ObjectPtr
IcePy::ServantLocatorWrapper::locate(const Ice::Current& current, shared_ptr<void>& cookie)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    CookiePtr c{make_shared<Cookie>()};
    c->current = createCurrent(current);
    if (!c->current)
    {
        throwPythonException();
    }

    // Invoke locate on the Python object. We expect the object to return either the servant by itself, or the servant
    // in a tuple with an optional cookie object.
    PyObjectHandle res{PyObject_CallMethod(_locator, "locate", "O", c->current)};
    if (PyErr_Occurred())
    {
        // Retrieve the exception before another Python API call clears it.
        PyException ex;

        // A locator that calls sys.exit() will raise the SystemExit exception.
        //
        // This is normally caught by the interpreter, causing it to exit. However, we have no way to pass this
        // exception to the interpreter, so we act on it directly.
        ex.checkSystemExit();

        PyObject* userExceptionType{lookupType("Ice.UserException")};
        if (PyObject_IsInstance(ex.ex.get(), userExceptionType))
        {
            throw ExceptionWriter(ex.ex);
        }

        ex.raise();
    }

    if (res.get() == Py_None)
    {
        return nullptr;
    }

    PyObject* servantObj{nullptr};
    PyObject* cookieObj{Py_None};
    if (PyTuple_Check(res.get()))
    {
        if (PyTuple_GET_SIZE(res.get()) > 2)
        {
            const Ice::CommunicatorPtr com{current.adapter->getCommunicator()};
            if (com->getProperties()->getIcePropertyAsInt("Ice.Warn.Dispatch") > 0)
            {
                com->getLogger()->warning("invalid return value for ServantLocator::locate");
            }
            return nullptr;
        }
        servantObj = PyTuple_GET_ITEM(res.get(), 0);
        if (PyTuple_GET_SIZE(res.get()) > 1)
        {
            cookieObj = PyTuple_GET_ITEM(res.get(), 1);
        }
    }
    else
    {
        servantObj = res.get();
    }

    // Verify that the servant is an Ice.Object instance.
    if (!PyObject_IsInstance(servantObj, _objectType))
    {
        const Ice::CommunicatorPtr com{current.adapter->getCommunicator()};
        if (com->getProperties()->getIcePropertyAsInt("Ice.Warn.Dispatch") > 0)
        {
            com->getLogger()->warning("return value of ServantLocator::locate is not an Ice object");
        }
        return nullptr;
    }

    // Save state in our cookie and return a wrapper for the servant.
    c->servant = createServantWrapper(servantObj);
    c->cookie = Py_NewRef(cookieObj);
    cookie = c;
    return c->servant;
}

void
IcePy::ServantLocatorWrapper::finished(const Ice::Current&, const Ice::ObjectPtr&, const shared_ptr<void>& cookie)
{
    // Ensure the current thread is able to call into Python.
    AdoptThread adoptThread;

    CookiePtr c{static_pointer_cast<Cookie>(cookie)};
    assert(c);

    ServantWrapperPtr wrapper{dynamic_pointer_cast<ServantWrapper>(c->servant)};

    PyObjectHandle res{PyObject_CallMethod(_locator, "finished", "OOO", c->current, wrapper->getObject(), c->cookie)};
    if (PyErr_Occurred())
    {
        // Retrieve the exception before another Python API call clears it.
        PyException ex;

        // A locator that calls sys.exit() will raise the SystemExit exception.
        //
        // This is normally caught by the interpreter, causing it to exit. However, we have no way to pass this
        // exception to the interpreter, so we act on it directly.
        ex.checkSystemExit();

        PyObject* userExceptionType{lookupType("Ice.UserException")};
        if (PyObject_IsInstance(ex.ex.get(), userExceptionType))
        {
            throw ExceptionWriter(ex.ex);
        }

        ex.raise();
    }
}

void
IcePy::ServantLocatorWrapper::deactivate(string_view category)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    string categoryStr{category};

    PyObjectHandle res{PyObject_CallMethod(_locator, "deactivate", "s", categoryStr.c_str())};
    if (PyErr_Occurred())
    {
        // Retrieve the exception before another Python API call clears it.
        PyException ex;

        // A locator that calls sys.exit() will raise the SystemExit exception.
        //
        // This is normally caught by the interpreter, causing it to exit. However, we have no way to pass this
        // exception to the interpreter, so we act on it directly.
        ex.checkSystemExit();

        ex.raise();
    }
}

PyObject*
IcePy::ServantLocatorWrapper::getObject()
{
    return Py_NewRef(_locator);
}

IcePy::ServantLocatorWrapper::Cookie::~Cookie()
{
    AdoptThread adoptThread;
    Py_XDECREF(current);
    Py_XDECREF(cookie);
}

extern "C" ObjectAdapterObject*
adapterNew(PyTypeObject* /*type*/, PyObject* /*args*/, PyObject* /*kwds*/)
{
    PyErr_Format(PyExc_RuntimeError, "Use communicator.createObjectAdapter to create an adapter");
    return nullptr;
}

extern "C" void
adapterDealloc(ObjectAdapterObject* self)
{
    delete self->adapter;

    delete self->deactivateException;
    delete self->deactivateFuture;

    delete self->holdMutex;
    delete self->holdException;
    delete self->holdFuture;

    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

extern "C" PyObject*
adapterGetName(ObjectAdapterObject* self, PyObject* /*args*/)
{
    assert(self->adapter);
    return createString((*self->adapter)->getName());
}

extern "C" PyObject*
adapterGetCommunicator(ObjectAdapterObject* self, PyObject* /*args*/)
{
    assert(self->adapter);
    return createCommunicator((*self->adapter)->getCommunicator());
}

extern "C" PyObject*
adapterActivate(ObjectAdapterObject* self, PyObject* /*args*/)
{
    assert(self->adapter);
    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during blocking calls.
        (*self->adapter)->activate();

        std::lock_guard lock(*self->holdMutex);
        self->held = false;
        if (self->holdFuture)
        {
            delete self->holdFuture;
            self->holdFuture = nullptr;
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
adapterHold(ObjectAdapterObject* self, PyObject* /*args*/)
{
    assert(self->adapter);
    try
    {
        (*self->adapter)->hold();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return Py_None;
}

extern "C" PyObject*
adapterWaitForHold(ObjectAdapterObject* self, PyObject* args)
{
    //
    // This method differs somewhat from the standard Ice API because of
    // signal issues. This method expects an integer timeout value, and
    // returns a boolean to indicate whether it was successful. When
    // called from the main thread, the timeout is used to allow control
    // to return to the caller (the Python interpreter) periodically.
    // When called from any other thread, we call waitForHold directly
    // and ignore the timeout.
    //
    int timeout = 0;
    if (!PyArg_ParseTuple(args, "i", &timeout))
    {
        return nullptr;
    }

    assert(timeout > 0);
    assert(self->adapter);

    //
    // Do not call waitForHold from the main thread, because it prevents
    // signals (such as keyboard interrupts) from being delivered to Python.
    //
    if (isMainThread())
    {
        std::lock_guard lock(*self->holdMutex);

        if (!self->held)
        {
            if (!self->holdFuture)
            {
                self->holdFuture = new std::future<void>();
                *self->holdFuture = std::async(std::launch::async, [&self] { (*self->adapter)->waitForHold(); });
            }

            {
                AllowThreads allowThreads; // Release Python's global interpreter lock during blocking calls.
                if (self->holdFuture->wait_for(std::chrono::milliseconds(timeout)) == std::future_status::timeout)
                {
                    return Py_False;
                }
            }

            self->held = true;
            try
            {
                self->holdFuture->get();
            }
            catch (...)
            {
                self->holdException = new std::exception_ptr(current_exception());
            }
        }

        assert(self->held);
        if (self->holdException)
        {
            setPythonException(*self->holdException);
            return nullptr;
        }
    }
    else
    {
        try
        {
            AllowThreads allowThreads; // Release Python's global interpreter lock during blocking calls.
            (*self->adapter)->waitForHold();
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
adapterDeactivate(ObjectAdapterObject* self, PyObject* /*args*/)
{
    assert(self->adapter);

    // Release Python's global interpreter lock during blocking calls.
    AllowThreads allowThreads;
    (*self->adapter)->deactivate();
    return Py_None;
}

extern "C" PyObject*
adapterWaitForDeactivate(ObjectAdapterObject* self, PyObject* args)
{
    //
    // This method differs somewhat from the standard Ice API because of
    // signal issues. This method expects an integer timeout value, and
    // returns a boolean to indicate whether it was successful. When
    // called from the main thread, the timeout is used to allow control
    // to return to the caller (the Python interpreter) periodically.
    // When called from any other thread, we call waitForDeactivate directly
    // and ignore the timeout.
    //
    int timeout = 0;
    if (!PyArg_ParseTuple(args, "i", &timeout))
    {
        return nullptr;
    }

    assert(timeout > 0);
    assert(self->adapter);

    //
    // Do not call waitForDeactivate from the main thread, because it prevents
    // signals (such as keyboard interrupts) from being delivered to Python.
    //
    if (isMainThread())
    {
        if (!self->deactivated)
        {
            if (!self->deactivateFuture)
            {
                self->deactivateFuture = new std::future<void>();
                *self->deactivateFuture =
                    std::async(std::launch::async, [&self] { (*self->adapter)->waitForDeactivate(); });
            }

            {
                AllowThreads allowThreads; // Release Python's global interpreter lock during blocking calls.
                if (self->deactivateFuture->wait_for(std::chrono::milliseconds(timeout)) == std::future_status::timeout)
                {
                    return Py_False;
                }
            }

            self->deactivated = true;
            try
            {
                self->deactivateFuture->get();
            }
            catch (...)
            {
                self->deactivateException = new std::exception_ptr(current_exception());
            }
        }

        assert(self->deactivated);
        if (self->deactivateException)
        {
            setPythonException(*self->deactivateException);
            return nullptr;
        }
    }
    else
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during blocking calls.
        (*self->adapter)->waitForDeactivate();
    }

    return Py_True;
}

extern "C" PyObject*
adapterIsDeactivated(ObjectAdapterObject* self, PyObject* /*args*/)
{
    assert(self->adapter);
    return (*self->adapter)->isDeactivated() ? Py_True : Py_False;
}

extern "C" PyObject*
adapterDestroy(ObjectAdapterObject* self, PyObject* /*args*/)
{
    assert(self->adapter);
    // Release Python's global interpreter lock during blocking calls.
    AllowThreads allowThreads;
    (*self->adapter)->destroy();
    return Py_None;
}

extern "C" PyObject*
adapterAdd(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* identityType{lookupType("Ice.Identity")};
    PyObject* servant{nullptr};
    PyObject* id{nullptr};
    if (!PyArg_ParseTuple(args, "OO!", &servant, identityType, &id))
    {
        return nullptr;
    }

    Ice::Identity ident;
    if (!getIdentity(id, ident))
    {
        return nullptr;
    }

    ServantWrapperPtr wrapper{nullptr};
    if (!getServantWrapper(servant, wrapper))
    {
        return nullptr;
    }

    assert(self->adapter);
    optional<Ice::ObjectPrx> proxy;
    try
    {
        proxy = (*self->adapter)->add(wrapper, ident);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createProxy(proxy.value(), (*self->adapter)->getCommunicator());
}

extern "C" PyObject*
adapterAddFacet(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* identityType{lookupType("Ice.Identity")};
    PyObject* servant{nullptr};
    PyObject* id{nullptr};
    PyObject* facetObj{nullptr};
    if (!PyArg_ParseTuple(args, "OO!O", &servant, identityType, &id, &facetObj))
    {
        return nullptr;
    }

    Ice::Identity ident;
    if (!getIdentity(id, ident))
    {
        return nullptr;
    }

    ServantWrapperPtr wrapper{nullptr};
    if (!getServantWrapper(servant, wrapper))
    {
        return nullptr;
    }

    string facet;
    if (!getStringArg(facetObj, "facet", facet))
    {
        return nullptr;
    }

    assert(self->adapter);
    optional<Ice::ObjectPrx> proxy;
    try
    {
        proxy = (*self->adapter)->addFacet(wrapper, ident, facet);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createProxy(proxy.value(), (*self->adapter)->getCommunicator());
}

extern "C" PyObject*
adapterAddWithUUID(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* servant;
    if (!PyArg_ParseTuple(args, "O", &servant))
    {
        return nullptr;
    }

    ServantWrapperPtr wrapper;
    if (!getServantWrapper(servant, wrapper))
    {
        return nullptr;
    }

    assert(self->adapter);
    optional<Ice::ObjectPrx> proxy;
    try
    {
        proxy = (*self->adapter)->addWithUUID(wrapper);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createProxy(proxy.value(), (*self->adapter)->getCommunicator());
}

extern "C" PyObject*
adapterAddFacetWithUUID(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* servant{nullptr};
    PyObject* facetObj{nullptr};
    if (!PyArg_ParseTuple(args, "OO", &servant, &facetObj))
    {
        return nullptr;
    }

    ServantWrapperPtr wrapper{nullptr};
    if (!getServantWrapper(servant, wrapper))
    {
        return nullptr;
    }

    string facet;
    if (!getStringArg(facetObj, "facet", facet))
    {
        return nullptr;
    }

    assert(self->adapter);
    optional<Ice::ObjectPrx> proxy;
    try
    {
        proxy = (*self->adapter)->addFacetWithUUID(wrapper, facet);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createProxy(proxy.value(), (*self->adapter)->getCommunicator());
}

extern "C" PyObject*
adapterAddDefaultServant(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* servant{nullptr};
    PyObject* categoryObj{nullptr};
    if (!PyArg_ParseTuple(args, "OO", &servant, &categoryObj))
    {
        return nullptr;
    }

    ServantWrapperPtr wrapper{nullptr};
    if (!getServantWrapper(servant, wrapper))
    {
        return nullptr;
    }

    string category;
    if (!getStringArg(categoryObj, "category", category))
    {
        return nullptr;
    }

    assert(self->adapter);
    try
    {
        (*self->adapter)->addDefaultServant(wrapper, category);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return Py_None;
}

extern "C" PyObject*
adapterRemove(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* identityType{lookupType("Ice.Identity")};
    PyObject* id{nullptr};
    if (!PyArg_ParseTuple(args, "O!", identityType, &id))
    {
        return nullptr;
    }

    Ice::Identity ident;
    if (!getIdentity(id, ident))
    {
        return nullptr;
    }

    assert(self->adapter);
    Ice::ObjectPtr obj{nullptr};
    try
    {
        obj = (*self->adapter)->remove(ident);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    if (obj)
    {
        auto wrapper = dynamic_pointer_cast<ServantWrapper>(obj);
        assert(wrapper);
        return wrapper->getObject();
    }
    else
    {
        return Py_None;
    }
}

extern "C" PyObject*
adapterRemoveFacet(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* identityType{lookupType("Ice.Identity")};
    PyObject* id{nullptr};
    PyObject* facetObj{nullptr};
    if (!PyArg_ParseTuple(args, "O!O", identityType, &id, &facetObj))
    {
        return nullptr;
    }

    Ice::Identity ident;
    if (!getIdentity(id, ident))
    {
        return nullptr;
    }

    string facet;
    if (!getStringArg(facetObj, "facet", facet))
    {
        return nullptr;
    }

    assert(self->adapter);
    Ice::ObjectPtr obj{nullptr};
    try
    {
        obj = (*self->adapter)->removeFacet(ident, facet);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    if (obj)
    {
        auto wrapper = dynamic_pointer_cast<ServantWrapper>(obj);
        assert(wrapper);
        return wrapper->getObject();
    }
    else
    {
        return Py_None;
    }
}

extern "C" PyObject*
adapterRemoveAllFacets(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* identityType{lookupType("Ice.Identity")};
    PyObject* id{nullptr};
    if (!PyArg_ParseTuple(args, "O!", identityType, &id))
    {
        return nullptr;
    }

    Ice::Identity ident;
    if (!getIdentity(id, ident))
    {
        return nullptr;
    }

    assert(self->adapter);
    Ice::FacetMap facetMap;
    try
    {
        facetMap = (*self->adapter)->removeAllFacets(ident);
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

    for (const auto& [facet, obj] : facetMap)
    {
        auto wrapper = dynamic_pointer_cast<ServantWrapper>(obj);
        assert(wrapper);
        if (PyDict_SetItemString(result.get(), facet.c_str(), wrapper->getObject()) < 0)
        {
            return nullptr;
        }
    }
    return result.release();
}

extern "C" PyObject*
adapterRemoveDefaultServant(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* categoryObj{nullptr};
    if (!PyArg_ParseTuple(args, "O", &categoryObj))
    {
        return nullptr;
    }

    string category;
    if (!getStringArg(categoryObj, "category", category))
    {
        return nullptr;
    }

    assert(self->adapter);
    Ice::ObjectPtr obj{nullptr};
    try
    {
        obj = (*self->adapter)->removeDefaultServant(category);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    if (obj)
    {
        auto wrapper = dynamic_pointer_cast<ServantWrapper>(obj);
        assert(wrapper);
        return wrapper->getObject();
    }
    else
    {
        return Py_None;
    }
}

extern "C" PyObject*
adapterFind(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* identityType{lookupType("Ice.Identity")};
    PyObject* id{nullptr};
    if (!PyArg_ParseTuple(args, "O!", identityType, &id))
    {
        return nullptr;
    }

    Ice::Identity ident;
    if (!getIdentity(id, ident))
    {
        return nullptr;
    }

    assert(self->adapter);
    Ice::ObjectPtr obj{nullptr};
    try
    {
        obj = (*self->adapter)->find(ident);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    if (!obj)
    {
        auto wrapper = dynamic_pointer_cast<ServantWrapper>(obj);
        assert(wrapper);
        return wrapper->getObject();
    }
    else
    {
        return Py_None;
    }
}

extern "C" PyObject*
adapterFindFacet(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* identityType{lookupType("Ice.Identity")};
    PyObject* id{nullptr};
    PyObject* facetObj{nullptr};
    if (!PyArg_ParseTuple(args, "O!O", identityType, &id, &facetObj))
    {
        return nullptr;
    }

    Ice::Identity ident;
    if (!getIdentity(id, ident))
    {
        return nullptr;
    }

    string facet;
    if (!getStringArg(facetObj, "facet", facet))
    {
        return nullptr;
    }

    assert(self->adapter);
    Ice::ObjectPtr obj{nullptr};
    try
    {
        obj = (*self->adapter)->findFacet(ident, facet);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    if (obj)
    {
        auto wrapper = dynamic_pointer_cast<ServantWrapper>(obj);
        assert(wrapper);
        return wrapper->getObject();
    }
    else
    {
        return Py_None;
    }
}

extern "C" PyObject*
adapterFindAllFacets(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* identityType{lookupType("Ice.Identity")};
    PyObject* id{nullptr};
    if (!PyArg_ParseTuple(args, "O!", identityType, &id))
    {
        return nullptr;
    }

    Ice::Identity ident;
    if (!getIdentity(id, ident))
    {
        return nullptr;
    }

    assert(self->adapter);
    Ice::FacetMap facetMap;
    try
    {
        facetMap = (*self->adapter)->findAllFacets(ident);
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

    for (const auto& [facet, obj] : facetMap)
    {
        auto wrapper = dynamic_pointer_cast<ServantWrapper>(obj);
        assert(wrapper);
        if (PyDict_SetItemString(result.get(), facet.c_str(), wrapper->getObject()) < 0)
        {
            return nullptr;
        }
    }
    return result.release();
}

extern "C" PyObject*
adapterFindByProxy(ObjectAdapterObject* self, PyObject* args)
{
    // We don't want to accept None here, so we can specify ProxyType and force the caller to supply a proxy object.
    PyObject* proxy{nullptr};
    if (!PyArg_ParseTuple(args, "O!", &ProxyType, &proxy))
    {
        return nullptr;
    }

    Ice::ObjectPrx prx{getProxy(proxy)};

    assert(self->adapter);
    Ice::ObjectPtr obj{nullptr};
    try
    {
        obj = (*self->adapter)->findByProxy(prx);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    if (obj)
    {
        auto wrapper = dynamic_pointer_cast<ServantWrapper>(obj);
        assert(wrapper);
        return wrapper->getObject();
    }
    else
    {
        return Py_None;
    }
}

extern "C" PyObject*
adapterFindDefaultServant(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* categoryObj{nullptr};
    if (!PyArg_ParseTuple(args, "O", &categoryObj))
    {
        return nullptr;
    }

    string category;
    if (!getStringArg(categoryObj, "category", category))
    {
        return nullptr;
    }

    assert(self->adapter);
    Ice::ObjectPtr obj{nullptr};
    try
    {
        obj = (*self->adapter)->findDefaultServant(category);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    if (obj)
    {
        auto wrapper = dynamic_pointer_cast<ServantWrapper>(obj);
        assert(wrapper);
        return wrapper->getObject();
    }
    else
    {
        return Py_None;
    }
}

extern "C" PyObject*
adapterAddServantLocator(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* locatorType{lookupType("Ice.ServantLocator")};
    PyObject* locator{nullptr};
    PyObject* categoryObj{nullptr};
    if (!PyArg_ParseTuple(args, "O!O", locatorType, &locator, &categoryObj))
    {
        return nullptr;
    }

    ServantLocatorWrapperPtr wrapper = make_shared<ServantLocatorWrapper>(locator);

    string category;
    if (!getStringArg(categoryObj, "category", category))
    {
        return nullptr;
    }
    assert(self->adapter);
    try
    {
        (*self->adapter)->addServantLocator(wrapper, category);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return Py_None;
}

extern "C" PyObject*
adapterRemoveServantLocator(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* categoryObj{nullptr};
    if (!PyArg_ParseTuple(args, "O", &categoryObj))
    {
        return nullptr;
    }

    string category;
    if (!getStringArg(categoryObj, "category", category))
    {
        return nullptr;
    }

    assert(self->adapter);
    Ice::ServantLocatorPtr locator;
    try
    {
        locator = (*self->adapter)->removeServantLocator(category);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    if (locator)
    {
        auto wrapper = dynamic_pointer_cast<ServantLocatorWrapper>(locator);
        assert(wrapper);
        return wrapper->getObject();
    }
    else
    {
        return Py_None;
    }
}

extern "C" PyObject*
adapterFindServantLocator(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* categoryObj{nullptr};
    if (!PyArg_ParseTuple(args, "O", &categoryObj))
    {
        return nullptr;
    }

    string category;
    if (!getStringArg(categoryObj, "category", category))
    {
        return nullptr;
    }

    assert(self->adapter);
    Ice::ServantLocatorPtr locator;
    try
    {
        locator = (*self->adapter)->findServantLocator(category);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    if (locator)
    {
        auto wrapper = dynamic_pointer_cast<ServantLocatorWrapper>(locator);
        assert(wrapper);
        return wrapper->getObject();
    }
    else
    {
        return Py_None;
    }
}

extern "C" PyObject*
adapterCreateProxy(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* identityType{lookupType("Ice.Identity")};
    PyObject* id{nullptr};
    if (!PyArg_ParseTuple(args, "O!", identityType, &id))
    {
        return nullptr;
    }

    Ice::Identity ident;
    if (!getIdentity(id, ident))
    {
        return nullptr;
    }

    assert(self->adapter);
    optional<Ice::ObjectPrx> proxy;
    try
    {
        proxy = (*self->adapter)->createProxy(ident);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createProxy(proxy.value(), (*self->adapter)->getCommunicator());
}

extern "C" PyObject*
adapterCreateDirectProxy(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* identityType{lookupType("Ice.Identity")};
    PyObject* id{nullptr};
    if (!PyArg_ParseTuple(args, "O!", identityType, &id))
    {
        return nullptr;
    }

    Ice::Identity ident;
    if (!getIdentity(id, ident))
    {
        return nullptr;
    }

    assert(self->adapter);
    optional<Ice::ObjectPrx> proxy;
    try
    {
        proxy = (*self->adapter)->createDirectProxy(ident);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createProxy(proxy.value(), (*self->adapter)->getCommunicator());
}

extern "C" PyObject*
adapterCreateIndirectProxy(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* identityType{lookupType("Ice.Identity")};
    PyObject* id{nullptr};
    if (!PyArg_ParseTuple(args, "O!", identityType, &id))
    {
        return nullptr;
    }

    Ice::Identity ident;
    if (!getIdentity(id, ident))
    {
        return nullptr;
    }

    assert(self->adapter);
    optional<Ice::ObjectPrx> proxy;
    try
    {
        proxy = (*self->adapter)->createIndirectProxy(ident);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createProxy(proxy.value(), (*self->adapter)->getCommunicator());
}

extern "C" PyObject*
adapterSetLocator(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* p{nullptr};
    if (!PyArg_ParseTuple(args, "O", &p))
    {
        return nullptr;
    }

    optional<Ice::ObjectPrx> proxy;
    if (!getProxyArg(p, "setLocator", "loc", proxy, "Ice.LocatorPrx"))
    {
        return nullptr;
    }

    optional<Ice::LocatorPrx> locator = Ice::uncheckedCast<Ice::LocatorPrx>(proxy);

    assert(self->adapter);
    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during blocking calls.
        (*self->adapter)->setLocator(locator);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return Py_None;
}

extern "C" PyObject*
adapterGetLocator(ObjectAdapterObject* self, PyObject* /*args*/)
{
    assert(self->adapter);
    optional<Ice::LocatorPrx> locator = (*self->adapter)->getLocator();
    if (locator)
    {
        PyObject* locatorProxyType{lookupType("Ice.LocatorPrx")};
        assert(locatorProxyType);
        return createProxy(*locator, (*self->adapter)->getCommunicator(), locatorProxyType);
    }
    else
    {
        return Py_None;
    }
}

extern "C" PyObject*
adapterGetEndpoints(ObjectAdapterObject* self, PyObject* /*args*/)
{
    assert(self->adapter);

    Ice::EndpointSeq endpoints = (*self->adapter)->getEndpoints();

    PyObjectHandle result{PyTuple_New(static_cast<int>(endpoints.size()))};
    int i = 0;
    for (const auto& endpoint : endpoints)
    {
        PyObjectHandle pyEndpoint{createEndpoint(endpoint)};
        if (!pyEndpoint.get())
        {
            return nullptr;
        }
        // PyTuple_SET_ITEM steals a reference.
        PyTuple_SET_ITEM(result.get(), i++, pyEndpoint.release());
    }

    return result.release();
}

extern "C" PyObject*
adapterGetPublishedEndpoints(ObjectAdapterObject* self, PyObject* /*args*/)
{
    assert(self->adapter);

    Ice::EndpointSeq endpoints = (*self->adapter)->getPublishedEndpoints();
    PyObjectHandle result{PyTuple_New(static_cast<int>(endpoints.size()))};
    int i = 0;
    for (const auto& endpoint : endpoints)
    {
        PyObjectHandle pyEndpoint{createEndpoint(endpoint)};
        if (!pyEndpoint.get())
        {
            return nullptr;
        }
        // PyTuple_SET_ITEM steals a reference.
        PyTuple_SET_ITEM(result.get(), i++, pyEndpoint.release());
    }
    return result.release();
}

extern "C" PyObject*
adapterSetPublishedEndpoints(ObjectAdapterObject* self, PyObject* args)
{
    assert(self->adapter);

    PyObject* endpoints{nullptr};
    if (!PyArg_ParseTuple(args, "O", &endpoints))
    {
        return nullptr;
    }

    if (!PyTuple_Check(endpoints) && !PyList_Check(endpoints))
    {
        PyErr_Format(PyExc_ValueError, "argument must be a tuple or list");
        return nullptr;
    }

    Ice::EndpointSeq seq;
    if (!toEndpointSeq(endpoints, seq))
    {
        return nullptr;
    }

    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during blocking calls.
        (*self->adapter)->setPublishedEndpoints(seq);
    }
    catch (const invalid_argument& ex)
    {
        PyErr_Format(PyExc_RuntimeError, "%s", ex.what());
        return nullptr;
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return Py_None;
}

static PyMethodDef AdapterMethods[] = {
    {"getName", reinterpret_cast<PyCFunction>(adapterGetName), METH_NOARGS, PyDoc_STR("getName() -> str")},
    {"getCommunicator",
     reinterpret_cast<PyCFunction>(adapterGetCommunicator),
     METH_NOARGS,
     PyDoc_STR("getCommunicator() -> Communicator")},
    {"activate", reinterpret_cast<PyCFunction>(adapterActivate), METH_NOARGS, PyDoc_STR("activate() -> None")},
    {"hold", reinterpret_cast<PyCFunction>(adapterHold), METH_NOARGS, PyDoc_STR("hold() -> None")},
    {"waitForHold",
     reinterpret_cast<PyCFunction>(adapterWaitForHold),
     METH_VARARGS,
     PyDoc_STR("waitForHold(timeout: int) -> None")},
    {"deactivate", reinterpret_cast<PyCFunction>(adapterDeactivate), METH_NOARGS, PyDoc_STR("deactivate() -> None")},
    {"waitForDeactivate",
     reinterpret_cast<PyCFunction>(adapterWaitForDeactivate),
     METH_VARARGS,
     PyDoc_STR("waitForDeactivate(timeout: int) -> bool")},
    {"isDeactivated",
     reinterpret_cast<PyCFunction>(adapterIsDeactivated),
     METH_NOARGS,
     PyDoc_STR("isDeactivated() -> bool")},
    {"destroy", reinterpret_cast<PyCFunction>(adapterDestroy), METH_NOARGS, PyDoc_STR("destroy() -> None")},
    {"add",
     reinterpret_cast<PyCFunction>(adapterAdd),
     METH_VARARGS,
     PyDoc_STR("add(servant: Ice.Object, id: Ice.Identity) -> Ice.ObjectPrx")},
    {"addFacet",
     reinterpret_cast<PyCFunction>(adapterAddFacet),
     METH_VARARGS,
     PyDoc_STR("addFacet(servant: Ice.Object, id: Ice.Identity, facet: str) -> Ice.ObjectPrx")},
    {"addWithUUID",
     reinterpret_cast<PyCFunction>(adapterAddWithUUID),
     METH_VARARGS,
     PyDoc_STR("addWithUUID(servant: Ice.Object) -> Ice.ObjectPrx")},
    {"addFacetWithUUID",
     reinterpret_cast<PyCFunction>(adapterAddFacetWithUUID),
     METH_VARARGS,
     PyDoc_STR("addFacetWithUUID(servant: Ice.Object, facet: str) -> Ice.ObjectPrx")},
    {"addDefaultServant",
     reinterpret_cast<PyCFunction>(adapterAddDefaultServant),
     METH_VARARGS,
     PyDoc_STR("addDefaultServant(servant: Ice.Object, category: str) -> None")},
    {"remove",
     reinterpret_cast<PyCFunction>(adapterRemove),
     METH_VARARGS,
     PyDoc_STR("remove(id: Ice.Identity) -> Ice.Object")},
    {"removeFacet",
     reinterpret_cast<PyCFunction>(adapterRemoveFacet),
     METH_VARARGS,
     PyDoc_STR("removeFacet(id: Ice.Identity, facet: str) -> Ice.Object")},
    {"removeAllFacets",
     reinterpret_cast<PyCFunction>(adapterRemoveAllFacets),
     METH_VARARGS,
     PyDoc_STR("removeAllFacets(id: Ice.Identity) -> dict[str, Ice.Object]")},
    {"removeDefaultServant",
     reinterpret_cast<PyCFunction>(adapterRemoveDefaultServant),
     METH_VARARGS,
     PyDoc_STR("removeDefaultServant(category: str) -> Ice.Object")},
    {"find",
     reinterpret_cast<PyCFunction>(adapterFind),
     METH_VARARGS,
     PyDoc_STR("find(identity: Ice.Identity) -> Ice.Object | None")},
    {"findFacet",
     reinterpret_cast<PyCFunction>(adapterFindFacet),
     METH_VARARGS,
     PyDoc_STR("findFacet(id: Ice.Identity, facet: str) -> Ice.Object | None")},
    {"findAllFacets",
     reinterpret_cast<PyCFunction>(adapterFindAllFacets),
     METH_VARARGS,
     PyDoc_STR("findAllFacets(id: Ice.Identity) -> dict[str, Ice.Object]")},
    {"findByProxy",
     reinterpret_cast<PyCFunction>(adapterFindByProxy),
     METH_VARARGS,
     PyDoc_STR("findByProxy(proxy: Ice.ObjectPrx) -> Ice.Object | None")},
    {"findDefaultServant",
     reinterpret_cast<PyCFunction>(adapterFindDefaultServant),
     METH_VARARGS,
     PyDoc_STR("findDefaultServant(category) -> Ice.Object")},
    {"addServantLocator",
     reinterpret_cast<PyCFunction>(adapterAddServantLocator),
     METH_VARARGS,
     PyDoc_STR("addServantLocator(Ice.ServantLocator, category) -> None")},
    {"removeServantLocator",
     reinterpret_cast<PyCFunction>(adapterRemoveServantLocator),
     METH_VARARGS,
     PyDoc_STR("removeServantLocator(category) -> Ice.ServantLocator")},
    {"findServantLocator",
     reinterpret_cast<PyCFunction>(adapterFindServantLocator),
     METH_VARARGS,
     PyDoc_STR("findServantLocator(category) -> Ice.ServantLocator")},
    {"createProxy",
     reinterpret_cast<PyCFunction>(adapterCreateProxy),
     METH_VARARGS,
     PyDoc_STR("createProxy(identity) -> Ice.ObjectPrx")},
    {"createDirectProxy",
     reinterpret_cast<PyCFunction>(adapterCreateDirectProxy),
     METH_VARARGS,
     PyDoc_STR("createDirectProxy(identity) -> Ice.ObjectPrx")},
    {"createIndirectProxy",
     reinterpret_cast<PyCFunction>(adapterCreateIndirectProxy),
     METH_VARARGS,
     PyDoc_STR("createIndirectProxy(identity) -> Ice.ObjectPrx")},
    {"setLocator",
     reinterpret_cast<PyCFunction>(adapterSetLocator),
     METH_VARARGS,
     PyDoc_STR("setLocator(proxy) -> None")},
    {"getLocator",
     reinterpret_cast<PyCFunction>(adapterGetLocator),
     METH_NOARGS,
     PyDoc_STR("getLocator() -> Ice.LocatorPrx")},
    {"getEndpoints",
     reinterpret_cast<PyCFunction>(adapterGetEndpoints),
     METH_NOARGS,
     PyDoc_STR("getEndpoints() -> None")},
    {"getPublishedEndpoints",
     reinterpret_cast<PyCFunction>(adapterGetPublishedEndpoints),
     METH_NOARGS,
     PyDoc_STR("getPublishedEndpoints() -> None")},
    {"setPublishedEndpoints",
     reinterpret_cast<PyCFunction>(adapterSetPublishedEndpoints),
     METH_VARARGS,
     PyDoc_STR("setPublishedEndpoints(endpoints) -> None")},
    {} /* sentinel */
};

namespace IcePy
{
    // clang-format off
    PyTypeObject ObjectAdapterType = {
        .ob_base = PyVarObject_HEAD_INIT(nullptr, 0)
        .tp_name = "IcePy.ObjectAdapter",
        .tp_basicsize = sizeof(ObjectAdapterObject),
        .tp_dealloc = reinterpret_cast<destructor>(adapterDealloc),
        .tp_flags = Py_TPFLAGS_DEFAULT,
        .tp_doc = PyDoc_STR("IcePy.ObjectAdapter"),
        .tp_methods = AdapterMethods,
        .tp_new = reinterpret_cast<newfunc>(adapterNew),
    };
    // clang-format on
}

bool
IcePy::initObjectAdapter(PyObject* module)
{
    if (PyType_Ready(&ObjectAdapterType) < 0)
    {
        return false;
    }

    if (PyModule_AddObject(module, "ObjectAdapter", reinterpret_cast<PyObject*>(&ObjectAdapterType)) < 0)
    {
        return false;
    }

    return true;
}

PyObject*
IcePy::createObjectAdapter(const Ice::ObjectAdapterPtr& adapter)
{
    auto* obj = reinterpret_cast<ObjectAdapterObject*>(ObjectAdapterType.tp_alloc(&ObjectAdapterType, 0));
    if (obj)
    {
        obj->adapter = new Ice::ObjectAdapterPtr(adapter);

        obj->deactivateException = nullptr;
        obj->deactivateFuture = nullptr;
        obj->deactivated = false;

        obj->holdMutex = new std::mutex;
        obj->holdException = nullptr;
        obj->holdFuture = nullptr;
        obj->held = false;
    }
    return reinterpret_cast<PyObject*>(obj);
}

Ice::ObjectAdapterPtr
IcePy::getObjectAdapter(PyObject* obj)
{
    assert(PyObject_IsInstance(obj, reinterpret_cast<PyObject*>(&ObjectAdapterType)));
    auto* oaobj = reinterpret_cast<ObjectAdapterObject*>(obj);
    return *oaobj->adapter;
}

PyObject*
IcePy::wrapObjectAdapter(const Ice::ObjectAdapterPtr& adapter)
{
    //
    // Create an Ice.ObjectAdapter wrapper for IcePy.ObjectAdapter.
    //
    PyObjectHandle adapterI{createObjectAdapter(adapter)};
    if (!adapterI.get())
    {
        return nullptr;
    }
    PyObject* wrapperType = lookupType("Ice.ObjectAdapter");
    assert(wrapperType);
    PyObjectHandle args{PyTuple_New(1)};
    if (!args.get())
    {
        return nullptr;
    }
    PyTuple_SET_ITEM(args.get(), 0, adapterI.release());
    return PyObject_Call(wrapperType, args.get(), nullptr);
}

Ice::ObjectAdapterPtr
IcePy::unwrapObjectAdapter(PyObject* obj)
{
    [[maybe_unused]] PyObject* wrapperType = lookupType("Ice.ObjectAdapter");
    assert(wrapperType);
    assert(PyObject_IsInstance(obj, wrapperType));
    PyObjectHandle impl{getAttr(obj, "_impl", false)};
    assert(impl.get());
    return getObjectAdapter(impl.get());
}
