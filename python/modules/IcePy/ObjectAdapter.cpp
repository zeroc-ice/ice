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
#include <ObjectAdapter.h>
#include <Communicator.h>
#include <Current.h>
#include <Endpoint.h>
#include <Operation.h>
#include <Proxy.h>
#include <Thread.h>
#include <Types.h>
#include <Util.h>
#include <Ice/Communicator.h>
#include <Ice/LocalException.h>
#include <Ice/Locator.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/Router.h>
#include <Ice/ServantLocator.h>

#include <pythread.h>

using namespace std;
using namespace IcePy;

static long _mainThreadId;

namespace IcePy
{

typedef InvokeThread<Ice::ObjectAdapter> AdapterInvokeThread;
typedef IceUtil::Handle<AdapterInvokeThread> AdapterInvokeThreadPtr;

struct ObjectAdapterObject
{
    PyObject_HEAD
    Ice::ObjectAdapterPtr* adapter;
    IceUtil::Monitor<IceUtil::Mutex>* deactivateMonitor;
    AdapterInvokeThreadPtr* deactivateThread;
    bool deactivated;
    IceUtil::Monitor<IceUtil::Mutex>* holdMonitor;
    AdapterInvokeThreadPtr* holdThread;
    bool held;
};

class ServantLocatorWrapper : public Ice::ServantLocator
{
public:

    ServantLocatorWrapper(PyObject*);
    ~ServantLocatorWrapper();

    virtual Ice::ObjectPtr locate(const Ice::Current&, Ice::LocalObjectPtr&);

    virtual void finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr&);

    virtual void deactivate(const string&);

    PyObject* getObject();

private:

    //
    // This object is created in locate() and destroyed after finished().
    //
    struct Cookie : public Ice::LocalObject
    {
        Cookie();
        ~Cookie();

        PyObject* current;
        ServantWrapperPtr servant;
        PyObject* cookie;
    };
    typedef IceUtil::Handle<Cookie> CookiePtr;

    PyObject* _locator;
    PyObject* _objectType;
};
typedef IceUtil::Handle<ServantLocatorWrapper> ServantLocatorWrapperPtr;

}

namespace
{

bool getServantWrapper(PyObject* servant, ServantWrapperPtr& wrapper)
{
    PyObject* objectType = lookupType("Ice.Object");
    if(servant != Py_None && !PyObject_IsInstance(servant, objectType))
    {
        PyErr_Format(PyExc_ValueError, STRCAST("expected Ice object or None"));
        return false;
    }

    if(servant != Py_None)
    {
        wrapper = createServantWrapper(servant);
        if(PyErr_Occurred())
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
IcePy::ServantLocatorWrapper::ServantLocatorWrapper(PyObject* locator) :
    _locator(locator)
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
IcePy::ServantLocatorWrapper::locate(const Ice::Current& current, Ice::LocalObjectPtr& cookie)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    CookiePtr c = new Cookie;
    c->current = createCurrent(current);
    if(!c->current)
    {
        throwPythonException();
    }

    //
    // Invoke locate on the Python object. We expect the object to
    // return either the servant by itself, or the servant in a tuple
    // with an optional cookie object.
    //
    PyObjectHandle res = PyObject_CallMethod(_locator, STRCAST("locate"), STRCAST("O"), c->current);
    if(PyErr_Occurred())
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
        if(PyObject_IsInstance(ex.ex.get(), userExceptionType))
        {
            throw ExceptionWriter(current.adapter->getCommunicator(), ex.ex);
        }

        ex.raise();
    }

    if(res.get() == Py_None)
    {
        return 0;
    }

    PyObject* servantObj = 0;
    PyObject* cookieObj = Py_None;
    if(PyTuple_Check(res.get()))
    {
        if(PyTuple_GET_SIZE(res.get()) > 2)
        {
            PyErr_Warn(PyExc_RuntimeWarning, STRCAST("invalid return value for ServantLocator::locate"));
            return 0;
        }
        servantObj = PyTuple_GET_ITEM(res.get(), 0);
        if(PyTuple_GET_SIZE(res.get()) > 1)
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
    if(!PyObject_IsInstance(servantObj, _objectType))
    {
        PyErr_Warn(PyExc_RuntimeWarning, STRCAST("return value of ServantLocator::locate is not an Ice object"));
        return 0;
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
IcePy::ServantLocatorWrapper::finished(const Ice::Current& current, const Ice::ObjectPtr&,
                                       const Ice::LocalObjectPtr& cookie)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    CookiePtr c = CookiePtr::dynamicCast(cookie);
    assert(c);

    ServantWrapperPtr wrapper = ServantWrapperPtr::dynamicCast(c->servant);
    PyObjectHandle servantObj = wrapper->getObject();

    PyObjectHandle res = PyObject_CallMethod(_locator, STRCAST("finished"), STRCAST("OOO"), c->current,
                                             servantObj.get(), c->cookie);
    if(PyErr_Occurred())
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
        if(PyObject_IsInstance(ex.ex.get(), userExceptionType))
        {
            throw ExceptionWriter(current.adapter->getCommunicator(), ex.ex);
        }

        ex.raise();
    }
}

void
IcePy::ServantLocatorWrapper::deactivate(const string& category)
{
    AdoptThread adoptThread; // Ensure the current thread is able to call into Python.

    PyObjectHandle res = PyObject_CallMethod(_locator, STRCAST("deactivate"), STRCAST("s"), category.c_str());
    if(PyErr_Occurred())
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

#ifdef WIN32
extern "C"
#endif
static ObjectAdapterObject*
adapterNew(PyTypeObject* /*type*/, PyObject* /*args*/, PyObject* /*kwds*/)
{
    PyErr_Format(PyExc_RuntimeError, STRCAST("Use communicator.createObjectAdapter to create an adapter"));
    return 0;
}

#ifdef WIN32
extern "C"
#endif
static void
adapterDealloc(ObjectAdapterObject* self)
{
    if(self->deactivateThread)
    {
        (*self->deactivateThread)->getThreadControl().join();
    }
    if(self->holdThread)
    {
        (*self->holdThread)->getThreadControl().join();
    }
    delete self->adapter;
    delete self->deactivateMonitor;
    delete self->deactivateThread;
    delete self->holdMonitor;
    delete self->holdThread;
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
adapterGetName(ObjectAdapterObject* self)
{
    assert(self->adapter);
    string name;
    try
    {
        name = (*self->adapter)->getName();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createString(name);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
adapterGetCommunicator(ObjectAdapterObject* self)
{
    assert(self->adapter);
    Ice::CommunicatorPtr communicator;
    try
    {
        communicator = (*self->adapter)->getCommunicator();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createCommunicator(communicator);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
adapterActivate(ObjectAdapterObject* self)
{
    assert(self->adapter);
    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during blocking calls.
        (*self->adapter)->activate();

        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*self->holdMonitor);
        self->held = false;
        if(self->holdThread)
        {
            (*self->holdThread)->getThreadControl().join();
            delete self->holdThread;
            self->holdThread = 0;
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
adapterHold(ObjectAdapterObject* self)
{
    assert(self->adapter);
    try
    {
        (*self->adapter)->hold();
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
    if(!PyArg_ParseTuple(args, STRCAST("i"), &timeout))
    {
        return 0;
    }

    assert(timeout > 0);
    assert(self->adapter);

    //
    // Do not call waitForHold from the main thread, because it prevents
    // signals (such as keyboard interrupts) from being delivered to Python.
    //
    if(PyThread_get_thread_ident() == _mainThreadId)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*self->holdMonitor);

        if(!self->held)
        {
            if(self->holdThread == 0)
            {
                AdapterInvokeThreadPtr t = new AdapterInvokeThread(*self->adapter,
                                                                   &Ice::ObjectAdapter::waitForHold,
                                                                   *self->holdMonitor, self->held);
                self->holdThread = new AdapterInvokeThreadPtr(t);
                t->start();
            }

            bool done;
            {
                AllowThreads allowThreads; // Release Python's global interpreter lock during blocking calls.
                done = (*self->holdMonitor).timedWait(IceUtil::Time::milliSeconds(timeout));
            }

            if(!done)
            {
                PyRETURN_FALSE;
            }
        }

        assert(self->held);

        Ice::Exception* ex = (*self->holdThread)->getException();
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
            (*self->adapter)->waitForHold();
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
adapterDeactivate(ObjectAdapterObject* self)
{
    assert(self->adapter);
    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during blocking calls.
        (*self->adapter)->deactivate();
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
    if(!PyArg_ParseTuple(args, STRCAST("i"), &timeout))
    {
        return 0;
    }

    assert(timeout > 0);
    assert(self->adapter);

    //
    // Do not call waitForDeactivate from the main thread, because it prevents
    // signals (such as keyboard interrupts) from being delivered to Python.
    //
    if(PyThread_get_thread_ident() == _mainThreadId)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*self->deactivateMonitor);

        if(!self->deactivated)
        {
            if(self->deactivateThread == 0)
            {
                AdapterInvokeThreadPtr t = new AdapterInvokeThread(*self->adapter,
                                                                   &Ice::ObjectAdapter::waitForDeactivate,
                                                                   *self->deactivateMonitor, self->deactivated);
                self->deactivateThread = new AdapterInvokeThreadPtr(t);
                t->start();
            }
            
            while(!self->deactivated)
            {
                bool done;
                {
                    AllowThreads allowThreads; // Release Python's global interpreter lock during blocking calls.
                    done = (*self->deactivateMonitor).timedWait(IceUtil::Time::milliSeconds(timeout));
                }
                
                if(!done)
                {
                    PyRETURN_FALSE;
                }
            }
        }

        assert(self->deactivated);

        Ice::Exception* ex = (*self->deactivateThread)->getException();
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
            (*self->adapter)->waitForDeactivate();
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
adapterIsDeactivated(ObjectAdapterObject* self)
{
    assert(self->adapter);
    try
    {
        (*self->adapter)->isDeactivated();
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
adapterDestroy(ObjectAdapterObject* self)
{
    assert(self->adapter);
    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during blocking calls.
        (*self->adapter)->destroy();
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
adapterAdd(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* identityType = lookupType("Ice.Identity");
    PyObject* servant;
    PyObject* id;
    if(!PyArg_ParseTuple(args, STRCAST("OO!"), &servant, identityType, &id))
    {
        return 0;
    }

    Ice::Identity ident;
    if(!getIdentity(id, ident))
    {
        return 0;
    }

    ServantWrapperPtr wrapper;
    if(!getServantWrapper(servant, wrapper))
    {
        return 0;
    }

    assert(self->adapter);
    Ice::ObjectPrx proxy;
    try
    {
        proxy = (*self->adapter)->add(wrapper, ident);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(proxy, (*self->adapter)->getCommunicator());
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
adapterAddFacet(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* identityType = lookupType("Ice.Identity");
    PyObject* servant;
    PyObject* id;
    PyObject* facetObj;
    if(!PyArg_ParseTuple(args, STRCAST("OO!O"), &servant, identityType, &id, &facetObj))
    {
        return 0;
    }

    Ice::Identity ident;
    if(!getIdentity(id, ident))
    {
        return 0;
    }

    ServantWrapperPtr wrapper;
    if(!getServantWrapper(servant, wrapper))
    {
        return 0;
    }

    string facet;
    if(!getStringArg(facetObj, "facet", facet))
    {
        return 0;
    }

    assert(self->adapter);
    Ice::ObjectPrx proxy;
    try
    {
        proxy = (*self->adapter)->addFacet(wrapper, ident, facet);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(proxy, (*self->adapter)->getCommunicator());
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
adapterAddWithUUID(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* servant;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &servant))
    {
        return 0;
    }

    ServantWrapperPtr wrapper;
    if(!getServantWrapper(servant, wrapper))
    {
        return 0;
    }

    assert(self->adapter);
    Ice::ObjectPrx proxy;
    try
    {
        proxy = (*self->adapter)->addWithUUID(wrapper);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(proxy, (*self->adapter)->getCommunicator());
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
adapterAddFacetWithUUID(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* servant;
    PyObject* facetObj;
    if(!PyArg_ParseTuple(args, STRCAST("OO"), &servant, &facetObj))
    {
        return 0;
    }

    ServantWrapperPtr wrapper;
    if(!getServantWrapper(servant, wrapper))
    {
        return 0;
    }

    string facet;
    if(!getStringArg(facetObj, "facet", facet))
    {
        return 0;
    }

    assert(self->adapter);
    Ice::ObjectPrx proxy;
    try
    {
        proxy = (*self->adapter)->addFacetWithUUID(wrapper, facet);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(proxy, (*self->adapter)->getCommunicator());
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
adapterAddDefaultServant(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* servant;
    PyObject* categoryObj;
    if(!PyArg_ParseTuple(args, STRCAST("OO"), &servant, &categoryObj))
    {
        return 0;
    }

    ServantWrapperPtr wrapper;
    if(!getServantWrapper(servant, wrapper))
    {
        return 0;
    }

    string category;
    if(!getStringArg(categoryObj, "category", category))
    {
        return 0;
    }

    assert(self->adapter);
    try
    {
       (*self->adapter)->addDefaultServant(wrapper, category);
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
adapterRemove(ObjectAdapterObject* self, PyObject* args)
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

    assert(self->adapter);
    Ice::ObjectPtr obj;
    try
    {
        obj = (*self->adapter)->remove(ident);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    if(!obj)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    ServantWrapperPtr wrapper = ServantWrapperPtr::dynamicCast(obj);
    assert(wrapper);
    return wrapper->getObject();
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
adapterRemoveFacet(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* identityType = lookupType("Ice.Identity");
    PyObject* id;
    PyObject* facetObj;
    if(!PyArg_ParseTuple(args, STRCAST("O!O"), identityType, &id, &facetObj))
    {
        return 0;
    }

    Ice::Identity ident;
    if(!getIdentity(id, ident))
    {
        return 0;
    }

    string facet;
    if(!getStringArg(facetObj, "facet", facet))
    {
        return 0;
    }

    assert(self->adapter);
    Ice::ObjectPtr obj;
    try
    {
        obj = (*self->adapter)->removeFacet(ident, facet);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    if(!obj)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    ServantWrapperPtr wrapper = ServantWrapperPtr::dynamicCast(obj);
    assert(wrapper);
    return wrapper->getObject();
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
adapterRemoveAllFacets(ObjectAdapterObject* self, PyObject* args)
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

    assert(self->adapter);
    Ice::FacetMap facetMap;
    try
    {
        facetMap = (*self->adapter)->removeAllFacets(ident);
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

    for(Ice::FacetMap::iterator p = facetMap.begin(); p != facetMap.end(); ++p)
    {
        ServantWrapperPtr wrapper = ServantWrapperPtr::dynamicCast(p->second);
        assert(wrapper);
        PyObjectHandle obj = wrapper->getObject();
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
adapterRemoveDefaultServant(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* categoryObj;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &categoryObj))
    {
        return 0;
    }

    string category;
    if(!getStringArg(categoryObj, "category", category))
    {
        return 0;
    }

    assert(self->adapter);
    Ice::ObjectPtr obj;
    try
    {
        obj = (*self->adapter)->removeDefaultServant(category);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    if(!obj)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    ServantWrapperPtr wrapper = ServantWrapperPtr::dynamicCast(obj);
    assert(wrapper);
    return wrapper->getObject();
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
adapterFind(ObjectAdapterObject* self, PyObject* args)
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

    assert(self->adapter);
    Ice::ObjectPtr obj;
    try
    {
        obj = (*self->adapter)->find(ident);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    if(!obj)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    ServantWrapperPtr wrapper = ServantWrapperPtr::dynamicCast(obj);
    assert(wrapper);
    return wrapper->getObject();
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
adapterFindFacet(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* identityType = lookupType("Ice.Identity");
    PyObject* id;
    PyObject* facetObj;
    if(!PyArg_ParseTuple(args, STRCAST("O!O"), identityType, &id, &facetObj))
    {
        return 0;
    }

    Ice::Identity ident;
    if(!getIdentity(id, ident))
    {
        return 0;
    }

    string facet;
    if(!getStringArg(facetObj, "facet", facet))
    {
        return 0;
    }

    assert(self->adapter);
    Ice::ObjectPtr obj;
    try
    {
        obj = (*self->adapter)->findFacet(ident, facet);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    if(!obj)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    ServantWrapperPtr wrapper = ServantWrapperPtr::dynamicCast(obj);
    assert(wrapper);
    return wrapper->getObject();
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
adapterFindAllFacets(ObjectAdapterObject* self, PyObject* args)
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

    assert(self->adapter);
    Ice::FacetMap facetMap;
    try
    {
        facetMap = (*self->adapter)->findAllFacets(ident);
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

    for(Ice::FacetMap::iterator p = facetMap.begin(); p != facetMap.end(); ++p)
    {
        ServantWrapperPtr wrapper = ServantWrapperPtr::dynamicCast(p->second);
        assert(wrapper);
        PyObjectHandle obj = wrapper->getObject();
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
adapterFindByProxy(ObjectAdapterObject* self, PyObject* args)
{
    //
    // We don't want to accept None here, so we can specify ProxyType and force
    // the caller to supply a proxy object.
    //
    PyObject* proxy;
    if(!PyArg_ParseTuple(args, STRCAST("O!"), &ProxyType, &proxy))
    {
        return 0;
    }

    Ice::ObjectPrx prx = getProxy(proxy);

    assert(self->adapter);
    Ice::ObjectPtr obj;
    try
    {
        obj = (*self->adapter)->findByProxy(prx);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    if(!obj)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    ServantWrapperPtr wrapper = ServantWrapperPtr::dynamicCast(obj);
    assert(wrapper);
    return wrapper->getObject();
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
adapterFindDefaultServant(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* categoryObj;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &categoryObj))
    {
        return 0;
    }

    string category;
    if(!getStringArg(categoryObj, "category", category))
    {
        return 0;
    }

    assert(self->adapter);
    Ice::ObjectPtr obj;
    try
    {
        obj = (*self->adapter)->findDefaultServant(category);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    if(!obj)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    ServantWrapperPtr wrapper = ServantWrapperPtr::dynamicCast(obj);
    assert(wrapper);
    return wrapper->getObject();
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
adapterAddServantLocator(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* locatorType = lookupType("Ice.ServantLocator");
    PyObject* locator;
    PyObject* categoryObj;
    if(!PyArg_ParseTuple(args, STRCAST("O!O"), locatorType, &locator, &categoryObj))
    {
        return 0;
    }

    ServantLocatorWrapperPtr wrapper = new ServantLocatorWrapper(locator);

    string category;
    if(!getStringArg(categoryObj, "category", category))
    {
        return 0;
    }
    assert(self->adapter);
    try
    {
        (*self->adapter)->addServantLocator(wrapper, category);
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
adapterRemoveServantLocator(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* categoryObj;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &categoryObj))
    {
        return 0;
    }

    string category;
    if(!getStringArg(categoryObj, "category", category))
    {
        return 0;
    }

    assert(self->adapter);
    Ice::ServantLocatorPtr locator;
    try
    {
        locator = (*self->adapter)->removeServantLocator(category);
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

    ServantLocatorWrapperPtr wrapper = ServantLocatorWrapperPtr::dynamicCast(locator);
    assert(wrapper);
    return wrapper->getObject();
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
adapterFindServantLocator(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* categoryObj;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &categoryObj))
    {
        return 0;
    }

    string category;
    if(!getStringArg(categoryObj, "category", category))
    {
        return 0;
    }

    assert(self->adapter);
    Ice::ServantLocatorPtr locator;
    try
    {
        locator = (*self->adapter)->findServantLocator(category);
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

    ServantLocatorWrapperPtr wrapper = ServantLocatorWrapperPtr::dynamicCast(locator);
    assert(wrapper);
    return wrapper->getObject();
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
adapterCreateProxy(ObjectAdapterObject* self, PyObject* args)
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

    assert(self->adapter);
    Ice::ObjectPrx proxy;
    try
    {
        proxy = (*self->adapter)->createProxy(ident);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(proxy, (*self->adapter)->getCommunicator());
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
adapterCreateDirectProxy(ObjectAdapterObject* self, PyObject* args)
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

    assert(self->adapter);
    Ice::ObjectPrx proxy;
    try
    {
        proxy = (*self->adapter)->createDirectProxy(ident);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(proxy, (*self->adapter)->getCommunicator());
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
adapterCreateIndirectProxy(ObjectAdapterObject* self, PyObject* args)
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

    assert(self->adapter);
    Ice::ObjectPrx proxy;
    try
    {
        proxy = (*self->adapter)->createIndirectProxy(ident);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(proxy, (*self->adapter)->getCommunicator());
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
adapterSetLocator(ObjectAdapterObject* self, PyObject* args)
{
    PyObject* p;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &p))
    {
        return 0;
    }

    Ice::ObjectPrx proxy;
    if(!getProxyArg(p, "setLocator", "loc", proxy, "Ice.LocatorPrx"))
    {
        return 0;
    }

    Ice::LocatorPrx locator = Ice::LocatorPrx::uncheckedCast(proxy);

    assert(self->adapter);
    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during blocking calls.
        (*self->adapter)->setLocator(locator);
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
adapterGetLocator(ObjectAdapterObject* self)
{
    assert(self->adapter);
    Ice::LocatorPrx locator;
    try
    {
        locator = (*self->adapter)->getLocator();
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
    return createProxy(locator, (*self->adapter)->getCommunicator(), locatorProxyType);
}   


#ifdef WIN32
extern "C"
#endif
static PyObject*
adapterRefreshPublishedEndpoints(ObjectAdapterObject* self)
{
    assert(self->adapter);
    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during blocking calls.
        (*self->adapter)->refreshPublishedEndpoints();
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
adapterGetEndpoints(ObjectAdapterObject* self)
{
    assert(self->adapter);

    Ice::EndpointSeq endpoints;
    try
    {
        endpoints = (*self->adapter)->getEndpoints();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    } 

    int count = static_cast<int>(endpoints.size());
    PyObjectHandle result = PyTuple_New(count);
    int i = 0;
    for(Ice::EndpointSeq::const_iterator p = endpoints.begin(); p != endpoints.end(); ++p, ++i)
    {
        PyObjectHandle endp = createEndpoint(*p);
        if(!endp.get())
        {
            return 0;
        }
        PyTuple_SET_ITEM(result.get(), i, endp.release()); // PyTuple_SET_ITEM steals a reference.
    }

    return result.release();
}

#ifdef WIN32
extern "C"
#endif 
static PyObject*
adapterGetPublishedEndpoints(ObjectAdapterObject* self)
{
    assert(self->adapter);
        
    Ice::EndpointSeq endpoints;
    try
    { 
        endpoints = (*self->adapter)->getPublishedEndpoints();
    } 
    catch(const Ice::Exception& ex)
    { 
        setPythonException(ex);
        return 0;
    }

    int count = static_cast<int>(endpoints.size());
    PyObjectHandle result = PyTuple_New(count);
    int i = 0;
    for(Ice::EndpointSeq::const_iterator p = endpoints.begin(); p != endpoints.end(); ++p, ++i)
    {
        PyObjectHandle endp = createEndpoint(*p);
        if(!endp.get())
        {
            return 0;
        }
        PyTuple_SET_ITEM(result.get(), i, endp.release()); // PyTuple_SET_ITEM steals a reference.
    }

    return result.release();
}

static PyMethodDef AdapterMethods[] =
{
    { STRCAST("getName"), reinterpret_cast<PyCFunction>(adapterGetName), METH_NOARGS,
        PyDoc_STR(STRCAST("getName() -> string")) },
    { STRCAST("getCommunicator"), reinterpret_cast<PyCFunction>(adapterGetCommunicator), METH_NOARGS,
        PyDoc_STR(STRCAST("getCommunicator() -> Ice.Communicator")) },
    { STRCAST("activate"), reinterpret_cast<PyCFunction>(adapterActivate), METH_NOARGS,
        PyDoc_STR(STRCAST("activate() -> None")) },
    { STRCAST("hold"), reinterpret_cast<PyCFunction>(adapterHold), METH_NOARGS,
        PyDoc_STR(STRCAST("hold() -> None")) },
    { STRCAST("waitForHold"), reinterpret_cast<PyCFunction>(adapterWaitForHold), METH_VARARGS,
        PyDoc_STR(STRCAST("waitForHold() -> None")) },
    { STRCAST("deactivate"), reinterpret_cast<PyCFunction>(adapterDeactivate), METH_NOARGS,
        PyDoc_STR(STRCAST("deactivate() -> None")) },
    { STRCAST("waitForDeactivate"), reinterpret_cast<PyCFunction>(adapterWaitForDeactivate), METH_VARARGS,
        PyDoc_STR(STRCAST("waitForDeactivate() -> None")) },
    { STRCAST("isDeactivated"), reinterpret_cast<PyCFunction>(adapterIsDeactivated), METH_NOARGS,
        PyDoc_STR(STRCAST("isDeactivatied() -> None")) },
    { STRCAST("destroy"), reinterpret_cast<PyCFunction>(adapterDestroy), METH_NOARGS,
        PyDoc_STR(STRCAST("destroy() -> None")) },
    { STRCAST("add"), reinterpret_cast<PyCFunction>(adapterAdd), METH_VARARGS,
        PyDoc_STR(STRCAST("add(servant, identity) -> Ice.ObjectPrx")) },
    { STRCAST("addFacet"), reinterpret_cast<PyCFunction>(adapterAddFacet), METH_VARARGS,
        PyDoc_STR(STRCAST("addFacet(servant, identity, facet) -> Ice.ObjectPrx")) },
    { STRCAST("addWithUUID"), reinterpret_cast<PyCFunction>(adapterAddWithUUID), METH_VARARGS,
        PyDoc_STR(STRCAST("addWithUUID(servant) -> Ice.ObjectPrx")) },
    { STRCAST("addFacetWithUUID"), reinterpret_cast<PyCFunction>(adapterAddFacetWithUUID), METH_VARARGS,
        PyDoc_STR(STRCAST("addFacetWithUUID(servant, facet) -> Ice.ObjectPrx")) },
    { STRCAST("addDefaultServant"), reinterpret_cast<PyCFunction>(adapterAddDefaultServant), METH_VARARGS,
        PyDoc_STR(STRCAST("addDefaultServant(servant, category) -> None")) },
    { STRCAST("remove"), reinterpret_cast<PyCFunction>(adapterRemove), METH_VARARGS,
        PyDoc_STR(STRCAST("remove(identity) -> Ice.Object")) },
    { STRCAST("removeFacet"), reinterpret_cast<PyCFunction>(adapterRemoveFacet), METH_VARARGS,
        PyDoc_STR(STRCAST("removeFacet(identity, facet) -> Ice.Object")) },
    { STRCAST("removeAllFacets"), reinterpret_cast<PyCFunction>(adapterRemoveAllFacets), METH_VARARGS,
        PyDoc_STR(STRCAST("removeAllFacets(identity) -> dictionary")) },
    { STRCAST("removeDefaultServant"), reinterpret_cast<PyCFunction>(adapterRemoveDefaultServant), METH_VARARGS,
        PyDoc_STR(STRCAST("removeDefaultServant(category) -> Ice.Object")) },
    { STRCAST("find"), reinterpret_cast<PyCFunction>(adapterFind), METH_VARARGS,
        PyDoc_STR(STRCAST("find(identity) -> Ice.Object")) },
    { STRCAST("findFacet"), reinterpret_cast<PyCFunction>(adapterFindFacet), METH_VARARGS,
        PyDoc_STR(STRCAST("findFacet(identity, facet) -> Ice.Object")) },
    { STRCAST("findAllFacets"), reinterpret_cast<PyCFunction>(adapterFindAllFacets), METH_VARARGS,
        PyDoc_STR(STRCAST("findAllFacets(identity) -> dictionary")) },
    { STRCAST("findByProxy"), reinterpret_cast<PyCFunction>(adapterFindByProxy), METH_VARARGS,
        PyDoc_STR(STRCAST("findByProxy(Ice.ObjectPrx) -> Ice.Object")) },
    { STRCAST("findDefaultServant"), reinterpret_cast<PyCFunction>(adapterFindDefaultServant), METH_VARARGS,
        PyDoc_STR(STRCAST("findDefaultServant(category) -> Ice.Object")) },
    { STRCAST("addServantLocator"), reinterpret_cast<PyCFunction>(adapterAddServantLocator), METH_VARARGS,
        PyDoc_STR(STRCAST("addServantLocator(Ice.ServantLocator, category) -> None")) },
    { STRCAST("removeServantLocator"), reinterpret_cast<PyCFunction>(adapterRemoveServantLocator), METH_VARARGS,
        PyDoc_STR(STRCAST("removeServantLocator(category) -> Ice.ServantLocator")) },
    { STRCAST("findServantLocator"), reinterpret_cast<PyCFunction>(adapterFindServantLocator), METH_VARARGS,
        PyDoc_STR(STRCAST("findServantLocator(category) -> Ice.ServantLocator")) },
    { STRCAST("createProxy"), reinterpret_cast<PyCFunction>(adapterCreateProxy), METH_VARARGS,
        PyDoc_STR(STRCAST("createProxy(identity) -> Ice.ObjectPrx")) },
    { STRCAST("createDirectProxy"), reinterpret_cast<PyCFunction>(adapterCreateDirectProxy), METH_VARARGS,
        PyDoc_STR(STRCAST("createDirectProxy(identity) -> Ice.ObjectPrx")) },
    { STRCAST("createIndirectProxy"), reinterpret_cast<PyCFunction>(adapterCreateIndirectProxy), METH_VARARGS,
        PyDoc_STR(STRCAST("createIndirectProxy(identity) -> Ice.ObjectPrx")) },
    { STRCAST("setLocator"), reinterpret_cast<PyCFunction>(adapterSetLocator), METH_VARARGS,
        PyDoc_STR(STRCAST("setLocator(proxy) -> None")) },
    { STRCAST("getLocator"), reinterpret_cast<PyCFunction>(adapterGetLocator), METH_NOARGS,
      PyDoc_STR(STRCAST("getLocator() -> Ice.LocatorPrx")) },
    { STRCAST("refreshPublishedEndpoints"), reinterpret_cast<PyCFunction>(adapterRefreshPublishedEndpoints), METH_NOARGS,
        PyDoc_STR(STRCAST("refreshPublishedEndpoints() -> None")) },
    { STRCAST("getEndpoints"), reinterpret_cast<PyCFunction>(adapterGetEndpoints), METH_NOARGS,
        PyDoc_STR(STRCAST("getEndpoints() -> None")) },
    { STRCAST("getPublishedEndpoints"), reinterpret_cast<PyCFunction>(adapterGetPublishedEndpoints), METH_NOARGS,
        PyDoc_STR(STRCAST("getPublishedEndpoints() -> None")) },
    { 0, 0 } /* sentinel */
};

namespace IcePy
{

PyTypeObject ObjectAdapterType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyVarObject_HEAD_INIT(0, 0)
    STRCAST("IcePy.ObjectAdapter"),  /* tp_name */
    sizeof(ObjectAdapterObject),     /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    reinterpret_cast<destructor>(adapterDealloc), /* tp_dealloc */
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
    AdapterMethods,                  /* tp_methods */
    0,                               /* tp_members */
    0,                               /* tp_getset */
    0,                               /* tp_base */
    0,                               /* tp_dict */
    0,                               /* tp_descr_get */
    0,                               /* tp_descr_set */
    0,                               /* tp_dictoffset */
    0,                               /* tp_init */
    0,                               /* tp_alloc */
    reinterpret_cast<newfunc>(adapterNew), /* tp_new */
    0,                               /* tp_free */
    0,                               /* tp_is_gc */
};

}

bool
IcePy::initObjectAdapter(PyObject* module)
{
    _mainThreadId = PyThread_get_thread_ident();

    if(PyType_Ready(&ObjectAdapterType) < 0)
    {
        return false;
    }
    PyTypeObject* type = &ObjectAdapterType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("ObjectAdapter"), reinterpret_cast<PyObject*>(type)) < 0)
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
    if(obj)
    {
        obj->adapter = new Ice::ObjectAdapterPtr(adapter);
        obj->deactivateMonitor = new IceUtil::Monitor<IceUtil::Mutex>;
        obj->deactivateThread = 0;
        obj->deactivated = false;
        obj->holdMonitor = new IceUtil::Monitor<IceUtil::Mutex>;
        obj->holdThread = 0;
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
    PyObjectHandle adapterI = createObjectAdapter(adapter);
    if(!adapterI.get())
    {
        return 0;
    }
    PyObject* wrapperType = lookupType("Ice.ObjectAdapterI");
    assert(wrapperType);
    PyObjectHandle args = PyTuple_New(1);
    if(!args.get())
    {
        return 0;
    }
    PyTuple_SET_ITEM(args.get(), 0, adapterI.release());
    return PyObject_Call(wrapperType, args.get(), 0);
}

Ice::ObjectAdapterPtr
IcePy::unwrapObjectAdapter(PyObject* obj)
{
#ifndef NDEBUG
    PyObject* wrapperType = lookupType("Ice.ObjectAdapterI");
#endif
    assert(wrapperType);
    assert(PyObject_IsInstance(obj, wrapperType));
    PyObjectHandle impl = PyObject_GetAttrString(obj, STRCAST("_impl"));
    assert(impl.get());
    return getObjectAdapter(impl.get());
}
