// Copyright (c) ZeroC, Inc.

#include "Proxy.h"
#include "Communicator.h"
#include "Connection.h"
#include "Endpoint.h"
#include "Ice/Communicator.h"
#include "Ice/DisableWarnings.h"
#include "Ice/LocalExceptions.h"
#include "Ice/Locator.h"
#include "Ice/Proxy.h"
#include "Ice/Router.h"
#include "Operation.h"
#include "Thread.h"
#include "Types.h"
#include "Util.h"

using namespace std;
using namespace IcePy;

#if defined(__GNUC__) && ((__GNUC__ >= 8))
#    pragma GCC diagnostic ignored "-Wcast-function-type"
#endif

namespace IcePy
{
    struct ProxyObject
    {
        PyObject_HEAD Ice::ObjectPrx* proxy;
        Ice::CommunicatorPtr* communicator;
    };
}

//
// Proxy implementation.
//
static ProxyObject*
allocateProxy(const Ice::ObjectPrx& proxy, const Ice::CommunicatorPtr& communicator, PyObject* type)
{
    auto* typeObj = reinterpret_cast<PyTypeObject*>(type);
    ProxyObject* p{reinterpret_cast<ProxyObject*>(typeObj->tp_alloc(typeObj, 0))};
    if (!p)
    {
        return nullptr;
    }

    //
    // Disabling collocation optimization can cause subtle problems with proxy
    // comparison (such as in RouterInfo::get) if a proxy from IcePy is
    // compared with a proxy from Ice/C++.
    //
    // if(proxy)
    //{
    //    p->proxy = new Ice::ObjectPrx(proxy->ice_collocationOptimized(false));
    //}
    //
    p->proxy = new Ice::ObjectPrx(proxy);
    p->communicator = new Ice::CommunicatorPtr(communicator);

    return p;
}

extern "C" ProxyObject*
proxyNew(PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwds*/)
{
    ProxyObject* self{reinterpret_cast<ProxyObject*>(type->tp_alloc(type, 0))};
    if (!self)
    {
        return nullptr;
    }
    self->proxy = nullptr;
    self->communicator = nullptr;
    return self;
}

extern "C" int
proxyInit(ProxyObject* self, PyObject* args, PyObject* /*kwds*/)
{
    PyObject* communicatorWrapperType{lookupType("Ice.Communicator")};
    assert(communicatorWrapperType);

    PyObject* communicatorWrapper{nullptr};
    char* proxyString{nullptr};

    if (!PyArg_ParseTuple(args, "O!s", communicatorWrapperType, &communicatorWrapper, &proxyString))
    {
        return -1;
    }

    PyObject* communicatorImpl{PyObject_GetAttrString(communicatorWrapper, "_impl")};

    Ice::CommunicatorPtr communicator = getCommunicator(communicatorImpl);
    try
    {
        Ice::ObjectPrx proxy{communicator, proxyString};
        self->proxy = new Ice::ObjectPrx(std::move(proxy));
        self->communicator = new Ice::CommunicatorPtr(std::move(communicator));
    }
    catch (...)
    {
        setPythonException(current_exception());
        return -1;
    }
    return 0;
}

extern "C" void
proxyDealloc(ProxyObject* self)
{
    delete self->proxy;
    delete self->communicator;
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

extern "C" PyObject*
proxyCompare(ProxyObject* p1, PyObject* other, int op)
{
    bool result = false;

    if (PyObject_TypeCheck(other, &ProxyType))
    {
        auto* p2 = reinterpret_cast<ProxyObject*>(other);

        switch (op)
        {
            case Py_EQ:
                result = *p1->proxy == *p2->proxy;
                break;
            case Py_NE:
                result = *p1->proxy != *p2->proxy;
                break;
            case Py_LE:
                result = *p1->proxy <= *p2->proxy;
                break;
            case Py_GE:
                result = *p1->proxy >= *p2->proxy;
                break;
            case Py_LT:
                result = *p1->proxy < *p2->proxy;
                break;
            case Py_GT:
                result = *p1->proxy > *p2->proxy;
                break;
        }
    }
    else if (other == Py_None)
    {
        result = op == Py_NE || op == Py_GE || op == Py_GT;
    }
    else
    {
        if (op == Py_EQ)
        {
            result = false;
        }
        else if (op == Py_NE)
        {
            result = true;
        }
        else
        {
            PyErr_Format(PyExc_TypeError, "can't compare %s to %s", Py_TYPE(p1)->tp_name, Py_TYPE(other)->tp_name);
            return nullptr;
        }
    }

    return result ? Py_True : Py_False;
}

extern "C" PyObject*
proxyRepr(ProxyObject* self, PyObject* /*args*/)
{
    string str = (*self->proxy)->ice_toString();
    return createString(str);
}

extern "C" long
proxyHash(ProxyObject* self)
{
    return static_cast<long>(std::hash<Ice::ObjectPrx>{}(*self->proxy));
}

extern "C" PyObject*
proxyIceGetCommunicator(ProxyObject* self, PyObject* /*args*/)
{
    return getCommunicatorWrapper(*self->communicator);
}

extern "C" PyObject*
proxyIceIsA(ProxyObject* self, PyObject* args)
{
    PyObject* type{nullptr};
    PyObject* ctx{Py_None};
    if (!PyArg_ParseTuple(args, "O|O!", &type, &PyDict_Type, &ctx))
    {
        return nullptr;
    }

    //
    // We need to reformat the arguments to match what is used by the generated code: ((params...), ctx|None)
    //
    PyObjectHandle newArgs{Py_BuildValue("((O), O)", type, ctx)};

    return invokeBuiltin(reinterpret_cast<PyObject*>(self), "ice_isA", newArgs.get());
}

extern "C" PyObject*
proxyIceIsAAsync(ProxyObject* self, PyObject* args)
{
    PyObject* type{nullptr};
    PyObject* ctx{Py_None};
    if (!PyArg_ParseTuple(args, "O|O!", &type, &PyDict_Type, &ctx))
    {
        return nullptr;
    }

    //
    // We need to reformat the arguments to match what is used by the generated code: ((params...), ctx|None)
    //
    PyObjectHandle newArgs{Py_BuildValue("((O), O)", type, ctx)};

    return invokeBuiltinAsync(reinterpret_cast<PyObject*>(self), "ice_isA", newArgs.get());
}

extern "C" PyObject*
proxyIcePing(ProxyObject* self, PyObject* args)
{
    PyObject* ctx{Py_None};
    if (!PyArg_ParseTuple(args, "|O!", &PyDict_Type, &ctx))
    {
        return nullptr;
    }

    //
    // We need to reformat the arguments to match what is used by the generated code: ((params...), ctx|None)
    //
    PyObjectHandle newArgs{Py_BuildValue("((), O)", ctx)};

    return invokeBuiltin(reinterpret_cast<PyObject*>(self), "ice_ping", newArgs.get());
}

extern "C" PyObject*
proxyIcePingAsync(ProxyObject* self, PyObject* args)
{
    PyObject* ctx{Py_None};
    if (!PyArg_ParseTuple(args, "|O!", &PyDict_Type, &ctx))
    {
        return nullptr;
    }

    //
    // We need to reformat the arguments to match what is used by the generated code: ((params...), ctx|None)
    //
    PyObjectHandle newArgs{Py_BuildValue("((), O)", ctx)};

    return invokeBuiltinAsync(reinterpret_cast<PyObject*>(self), "ice_ping", newArgs.get());
}

extern "C" PyObject*
proxyIceIds(ProxyObject* self, PyObject* args)
{
    PyObject* ctx{Py_None};
    if (!PyArg_ParseTuple(args, "|O!", &PyDict_Type, &ctx))
    {
        return nullptr;
    }

    //
    // We need to reformat the arguments to match what is used by the generated code: ((params...), ctx|None)
    //
    PyObjectHandle newArgs{Py_BuildValue("((), O)", ctx)};

    return invokeBuiltin(reinterpret_cast<PyObject*>(self), "ice_ids", newArgs.get());
}

extern "C" PyObject*
proxyIceIdsAsync(ProxyObject* self, PyObject* args)
{
    PyObject* ctx{Py_None};
    if (!PyArg_ParseTuple(args, "|O!", &PyDict_Type, &ctx))
    {
        return nullptr;
    }

    //
    // We need to reformat the arguments to match what is used by the generated code: ((params...), ctx|None)
    //
    PyObjectHandle newArgs{Py_BuildValue("((), O)", ctx)};

    return invokeBuiltinAsync(reinterpret_cast<PyObject*>(self), "ice_ids", newArgs.get());
}

extern "C" PyObject*
proxyIceId(ProxyObject* self, PyObject* args)
{
    PyObject* ctx{Py_None};
    if (!PyArg_ParseTuple(args, "|O!", &PyDict_Type, &ctx))
    {
        return nullptr;
    }

    //
    // We need to reformat the arguments to match what is used by the generated code: ((params...), ctx|None)
    //
    PyObjectHandle newArgs{Py_BuildValue("((), O)", ctx)};

    return invokeBuiltin(reinterpret_cast<PyObject*>(self), "ice_id", newArgs.get());
}

extern "C" PyObject*
proxyIceIdAsync(ProxyObject* self, PyObject* args)
{
    PyObject* ctx = Py_None;
    if (!PyArg_ParseTuple(args, "|O!", &PyDict_Type, &ctx))
    {
        return nullptr;
    }

    //
    // We need to reformat the arguments to match what is used by the generated code: ((params...), ctx|None)
    //
    PyObjectHandle newArgs{Py_BuildValue("((), O)", ctx)};

    return invokeBuiltinAsync(reinterpret_cast<PyObject*>(self), "ice_id", newArgs.get());
}

extern "C" PyObject*
proxyIceGetIdentity(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    Ice::Identity id;
    try
    {
        id = (*self->proxy)->ice_getIdentity();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createIdentity(id);
}

extern "C" PyObject*
proxyIceIdentity(ProxyObject* self, PyObject* args)
{
    PyObject* identityType{lookupType("Ice.Identity")};
    assert(identityType);
    PyObject* id;
    if (!PyArg_ParseTuple(args, "O!", identityType, &id))
    {
        return nullptr;
    }

    assert(self->proxy);

    Ice::Identity ident;
    if (!getIdentity(id, ident))
    {
        return nullptr;
    }

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_identity(ident);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createProxy(newProxy.value(), *self->communicator);
}

extern "C" PyObject*
proxyIceGetContext(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    Ice::Context ctx;
    try
    {
        ctx = (*self->proxy)->ice_getContext();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    PyObjectHandle result{PyDict_New()};
    if (result.get() && contextToDictionary(ctx, result.get()))
    {
        return result.release();
    }
    return nullptr;
}

extern "C" PyObject*
proxyIceContext(ProxyObject* self, PyObject* args)
{
    PyObject* dict{nullptr};
    if (!PyArg_ParseTuple(args, "O!", &PyDict_Type, &dict))
    {
        return nullptr;
    }

    assert(self->proxy);

    Ice::Context ctx;
    if (!dictionaryToContext(dict, ctx))
    {
        return nullptr;
    }

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_context(ctx);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

extern "C" PyObject*
proxyIceGetFacet(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    string facet;
    try
    {
        facet = (*self->proxy)->ice_getFacet();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createString(facet);
}

extern "C" PyObject*
proxyIceFacet(ProxyObject* self, PyObject* args)
{
    PyObject* facetObj{nullptr};
    if (!PyArg_ParseTuple(args, "O", &facetObj))
    {
        return nullptr;
    }

    string facet;
    if (!getStringArg(facetObj, "facet", facet))
    {
        return nullptr;
    }

    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_facet(facet);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createProxy(newProxy.value(), *self->communicator);
}

extern "C" PyObject*
proxyIceGetAdapterId(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    string id;
    try
    {
        id = (*self->proxy)->ice_getAdapterId();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createString(id);
}

extern "C" PyObject*
proxyIceAdapterId(ProxyObject* self, PyObject* args)
{
    PyObject* idObj;
    if (!PyArg_ParseTuple(args, "O", &idObj))
    {
        return nullptr;
    }

    string id;
    if (!getStringArg(idObj, "id", id))
    {
        return nullptr;
    }

    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_adapterId(id);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

extern "C" PyObject*
proxyIceGetEndpoints(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    Ice::EndpointSeq endpoints;
    try
    {
        endpoints = (*self->proxy)->ice_getEndpoints();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    PyObjectHandle result{PyTuple_New(static_cast<int>(endpoints.size()))};
    int i = 0;
    for (const auto& endpoint : endpoints)
    {
        PyObjectHandle pythonEndpoint{createEndpoint(endpoint)};
        if (!pythonEndpoint.get())
        {
            return nullptr;
        }
        // PyTuple_SET_ITEM steals a reference.
        PyTuple_SET_ITEM(result.get(), i++, pythonEndpoint.release());
    }

    return result.release();
}

extern "C" PyObject*
proxyIceEndpoints(ProxyObject* self, PyObject* args)
{
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

    assert(self->proxy);

    Ice::EndpointSeq seq;
    if (!toEndpointSeq(endpoints, seq))
    {
        return nullptr;
    }

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_endpoints(seq);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

extern "C" PyObject*
proxyIceGetLocatorCacheTimeout(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    try
    {
        auto timeout = chrono::duration_cast<chrono::seconds>((*self->proxy)->ice_getLocatorCacheTimeout());
        return PyLong_FromLong(static_cast<int32_t>(timeout.count()));
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }
}

extern "C" PyObject*
proxyIceGetInvocationTimeout(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    try
    {
        chrono::milliseconds timeout = (*self->proxy)->ice_getInvocationTimeout();
        return PyLong_FromLong(static_cast<int32_t>(timeout.count()));
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }
}

extern "C" PyObject*
proxyIceGetConnectionId(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    try
    {
        string connectionId = (*self->proxy)->ice_getConnectionId();
        return createString(connectionId);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }
}

extern "C" PyObject*
proxyIceLocatorCacheTimeout(ProxyObject* self, PyObject* args)
{
    int timeout;
    if (!PyArg_ParseTuple(args, "i", &timeout))
    {
        return nullptr;
    }

    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_locatorCacheTimeout(timeout);
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

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

extern "C" PyObject*
proxyIceInvocationTimeout(ProxyObject* self, PyObject* args)
{
    int timeout;
    if (!PyArg_ParseTuple(args, "i", &timeout))
    {
        return nullptr;
    }

    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_invocationTimeout(timeout);
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

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

extern "C" PyObject*
proxyIceIsConnectionCached(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);
    return (*self->proxy)->ice_isConnectionCached() ? Py_True : Py_False;
}

extern "C" PyObject*
proxyIceConnectionCached(ProxyObject* self, PyObject* args)
{
    PyObject* flag{nullptr};
    if (!PyArg_ParseTuple(args, "O", &flag))
    {
        return nullptr;
    }

    int n = PyObject_IsTrue(flag);
    if (n < 0)
    {
        return nullptr;
    }

    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_connectionCached(n == 1);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

extern "C" PyObject*
proxyIceGetEndpointSelection(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);
    PyObject* cls{lookupType("Ice.EndpointSelectionType")};
    assert(cls);
    PyObjectHandle type{
        (*self->proxy)->ice_getEndpointSelection() == Ice::EndpointSelectionType::Random
            ? getAttr(cls, "Random", false)
            : getAttr(cls, "Ordered", false)};
    assert(type.get());
    return type.release();
}

extern "C" PyObject*
proxyIceEndpointSelection(ProxyObject* self, PyObject* args)
{
    PyObject* cls{lookupType("Ice.EndpointSelectionType")};
    assert(cls);
    PyObject* type;
    if (!PyArg_ParseTuple(args, "O!", cls, &type))
    {
        return nullptr;
    }

    Ice::EndpointSelectionType val;
    PyObjectHandle rnd{getAttr(cls, "Random", false)};
    PyObjectHandle ord{getAttr(cls, "Ordered", false)};
    assert(rnd.get());
    assert(ord.get());
    if (rnd.get() == type)
    {
        val = Ice::EndpointSelectionType::Random;
    }
    else if (ord.get() == type)
    {
        val = Ice::EndpointSelectionType::Ordered;
    }
    else
    {
        PyErr_Format(PyExc_ValueError, "ice_endpointSelection requires Random or Ordered");
        return nullptr;
    }

    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_endpointSelection(val);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

extern "C" PyObject*
proxyIceIsSecure(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);
    return (*self->proxy)->ice_isSecure() ? Py_True : Py_False;
}

extern "C" PyObject*
proxyIceSecure(ProxyObject* self, PyObject* args)
{
    PyObject* flag{nullptr};
    if (!PyArg_ParseTuple(args, "O", &flag))
    {
        return nullptr;
    }

    int n = PyObject_IsTrue(flag);
    if (n < 0)
    {
        return nullptr;
    }

    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_secure(n == 1);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

extern "C" PyObject*
proxyIceGetEncodingVersion(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    PyObject* version{nullptr};
    try
    {
        version = IcePy::createEncodingVersion((*self->proxy)->ice_getEncodingVersion());
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return Py_NewRef(version);
}

extern "C" PyObject*
proxyIceEncodingVersion(ProxyObject* self, PyObject* args)
{
    PyObject* versionType = IcePy::lookupType("Ice.EncodingVersion");
    PyObject* p;
    if (!PyArg_ParseTuple(args, "O!", versionType, &p))
    {
        return nullptr;
    }

    Ice::EncodingVersion val;
    if (!getEncodingVersion(p, val))
    {
        PyErr_Format(PyExc_ValueError, "ice_encodingVersion requires an encoding version");
        return nullptr;
    }

    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_encodingVersion(val);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

extern "C" PyObject*
proxyIceIsPreferSecure(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    try
    {
        return (*self->proxy)->ice_isPreferSecure() ? Py_True : Py_False;
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }
}

extern "C" PyObject*
proxyIcePreferSecure(ProxyObject* self, PyObject* args)
{
    PyObject* flag;
    if (!PyArg_ParseTuple(args, "O", &flag))
    {
        return nullptr;
    }

    int n = PyObject_IsTrue(flag);
    if (n < 0)
    {
        return nullptr;
    }

    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_preferSecure(n == 1);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

extern "C" PyObject*
proxyIceGetRouter(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    optional<Ice::RouterPrx> router;
    try
    {
        router = (*self->proxy)->ice_getRouter();
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
proxyIceRouter(ProxyObject* self, PyObject* args)
{
    PyObject* p;
    if (!PyArg_ParseTuple(args, "O", &p))
    {
        return nullptr;
    }

    optional<Ice::ObjectPrx> proxy;
    if (!getProxyArg(p, "ice_router", "rtr", proxy, "Ice.RouterPrx"))
    {
        return nullptr;
    }

    optional<Ice::RouterPrx> router = Ice::uncheckedCast<Ice::RouterPrx>(proxy);

    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_router(router);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

extern "C" PyObject*
proxyIceGetLocator(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    optional<Ice::LocatorPrx> locator;
    try
    {
        locator = (*self->proxy)->ice_getLocator();
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
proxyIceLocator(ProxyObject* self, PyObject* args)
{
    PyObject* p;
    if (!PyArg_ParseTuple(args, "O", &p))
    {
        return nullptr;
    }

    optional<Ice::ObjectPrx> proxy;
    if (!getProxyArg(p, "ice_locator", "loc", proxy, "Ice.LocatorPrx"))
    {
        return nullptr;
    }

    optional<Ice::LocatorPrx> locator = Ice::uncheckedCast<Ice::LocatorPrx>(proxy);

    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_locator(locator);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

extern "C" PyObject*
proxyIceTwoway(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_twoway();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

extern "C" PyObject*
proxyIceIsTwoway(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    try
    {
        return (*self->proxy)->ice_isTwoway() ? Py_True : Py_False;
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }
}

extern "C" PyObject*
proxyIceOneway(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_oneway();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

extern "C" PyObject*
proxyIceIsOneway(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    try
    {
        return (*self->proxy)->ice_isOneway() ? Py_True : Py_False;
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }
}

extern "C" PyObject*
proxyIceBatchOneway(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_batchOneway();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

extern "C" PyObject*
proxyIceIsBatchOneway(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    try
    {
        return (*self->proxy)->ice_isBatchOneway() ? Py_True : Py_False;
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }
}

extern "C" PyObject*
proxyIceDatagram(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_datagram();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

extern "C" PyObject*
proxyIceIsDatagram(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);
    try
    {
        return (*self->proxy)->ice_isDatagram() ? Py_True : Py_False;
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }
}

extern "C" PyObject*
proxyIceBatchDatagram(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_batchDatagram();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

extern "C" PyObject*
proxyIceIsBatchDatagram(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);
    try
    {
        return (*self->proxy)->ice_isBatchDatagram() ? Py_True : Py_False;
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }
}

extern "C" PyObject*
proxyIceCompress(ProxyObject* self, PyObject* args)
{
    PyObject* flag;
    if (!PyArg_ParseTuple(args, "O", &flag))
    {
        return nullptr;
    }

    int n = PyObject_IsTrue(flag);
    if (n < 0)
    {
        return nullptr;
    }

    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_compress(n == 1);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

extern "C" PyObject*
proxyIceGetCompress(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);
    try
    {
        optional<bool> compress = (*self->proxy)->ice_getCompress();
        if (compress)
        {
            return *compress ? Py_True : Py_False;
        }
        else
        {
            return Py_None;
        }
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }
}

extern "C" PyObject*
proxyIceIsCollocationOptimized(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    try
    {
        return (*self->proxy)->ice_isCollocationOptimized() ? Py_True : Py_False;
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }
}

extern "C" PyObject*
proxyIceCollocationOptimized(ProxyObject* self, PyObject* args)
{
    PyObject* flag;
    if (!PyArg_ParseTuple(args, "O", &flag))
    {
        return nullptr;
    }

    int n = PyObject_IsTrue(flag);
    if (n < 0)
    {
        return nullptr;
    }

    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_collocationOptimized(n == 1);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

extern "C" PyObject*
proxyIceConnectionId(ProxyObject* self, PyObject* args)
{
    PyObject* idObj;
    if (!PyArg_ParseTuple(args, "O", &idObj))
    {
        return nullptr;
    }

    string id;
    if (!getStringArg(idObj, "id", id))
    {
        return nullptr;
    }

    assert(self->proxy);

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_connectionId(id);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

extern "C" PyObject*
proxyIceFixed(ProxyObject* self, PyObject* args)
{
    assert(self->proxy);

    PyObject* p;
    if (!PyArg_ParseTuple(args, "O", &p))
    {
        return nullptr;
    }

    Ice::ConnectionPtr connection;
    if (!getConnectionArg(p, "ice_fixed", "connection", connection))
    {
        return nullptr;
    }

    optional<Ice::ObjectPrx> newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_fixed(connection);
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

    return createProxy(newProxy.value(), *self->communicator, reinterpret_cast<PyObject*>(Py_TYPE(self)));
}

extern "C" PyObject*
proxyIceIsFixed(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    try
    {
        return (*self->proxy)->ice_isFixed() ? Py_True : Py_False;
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }
}

extern "C" PyObject*
proxyIceGetConnection(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    Ice::ConnectionPtr con;
    try
    {
        con = (*self->proxy)->ice_getConnection();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    if (con)
    {
        return createConnection(con, *self->communicator);
    }
    else
    {
        return Py_None;
    }
}

extern "C" PyObject*
proxyIceGetConnectionAsync(ProxyObject* self, PyObject* /*args*/, PyObject* /*kwds*/)
{
    assert(self->proxy);
    const string op = "ice_getConnection";

    auto callback = make_shared<GetConnectionAsyncCallback>(*self->communicator, op);
    function<void()> cancel;
    try
    {
        cancel = (*self->proxy)
                     ->ice_getConnectionAsync(
                         [callback](const Ice::ConnectionPtr& connection) { callback->response(connection); },
                         [callback](exception_ptr ex) { callback->exception(ex); });
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    PyObjectHandle asyncInvocationContextObj{createAsyncInvocationContext(cancel, *self->communicator)};
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
    return future.release();
}

extern "C" PyObject*
proxyIceGetCachedConnection(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    Ice::ConnectionPtr con;
    try
    {
        con = (*self->proxy)->ice_getCachedConnection();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    if (con)
    {
        return createConnection(con, *self->communicator);
    }
    else
    {
        return Py_None;
    }
}

extern "C" PyObject*
proxyIceFlushBatchRequests(ProxyObject* self, PyObject* /*args*/)
{
    assert(self->proxy);

    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.
        (*self->proxy)->ice_flushBatchRequests();
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    return Py_None;
}

extern "C" PyObject*
proxyIceFlushBatchRequestsAsync(ProxyObject* self, PyObject* /*args*/, PyObject* /*kwds*/)
{
    assert(self->proxy);
    const string op = "ice_flushBatchRequests";

    auto callback = make_shared<FlushAsyncCallback>(op);
    function<void()> cancel;
    try
    {
        cancel = (*self->proxy)
                     ->ice_flushBatchRequestsAsync(
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
    return future.release();
}

extern "C" PyObject*
proxyIceInvoke(ProxyObject* self, PyObject* args)
{
    return iceInvoke(reinterpret_cast<PyObject*>(self), args);
}

extern "C" PyObject*
proxyIceInvokeAsync(ProxyObject* self, PyObject* args, PyObject* /*kwds*/)
{
    return iceInvokeAsync(reinterpret_cast<PyObject*>(self), args);
}

static PyObject*
checkedCastImpl(ProxyObject* p, const string& id, PyObject* facet, PyObject* ctx, PyObject* type)
{
    optional<Ice::ObjectPrx> target;
    if (!facet || facet == Py_None)
    {
        target = *p->proxy;
    }
    else
    {
        string facetStr = getString(facet);
        target = (*p->proxy)->ice_facet(facetStr);
    }
    assert(target);

    bool b = false;
    try
    {
        Ice::Context c;
        if (ctx && ctx != Py_None)
        {
            if (!dictionaryToContext(ctx, c))
            {
                return nullptr;
            }
        }

        AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.
        b = target->ice_isA(id, (!ctx || ctx == Py_None) ? Ice::noExplicitContext : c);
    }
    catch (...)
    {
        setPythonException(current_exception());
        return nullptr;
    }

    if (b)
    {
        return createProxy(target.value(), *p->communicator, type);
    }

    return Py_None;
}

extern "C" PyObject*
proxyIceCheckedCast(PyObject* type, PyObject* args)
{
    //
    // ice_checkedCast is called from generated code, therefore we always expect
    // to receive four arguments.
    //
    PyObject* obj{nullptr};
    char* id{nullptr};
    PyObject* facetOrContext{nullptr};
    PyObject* ctx{nullptr};
    if (!PyArg_ParseTuple(args, "OsOO", &obj, &id, &facetOrContext, &ctx))
    {
        return nullptr;
    }

    if (obj == Py_None)
    {
        return Py_None;
    }

    if (!checkProxy(obj))
    {
        PyErr_Format(PyExc_ValueError, "ice_checkedCast requires a proxy argument");
        return nullptr;
    }

    PyObject* facet{nullptr};

    if (checkString(facetOrContext))
    {
        facet = facetOrContext;
    }
    else if (PyDict_Check(facetOrContext))
    {
        if (ctx != Py_None)
        {
            PyErr_Format(PyExc_ValueError, "facet argument to checkedCast must be a string");
            return nullptr;
        }
        ctx = facetOrContext;
    }
    else if (facetOrContext != Py_None)
    {
        PyErr_Format(PyExc_ValueError, "second argument to checkedCast must be a facet or context");
        return nullptr;
    }

    if (ctx != Py_None && !PyDict_Check(ctx))
    {
        PyErr_Format(PyExc_ValueError, "context argument to checkedCast must be a dictionary");
        return nullptr;
    }

    return checkedCastImpl(reinterpret_cast<ProxyObject*>(obj), id, facet, ctx, type);
}

extern "C" PyObject*
proxyIceUncheckedCast(PyObject* type, PyObject* args)
{
    //
    // ice_uncheckedCast is called from generated code, therefore we always expect
    // to receive two arguments.
    //
    PyObject* obj;
    char* facet{nullptr};
    if (!PyArg_ParseTuple(args, "Oz", &obj, &facet))
    {
        return nullptr;
    }

    if (obj == Py_None)
    {
        return Py_None;
    }

    if (!checkProxy(obj))
    {
        PyErr_Format(PyExc_ValueError, "ice_uncheckedCast requires a proxy argument");
        return nullptr;
    }

    auto* p = reinterpret_cast<ProxyObject*>(obj);

    if (facet)
    {
        return createProxy((*p->proxy)->ice_facet(facet), *p->communicator, type);
    }
    else
    {
        return createProxy(*p->proxy, *p->communicator, type);
    }
}

extern "C" PyObject*
proxyCheckedCast(PyObject* /*self*/, PyObject* args)
{
    PyObject* obj;
    PyObject* arg1{nullptr};
    PyObject* arg2{nullptr};
    if (!PyArg_ParseTuple(args, "O|OO", &obj, &arg1, &arg2))
    {
        return nullptr;
    }

    if (obj == Py_None)
    {
        return Py_None;
    }

    if (!checkProxy(obj))
    {
        PyErr_Format(PyExc_ValueError, "checkedCast requires a proxy argument");
        return nullptr;
    }

    PyObject* facet{nullptr};
    PyObject* ctx{nullptr};

    if (arg1 && arg2)
    {
        if (arg1 == Py_None)
        {
            arg1 = nullptr;
        }

        if (arg2 == Py_None)
        {
            arg2 = nullptr;
        }

        if (arg1)
        {
            if (!checkString(arg1))
            {
                PyErr_Format(PyExc_ValueError, "facet argument to checkedCast must be a string");
                return nullptr;
            }
            facet = arg1;
        }

        if (arg2 && !PyDict_Check(arg2))
        {
            PyErr_Format(PyExc_ValueError, "context argument to checkedCast must be a dictionary");
            return nullptr;
        }
        ctx = arg2;
    }
    else if (arg1 && arg1 != Py_None)
    {
        if (checkString(arg1))
        {
            facet = arg1;
        }
        else if (PyDict_Check(arg1))
        {
            ctx = arg1;
        }
        else
        {
            PyErr_Format(PyExc_ValueError, "second argument to checkedCast must be a facet or context");
            return nullptr;
        }
    }

    return checkedCastImpl(reinterpret_cast<ProxyObject*>(obj), "::Ice::Object", facet, ctx, nullptr);
}

extern "C" PyObject*
proxyUncheckedCast(PyObject* /*self*/, PyObject* args)
{
    PyObject* obj{nullptr};
    PyObject* facetObj{nullptr};
    if (!PyArg_ParseTuple(args, "O|O", &obj, &facetObj))
    {
        return nullptr;
    }

    if (obj == Py_None)
    {
        return Py_None;
    }

    string facet;
    if (facetObj)
    {
        if (!getStringArg(facetObj, "facet", facet))
        {
            return nullptr;
        }
    }

    if (!checkProxy(obj))
    {
        PyErr_Format(PyExc_ValueError, "uncheckedCast requires a proxy argument");
        return nullptr;
    }

    auto* p = reinterpret_cast<ProxyObject*>(obj);

    if (facetObj)
    {
        return createProxy((*p->proxy)->ice_facet(facet), *p->communicator);
    }
    else
    {
        return createProxy(*p->proxy, *p->communicator);
    }
}

extern "C" PyObject*
proxyIceStaticId(PyObject* /*self*/, PyObject* /*args*/)
{
    return createString(Ice::Object::ice_staticId());
}

static PyMethodDef ProxyMethods[] = {
    {"ice_getCommunicator",
     reinterpret_cast<PyCFunction>(proxyIceGetCommunicator),
     METH_NOARGS,
     PyDoc_STR("ice_getCommunicator() -> Ice.Communicator")},
    {"ice_toString", reinterpret_cast<PyCFunction>(proxyRepr), METH_NOARGS, PyDoc_STR("ice_toString() -> string")},
    {"ice_isA", reinterpret_cast<PyCFunction>(proxyIceIsA), METH_VARARGS, PyDoc_STR("ice_isA(type, [ctx]) -> bool")},
    {"ice_isAAsync",
     reinterpret_cast<PyCFunction>(proxyIceIsAAsync),
     METH_VARARGS,
     PyDoc_STR("ice_isAAsync(type, [ctx]) -> Ice.Future")},
    {"ice_ping", reinterpret_cast<PyCFunction>(proxyIcePing), METH_VARARGS, PyDoc_STR("ice_ping([ctx]) -> None")},
    {"ice_pingAsync",
     reinterpret_cast<PyCFunction>(proxyIcePingAsync),
     METH_VARARGS,
     PyDoc_STR("ice_pingAsync([ctx]) -> Ice.Future")},
    {"ice_ids", reinterpret_cast<PyCFunction>(proxyIceIds), METH_VARARGS, PyDoc_STR("ice_ids([ctx]) -> list")},
    {"ice_idsAsync",
     reinterpret_cast<PyCFunction>(proxyIceIdsAsync),
     METH_VARARGS,
     PyDoc_STR("ice_idsAsync([ctx]) -> Ice.Future")},
    {"ice_id", reinterpret_cast<PyCFunction>(proxyIceId), METH_VARARGS, PyDoc_STR("ice_id([ctx]) -> string")},
    {"ice_idAsync",
     reinterpret_cast<PyCFunction>(proxyIceIdAsync),
     METH_VARARGS,
     PyDoc_STR("ice_idAsync([ctx]) -> Ice.Future")},
    {"ice_getIdentity",
     reinterpret_cast<PyCFunction>(proxyIceGetIdentity),
     METH_NOARGS,
     PyDoc_STR("ice_getIdentity() -> Ice.Identity")},
    {"ice_identity",
     reinterpret_cast<PyCFunction>(proxyIceIdentity),
     METH_VARARGS,
     PyDoc_STR("ice_identity(id) -> Ice.ObjectPrx")},
    {"ice_getContext",
     reinterpret_cast<PyCFunction>(proxyIceGetContext),
     METH_NOARGS,
     PyDoc_STR("ice_getContext() -> dict")},
    {"ice_context",
     reinterpret_cast<PyCFunction>(proxyIceContext),
     METH_VARARGS,
     PyDoc_STR("ice_context(dict) -> Ice.ObjectPrx")},
    {"ice_getFacet",
     reinterpret_cast<PyCFunction>(proxyIceGetFacet),
     METH_NOARGS,
     PyDoc_STR("ice_getFacet() -> string")},
    {"ice_facet",
     reinterpret_cast<PyCFunction>(proxyIceFacet),
     METH_VARARGS,
     PyDoc_STR("ice_facet(string) -> Ice.ObjectPrx")},
    {"ice_getAdapterId",
     reinterpret_cast<PyCFunction>(proxyIceGetAdapterId),
     METH_NOARGS,
     PyDoc_STR("ice_getAdapterId() -> string")},
    {"ice_adapterId",
     reinterpret_cast<PyCFunction>(proxyIceAdapterId),
     METH_VARARGS,
     PyDoc_STR("ice_adapterId(string) -> proxy")},
    {"ice_getEndpoints",
     reinterpret_cast<PyCFunction>(proxyIceGetEndpoints),
     METH_NOARGS,
     PyDoc_STR("ice_getEndpoints() -> tuple")},
    {"ice_endpoints",
     reinterpret_cast<PyCFunction>(proxyIceEndpoints),
     METH_VARARGS,
     PyDoc_STR("ice_endpoints(tuple) -> proxy")},
    {"ice_getLocatorCacheTimeout",
     reinterpret_cast<PyCFunction>(proxyIceGetLocatorCacheTimeout),
     METH_NOARGS,
     PyDoc_STR("ice_getLocatorCacheTimeout() -> int")},
    {"ice_getInvocationTimeout",
     reinterpret_cast<PyCFunction>(proxyIceGetInvocationTimeout),
     METH_NOARGS,
     PyDoc_STR("ice_getInvocationTimeout() -> int")},
    {"ice_getConnectionId",
     reinterpret_cast<PyCFunction>(proxyIceGetConnectionId),
     METH_NOARGS,
     PyDoc_STR("ice_getConnectionId() -> string")},
    {"ice_isCollocationOptimized",
     reinterpret_cast<PyCFunction>(proxyIceIsCollocationOptimized),
     METH_NOARGS,
     PyDoc_STR("ice_isCollocationOptimized() -> bool")},
    {"ice_collocationOptimized",
     reinterpret_cast<PyCFunction>(proxyIceCollocationOptimized),
     METH_VARARGS,
     PyDoc_STR("ice_collocationOptimized(bool) -> Ice.ObjectPrx")},
    {"ice_locatorCacheTimeout",
     reinterpret_cast<PyCFunction>(proxyIceLocatorCacheTimeout),
     METH_VARARGS,
     PyDoc_STR("ice_locatorCacheTimeout(int) -> Ice.ObjectPrx")},
    {"ice_invocationTimeout",
     reinterpret_cast<PyCFunction>(proxyIceInvocationTimeout),
     METH_VARARGS,
     PyDoc_STR("ice_invocationTimeout(int) -> Ice.ObjectPrx")},
    {"ice_isConnectionCached",
     reinterpret_cast<PyCFunction>(proxyIceIsConnectionCached),
     METH_NOARGS,
     PyDoc_STR("ice_isConnectionCached() -> bool")},
    {"ice_connectionCached",
     reinterpret_cast<PyCFunction>(proxyIceConnectionCached),
     METH_VARARGS,
     PyDoc_STR("ice_connectionCached(bool) -> Ice.ObjectPrx")},
    {"ice_getEndpointSelection",
     reinterpret_cast<PyCFunction>(proxyIceGetEndpointSelection),
     METH_NOARGS,
     PyDoc_STR("ice_getEndpointSelection() -> bool")},
    {"ice_endpointSelection",
     reinterpret_cast<PyCFunction>(proxyIceEndpointSelection),
     METH_VARARGS,
     PyDoc_STR("ice_endpointSelection(Ice.EndpointSelectionType) -> Ice.ObjectPrx")},
    {"ice_isSecure", reinterpret_cast<PyCFunction>(proxyIceIsSecure), METH_NOARGS, PyDoc_STR("ice_isSecure() -> bool")},
    {"ice_secure",
     reinterpret_cast<PyCFunction>(proxyIceSecure),
     METH_VARARGS,
     PyDoc_STR("ice_secure(bool) -> Ice.ObjectPrx")},
    {"ice_getEncodingVersion",
     reinterpret_cast<PyCFunction>(proxyIceGetEncodingVersion),
     METH_NOARGS,
     PyDoc_STR("ice_getEncodingVersion() -> Ice.EncodingVersion")},
    {"ice_encodingVersion",
     reinterpret_cast<PyCFunction>(proxyIceEncodingVersion),
     METH_VARARGS,
     PyDoc_STR("ice_endpointSelection(Ice.EncodingVersion) -> Ice.ObjectPrx")},
    {"ice_isPreferSecure",
     reinterpret_cast<PyCFunction>(proxyIceIsPreferSecure),
     METH_NOARGS,
     PyDoc_STR("ice_isPreferSecure() -> bool")},
    {"ice_preferSecure",
     reinterpret_cast<PyCFunction>(proxyIcePreferSecure),
     METH_VARARGS,
     PyDoc_STR("ice_preferSecure(bool) -> Ice.ObjectPrx")},
    {"ice_getRouter",
     reinterpret_cast<PyCFunction>(proxyIceGetRouter),
     METH_NOARGS,
     PyDoc_STR("ice_getRouter() -> Ice.RouterPrx")},
    {"ice_router",
     reinterpret_cast<PyCFunction>(proxyIceRouter),
     METH_VARARGS,
     PyDoc_STR("ice_router(Ice.RouterPrx) -> Ice.ObjectPrx")},
    {"ice_getLocator",
     reinterpret_cast<PyCFunction>(proxyIceGetLocator),
     METH_NOARGS,
     PyDoc_STR("ice_getLocator() -> Ice.LocatorPrx")},
    {"ice_locator",
     reinterpret_cast<PyCFunction>(proxyIceLocator),
     METH_VARARGS,
     PyDoc_STR("ice_locator(Ice.LocatorPrx) -> Ice.ObjectPrx")},
    {"ice_twoway",
     reinterpret_cast<PyCFunction>(proxyIceTwoway),
     METH_NOARGS,
     PyDoc_STR("ice_twoway() -> Ice.ObjectPrx")},
    {"ice_isTwoway", reinterpret_cast<PyCFunction>(proxyIceIsTwoway), METH_NOARGS, PyDoc_STR("ice_isTwoway() -> bool")},
    {"ice_oneway",
     reinterpret_cast<PyCFunction>(proxyIceOneway),
     METH_NOARGS,
     PyDoc_STR("ice_oneway() -> Ice.ObjectPrx")},
    {"ice_isOneway", reinterpret_cast<PyCFunction>(proxyIceIsOneway), METH_NOARGS, PyDoc_STR("ice_isOneway() -> bool")},
    {"ice_batchOneway",
     reinterpret_cast<PyCFunction>(proxyIceBatchOneway),
     METH_NOARGS,
     PyDoc_STR("ice_batchOneway() -> Ice.ObjectPrx")},
    {"ice_isBatchOneway",
     reinterpret_cast<PyCFunction>(proxyIceIsBatchOneway),
     METH_NOARGS,
     PyDoc_STR("ice_isBatchOneway() -> bool")},
    {"ice_datagram",
     reinterpret_cast<PyCFunction>(proxyIceDatagram),
     METH_NOARGS,
     PyDoc_STR("ice_datagram() -> Ice.ObjectPrx")},
    {"ice_isDatagram",
     reinterpret_cast<PyCFunction>(proxyIceIsDatagram),
     METH_NOARGS,
     PyDoc_STR("ice_isDatagram() -> bool")},
    {"ice_batchDatagram",
     reinterpret_cast<PyCFunction>(proxyIceBatchDatagram),
     METH_NOARGS,
     PyDoc_STR("ice_batchDatagram() -> Ice.ObjectPrx")},
    {"ice_isBatchDatagram",
     reinterpret_cast<PyCFunction>(proxyIceIsBatchDatagram),
     METH_NOARGS,
     PyDoc_STR("ice_isBatchDatagram() -> bool")},
    {"ice_compress",
     reinterpret_cast<PyCFunction>(proxyIceCompress),
     METH_VARARGS,
     PyDoc_STR("ice_compress(bool) -> Ice.ObjectPrx")},
    {"ice_getCompress",
     reinterpret_cast<PyCFunction>(proxyIceGetCompress),
     METH_VARARGS,
     PyDoc_STR("ice_getCompress() -> bool")},
    {"ice_connectionId",
     reinterpret_cast<PyCFunction>(proxyIceConnectionId),
     METH_VARARGS,
     PyDoc_STR("ice_connectionId(string) -> Ice.ObjectPrx")},
    {"ice_fixed",
     reinterpret_cast<PyCFunction>(proxyIceFixed),
     METH_VARARGS,
     PyDoc_STR("ice_fixed(Ice.Connection) -> Ice.ObjectPrx")},
    {"ice_isFixed", reinterpret_cast<PyCFunction>(proxyIceIsFixed), METH_NOARGS, PyDoc_STR("ice_isFixed() -> bool")},
    {"ice_getConnection",
     reinterpret_cast<PyCFunction>(proxyIceGetConnection),
     METH_NOARGS,
     PyDoc_STR("ice_getConnection() -> Ice.Connection")},
    {"ice_getConnectionAsync",
     reinterpret_cast<PyCFunction>(proxyIceGetConnectionAsync),
     METH_NOARGS,
     PyDoc_STR("ice_getConnectionAsync() -> Ice.Future")},
    {"ice_getCachedConnection",
     reinterpret_cast<PyCFunction>(proxyIceGetCachedConnection),
     METH_NOARGS,
     PyDoc_STR("ice_getCachedConnection() -> Ice.Connection")},
    {"ice_flushBatchRequests",
     reinterpret_cast<PyCFunction>(proxyIceFlushBatchRequests),
     METH_NOARGS,
     PyDoc_STR("ice_flushBatchRequests() -> void")},
    {"ice_flushBatchRequestsAsync",
     reinterpret_cast<PyCFunction>(proxyIceFlushBatchRequestsAsync),
     METH_NOARGS,
     PyDoc_STR("ice_flushBatchRequestsAsync() -> Ice.Future")},
    {"ice_invoke",
     reinterpret_cast<PyCFunction>(proxyIceInvoke),
     METH_VARARGS,
     PyDoc_STR("ice_invoke(operation, mode, inParams) -> bool, outParams")},
    {"ice_invokeAsync",
     reinterpret_cast<PyCFunction>(proxyIceInvokeAsync),
     METH_VARARGS | METH_KEYWORDS,
     PyDoc_STR("ice_invokeAsync(op, mode, inParams[, context]) -> Ice.Future")},
    {"ice_checkedCast",
     reinterpret_cast<PyCFunction>(proxyIceCheckedCast),
     METH_VARARGS | METH_CLASS,
     PyDoc_STR("ice_checkedCast(proxy, id[, facetOrContext[, context]]) -> proxy")},
    {"ice_uncheckedCast",
     reinterpret_cast<PyCFunction>(proxyIceUncheckedCast),
     METH_VARARGS | METH_CLASS,
     PyDoc_STR("ice_uncheckedCast(proxy) -> proxy")},
    {"checkedCast",
     reinterpret_cast<PyCFunction>(proxyCheckedCast),
     METH_VARARGS | METH_STATIC,
     PyDoc_STR("checkedCast(proxy) -> proxy")},
    {"uncheckedCast",
     reinterpret_cast<PyCFunction>(proxyUncheckedCast),
     METH_VARARGS | METH_STATIC,
     PyDoc_STR("uncheckedCast(proxy) -> proxy")},
    {"ice_staticId",
     reinterpret_cast<PyCFunction>(proxyIceStaticId),
     METH_NOARGS | METH_STATIC,
     PyDoc_STR("ice_staticId() -> string")},
    {} /* sentinel */
};

namespace IcePy
{
    // clang-format off
    PyTypeObject ProxyType = {
        .ob_base = PyVarObject_HEAD_INIT(nullptr, 0)
        .tp_name = "IcePy.ObjectPrx",
        .tp_basicsize = sizeof(ProxyObject),
        .tp_dealloc = reinterpret_cast<destructor>(proxyDealloc),
        .tp_repr = reinterpret_cast<reprfunc>(proxyRepr),
        .tp_hash = reinterpret_cast<hashfunc>(proxyHash),
        .tp_flags = Py_TPFLAGS_BASETYPE,
        .tp_richcompare = reinterpret_cast<richcmpfunc>(proxyCompare),
        .tp_methods = ProxyMethods,
        .tp_init = reinterpret_cast<initproc>(proxyInit),
        .tp_new = reinterpret_cast<newfunc>(proxyNew),
    };
    // clang-format on
}

bool
IcePy::initProxy(PyObject* module)
{
    if (PyType_Ready(&ProxyType) < 0)
    {
        return false;
    }

    if (PyModule_AddObject(module, "ObjectPrx", reinterpret_cast<PyObject*>(&ProxyType)) < 0)
    {
        return false;
    }

    return true;
}

PyObject*
IcePy::createProxy(const Ice::ObjectPrx& proxy, const Ice::CommunicatorPtr& communicator, PyObject* type)
{
    if (!type)
    {
        type = lookupType("Ice.ObjectPrx");
    }
    return reinterpret_cast<PyObject*>(allocateProxy(proxy, communicator, type));
}

bool
IcePy::checkProxy(PyObject* p)
{
    return PyObject_IsInstance(p, reinterpret_cast<PyObject*>(&ProxyType)) == 1;
}

Ice::ObjectPrx
IcePy::getProxy(PyObject* p)
{
    assert(checkProxy(p));
    auto* obj = reinterpret_cast<ProxyObject*>(p);
    return *obj->proxy;
}

bool
IcePy::getProxyArg(
    PyObject* p,
    const string& func,
    const string& arg,
    optional<Ice::ObjectPrx>& proxy,
    const string& type)
{
    bool result = true;

    if (checkProxy(p))
    {
        if (!type.empty())
        {
            PyObject* proxyType = lookupType(type);
            assert(proxyType);
            if (!PyObject_IsInstance(p, proxyType))
            {
                result = false;
            }
        }
    }
    else if (p != Py_None)
    {
        result = false;
    }

    if (result)
    {
        if (p != Py_None)
        {
            auto* obj = reinterpret_cast<ProxyObject*>(p);
            proxy = *obj->proxy;
        }
        else
        {
            proxy = nullopt;
        }
    }
    else
    {
        string typeName = type.empty() ? "Ice.ObjectPrx" : type;
        PyErr_Format(
            PyExc_ValueError,
            "%s expects a proxy of type %s or None for argument '%s'",
            func.c_str(),
            typeName.c_str(),
            arg.c_str());
    }

    return result;
}

Ice::CommunicatorPtr
IcePy::getProxyCommunicator(PyObject* p)
{
    assert(checkProxy(p));
    auto* obj = reinterpret_cast<ProxyObject*>(p);
    return *obj->communicator;
}
