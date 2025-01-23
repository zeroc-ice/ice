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

static unsigned long _mainThreadId;

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
            Cookie();
            ~Cookie();

            PyObject* current;
            ServantWrapperPtr servant;
            PyObject* cookie;
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
        PyObject* objectType = lookupType("Ice.Object");
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
IcePy::ServantLocatorWrapper::ServantLocatorWrapper(PyObject* locator) : _locator(locator)
{
    Py_INCREF(_locator);
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

    CookiePtr c = make_shared<Cookie>();
    c->current = createCurrent(current);
    if (!c->current)
    {
        throwPythonException();
    }

    //
    // Invoke locate on the Python object. We expect the object to
    // return either the servant by itself, or the servant in a tuple
    // with an optional cookie object.
    //
    PyObjectHandle res{PyObject_CallMethod(_locator, "locate", "O", c->current)};
    if (PyErr_Occurred())
    {
        PyException ex; // Retrieve the exception before another Python API call clears it.

        //
        // A locator that calls sys.exit() will raise the SystemExit exception.
        // This is normally caught by the interpreter, causing it to exit.
        // However, we have no way to pass this exception to the interpreter,
        // so we act on it directly.
        //
        ex.checkSystemExit();

        PyObject* userExceptionType = lookupType("Ice.UserException");
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

    PyObject* servantObj = nullptr;
    PyObject* cookieObj = Py_None;
    if (PyTuple_Check(res.get()))
    {
        if (PyTuple_GET_SIZE(res.get()) > 2)
        {
            const Ice::CommunicatorPtr com = current.adapter->getCommunicator();
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

    //
    // Verify that the servant is an Ice object.
    //
    if (!PyObject_IsInstance(servantObj, _objectType))
    {
        const Ice::CommunicatorPtr com = current.adapter->getCommunicator();
        if (com->getProperties()->getIcePropertyAsInt("Ice.Warn.Dispatch") > 0)
        {
            com->getLogger()->warning("return value of ServantLocator::locate is not an Ice object");
        }
        return nullptr;
    }

    //
    // Save state in our cookie and return a wrapper for the servant.
    //
    c->servant = createServantWrapper(servantObj);
    c->cookie = cookieObj;
    Py_INCREF(c->cookie);
    cookie = c;
    return c->servant;
}

void
IcePy::ServantLocatorWrapper::finished(const Ice::Current&, const Ice::ObjectPtr&, const shared_ptr<void>& cookie)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    CookiePtr c = static_pointer_cast<Cookie>(cookie);
    assert(c);

    ServantWrapperPtr wrapper = dynamic_pointer_cast<ServantWrapper>(c->servant);

    PyObjectHandle res{PyObject_CallMethod(_locator, "finished", "OOO", c->current, wrapper->getObject(), c->cookie)};
    if (PyErr_Occurred())
    {
        PyException ex; // Retrieve the exception before another Python API call clears it.

        //
        // A locator that calls sys.exit() will raise the SystemExit exception.
        // This is normally caught by the interpreter, causing it to exit.
        // However, we have no way to pass this exception to the interpreter,
        // so we act on it directly.
        //
        ex.checkSystemExit();

        PyObject* userExceptionType = lookupType("Ice.UserException");
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
        PyException ex; // Retrieve the exception before another Python API call clears it.

        //
        // A locator that calls sys.exit() will raise the SystemExit exception.
        // This is normally caught by the interpreter, causing it to exit.
        // However, we have no way to pass this exception to the interpreter,
        // so we act on it directly.
        //
        ex.checkSystemExit();

        ex.raise();
    }
}

PyObject*
IcePy::ServantLocatorWrapper::getObject()
{
    Py_INCREF(_locator);
    return _locator;
}

IcePy::ServantLocatorWrapper::Cookie::Cookie()
{
    current = 0;
    cookie = 0;
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
    string name;
    try
    {
        name = (*self->adapter)->getName();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createString(name);
}

extern "C" PyObject*
adapterGetCommunicator(ObjectAdapterObject* self, PyObject* /*args*/)
{
    assert(self->adapter);
    Ice::CommunicatorPtr communicator;
    try
    {
        communicator = (*self->adapter)->getCommunicator();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createCommunicator(communicator);
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
    if (PyThread_get_thread_ident() == _mainThreadId)
    {
        std::lock_guard lock(*self->holdMutex);

        if (!self->held)
        {
            if (self->holdFuture == nullptr)
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
    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during blocking calls.
        (*self->adapter)->deactivate();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

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
    if (PyThread_get_thread_ident() == _mainThreadId)
    {
        if (!self->deactivated)
        {
            if (self->deactivateFuture == nullptr)
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
        try
        {
            AllowThreads allowThreads; // Release Python's global interpreter lock during blocking calls.
            (*self->adapter)->waitForDeactivate();
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
adapterIsDeactivated(ObjectAdapterObject* self, PyObject* /*args*/)
{
    assert(self->adapter);
    try
    {
        return (*self->adapter)->isDeactivated() ? Py_True : Py_False;
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }
}

extern "C" PyObject*
adapterDestroy(ObjectAdapterObject* self, PyObject* /*args*/)
{
    assert(self->adapter);
    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during blocking calls.
        (*self->adapter)->destroy();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return Py_None;
}

extern "C" PyObject*
adapterAdd(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* identityType = lookupType("Ice.Identity");
    PyObject* servant;
    PyObject* id;
    if (!PyArg_ParseTuple(args, "OO!", &servant, identityType, &id))
    {
        return nullptr;
    }

    Ice::Identity ident;
    if (!getIdentity(id, ident))
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
    PyObject* identityType = lookupType("Ice.Identity");
    PyObject* servant;
    PyObject* id;
    PyObject* facetObj;
    if (!PyArg_ParseTuple(args, "OO!O", &servant, identityType, &id, &facetObj))
    {
        return nullptr;
    }

    Ice::Identity ident;
    if (!getIdentity(id, ident))
    {
        return nullptr;
    }

    ServantWrapperPtr wrapper;
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
    PyObject* servant;
    PyObject* facetObj;
    if (!PyArg_ParseTuple(args, "OO", &servant, &facetObj))
    {
        return nullptr;
    }

    ServantWrapperPtr wrapper;
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
    PyObject* servant;
    PyObject* categoryObj;
    if (!PyArg_ParseTuple(args, "OO", &servant, &categoryObj))
    {
        return nullptr;
    }

    ServantWrapperPtr wrapper;
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
    PyObject* identityType = lookupType("Ice.Identity");
    PyObject* id;
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
    Ice::ObjectPtr obj;
    try
    {
        obj = (*self->adapter)->remove(ident);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    if (!obj)
    {
        return Py_None;
    }

    ServantWrapperPtr wrapper = dynamic_pointer_cast<ServantWrapper>(obj);
    assert(wrapper);
    return wrapper->getObject();
}

extern "C" PyObject*
adapterRemoveFacet(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* identityType = lookupType("Ice.Identity");
    PyObject* id;
    PyObject* facetObj;
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
    Ice::ObjectPtr obj;
    try
    {
        obj = (*self->adapter)->removeFacet(ident, facet);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    if (!obj)
    {
        return Py_None;
    }

    ServantWrapperPtr wrapper = dynamic_pointer_cast<ServantWrapper>(obj);
    assert(wrapper);
    return wrapper->getObject();
}

extern "C" PyObject*
adapterRemoveAllFacets(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* identityType = lookupType("Ice.Identity");
    PyObject* id;
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

    for (Ice::FacetMap::iterator p = facetMap.begin(); p != facetMap.end(); ++p)
    {
        ServantWrapperPtr wrapper = dynamic_pointer_cast<ServantWrapper>(p->second);
        assert(wrapper);
        if (PyDict_SetItemString(result.get(), const_cast<char*>(p->first.c_str()), wrapper->getObject()) < 0)
        {
            return nullptr;
        }
    }

    return result.release();
}

extern "C" PyObject*
adapterRemoveDefaultServant(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* categoryObj;
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
    Ice::ObjectPtr obj;
    try
    {
        obj = (*self->adapter)->removeDefaultServant(category);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    if (!obj)
    {
        return Py_None;
    }

    ServantWrapperPtr wrapper = dynamic_pointer_cast<ServantWrapper>(obj);
    assert(wrapper);
    return wrapper->getObject();
}

extern "C" PyObject*
adapterFind(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* identityType = lookupType("Ice.Identity");
    PyObject* id;
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
    Ice::ObjectPtr obj;
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
        return Py_None;
    }

    ServantWrapperPtr wrapper = dynamic_pointer_cast<ServantWrapper>(obj);
    assert(wrapper);
    return wrapper->getObject();
}

extern "C" PyObject*
adapterFindFacet(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* identityType = lookupType("Ice.Identity");
    PyObject* id;
    PyObject* facetObj;
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
    Ice::ObjectPtr obj;
    try
    {
        obj = (*self->adapter)->findFacet(ident, facet);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    if (!obj)
    {
        return Py_None;
    }

    ServantWrapperPtr wrapper = dynamic_pointer_cast<ServantWrapper>(obj);
    assert(wrapper);
    return wrapper->getObject();
}

extern "C" PyObject*
adapterFindAllFacets(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* identityType = lookupType("Ice.Identity");
    PyObject* id;
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

    for (Ice::FacetMap::iterator p = facetMap.begin(); p != facetMap.end(); ++p)
    {
        ServantWrapperPtr wrapper = dynamic_pointer_cast<ServantWrapper>(p->second);
        assert(wrapper);
        if (PyDict_SetItemString(result.get(), const_cast<char*>(p->first.c_str()), wrapper->getObject()) < 0)
        {
            return nullptr;
        }
    }

    return result.release();
}

extern "C" PyObject*
adapterFindByProxy(ObjectAdapterObject* self, PyObject* args)
{
    //
    // We don't want to accept None here, so we can specify ProxyType and force
    // the caller to supply a proxy object.
    //
    PyObject* proxy;
    if (!PyArg_ParseTuple(args, "O!", &ProxyType, &proxy))
    {
        return nullptr;
    }

    Ice::ObjectPrx prx = getProxy(proxy);

    assert(self->adapter);
    Ice::ObjectPtr obj;
    try
    {
        obj = (*self->adapter)->findByProxy(prx);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    if (!obj)
    {
        return Py_None;
    }

    ServantWrapperPtr wrapper = dynamic_pointer_cast<ServantWrapper>(obj);
    assert(wrapper);
    return wrapper->getObject();
}

extern "C" PyObject*
adapterFindDefaultServant(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* categoryObj;
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
    Ice::ObjectPtr obj;
    try
    {
        obj = (*self->adapter)->findDefaultServant(category);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    if (!obj)
    {
        return Py_None;
    }

    ServantWrapperPtr wrapper = dynamic_pointer_cast<ServantWrapper>(obj);
    assert(wrapper);
    return wrapper->getObject();
}

extern "C" PyObject*
adapterAddServantLocator(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* locatorType = lookupType("Ice.ServantLocator");
    PyObject* locator;
    PyObject* categoryObj;
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
    PyObject* categoryObj;
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

    if (!locator)
    {
        return Py_None;
    }

    ServantLocatorWrapperPtr wrapper = dynamic_pointer_cast<ServantLocatorWrapper>(locator);
    assert(wrapper);
    return wrapper->getObject();
}

extern "C" PyObject*
adapterFindServantLocator(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* categoryObj;
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

    if (!locator)
    {
        return Py_None;
    }

    ServantLocatorWrapperPtr wrapper = dynamic_pointer_cast<ServantLocatorWrapper>(locator);
    assert(wrapper);
    return wrapper->getObject();
}

extern "C" PyObject*
adapterCreateProxy(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* identityType = lookupType("Ice.Identity");
    PyObject* id;
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
    PyObject* identityType = lookupType("Ice.Identity");
    PyObject* id;
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
    PyObject* identityType = lookupType("Ice.Identity");
    PyObject* id;
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
    PyObject* p;
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

    if (!locator)
    {
        return Py_None;
    }

    PyObject* locatorProxyType = lookupType("Ice.LocatorPrx");
    assert(locatorProxyType);
    return createProxy(locator.value(), (*self->adapter)->getCommunicator(), locatorProxyType);
}

extern "C" PyObject*
adapterGetEndpoints(ObjectAdapterObject* self, PyObject* /*args*/)
{
    assert(self->adapter);

    Ice::EndpointSeq endpoints;
    try
    {
        endpoints = (*self->adapter)->getEndpoints();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    int count = static_cast<int>(endpoints.size());
    PyObjectHandle result{PyTuple_New(count)};
    int i = 0;
    for (Ice::EndpointSeq::const_iterator p = endpoints.begin(); p != endpoints.end(); ++p, ++i)
    {
        PyObjectHandle endp{createEndpoint(*p)};
        if (!endp.get())
        {
            return nullptr;
        }
        PyTuple_SET_ITEM(result.get(), i, endp.release()); // PyTuple_SET_ITEM steals a reference.
    }

    return result.release();
}

extern "C" PyObject*
adapterGetPublishedEndpoints(ObjectAdapterObject* self, PyObject* /*args*/)
{
    assert(self->adapter);

    Ice::EndpointSeq endpoints;
    try
    {
        endpoints = (*self->adapter)->getPublishedEndpoints();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    int count = static_cast<int>(endpoints.size());
    PyObjectHandle result{PyTuple_New(count)};
    int i = 0;
    for (Ice::EndpointSeq::const_iterator p = endpoints.begin(); p != endpoints.end(); ++p, ++i)
    {
        PyObjectHandle endp{createEndpoint(*p)};
        if (!endp.get())
        {
            return nullptr;
        }
        PyTuple_SET_ITEM(result.get(), i, endp.release()); // PyTuple_SET_ITEM steals a reference.
    }

    return result.release();
}

extern "C" PyObject*
adapterSetPublishedEndpoints(ObjectAdapterObject* self, PyObject* args)
{
    assert(self->adapter);

    PyObject* endpoints;
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
    {"getName", reinterpret_cast<PyCFunction>(adapterGetName), METH_NOARGS, PyDoc_STR("getName() -> string")},
    {"getCommunicator",
     reinterpret_cast<PyCFunction>(adapterGetCommunicator),
     METH_NOARGS,
     PyDoc_STR("getCommunicator() -> Ice.Communicator")},
    {"activate", reinterpret_cast<PyCFunction>(adapterActivate), METH_NOARGS, PyDoc_STR("activate() -> None")},
    {"hold", reinterpret_cast<PyCFunction>(adapterHold), METH_NOARGS, PyDoc_STR("hold() -> None")},
    {"waitForHold",
     reinterpret_cast<PyCFunction>(adapterWaitForHold),
     METH_VARARGS,
     PyDoc_STR("waitForHold() -> None")},
    {"deactivate", reinterpret_cast<PyCFunction>(adapterDeactivate), METH_NOARGS, PyDoc_STR("deactivate() -> None")},
    {"waitForDeactivate",
     reinterpret_cast<PyCFunction>(adapterWaitForDeactivate),
     METH_VARARGS,
     PyDoc_STR("waitForDeactivate() -> None")},
    {"isDeactivated",
     reinterpret_cast<PyCFunction>(adapterIsDeactivated),
     METH_NOARGS,
     PyDoc_STR("isDeactivated() -> None")},
    {"destroy", reinterpret_cast<PyCFunction>(adapterDestroy), METH_NOARGS, PyDoc_STR("destroy() -> None")},
    {"add",
     reinterpret_cast<PyCFunction>(adapterAdd),
     METH_VARARGS,
     PyDoc_STR("add(servant, identity) -> Ice.ObjectPrx")},
    {"addFacet",
     reinterpret_cast<PyCFunction>(adapterAddFacet),
     METH_VARARGS,
     PyDoc_STR("addFacet(servant, identity, facet) -> Ice.ObjectPrx")},
    {"addWithUUID",
     reinterpret_cast<PyCFunction>(adapterAddWithUUID),
     METH_VARARGS,
     PyDoc_STR("addWithUUID(servant) -> Ice.ObjectPrx")},
    {"addFacetWithUUID",
     reinterpret_cast<PyCFunction>(adapterAddFacetWithUUID),
     METH_VARARGS,
     PyDoc_STR("addFacetWithUUID(servant, facet) -> Ice.ObjectPrx")},
    {"addDefaultServant",
     reinterpret_cast<PyCFunction>(adapterAddDefaultServant),
     METH_VARARGS,
     PyDoc_STR("addDefaultServant(servant, category) -> None")},
    {"remove", reinterpret_cast<PyCFunction>(adapterRemove), METH_VARARGS, PyDoc_STR("remove(identity) -> Ice.Object")},
    {"removeFacet",
     reinterpret_cast<PyCFunction>(adapterRemoveFacet),
     METH_VARARGS,
     PyDoc_STR("removeFacet(identity, facet) -> Ice.Object")},
    {"removeAllFacets",
     reinterpret_cast<PyCFunction>(adapterRemoveAllFacets),
     METH_VARARGS,
     PyDoc_STR("removeAllFacets(identity) -> dictionary")},
    {"removeDefaultServant",
     reinterpret_cast<PyCFunction>(adapterRemoveDefaultServant),
     METH_VARARGS,
     PyDoc_STR("removeDefaultServant(category) -> Ice.Object")},
    {"find", reinterpret_cast<PyCFunction>(adapterFind), METH_VARARGS, PyDoc_STR("find(identity) -> Ice.Object")},
    {"findFacet",
     reinterpret_cast<PyCFunction>(adapterFindFacet),
     METH_VARARGS,
     PyDoc_STR("findFacet(identity, facet) -> Ice.Object")},
    {"findAllFacets",
     reinterpret_cast<PyCFunction>(adapterFindAllFacets),
     METH_VARARGS,
     PyDoc_STR("findAllFacets(identity) -> dictionary")},
    {"findByProxy",
     reinterpret_cast<PyCFunction>(adapterFindByProxy),
     METH_VARARGS,
     PyDoc_STR("findByProxy(Ice.ObjectPrx) -> Ice.Object")},
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
    {0, 0} /* sentinel */
};

namespace IcePy
{
    PyTypeObject ObjectAdapterType = {
        /* The ob_type field must be initialized in the module init function
         * to be portable to Windows without using C++. */
        PyVarObject_HEAD_INIT(0, 0) "IcePy.ObjectAdapter", /* tp_name */
        sizeof(ObjectAdapterObject),                       /* tp_basicsize */
        0,                                                 /* tp_itemsize */
        /* methods */
        reinterpret_cast<destructor>(adapterDealloc), /* tp_dealloc */
        0,                                            /* tp_print */
        0,                                            /* tp_getattr */
        0,                                            /* tp_setattr */
        0,                                            /* tp_reserved */
        0,                                            /* tp_repr */
        0,                                            /* tp_as_number */
        0,                                            /* tp_as_sequence */
        0,                                            /* tp_as_mapping */
        0,                                            /* tp_hash */
        0,                                            /* tp_call */
        0,                                            /* tp_str */
        0,                                            /* tp_getattro */
        0,                                            /* tp_setattro */
        0,                                            /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT,                           /* tp_flags */
        0,                                            /* tp_doc */
        0,                                            /* tp_traverse */
        0,                                            /* tp_clear */
        0,                                            /* tp_richcompare */
        0,                                            /* tp_weaklistoffset */
        0,                                            /* tp_iter */
        0,                                            /* tp_iternext */
        AdapterMethods,                               /* tp_methods */
        0,                                            /* tp_members */
        0,                                            /* tp_getset */
        0,                                            /* tp_base */
        0,                                            /* tp_dict */
        0,                                            /* tp_descr_get */
        0,                                            /* tp_descr_set */
        0,                                            /* tp_dictoffset */
        0,                                            /* tp_init */
        0,                                            /* tp_alloc */
        reinterpret_cast<newfunc>(adapterNew),        /* tp_new */
        0,                                            /* tp_free */
        0,                                            /* tp_is_gc */
    };
}

bool
IcePy::initObjectAdapter(PyObject* module)
{
    _mainThreadId = PyThread_get_thread_ident();

    if (PyType_Ready(&ObjectAdapterType) < 0)
    {
        return false;
    }
    PyTypeObject* type = &ObjectAdapterType; // Necessary to prevent GCC's strict-alias warnings.
    if (PyModule_AddObject(module, "ObjectAdapter", reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    return true;
}

PyObject*
IcePy::createObjectAdapter(const Ice::ObjectAdapterPtr& adapter)
{
    ObjectAdapterObject* obj =
        reinterpret_cast<ObjectAdapterObject*>(ObjectAdapterType.tp_alloc(&ObjectAdapterType, 0));
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
    ObjectAdapterObject* oaobj = reinterpret_cast<ObjectAdapterObject*>(obj);
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
    return PyObject_Call(wrapperType, args.get(), 0);
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
