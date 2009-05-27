// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifdef _WIN32
#   include <IceUtil/Config.h>
#endif
#include <Endpoint.h>
#include <Util.h>

using namespace std;
using namespace IcePy;

namespace IcePy
{

struct EndpointObject
{
    PyObject_HEAD
    Ice::EndpointPtr* endpoint;
};

extern PyTypeObject TcpEndpointType;
extern PyTypeObject UdpEndpointType;
extern PyTypeObject OpaqueEndpointType;

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
endpointToString(EndpointObject* self)
{
    assert(self->endpoint);
    try
    {
        string str = (*self->endpoint)->toString();
        return createString(str);
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
endpointRepr(EndpointObject* self)
{
    return endpointToString(self);
}

//
// Ice::Endpoint::timeout
//
#ifdef WIN32
extern "C"
#endif
static PyObject*
endpointTimeout(EndpointObject* self)
{
    assert(self->endpoint);
    try
    {
        Ice::Int timeout = (*self->endpoint)->timeout();
        return PyInt_FromLong(timeout);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }
}

//
// Ice::Endpoint::compress
//
#ifdef WIN32
extern "C"
#endif
static PyObject*
endpointCompress(EndpointObject* self)
{
    assert(self->endpoint);
    PyObject* b;
    try
    {
        b = (*self->endpoint)->compress() ? getTrue() : getFalse();
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }

    Py_INCREF(b);
    return b;
}

//
// Ice::TcpEndpoint::host
//
#ifdef WIN32
extern "C"
#endif
static PyObject*
tcpEndpointHost(EndpointObject* self)
{
    assert(self->endpoint);
    Ice::TcpEndpointPtr p = Ice::TcpEndpointPtr::dynamicCast(*self->endpoint);
    assert(p);

    try
    {
        string str = p->host();
        return createString(str);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }
}

//
// Ice::TcpEndpoint::port
//
#ifdef WIN32
extern "C"
#endif
static PyObject*
tcpEndpointPort(EndpointObject* self)
{
    assert(self->endpoint);
    Ice::TcpEndpointPtr p = Ice::TcpEndpointPtr::dynamicCast(*self->endpoint);
    assert(p);

    try
    {
        Ice::Int port = p->port();
        return PyInt_FromLong(port);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }
}

//
// Ice::UdpEndpoint::host
//
#ifdef WIN32
extern "C"
#endif
static PyObject*
udpEndpointHost(EndpointObject* self)
{
    assert(self->endpoint);
    Ice::UdpEndpointPtr p = Ice::UdpEndpointPtr::dynamicCast(*self->endpoint);
    assert(p);

    try
    {
        string str = p->host();
        return createString(str);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }
}

//
// Ice::UdpEndpoint::port
//
#ifdef WIN32
extern "C"
#endif
static PyObject*
udpEndpointPort(EndpointObject* self)
{
    assert(self->endpoint);
    Ice::UdpEndpointPtr p = Ice::UdpEndpointPtr::dynamicCast(*self->endpoint);
    assert(p);

    try
    {
        Ice::Int port = p->port();
        return PyInt_FromLong(port);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }
}

//
// Ice::UdpEndpoint::mcastInterface
//
#ifdef WIN32
extern "C"
#endif
static PyObject*
udpEndpointMcastInterface(EndpointObject* self)
{
    assert(self->endpoint);
    Ice::UdpEndpointPtr p = Ice::UdpEndpointPtr::dynamicCast(*self->endpoint);
    assert(p);

    try
    {
        string iface = p->mcastInterface();
        return createString(iface);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }
}

//
// Ice::UdpEndpoint::mcastTtl
//
#ifdef WIN32
extern "C"
#endif
static PyObject*
udpEndpointMcastTtl(EndpointObject* self)
{
    assert(self->endpoint);
    Ice::UdpEndpointPtr p = Ice::UdpEndpointPtr::dynamicCast(*self->endpoint);
    assert(p);

    try
    {
        Ice::Int ttl = p->mcastTtl();
        return PyInt_FromLong(ttl);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }
}

//
// Ice::OpaqueEndpoint::rawBytes
//
#ifdef WIN32
extern "C"
#endif
static PyObject*
opaqueEndpointRawBytes(EndpointObject* self)
{
    assert(self->endpoint);
    Ice::OpaqueEndpointPtr p = Ice::OpaqueEndpointPtr::dynamicCast(*self->endpoint);
    assert(p);

    try
    {
        Ice::ByteSeq b = p->rawBytes();
        return PyString_FromStringAndSize(reinterpret_cast<const char*>(&b[0]), static_cast<int>(b.size()));
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }
}

static PyMethodDef EndpointMethods[] =
{
    { STRCAST("toString"), reinterpret_cast<PyCFunction>(endpointToString), METH_NOARGS,
        PyDoc_STR(STRCAST("toString() -> string")) },
    { STRCAST("timeout"), reinterpret_cast<PyCFunction>(endpointTimeout), METH_NOARGS,
        PyDoc_STR(STRCAST("timeout() -> int")) },
    { STRCAST("compress"), reinterpret_cast<PyCFunction>(endpointCompress), METH_NOARGS,
        PyDoc_STR(STRCAST("compress() -> bool")) },
    { 0, 0 } /* sentinel */
};

static PyMethodDef TcpEndpointMethods[] =
{
    { STRCAST("host"), reinterpret_cast<PyCFunction>(tcpEndpointHost), METH_NOARGS,
        PyDoc_STR(STRCAST("host() -> string")) },
    { STRCAST("port"), reinterpret_cast<PyCFunction>(tcpEndpointPort), METH_NOARGS,
        PyDoc_STR(STRCAST("port() -> int")) },
    { 0, 0 } /* sentinel */
};

static PyMethodDef UdpEndpointMethods[] =
{
    { STRCAST("host"), reinterpret_cast<PyCFunction>(udpEndpointHost), METH_NOARGS,
        PyDoc_STR(STRCAST("host() -> string")) },
    { STRCAST("port"), reinterpret_cast<PyCFunction>(udpEndpointPort), METH_NOARGS,
        PyDoc_STR(STRCAST("port() -> int")) },
    { STRCAST("mcastInterface"), reinterpret_cast<PyCFunction>(udpEndpointMcastInterface), METH_NOARGS,
        PyDoc_STR(STRCAST("mcastInterface() -> string")) },
    { STRCAST("mcastTtl"), reinterpret_cast<PyCFunction>(udpEndpointMcastTtl), METH_NOARGS,
        PyDoc_STR(STRCAST("mcastTtl() -> int")) },
    { 0, 0 } /* sentinel */
};

static PyMethodDef OpaqueEndpointMethods[] =
{
    { STRCAST("rawBytes"), reinterpret_cast<PyCFunction>(opaqueEndpointRawBytes), METH_NOARGS,
        PyDoc_STR(STRCAST("rawBytes() -> string")) },
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
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
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

PyTypeObject TcpEndpointType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyObject_HEAD_INIT(0)
    0,                               /* ob_size */
    STRCAST("Ice.TcpEndpoint"),      /* tp_name */
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
    TcpEndpointMethods,              /* tp_methods */
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

PyTypeObject UdpEndpointType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyObject_HEAD_INIT(0)
    0,                               /* ob_size */
    STRCAST("Ice.UdpEndpoint"),      /* tp_name */
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
    UdpEndpointMethods,              /* tp_methods */
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

PyTypeObject OpaqueEndpointType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyObject_HEAD_INIT(0)
    0,                               /* ob_size */
    STRCAST("Ice.OpaqueEndpoint"),   /* tp_name */
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
    OpaqueEndpointMethods,           /* tp_methods */
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

}

bool
IcePy::initEndpoint(PyObject* module)
{
    if(PyType_Ready(&EndpointType) < 0)
    {
        return false;
    }
    PyTypeObject* type = &EndpointType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("Endpoint"), reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    TcpEndpointType.tp_base = &EndpointType; // Force inheritance from EndpointType.
    if(PyType_Ready(&TcpEndpointType) < 0)
    {
        return false;
    }
    type = &TcpEndpointType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("TcpEndpoint"), reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    UdpEndpointType.tp_base = &EndpointType; // Force inheritance from EndpointType.
    if(PyType_Ready(&UdpEndpointType) < 0)
    {
        return false;
    }
    type = &UdpEndpointType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("UdpEndpoint"), reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    OpaqueEndpointType.tp_base = &EndpointType; // Force inheritance from EndpointType.
    if(PyType_Ready(&OpaqueEndpointType) < 0)
    {
        return false;
    }
    type = &OpaqueEndpointType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("OpaqueEndpoint"), reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    return true;
}

Ice::EndpointPtr
IcePy::getEndpoint(PyObject* obj)
{
    assert(PyObject_IsInstance(obj, reinterpret_cast<PyObject*>(&EndpointType)));
    EndpointObject* eobj = reinterpret_cast<EndpointObject*>(obj);
    return *eobj->endpoint;
}

PyObject*
IcePy::createEndpoint(const Ice::EndpointPtr& endpoint)
{
    PyTypeObject* type;
    if(Ice::TcpEndpointPtr::dynamicCast(endpoint))
    {
        type = &TcpEndpointType;
    }
    else if(Ice::UdpEndpointPtr::dynamicCast(endpoint))
    {
        type = &UdpEndpointType;
    }
    else if(Ice::OpaqueEndpointPtr::dynamicCast(endpoint))
    {
        type = &OpaqueEndpointType;
    }
    else
    {
        type = &EndpointType;
    }
    EndpointObject* obj = PyObject_New(EndpointObject, type);
    if(!obj)
    {
        return 0;
    }
    obj->endpoint = new Ice::EndpointPtr(endpoint);
    return (PyObject*)obj;
}
