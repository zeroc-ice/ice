// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifdef _WIN32
#   include <IceUtil/Config.h>
#endif
#include <Proxy.h>
#include <structmember.h>
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

struct ProxyObject
{
    PyObject_HEAD
    Ice::ObjectPrx* proxy;
    Ice::CommunicatorPtr* communicator;
};

}

static ProxyObject*
allocateProxy(const Ice::ObjectPrx& proxy, const Ice::CommunicatorPtr& communicator, PyObject* type)
{
    ProxyObject* p = (ProxyObject*)((PyTypeObject*)type)->tp_alloc((PyTypeObject*)type, 0);
    if(p == NULL)
    {
        return NULL;
    }

    //
    // Disabling collocation optimization can cause subtle problems with proxy
    // comparison (such as in RouterInfo::get) if a proxy from IcePy is
    // compared with a proxy from Ice/C++.
    //
    //if(proxy)
    //{
    //    p->proxy = new Ice::ObjectPrx(proxy->ice_collocationOptimization(false));
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
    PyErr_Format(PyExc_RuntimeError, "A proxy cannot be created directly");
    return NULL;
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
    string str = (*self->communicator)->proxyToString(*self->proxy);
    return PyString_FromString(const_cast<char*>(str.c_str()));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceIsA(ProxyObject* self, PyObject* args)
{
    char* type;
    PyObject* ctx = 0;
    if(!PyArg_ParseTuple(args, "s|O!", &type, &PyDict_Type, &ctx))
    {
        return NULL;
    }

    Ice::Context context;
    if(ctx && !dictionaryToContext(ctx, context))
    {
        return NULL;
    }

    assert(self->proxy);

    bool b;
    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.
        b = (*self->proxy)->ice_isA(type, context);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    PyObject* result = b ? Py_True : Py_False;
    Py_INCREF(result);
    return result;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIcePing(ProxyObject* self, PyObject* args)
{
    PyObject* ctx = 0;
    if(!PyArg_ParseTuple(args, "|O!", &PyDict_Type, &ctx))
    {
        return NULL;
    }

    Ice::Context context;
    if(ctx && !dictionaryToContext(ctx, context))
    {
        return NULL;
    }

    assert(self->proxy);

    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.
        (*self->proxy)->ice_ping(context);
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
proxyIceIds(ProxyObject* self, PyObject* args)
{
    PyObject* ctx = 0;
    if(!PyArg_ParseTuple(args, "|O!", &PyDict_Type, &ctx))
    {
        return NULL;
    }

    Ice::Context context;
    if(ctx && !dictionaryToContext(ctx, context))
    {
        return NULL;
    }

    assert(self->proxy);

    Ice::StringSeq ids;
    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.
        ids = (*self->proxy)->ice_ids(context);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    PyObject* list = PyList_New(0);
    if(list == NULL || !stringSeqToList(ids, list))
    {
        return NULL;
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
    if(!PyArg_ParseTuple(args, "|O!", &PyDict_Type, &ctx))
    {
        return NULL;
    }

    Ice::Context context;
    if(ctx && !dictionaryToContext(ctx, context))
    {
        return NULL;
    }

    assert(self->proxy);

    string id;
    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.
        id = (*self->proxy)->ice_id(context);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    return Py_BuildValue("s", id.c_str());
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
        return NULL;
    }

    return createIdentity(id);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceNewIdentity(ProxyObject* self, PyObject* args)
{
    PyObject* identityType = lookupType("Ice.Identity");
    assert(identityType);
    PyObject* id;
    if(!PyArg_ParseTuple(args, "O!", identityType, &id))
    {
        return NULL;
    }

    assert(self->proxy);

    Ice::Identity ident;
    if(!getIdentity(id, ident))
    {
        return NULL;
    }

    Ice::ObjectPrx newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_newIdentity(ident);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    return createProxy(newProxy, *self->communicator);
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
        return NULL;
    }

    PyObjectHandle result = PyDict_New();
    if(result.get() && contextToDictionary(ctx, result.get()))
    {
        return result.release();
    }
    return NULL;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceNewContext(ProxyObject* self, PyObject* args)
{
    PyObject* dict;
    if(!PyArg_ParseTuple(args, "O!", &PyDict_Type, &dict))
    {
        return NULL;
    }

    assert(self->proxy);

    Ice::Context ctx;
    if(!dictionaryToContext(dict, ctx))
    {
        return NULL;
    }

    Ice::ObjectPrx newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_newContext(ctx);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    return createProxy(newProxy, *self->communicator);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceDefaultContext(ProxyObject* self)
{
    assert(self->proxy);

    Ice::ObjectPrx newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_defaultContext();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    return createProxy(newProxy, *self->communicator);
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
        return NULL;
    }

    return PyString_FromString(const_cast<char*>(facet.c_str()));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceNewFacet(ProxyObject* self, PyObject* args)
{
    char* facet;
    if(!PyArg_ParseTuple(args, "s", &facet))
    {
        return NULL;
    }

    assert(self->proxy);

    Ice::ObjectPrx newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_newFacet(facet);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    return createProxy(newProxy, *self->communicator);
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
        return NULL;
    }

    return createProxy(newProxy, *self->communicator);
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
        b = (*self->proxy)->ice_isTwoway() ? Py_True : Py_False;
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
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
        return NULL;
    }

    return createProxy(newProxy, *self->communicator);
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
        b = (*self->proxy)->ice_isOneway() ? Py_True : Py_False;
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
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
        return NULL;
    }

    return createProxy(newProxy, *self->communicator);
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
        b = (*self->proxy)->ice_isBatchOneway() ? Py_True : Py_False;
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
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
        return NULL;
    }

    return createProxy(newProxy, *self->communicator);
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
        b = (*self->proxy)->ice_isDatagram() ? Py_True : Py_False;
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
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
        return NULL;
    }

    return createProxy(newProxy, *self->communicator);
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
        b = (*self->proxy)->ice_isBatchDatagram() ? Py_True : Py_False;
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
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
    if(!PyArg_ParseTuple(args, "O", &flag))
    {
        return NULL;
    }

    int n = PyObject_IsTrue(flag);
    if(n < 0)
    {
        return NULL;
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
        return NULL;
    }

    return createProxy(newProxy, *self->communicator);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceCompress(ProxyObject* self, PyObject* args)
{
    PyObject* flag;
    if(!PyArg_ParseTuple(args, "O", &flag))
    {
        return NULL;
    }

    int n = PyObject_IsTrue(flag);
    if(n < 0)
    {
        return NULL;
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
        return NULL;
    }

    return createProxy(newProxy, *self->communicator);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceTimeout(ProxyObject* self, PyObject* args)
{
    int timeout;
    if(!PyArg_ParseTuple(args, "i", &timeout))
    {
        return NULL;
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
        return NULL;
    }

    return createProxy(newProxy, *self->communicator);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceRouter(ProxyObject* self, PyObject* args)
{
    PyObject* p;
    if(!PyArg_ParseTuple(args, "O", &p))
    {
        return NULL;
    }

    PyObject* routerProxyType = lookupType("Ice.RouterPrx");
    assert(routerProxyType != NULL);
    Ice::RouterPrx routerProxy;
    if(PyObject_IsInstance(p, routerProxyType))
    {
        routerProxy = Ice::RouterPrx::uncheckedCast(getProxy(p));
    }
    else if(p != Py_None)
    {
        PyErr_Format(PyExc_ValueError, "ice_router requires None or Ice.RouterPrx");
        return NULL;
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
        return NULL;
    }

    return createProxy(newProxy, *self->communicator);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceLocator(ProxyObject* self, PyObject* args)
{
    PyObject* p;
    if(!PyArg_ParseTuple(args, "O", &p))
    {
        return NULL;
    }

    PyObject* locatorProxyType = lookupType("Ice.LocatorPrx");
    assert(locatorProxyType != NULL);
    Ice::LocatorPrx locatorProxy;
    if(PyObject_IsInstance(p, locatorProxyType))
    {
        locatorProxy = Ice::LocatorPrx::uncheckedCast(getProxy(p));
    }
    else if(p != Py_None)
    {
        PyErr_Format(PyExc_ValueError, "ice_locator requires None or Ice.LocatorPrx");
        return NULL;
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
        return NULL;
    }

    return createProxy(newProxy, *self->communicator);
}

// NOTE: ice_collocationOptimization is not currently supported.

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceDefault(ProxyObject* self)
{
    assert(self->proxy);

    Ice::ObjectPrx newProxy;
    try
    {
        newProxy = (*self->proxy)->ice_default();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    return createProxy(newProxy, *self->communicator);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceConnection(ProxyObject* self)
{
    assert(self->proxy);

    Ice::ConnectionPtr con;
    try
    {
        con = (*self->proxy)->ice_connection();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
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

static PyObject*
checkedCastImpl(ProxyObject* p, const string& id, const string& facet, PyObject* type)
{
    Ice::ObjectPrx target;
    if(facet.empty())
    {
        target = *p->proxy;
    }
    else
    {
        target = (*p->proxy)->ice_newFacet(facet);
    }

    bool b;
    try
    {
        AllowThreads allowThreads; // Release Python's global interpreter lock during remote invocations.
        b = target->ice_isA(id);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
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
    PyObject* obj;
    char* id;
    char* facet;
    if(!PyArg_ParseTuple(args, "Oss", &obj, &id, &facet))
    {
        return NULL;
    }

    if(obj == Py_None)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    if(!checkProxy(obj))
    {
        PyErr_Format(PyExc_ValueError, "ice_checkedCast requires a proxy argument");
        return NULL;
    }

    return checkedCastImpl((ProxyObject*)obj, id, facet, type);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyIceUncheckedCast(PyObject* type, PyObject* args)
{
    PyObject* obj;
    char* facet;
    if(!PyArg_ParseTuple(args, "Os", &obj, &facet))
    {
        return NULL;
    }

    if(obj == Py_None)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    if(!checkProxy(obj))
    {
        PyErr_Format(PyExc_ValueError, "ice_uncheckedCast requires a proxy argument");
        return NULL;
    }

    ProxyObject* p = (ProxyObject*)obj;

    if(strlen(facet) > 0)
    {
        return createProxy((*p->proxy)->ice_newFacet(facet), *p->communicator, type);
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
    char* facet = "";
    if(!PyArg_ParseTuple(args, "O|s", &obj, &facet))
    {
        return NULL;
    }

    if(obj == Py_None)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    if(!checkProxy(obj))
    {
        PyErr_Format(PyExc_ValueError, "checkedCast requires a proxy argument");
        return NULL;
    }

    return checkedCastImpl((ProxyObject*)obj, "::Ice::Object", facet, NULL);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
proxyUncheckedCast(PyObject* /*self*/, PyObject* args)
{
    PyObject* obj;
    char* facet = NULL;
    if(!PyArg_ParseTuple(args, "O|s", &obj, &facet))
    {
        return NULL;
    }

    if(obj == Py_None)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    if(!checkProxy(obj))
    {
        PyErr_Format(PyExc_ValueError, "uncheckedCast requires a proxy argument");
        return NULL;
    }

    ProxyObject* p = (ProxyObject*)obj;

    if(facet && strlen(facet) > 0)
    {
        return createProxy((*p->proxy)->ice_newFacet(facet), *p->communicator, NULL);
    }
    else
    {
        return createProxy(*p->proxy, *p->communicator, NULL);
    }
}

static PyMethodDef ProxyMethods[] =
{
    { "ice_isA", (PyCFunction)proxyIceIsA, METH_VARARGS,
        PyDoc_STR("ice_isA(type, [ctx]) -> bool") },
    { "ice_ping", (PyCFunction)proxyIcePing, METH_VARARGS,
        PyDoc_STR("ice_ping([ctx]) -> None") },
    { "ice_ids", (PyCFunction)proxyIceIds, METH_VARARGS,
        PyDoc_STR("ice_ids([ctx]) -> list") },
    { "ice_id", (PyCFunction)proxyIceId, METH_VARARGS,
        PyDoc_STR("ice_id([ctx]) -> string") },
    { "ice_getIdentity", (PyCFunction)proxyIceGetIdentity, METH_NOARGS,
        PyDoc_STR("ice_getIdentity() -> Ice.Identity") },
    { "ice_newIdentity", (PyCFunction)proxyIceNewIdentity, METH_VARARGS,
        PyDoc_STR("ice_newIdentity(id) -> Ice.ObjectPrx") },
    { "ice_getContext", (PyCFunction)proxyIceGetContext, METH_NOARGS,
        PyDoc_STR("ice_getContext() -> dict") },
    { "ice_newContext", (PyCFunction)proxyIceNewContext, METH_VARARGS,
        PyDoc_STR("ice_newContext(dict) -> Ice.ObjectPrx") },
    { "ice_defaultContext", (PyCFunction)proxyIceDefaultContext, METH_NOARGS,
        PyDoc_STR("ice_defaultContext() -> Ice.ObjectPrx") },
    { "ice_getFacet", (PyCFunction)proxyIceGetFacet, METH_NOARGS,
        PyDoc_STR("ice_getFacet() -> string") },
    { "ice_newFacet", (PyCFunction)proxyIceNewFacet, METH_VARARGS,
        PyDoc_STR("ice_newFacet(string) -> Ice.ObjectPrx") },
    { "ice_twoway", (PyCFunction)proxyIceTwoway, METH_NOARGS,
        PyDoc_STR("ice_twoway() -> Ice.ObjectPrx") },
    { "ice_isTwoway", (PyCFunction)proxyIceIsTwoway, METH_NOARGS,
        PyDoc_STR("ice_isTwoway() -> bool") },
    { "ice_oneway", (PyCFunction)proxyIceOneway, METH_NOARGS,
        PyDoc_STR("ice_oneway() -> Ice.ObjectPrx") },
    { "ice_isOneway", (PyCFunction)proxyIceIsOneway, METH_NOARGS,
        PyDoc_STR("ice_isOneway() -> bool") },
    { "ice_batchOneway", (PyCFunction)proxyIceBatchOneway, METH_NOARGS,
        PyDoc_STR("ice_batchOneway() -> Ice.ObjectPrx") },
    { "ice_isBatchOneway", (PyCFunction)proxyIceIsBatchOneway, METH_NOARGS,
        PyDoc_STR("ice_isBatchOneway() -> bool") },
    { "ice_datagram", (PyCFunction)proxyIceDatagram, METH_NOARGS,
        PyDoc_STR("ice_datagram() -> Ice.ObjectPrx") },
    { "ice_isDatagram", (PyCFunction)proxyIceIsDatagram, METH_NOARGS,
        PyDoc_STR("ice_isDatagram() -> bool") },
    { "ice_batchDatagram", (PyCFunction)proxyIceBatchDatagram, METH_NOARGS,
        PyDoc_STR("ice_batchDatagram() -> Ice.ObjectPrx") },
    { "ice_isBatchDatagram", (PyCFunction)proxyIceIsBatchDatagram, METH_NOARGS,
        PyDoc_STR("ice_isBatchDatagram() -> bool") },
    { "ice_secure", (PyCFunction)proxyIceSecure, METH_VARARGS,
        PyDoc_STR("ice_secure(bool) -> Ice.ObjectPrx") },
    { "ice_compress", (PyCFunction)proxyIceCompress, METH_VARARGS,
        PyDoc_STR("ice_compress(bool) -> Ice.ObjectPrx") },
    { "ice_timeout", (PyCFunction)proxyIceTimeout, METH_VARARGS,
        PyDoc_STR("ice_timeout(int) -> Ice.ObjectPrx") },
    { "ice_router", (PyCFunction)proxyIceRouter, METH_VARARGS,
        PyDoc_STR("ice_router(Ice.RouterPrx) -> Ice.ObjectPrx") },
    { "ice_locator", (PyCFunction)proxyIceLocator, METH_VARARGS,
        PyDoc_STR("ice_locator(Ice.LocatorPrx) -> Ice.ObjectPrx") },
    { "ice_default", (PyCFunction)proxyIceDefault, METH_NOARGS,
        PyDoc_STR("ice_default() -> Ice.ObjectPrx") },
    { "ice_connection", (PyCFunction)proxyIceConnection, METH_NOARGS,
        PyDoc_STR("ice_connection() -> Ice.Connection") },
    { "ice_checkedCast", (PyCFunction)proxyIceCheckedCast, METH_VARARGS | METH_CLASS,
        PyDoc_STR("ice_checkedCast(proxy, id) -> proxy") },
    { "ice_uncheckedCast", (PyCFunction)proxyIceUncheckedCast, METH_VARARGS | METH_CLASS,
        PyDoc_STR("ice_uncheckedCast(proxy) -> proxy") },
    { "checkedCast", (PyCFunction)proxyCheckedCast, METH_VARARGS | METH_STATIC,
        PyDoc_STR("checkedCast(proxy) -> proxy") },
    { "uncheckedCast", (PyCFunction)proxyUncheckedCast, METH_VARARGS | METH_STATIC,
        PyDoc_STR("uncheckedCast(proxy) -> proxy") },
    { NULL, NULL} /* sentinel */
};

namespace IcePy
{

PyTypeObject ProxyType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyObject_HEAD_INIT(NULL)
    0,                               /* ob_size */
    "IcePy.ObjectPrx",               /* tp_name */
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
    if(PyModule_AddObject(module, "ObjectPrx", (PyObject*)&ProxyType) < 0)
    {
        return false;
    }
    return true;
}

PyObject*
IcePy::createProxy(const Ice::ObjectPrx& proxy, const Ice::CommunicatorPtr& communicator, PyObject* type)
{
    if(type == NULL)
    {
        type = (PyObject*)&ProxyType;
    }
    return (PyObject*)allocateProxy(proxy, communicator, type);
}

bool
IcePy::checkProxy(PyObject* p)
{
    return PyObject_IsInstance(p, (PyObject*)&ProxyType) == 1;
}

Ice::ObjectPrx
IcePy::getProxy(PyObject* p)
{
    assert(checkProxy(p));
    ProxyObject* obj = (ProxyObject*)p;
    return *obj->proxy;
}

Ice::CommunicatorPtr
IcePy::getProxyCommunicator(PyObject* p)
{
    assert(checkProxy(p));
    ProxyObject* obj = (ProxyObject*)p;
    return *obj->communicator;
}
