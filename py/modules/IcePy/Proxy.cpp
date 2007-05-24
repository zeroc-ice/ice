// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#ifdef _WIN32
#   include <IceUtil/Config.h>
#endif
#include <Proxy.h>
#include <structmember.h>
#include <Communicator.h>
#include <Connection.h>
#include <Util.h>
#include <Ice/Communicator.h>
#include <Ice/LocalException.h>
#include <Ice/Locator.h>
#include <Ice/Proxy.h>
#include <Ice/Router.h>

using namespace std;
using namespace IcePy;

namespace IcePy
{

extern PyTypeObject EndpointType;

struct EndpointObject
{
    PyObject_HEAD
    Ice::EndpointPtr* endpoint;
};

struct ProxyObject
{
    PyObject_HEAD
    Ice::ObjectPrx* proxy;
    Ice::CommunicatorPtr* communicator;
};

}

//
// Endpoint implementation.
//
static EndpointObject*
allocateEndpoint(const Ice::EndpointPtr& endpoint)
{
    EndpointObject* p = PyObject_New(EndpointObject, &EndpointType);
    if(!p)
    {
        return 0;
    }
    p->endpoint = new Ice::EndpointPtr(endpoint);

    return p;
}

#ifdef WIN32
extern "C"
#endif
static EndpointObject*
endpointNew(PyObject* /*arg*/)
{
    PyErr_Format(PyExc_RuntimeError, STRCAST("An endpoint cannot be created directly"));
    return 0;
}

#ifdef WIN32
extern "C"
#endif
static void
endpointDealloc(EndpointObject* self)
{
    delete self->endpoint;
    PyObject_Del(self);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
endpointToString(EndpointObject*);

#ifdef WIN32
extern "C"
#endif
static PyObject*
endpointRepr(EndpointObject* self)
{
    return endpointToString(self);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
endpointToString(EndpointObject* self)
{
    string str = (*self->endpoint)->toString();
    return PyString_FromString(const_cast<char*>(str.c_str()));
}

//
// Proxy implementation.
//
static ProxyObject*
allocateProxy(const Ice::ObjectPrx& proxy, const Ice::CommunicatorPtr& communicator, PyObject* type)
{
    PyTypeObject* typeObj = reinterpret_cast<PyTypeObject*>(type);
    ProxyObject* p = reinterpret_cast<ProxyObject*>(typeObj->tp_alloc(typeObj, 0));
    if(!p)
    {
        return 0;
    }

    //
    // Disabling collocation optimization can cause subtle problems with proxy
    // comparison (such as in RouterInfo::get) if a proxy from IcePy is
    // compared with a proxy from Ice/C++.
    //
    //if(proxy)
    //{
    //    p->proxy = new Ice::ObjectPrx(proxy->ice_collocationOptimized(false));
    //}
    //
    p->proxy = new Ice::ObjectPrx(proxy);
    p->communicator = new Ice::CommunicatorPtr(communicator);

    return p;
}

#ifdef WIN32
extern "C"
#endif
static ProxyObject*
proxyNew(PyObject* /*arg*/)
{
    PyErr_Format(PyExc_RuntimeError, STRCAST("A proxy cannot be created directly"));
    return 0;
}

#ifdef WIN32
extern "C"
#endif
static void
proxyDealloc(ProxyObject* self)
{
    delete self->proxy;
    delete self->communicator;
    self->ob_type->tp_free(self);
}

#ifdef WIN32
extern "C"
#endif
static int
proxyCompare(ProxyObject* p1, ProxyObject* p2)
{
    if(*p1->proxy < *p2->proxy)
    {
        return -1;
    }
    else if(*p1->proxy == *p2->proxy)
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
proxyRepr(ProxyObject* self)
{
    string str = (*self->proxy)->ice_toString();
    return PyString_FromString(const_cast<char*>(str.c_str()));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceCommunicator(ProxyObject* self)
{
    PyErr_Warn(PyExc_DeprecationWarning, STRCAST("ice_communicator is deprecated, use ice_getCommunicator instead."));
    return getCommunicatorWrapper(*self->communicator);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceGetCommunicator(ProxyObject* self)
{
    return getCommunicatorWrapper(*self->communicator);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceIsA(ProxyObject* self, PyObject* args)
{
    char* type;
    PyObject* ctx = 0;
    if(!PyArg_ParseTuple(args, STRCAST("s|O!"), &type, &PyDict_Type, &ctx))
    {
        return 0;
    }

    assert(self->proxy);

    bool b;
    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.
        if(ctx)
        {
            Ice::Context context;
            if(!dictionaryToContext(ctx, context))
            {
                return 0;
            }
            b = (*self->proxy)->ice_isA(type, context);
        }
        else
        {
            b = (*self->proxy)->ice_isA(type);
        }
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    if(b)
    {
        PyRETURN_TRUE;
    }
    else
    {
        PyRETURN_FALSE;
    }
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIcePing(ProxyObject* self, PyObject* args)
{
    PyObject* ctx = 0;
    if(!PyArg_ParseTuple(args, STRCAST("|O!"), &PyDict_Type, &ctx))
    {
        return 0;
    }

    assert(self->proxy);

    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.
        if(ctx)
        {
            Ice::Context context;
            if(!dictionaryToContext(ctx, context))
            {
                return 0;
            }
            (*self->proxy)->ice_ping(context);
        }
        else
        {
            (*self->proxy)->ice_ping();
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
proxyIceIds(ProxyObject* self, PyObject* args)
{
    PyObject* ctx = 0;
    if(!PyArg_ParseTuple(args, STRCAST("|O!"), &PyDict_Type, &ctx))
    {
        return 0;
    }

    assert(self->proxy);

    Ice::StringSeq ids;
    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.
        if(ctx)
        {
            Ice::Context context;
            if(!dictionaryToContext(ctx, context))
            {
                return 0;
            }
            ids = (*self->proxy)->ice_ids(context);
        }
        else
        {
            ids = (*self->proxy)->ice_ids();
        }
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    PyObject* list = PyList_New(0);
    if(!list || !stringSeqToList(ids, list))
    {
        return 0;
    }

    return list;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceId(ProxyObject* self, PyObject* args)
{
    PyObject* ctx = 0;
    if(!PyArg_ParseTuple(args, STRCAST("|O!"), &PyDict_Type, &ctx))
    {
        return 0;
    }

    assert(self->proxy);

    string id;
    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.
        if(ctx)
        {
            Ice::Context context;
            if(!dictionaryToContext(ctx, context))
            {
                return 0;
            }
            id = (*self->proxy)->ice_id(context);
        }
        else
        {
            id = (*self->proxy)->ice_id();
        }
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return Py_BuildValue(STRCAST("s"), id.c_str());
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceGetIdentity(ProxyObject* self)
{
    assert(self->proxy);

    Ice::Identity id;
    try
    {
        id = (*self->proxy)->ice_getIdentity();
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
proxyIceIdentity(ProxyObject* self, PyObject* args)
{
    PyObject* identityType = lookupType("Ice.Identity");
    assert(identityType);
    PyObject* id;
    if(!PyArg_ParseTuple(args, STRCAST("O!"), identityType, &id))
    {
        return 0;
    }

    assert(self->proxy);

    Ice::Identity ident;
    if(!getIdentity(id, ident))
    {
        return 0;
    }

    Ice::ObjectPrx newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_identity(ident);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy, *self->communicator);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceNewIdentity(ProxyObject* self, PyObject* args)
{
    PyErr_Warn(PyExc_DeprecationWarning, STRCAST("ice_newIdentity is deprecated, use ice_identity instead."));
    return proxyIceIdentity(self, args);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceGetContext(ProxyObject* self)
{
    assert(self->proxy);

    Ice::Context ctx;
    try
    {
        ctx = (*self->proxy)->ice_getContext();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    PyObjectHandle result = PyDict_New();
    if(result.get() && contextToDictionary(ctx, result.get()))
    {
        return result.release();
    }
    return 0;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceContext(ProxyObject* self, PyObject* args)
{
    PyObject* dict;
    if(!PyArg_ParseTuple(args, STRCAST("O!"), &PyDict_Type, &dict))
    {
        return 0;
    }

    assert(self->proxy);

    Ice::Context ctx;
    if(!dictionaryToContext(dict, ctx))
    {
        return 0;
    }

    Ice::ObjectPrx newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_context(ctx);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy, *self->communicator, reinterpret_cast<PyObject*>(self->ob_type));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceNewContext(ProxyObject* self, PyObject* args)
{
    PyErr_Warn(PyExc_DeprecationWarning, STRCAST("ice_newContext is deprecated, use ice_context instead."));
    return proxyIceContext(self, args);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceDefaultContext(ProxyObject* self)
{
    PyErr_Warn(PyExc_DeprecationWarning, STRCAST("ice_defaultContext is deprecated."));
    assert(self->proxy);

    Ice::ObjectPrx newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_defaultContext();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    PyTypeObject* type = self->ob_type; // Necessary to prevent GCC's strict-alias warnings.
    return createProxy(newProxy, *self->communicator, reinterpret_cast<PyObject*>(type));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceGetFacet(ProxyObject* self)
{
    assert(self->proxy);

    string facet;
    try
    {
        facet = (*self->proxy)->ice_getFacet();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return PyString_FromString(const_cast<char*>(facet.c_str()));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceFacet(ProxyObject* self, PyObject* args)
{
    char* facet;
    if(!PyArg_ParseTuple(args, STRCAST("s"), &facet))
    {
        return 0;
    }

    assert(self->proxy);

    Ice::ObjectPrx newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_facet(facet);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy, *self->communicator);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceNewFacet(ProxyObject* self, PyObject* args)
{
    PyErr_Warn(PyExc_DeprecationWarning, STRCAST("ice_newFacet is deprecated, use ice_facet instead."));
    return proxyIceFacet(self, args);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceGetAdapterId(ProxyObject* self)
{
    assert(self->proxy);

    string id;
    try
    {
        id = (*self->proxy)->ice_getAdapterId();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return PyString_FromString(const_cast<char*>(id.c_str()));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceAdapterId(ProxyObject* self, PyObject* args)
{
    char* id;
    if(!PyArg_ParseTuple(args, STRCAST("s"), &id))
    {
        return 0;
    }

    assert(self->proxy);

    Ice::ObjectPrx newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_adapterId(id);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy, *self->communicator, reinterpret_cast<PyObject*>(self->ob_type));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceNewAdapterId(ProxyObject* self, PyObject* args)
{
    PyErr_Warn(PyExc_DeprecationWarning, STRCAST("ice_newAdapterId is deprecated, use ice_adapterId instead."));
    return proxyIceAdapterId(self, args);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceGetEndpoints(ProxyObject* self)
{
    assert(self->proxy);

    Ice::EndpointSeq endpoints;
    try
    {
        endpoints = (*self->proxy)->ice_getEndpoints();
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
        PyObjectHandle endp = reinterpret_cast<PyObject*>(allocateEndpoint(*p));
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
proxyIceEndpoints(ProxyObject* self, PyObject* args)
{
    PyObject* endpoints;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &endpoints))
    {
        return 0;
    }

    if(!PyTuple_Check(endpoints) && !PyList_Check(endpoints))
    {
        PyErr_Format(PyExc_ValueError, STRCAST("argument must be a tuple or list"));
        return 0;
    }

    assert(self->proxy);

    Ice::EndpointSeq seq;
    Py_ssize_t sz = PySequence_Fast_GET_SIZE(endpoints);
    for(Py_ssize_t i = 0; i < sz; ++i)
    {
        PyObject* p = PySequence_Fast_GET_ITEM(endpoints, i);
        PyTypeObject* type = &EndpointType; // Necessary to prevent GCC's strict-alias warnings.
        if(!PyObject_IsInstance(p, reinterpret_cast<PyObject*>(type)))
        {
            PyErr_Format(PyExc_ValueError, STRCAST("expected element of type Ice.Endpoint"));
            return 0;
        }
        EndpointObject* o = reinterpret_cast<EndpointObject*>(p);
        assert(*o->endpoint);
        seq.push_back(*o->endpoint);
    }

    Ice::ObjectPrx newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_endpoints(seq);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy, *self->communicator, reinterpret_cast<PyObject*>(self->ob_type));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceNewEndpoints(ProxyObject* self, PyObject* args)
{
    PyErr_Warn(PyExc_DeprecationWarning, STRCAST("ice_newEndpoints is deprecated, use ice_endpoints instead."));
    return proxyIceEndpoints(self, args);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceGetLocatorCacheTimeout(ProxyObject* self)
{
    assert(self->proxy);

    try
    {
        Ice::Int timeout = (*self->proxy)->ice_getLocatorCacheTimeout();
        return PyInt_FromLong(timeout);
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
proxyIceLocatorCacheTimeout(ProxyObject* self, PyObject* args)
{
    int timeout;
    if(!PyArg_ParseTuple(args, STRCAST("i"), &timeout))
    {
        return 0;
    }

    assert(self->proxy);

    Ice::ObjectPrx newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_locatorCacheTimeout(timeout);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy, *self->communicator, reinterpret_cast<PyObject*>(self->ob_type));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceIsConnectionCached(ProxyObject* self)
{
    assert(self->proxy);

    PyObject* b;
    try
    {
        b = (*self->proxy)->ice_isConnectionCached() ? getTrue() : getFalse();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(b);
    return b;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceConnectionCached(ProxyObject* self, PyObject* args)
{
    PyObject* flag;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &flag))
    {
        return 0;
    }

    int n = PyObject_IsTrue(flag);
    if(n < 0)
    {
        return 0;
    }

    assert(self->proxy);

    Ice::ObjectPrx newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_connectionCached(n == 1);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy, *self->communicator, reinterpret_cast<PyObject*>(self->ob_type));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceGetEndpointSelection(ProxyObject* self)
{
    PyObject* cls = lookupType("Ice.EndpointSelectionType");
    assert(cls);

    PyObjectHandle rnd = PyObject_GetAttrString(cls, STRCAST("Random"));
    PyObjectHandle ord = PyObject_GetAttrString(cls, STRCAST("Ordered"));
    assert(rnd.get());
    assert(ord.get());

    assert(self->proxy);

    PyObject* type;
    try
    {
        Ice::EndpointSelectionType val = (*self->proxy)->ice_getEndpointSelection();
        if(val == Ice::Random)
        {
            type = rnd.get();
        }
        else
        {
            type = ord.get();
        }
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(type);
    return type;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceEndpointSelection(ProxyObject* self, PyObject* args)
{
    PyObject* cls = lookupType("Ice.EndpointSelectionType");
    assert(cls);
    PyObject* type;
    if(!PyArg_ParseTuple(args, STRCAST("O!"), cls, &type))
    {
        return 0;
    }

    Ice::EndpointSelectionType val;
    PyObjectHandle rnd = PyObject_GetAttrString(cls, STRCAST("Random"));
    PyObjectHandle ord = PyObject_GetAttrString(cls, STRCAST("Ordered"));
    assert(rnd.get());
    assert(ord.get());
    if(rnd.get() == type)
    {
        val = Ice::Random;
    }
    else if(ord.get() == type)
    {
        val = Ice::Ordered;
    }
    else
    {
        PyErr_Format(PyExc_ValueError, STRCAST("ice_endpointSelection requires Random or Ordered"));
        return 0;
    }

    assert(self->proxy);

    Ice::ObjectPrx newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_endpointSelection(val);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy, *self->communicator, reinterpret_cast<PyObject*>(self->ob_type));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceIsSecure(ProxyObject* self)
{
    assert(self->proxy);

    PyObject* b;
    try
    {
        b = (*self->proxy)->ice_isSecure() ? getTrue() : getFalse();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(b);
    return b;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceSecure(ProxyObject* self, PyObject* args)
{
    PyObject* flag;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &flag))
    {
        return 0;
    }

    int n = PyObject_IsTrue(flag);
    if(n < 0)
    {
        return 0;
    }

    assert(self->proxy);

    Ice::ObjectPrx newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_secure(n == 1);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    PyTypeObject* type = self->ob_type; // Necessary to prevent GCC's strict-alias warnings.
    return createProxy(newProxy, *self->communicator, reinterpret_cast<PyObject*>(type));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceIsPreferSecure(ProxyObject* self)
{
    assert(self->proxy);

    PyObject* b;
    try
    {
        b = (*self->proxy)->ice_isPreferSecure() ? getTrue() : getFalse();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(b);
    return b;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIcePreferSecure(ProxyObject* self, PyObject* args)
{
    PyObject* flag;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &flag))
    {
        return 0;
    }

    int n = PyObject_IsTrue(flag);
    if(n < 0)
    {
        return 0;
    }

    assert(self->proxy);

    Ice::ObjectPrx newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_preferSecure(n == 1);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy, *self->communicator, reinterpret_cast<PyObject*>(self->ob_type));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceGetRouter(ProxyObject* self)
{
    assert(self->proxy);

    Ice::RouterPrx router;
    try
    {
        router = (*self->proxy)->ice_getRouter();
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
proxyIceRouter(ProxyObject* self, PyObject* args)
{
    PyObject* p;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &p))
    {
        return 0;
    }

    PyObject* routerProxyType = lookupType("Ice.RouterPrx");
    assert(routerProxyType);
    Ice::RouterPrx routerProxy;
    if(PyObject_IsInstance(p, routerProxyType))
    {
        routerProxy = Ice::RouterPrx::uncheckedCast(getProxy(p));
    }
    else if(p != Py_None)
    {
        PyErr_Format(PyExc_ValueError, STRCAST("ice_router requires None or Ice.RouterPrx"));
        return 0;
    }

    assert(self->proxy);

    Ice::ObjectPrx newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_router(routerProxy);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy, *self->communicator, reinterpret_cast<PyObject*>(self->ob_type));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceGetLocator(ProxyObject* self)
{
    assert(self->proxy);

    Ice::LocatorPrx locator;
    try
    {
        locator = (*self->proxy)->ice_getLocator();
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
proxyIceLocator(ProxyObject* self, PyObject* args)
{
    PyObject* p;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &p))
    {
        return 0;
    }

    PyObject* locatorProxyType = lookupType("Ice.LocatorPrx");
    assert(locatorProxyType);
    Ice::LocatorPrx locatorProxy;
    if(PyObject_IsInstance(p, locatorProxyType))
    {
        locatorProxy = Ice::LocatorPrx::uncheckedCast(getProxy(p));
    }
    else if(p != Py_None)
    {
        PyErr_Format(PyExc_ValueError, STRCAST("ice_locator requires None or Ice.LocatorPrx"));
        return 0;
    }

    assert(self->proxy);

    Ice::ObjectPrx newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_locator(locatorProxy);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy, *self->communicator, reinterpret_cast<PyObject*>(self->ob_type));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceTwoway(ProxyObject* self)
{
    assert(self->proxy);

    Ice::ObjectPrx newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_twoway();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy, *self->communicator, reinterpret_cast<PyObject*>(self->ob_type));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceIsTwoway(ProxyObject* self)
{
    assert(self->proxy);

    PyObject* b;
    try
    {
        b = (*self->proxy)->ice_isTwoway() ? getTrue() : getFalse();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(b);
    return b;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceOneway(ProxyObject* self)
{
    assert(self->proxy);

    Ice::ObjectPrx newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_oneway();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy, *self->communicator, reinterpret_cast<PyObject*>(self->ob_type));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceIsOneway(ProxyObject* self)
{
    assert(self->proxy);

    PyObject* b;
    try
    {
        b = (*self->proxy)->ice_isOneway() ? getTrue() : getFalse();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(b);
    return b;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceBatchOneway(ProxyObject* self)
{
    assert(self->proxy);

    Ice::ObjectPrx newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_batchOneway();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy, *self->communicator, reinterpret_cast<PyObject*>(self->ob_type));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceIsBatchOneway(ProxyObject* self)
{
    assert(self->proxy);

    PyObject* b;
    try
    {
        b = (*self->proxy)->ice_isBatchOneway() ? getTrue() : getFalse();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(b);
    return b;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceDatagram(ProxyObject* self)
{
    assert(self->proxy);

    Ice::ObjectPrx newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_datagram();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy, *self->communicator, reinterpret_cast<PyObject*>(self->ob_type));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceIsDatagram(ProxyObject* self)
{
    assert(self->proxy);

    PyObject* b;
    try
    {
        b = (*self->proxy)->ice_isDatagram() ? getTrue() : getFalse();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(b);
    return b;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceBatchDatagram(ProxyObject* self)
{
    assert(self->proxy);

    Ice::ObjectPrx newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_batchDatagram();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy, *self->communicator, reinterpret_cast<PyObject*>(self->ob_type));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceIsBatchDatagram(ProxyObject* self)
{
    assert(self->proxy);

    PyObject* b;
    try
    {
        b = (*self->proxy)->ice_isBatchDatagram() ? getTrue() : getFalse();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(b);
    return b;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceCompress(ProxyObject* self, PyObject* args)
{
    PyObject* flag;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &flag))
    {
        return 0;
    }

    int n = PyObject_IsTrue(flag);
    if(n < 0)
    {
        return 0;
    }

    assert(self->proxy);

    Ice::ObjectPrx newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_compress(n == 1);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy, *self->communicator, reinterpret_cast<PyObject*>(self->ob_type));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceTimeout(ProxyObject* self, PyObject* args)
{
    int timeout;
    if(!PyArg_ParseTuple(args, STRCAST("i"), &timeout))
    {
        return 0;
    }

    assert(self->proxy);

    Ice::ObjectPrx newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_timeout(timeout);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy, *self->communicator, reinterpret_cast<PyObject*>(self->ob_type));
}

// NOTE: ice_collocationOptimized is not currently supported.

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceConnectionId(ProxyObject* self, PyObject* args)
{
    char* connectionId;
    if(!PyArg_ParseTuple(args, STRCAST("s"), &connectionId))
    {
        return 0;
    }

    assert(self->proxy);

    Ice::ObjectPrx newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_connectionId(connectionId);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy, *self->communicator, reinterpret_cast<PyObject*>(self->ob_type));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceIsThreadPerConnection(ProxyObject* self)
{
    assert(self->proxy);

    PyObject* b;
    try
    {
        b = (*self->proxy)->ice_isThreadPerConnection() ? getTrue() : getFalse();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(b);
    return b;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceThreadPerConnection(ProxyObject* self, PyObject* args)
{
    PyObject* flag;
    if(!PyArg_ParseTuple(args, STRCAST("O"), &flag))
    {
        return 0;
    }

    int n = PyObject_IsTrue(flag);
    if(n < 0)
    {
        return 0;
    }

    assert(self->proxy);

    Ice::ObjectPrx newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_threadPerConnection(n == 1);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    return createProxy(newProxy, *self->communicator, reinterpret_cast<PyObject*>(self->ob_type));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceGetConnection(ProxyObject* self)
{
    assert(self->proxy);

    Ice::ConnectionPtr con;
    try
    {
        con = (*self->proxy)->ice_getConnection();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    if(con)
    {
        return createConnection(con, *self->communicator);
    }
    else
    {
        Py_INCREF(Py_None);
        return Py_None;
    }
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceGetCachedConnection(ProxyObject* self)
{
    assert(self->proxy);

    Ice::ConnectionPtr con;
    try
    {
        con = (*self->proxy)->ice_getCachedConnection();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    if(con)
    {
        return createConnection(con, *self->communicator);
    }
    else
    {
        Py_INCREF(Py_None);
        return Py_None;
    }
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceConnection(ProxyObject* self)
{
    PyErr_Warn(PyExc_DeprecationWarning, STRCAST("ice_connection is deprecated, use ice_getConnection instead."));
    return proxyIceGetConnection(self);
}

static PyObject*
checkedCastImpl(ProxyObject* p, const string& id, PyObject* facet, PyObject* ctx, PyObject* type)
{
    Ice::ObjectPrx target;
    if(!facet || facet == Py_None)
    {
        target = *p->proxy;
    }
    else
    {
        char* facetStr = PyString_AS_STRING(facet);
        target = (*p->proxy)->ice_facet(facetStr);
    }

    bool b;
    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.
        if(!ctx || ctx == Py_None)
        {
            b = target->ice_isA(id);
        }
        else
        {
            Ice::Context c;
            if(!dictionaryToContext(ctx, c))
            {
                return 0;
            }
            b = target->ice_isA(id, c);
        }
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    if(b)
    {
        return createProxy(target, *p->communicator, type);
    }

    Py_INCREF(Py_None);
    return Py_None;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceCheckedCast(PyObject* type, PyObject* args)
{
    //
    // ice_checkedCast is called from generated code, therefore we always expect
    // to receive four arguments.
    //
    PyObject* obj;
    char* id;
    PyObject* facetOrCtx = 0;
    PyObject* ctx = 0;
    if(!PyArg_ParseTuple(args, STRCAST("OsOO"), &obj, &id, &facetOrCtx, &ctx))
    {
        return 0;
    }

    if(obj == Py_None)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    if(!checkProxy(obj))
    {
        PyErr_Format(PyExc_ValueError, STRCAST("ice_checkedCast requires a proxy argument"));
        return 0;
    }

    PyObject* facet = 0;

    if(PyString_Check(facetOrCtx))
    {
        facet = facetOrCtx;
    }
    else if(PyDict_Check(facetOrCtx))
    {
        if(ctx != Py_None)
        {
            PyErr_Format(PyExc_ValueError, STRCAST("facet argument to checkedCast must be a string"));
            return 0;
        }
        ctx = facetOrCtx;
    }
    else if(facetOrCtx != Py_None)
    {
        PyErr_Format(PyExc_ValueError, STRCAST("second argument to checkedCast must be a facet or context"));
        return 0;
    }

    if(ctx != Py_None && !PyDict_Check(ctx))
    {
        PyErr_Format(PyExc_ValueError, STRCAST("context argument to checkedCast must be a dictionary"));
        return 0;
    }

    return checkedCastImpl(reinterpret_cast<ProxyObject*>(obj), id, facet, ctx, type);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceUncheckedCast(PyObject* type, PyObject* args)
{
    //
    // ice_uncheckedCast is called from generated code, therefore we always expect
    // to receive two arguments.
    //
    PyObject* obj;
    char* facet = 0;
    if(!PyArg_ParseTuple(args, STRCAST("Oz"), &obj, &facet))
    {
        return 0;
    }

    if(obj == Py_None)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    if(!checkProxy(obj))
    {
        PyErr_Format(PyExc_ValueError, STRCAST("ice_uncheckedCast requires a proxy argument"));
        return 0;
    }

    ProxyObject* p = reinterpret_cast<ProxyObject*>(obj);

    if(facet)
    {
        return createProxy((*p->proxy)->ice_facet(facet), *p->communicator, type);
    }
    else
    {
        return createProxy(*p->proxy, *p->communicator, type);
    }
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyCheckedCast(PyObject* /*self*/, PyObject* args)
{
    PyObject* obj;
    PyObject* arg1 = 0;
    PyObject* arg2 = 0;
    if(!PyArg_ParseTuple(args, STRCAST("O|OO"), &obj, &arg1, &arg2))
    {
        return 0;
    }

    if(obj == Py_None)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    if(!checkProxy(obj))
    {
        PyErr_Format(PyExc_ValueError, STRCAST("checkedCast requires a proxy argument"));
        return 0;
    }

    PyObject* facet = 0;
    PyObject* ctx = 0;

    if(arg1 != 0 && arg2 != 0)
    {
        if(arg1 == Py_None)
        {
            arg1 = 0;
        }

        if(arg2 == Py_None)
        {
            arg2 = 0;
        }

        if(arg1 != 0)
        {
            if(!PyString_Check(arg1))
            {
                PyErr_Format(PyExc_ValueError, STRCAST("facet argument to checkedCast must be a string"));
                return 0;
            }
            facet = arg1;
        }

        if(arg2 != 0 && !PyDict_Check(arg2))
        {
            PyErr_Format(PyExc_ValueError, STRCAST("context argument to checkedCast must be a dictionary"));
            return 0;
        }
        ctx = arg2;
    }
    else if(arg1 != 0 && arg1 != Py_None)
    {
        if(PyString_Check(arg1))
        {
            facet = arg1;
        }
        else if(PyDict_Check(arg1))
        {
            ctx = arg1;
        }
        else
        {
            PyErr_Format(PyExc_ValueError, STRCAST("second argument to checkedCast must be a facet or context"));
            return 0;
        }
    }

    return checkedCastImpl(reinterpret_cast<ProxyObject*>(obj), "::Ice::Object", facet, ctx, 0);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyUncheckedCast(PyObject* /*self*/, PyObject* args)
{
    PyObject* obj;
    char* facet = 0;
    if(!PyArg_ParseTuple(args, STRCAST("O|z"), &obj, &facet))
    {
        return 0;
    }

    if(obj == Py_None)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    if(!checkProxy(obj))
    {
        PyErr_Format(PyExc_ValueError, STRCAST("uncheckedCast requires a proxy argument"));
        return 0;
    }

    ProxyObject* p = reinterpret_cast<ProxyObject*>(obj);

    if(facet)
    {
        return createProxy((*p->proxy)->ice_facet(facet), *p->communicator);
    }
    else
    {
        return createProxy(*p->proxy, *p->communicator);
    }
}

static PyMethodDef EndpointMethods[] =
{
    { STRCAST("toString"), reinterpret_cast<PyCFunction>(endpointToString), METH_NOARGS,
        PyDoc_STR(STRCAST("toString() -> string")) },
    { 0, 0 } /* sentinel */
};

static PyMethodDef ProxyMethods[] =
{
    { STRCAST("ice_communicator"), reinterpret_cast<PyCFunction>(proxyIceCommunicator), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_communicator() -> Ice.Communicator")) }, // Deprecated
    { STRCAST("ice_getCommunicator"), reinterpret_cast<PyCFunction>(proxyIceGetCommunicator), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_getCommunicator() -> Ice.Communicator")) },
    { STRCAST("ice_toString"), reinterpret_cast<PyCFunction>(proxyRepr), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_toString() -> string")) },
    { STRCAST("ice_isA"), reinterpret_cast<PyCFunction>(proxyIceIsA), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_isA(type, [ctx]) -> bool")) },
    { STRCAST("ice_ping"), reinterpret_cast<PyCFunction>(proxyIcePing), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_ping([ctx]) -> None")) },
    { STRCAST("ice_ids"), reinterpret_cast<PyCFunction>(proxyIceIds), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_ids([ctx]) -> list")) },
    { STRCAST("ice_id"), reinterpret_cast<PyCFunction>(proxyIceId), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_id([ctx]) -> string")) },
    { STRCAST("ice_getIdentity"), reinterpret_cast<PyCFunction>(proxyIceGetIdentity), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_getIdentity() -> Ice.Identity")) },
    { STRCAST("ice_newIdentity"), reinterpret_cast<PyCFunction>(proxyIceNewIdentity), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_newIdentity(id) -> Ice.ObjectPrx")) }, // Deprecated
    { STRCAST("ice_identity"), reinterpret_cast<PyCFunction>(proxyIceIdentity), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_identity(id) -> Ice.ObjectPrx")) },
    { STRCAST("ice_getContext"), reinterpret_cast<PyCFunction>(proxyIceGetContext), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_getContext() -> dict")) },
    { STRCAST("ice_newContext"), reinterpret_cast<PyCFunction>(proxyIceNewContext), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_newContext(dict) -> Ice.ObjectPrx")) }, // Deprecated
    { STRCAST("ice_context"), reinterpret_cast<PyCFunction>(proxyIceContext), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_context(dict) -> Ice.ObjectPrx")) },
    { STRCAST("ice_defaultContext"), reinterpret_cast<PyCFunction>(proxyIceDefaultContext), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_defaultContext() -> Ice.ObjectPrx")) },
    { STRCAST("ice_getFacet"), reinterpret_cast<PyCFunction>(proxyIceGetFacet), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_getFacet() -> string")) },
    { STRCAST("ice_newFacet"), reinterpret_cast<PyCFunction>(proxyIceNewFacet), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_newFacet(string) -> Ice.ObjectPrx")) }, // Deprecated
    { STRCAST("ice_facet"), reinterpret_cast<PyCFunction>(proxyIceFacet), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_facet(string) -> Ice.ObjectPrx")) },
    { STRCAST("ice_getAdapterId"), reinterpret_cast<PyCFunction>(proxyIceGetAdapterId), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_getAdapterId() -> string")) },
    { STRCAST("ice_newAdapterId"), reinterpret_cast<PyCFunction>(proxyIceNewAdapterId), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_newAdapterId(string) -> proxy")) }, // Deprecated
    { STRCAST("ice_adapterId"), reinterpret_cast<PyCFunction>(proxyIceAdapterId), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_adapterId(string) -> proxy")) },
    { STRCAST("ice_getEndpoints"), reinterpret_cast<PyCFunction>(proxyIceGetEndpoints), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_getEndpoints() -> tuple")) },
    { STRCAST("ice_newEndpoints"), reinterpret_cast<PyCFunction>(proxyIceNewEndpoints), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_newEndpoints(tuple) -> proxy")) }, // Deprecated
    { STRCAST("ice_endpoints"), reinterpret_cast<PyCFunction>(proxyIceEndpoints), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_endpoints(tuple) -> proxy")) },
    { STRCAST("ice_getLocatorCacheTimeout"), reinterpret_cast<PyCFunction>(proxyIceGetLocatorCacheTimeout), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_getLocatorCacheTimeout() -> int")) },
    { STRCAST("ice_locatorCacheTimeout"), reinterpret_cast<PyCFunction>(proxyIceLocatorCacheTimeout), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_locatorCacheTimeout(int) -> Ice.ObjectPrx")) },
    { STRCAST("ice_isConnectionCached"), reinterpret_cast<PyCFunction>(proxyIceIsConnectionCached), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_isConnectionCached() -> bool")) },
    { STRCAST("ice_connectionCached"), reinterpret_cast<PyCFunction>(proxyIceConnectionCached), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_connectionCached(bool) -> Ice.ObjectPrx")) },
    { STRCAST("ice_getEndpointSelection"), reinterpret_cast<PyCFunction>(proxyIceGetEndpointSelection), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_getEndpointSelection() -> bool")) },
    { STRCAST("ice_endpointSelection"), reinterpret_cast<PyCFunction>(proxyIceEndpointSelection), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_endpointSelection(Ice.EndpointSelectionType) -> Ice.ObjectPrx")) },
    { STRCAST("ice_isSecure"), reinterpret_cast<PyCFunction>(proxyIceIsSecure), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_isSecure() -> bool")) },
    { STRCAST("ice_secure"), reinterpret_cast<PyCFunction>(proxyIceSecure), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_secure(bool) -> Ice.ObjectPrx")) },
    { STRCAST("ice_isPreferSecure"), reinterpret_cast<PyCFunction>(proxyIceIsPreferSecure), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_isPreferSecure() -> bool")) },
    { STRCAST("ice_preferSecure"), reinterpret_cast<PyCFunction>(proxyIcePreferSecure), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_preferSecure(bool) -> Ice.ObjectPrx")) },
    { STRCAST("ice_getRouter"), reinterpret_cast<PyCFunction>(proxyIceGetRouter), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_getRouter() -> Ice.RouterPrx")) },
    { STRCAST("ice_router"), reinterpret_cast<PyCFunction>(proxyIceRouter), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_router(Ice.RouterPrx) -> Ice.ObjectPrx")) },
    { STRCAST("ice_getLocator"), reinterpret_cast<PyCFunction>(proxyIceGetLocator), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_getLocator() -> Ice.LocatorPrx")) },
    { STRCAST("ice_locator"), reinterpret_cast<PyCFunction>(proxyIceLocator), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_locator(Ice.LocatorPrx) -> Ice.ObjectPrx")) },
    { STRCAST("ice_twoway"), reinterpret_cast<PyCFunction>(proxyIceTwoway), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_twoway() -> Ice.ObjectPrx")) },
    { STRCAST("ice_isTwoway"), reinterpret_cast<PyCFunction>(proxyIceIsTwoway), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_isTwoway() -> bool")) },
    { STRCAST("ice_oneway"), reinterpret_cast<PyCFunction>(proxyIceOneway), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_oneway() -> Ice.ObjectPrx")) },
    { STRCAST("ice_isOneway"), reinterpret_cast<PyCFunction>(proxyIceIsOneway), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_isOneway() -> bool")) },
    { STRCAST("ice_batchOneway"), reinterpret_cast<PyCFunction>(proxyIceBatchOneway), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_batchOneway() -> Ice.ObjectPrx")) },
    { STRCAST("ice_isBatchOneway"), reinterpret_cast<PyCFunction>(proxyIceIsBatchOneway), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_isBatchOneway() -> bool")) },
    { STRCAST("ice_datagram"), reinterpret_cast<PyCFunction>(proxyIceDatagram), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_datagram() -> Ice.ObjectPrx")) },
    { STRCAST("ice_isDatagram"), reinterpret_cast<PyCFunction>(proxyIceIsDatagram), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_isDatagram() -> bool")) },
    { STRCAST("ice_batchDatagram"), reinterpret_cast<PyCFunction>(proxyIceBatchDatagram), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_batchDatagram() -> Ice.ObjectPrx")) },
    { STRCAST("ice_isBatchDatagram"), reinterpret_cast<PyCFunction>(proxyIceIsBatchDatagram), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_isBatchDatagram() -> bool")) },
    { STRCAST("ice_compress"), reinterpret_cast<PyCFunction>(proxyIceCompress), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_compress(bool) -> Ice.ObjectPrx")) },
    { STRCAST("ice_timeout"), reinterpret_cast<PyCFunction>(proxyIceTimeout), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_timeout(int) -> Ice.ObjectPrx")) },
    { STRCAST("ice_connectionId"), reinterpret_cast<PyCFunction>(proxyIceConnectionId), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_connectionId(string) -> Ice.ObjectPrx")) },
    { STRCAST("ice_isThreadPerConnection"), reinterpret_cast<PyCFunction>(proxyIceIsThreadPerConnection), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_isThreadPerConnection() -> bool")) },
    { STRCAST("ice_threadPerConnection"), reinterpret_cast<PyCFunction>(proxyIceThreadPerConnection), METH_VARARGS,
        PyDoc_STR(STRCAST("ice_threadPerConnection(bool) -> Ice.ObjectPrx")) },
    { STRCAST("ice_connection"), reinterpret_cast<PyCFunction>(proxyIceConnection), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_connection() -> Ice.Connection")) }, // Deprecated
    { STRCAST("ice_getConnection"), reinterpret_cast<PyCFunction>(proxyIceGetConnection), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_getConnection() -> Ice.Connection")) },
    { STRCAST("ice_getCachedConnection"), reinterpret_cast<PyCFunction>(proxyIceGetCachedConnection), METH_NOARGS,
        PyDoc_STR(STRCAST("ice_getCachedConnection() -> Ice.Connection")) },
    { STRCAST("ice_checkedCast"), reinterpret_cast<PyCFunction>(proxyIceCheckedCast), METH_VARARGS | METH_CLASS,
        PyDoc_STR(STRCAST("ice_checkedCast(proxy, id[, facetOrCtx[, ctx]]) -> proxy")) },
    { STRCAST("ice_uncheckedCast"), reinterpret_cast<PyCFunction>(proxyIceUncheckedCast), METH_VARARGS | METH_CLASS,
        PyDoc_STR(STRCAST("ice_uncheckedCast(proxy) -> proxy")) },
    { STRCAST("checkedCast"), reinterpret_cast<PyCFunction>(proxyCheckedCast), METH_VARARGS | METH_STATIC,
        PyDoc_STR(STRCAST("checkedCast(proxy) -> proxy")) },
    { STRCAST("uncheckedCast"), reinterpret_cast<PyCFunction>(proxyUncheckedCast), METH_VARARGS | METH_STATIC,
        PyDoc_STR(STRCAST("uncheckedCast(proxy) -> proxy")) },
    { 0, 0 } /* sentinel */
};

namespace IcePy
{

PyTypeObject EndpointType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyObject_HEAD_INIT(0)
    0,                               /* ob_size */
    STRCAST("Ice.Endpoint"),         /* tp_name */
    sizeof(EndpointObject),          /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    (destructor)endpointDealloc,     /* tp_dealloc */
    0,                               /* tp_print */
    0,                               /* tp_getattr */
    0,                               /* tp_setattr */
    0,                               /* tp_compare */
    (reprfunc)endpointRepr,          /* tp_repr */
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
    EndpointMethods,                 /* tp_methods */
    0,                               /* tp_members */
    0,                               /* tp_getset */
    0,                               /* tp_base */
    0,                               /* tp_dict */
    0,                               /* tp_descr_get */
    0,                               /* tp_descr_set */
    0,                               /* tp_dictoffset */
    0,                               /* tp_init */
    0,                               /* tp_alloc */
    (newfunc)endpointNew,            /* tp_new */
    0,                               /* tp_free */
    0,                               /* tp_is_gc */
};

PyTypeObject ProxyType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyObject_HEAD_INIT(0)
    0,                               /* ob_size */
    STRCAST("IcePy.ObjectPrx"),      /* tp_name */
    sizeof(ProxyObject),             /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    (destructor)proxyDealloc,        /* tp_dealloc */
    0,                               /* tp_print */
    0,                               /* tp_getattr */
    0,                               /* tp_setattr */
    (cmpfunc)proxyCompare,           /* tp_compare */
    (reprfunc)proxyRepr,             /* tp_repr */
    0,                               /* tp_as_number */
    0,                               /* tp_as_sequence */
    0,                               /* tp_as_mapping */
    0,                               /* tp_hash */
    0,                               /* tp_call */
    0,                               /* tp_str */
    0,                               /* tp_getattro */
    0,                               /* tp_setattro */
    0,                               /* tp_as_buffer */
    Py_TPFLAGS_BASETYPE |
    Py_TPFLAGS_HAVE_CLASS,           /* tp_flags */
    0,                               /* tp_doc */
    0,                               /* tp_traverse */
    0,                               /* tp_clear */
    0,                               /* tp_richcompare */
    0,                               /* tp_weaklistoffset */
    0,                               /* tp_iter */
    0,                               /* tp_iternext */
    ProxyMethods,                    /* tp_methods */
    0,                               /* tp_members */
    0,                               /* tp_getset */
    0,                               /* tp_base */
    0,                               /* tp_dict */
    0,                               /* tp_descr_get */
    0,                               /* tp_descr_set */
    0,                               /* tp_dictoffset */
    0,                               /* tp_init */
    0,                               /* tp_alloc */
    (newfunc)proxyNew,               /* tp_new */
    0,                               /* tp_free */
    0,                               /* tp_is_gc */
};

}

bool
IcePy::initProxy(PyObject* module)
{
    if(PyType_Ready(&ProxyType) < 0)
    {
        return false;
    }
    PyTypeObject* proxyType = &ProxyType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("ObjectPrx"), reinterpret_cast<PyObject*>(proxyType)) < 0)
    {
        return false;
    }

    if(PyType_Ready(&EndpointType) < 0)
    {
        return false;
    }
    PyTypeObject* endpointType = &EndpointType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("Endpoint"), reinterpret_cast<PyObject*>(endpointType)) < 0)
    {
        return false;
    }
    return true;
}

PyObject*
IcePy::createProxy(const Ice::ObjectPrx& proxy, const Ice::CommunicatorPtr& communicator, PyObject* type)
{
    if(!type)
    {
        PyTypeObject* proxyType = &ProxyType; // Necessary to prevent GCC's strict-alias warnings.
        type = reinterpret_cast<PyObject*>(proxyType);
    }
    return reinterpret_cast<PyObject*>(allocateProxy(proxy, communicator, type));
}

bool
IcePy::checkProxy(PyObject* p)
{
    PyTypeObject* type = &ProxyType; // Necessary to prevent GCC's strict-alias warnings.
    return PyObject_IsInstance(p, reinterpret_cast<PyObject*>(type)) == 1;
}

Ice::ObjectPrx
IcePy::getProxy(PyObject* p)
{
    assert(checkProxy(p));
    ProxyObject* obj = reinterpret_cast<ProxyObject*>(p);
    return *obj->proxy;
}

Ice::CommunicatorPtr
IcePy::getProxyCommunicator(PyObject* p)
{
    assert(checkProxy(p));
    ProxyObject* obj = reinterpret_cast<ProxyObject*>(p);
    return *obj->communicator;
}
