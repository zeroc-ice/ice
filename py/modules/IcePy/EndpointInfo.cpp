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
#include <EndpointInfo.h>
#include <Util.h>

using namespace std;
using namespace IcePy;

namespace IcePy
{

struct EndpointInfoObject
{
    PyObject_HEAD
    Ice::EndpointInfoPtr* endpointInfo;
};

}

//
// EndpointInfo members
//
#define MEMBER_TIMEOUT          0
#define MEMBER_COMPRESS         1

//
// Shared TCP/UDP members
//
#define MEMBER_HOST             2
#define MEMBER_PORT             3

//
// UdpEndpointInfo members
//
#define MEMBER_PROTOCOL_MAJOR   4
#define MEMBER_PROTOCOL_MINOR   5
#define MEMBER_ENCODING_MAJOR   6
#define MEMBER_ENCODING_MINOR   7
#define MEMBER_MCAST_INTERFACE  8
#define MEMBER_MCAST_TTL        9

//
// OpaqueEndpointInfo members
//
#define MEMBER_RAW_BYTES        2

#ifdef WIN32
extern "C"
#endif
static EndpointInfoObject*
endpointInfoNew(PyObject* /*arg*/)
{
    PyErr_Format(PyExc_RuntimeError, STRCAST("An endpoint info cannot be created directly"));
    return 0;
}

#ifdef WIN32
extern "C"
#endif
static void
endpointInfoDealloc(EndpointInfoObject* self)
{
    delete self->endpointInfo;
    PyObject_Del(self);
}

//
// Ice::EndpointInfo::type
//
#ifdef WIN32
extern "C"
#endif
static PyObject*
endpointInfoType(EndpointInfoObject* self)
{
    assert(self->endpointInfo);
    try
    {
        Ice::Short type = (*self->endpointInfo)->type();
        return PyInt_FromLong(type);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return 0;
    }
}

//
// Ice::EndpointInfo::datagram
//
#ifdef WIN32
extern "C"
#endif
static PyObject*
endpointInfoDatagram(EndpointInfoObject* self)
{
    assert(self->endpointInfo);
    PyObject* b;
    try
    {
        b = (*self->endpointInfo)->datagram() ? getTrue() : getFalse();
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
// Ice::EndpointInfo::secure
//
#ifdef WIN32
extern "C"
#endif
static PyObject*
endpointInfoSecure(EndpointInfoObject* self)
{
    assert(self->endpointInfo);
    PyObject* b;
    try
    {
        b = (*self->endpointInfo)->secure() ? getTrue() : getFalse();
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
endpointInfoGetter(EndpointInfoObject* self, void* closure)
{
    int member = reinterpret_cast<int>(closure);
    PyObject* result = 0;

    switch(member)
    {
    case MEMBER_TIMEOUT:
        result = PyInt_FromLong((*self->endpointInfo)->timeout);
        break;
    case MEMBER_COMPRESS:
        result = (*self->endpointInfo)->compress ? getTrue() : getFalse();
        Py_INCREF(result);
        break;
    default:
        assert(false);
    }

    return result;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
tcpEndpointInfoGetter(EndpointInfoObject* self, void* closure)
{
    int member = reinterpret_cast<int>(closure);
    PyObject* result = 0;
    Ice::TcpEndpointInfoPtr info = Ice::TcpEndpointInfoPtr::dynamicCast(*self->endpointInfo);
    assert(info);

    switch(member)
    {
    case MEMBER_HOST:
        result = createString(info->host);
        break;
    case MEMBER_PORT:
        result = PyInt_FromLong(info->port);
        break;
    default:
        assert(false);
    }

    return result;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
udpEndpointInfoGetter(EndpointInfoObject* self, void* closure)
{
    int member = reinterpret_cast<int>(closure);
    PyObject* result = 0;
    Ice::UdpEndpointInfoPtr info = Ice::UdpEndpointInfoPtr::dynamicCast(*self->endpointInfo);
    assert(info);

    switch(member)
    {
    case MEMBER_HOST:
        result = createString(info->host);
        break;
    case MEMBER_PORT:
        result = PyInt_FromLong(info->port);
        break;
    case MEMBER_PROTOCOL_MAJOR:
        result = PyInt_FromLong(info->protocolMajor);
        break;
    case MEMBER_PROTOCOL_MINOR:
        result = PyInt_FromLong(info->protocolMinor);
        break;
    case MEMBER_ENCODING_MAJOR:
        result = PyInt_FromLong(info->encodingMajor);
        break;
    case MEMBER_ENCODING_MINOR:
        result = PyInt_FromLong(info->encodingMinor);
        break;
    case MEMBER_MCAST_INTERFACE:
        result = createString(info->mcastInterface);
        break;
    case MEMBER_MCAST_TTL:
        result = PyInt_FromLong(info->mcastTtl);
        break;
    default:
        assert(false);
    }

    return result;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
opaqueEndpointInfoGetter(EndpointInfoObject* self, void* closure)
{
    int member = reinterpret_cast<int>(closure);
    PyObject* result = 0;
    Ice::OpaqueEndpointInfoPtr info = Ice::OpaqueEndpointInfoPtr::dynamicCast(*self->endpointInfo);
    assert(info);

    switch(member)
    {
    case MEMBER_RAW_BYTES:
        result = PyString_FromStringAndSize(reinterpret_cast<const char*>(&info->rawBytes[0]),
                                            static_cast<int>(info->rawBytes.size()));
        break;
    default:
        assert(false);
    }

    return result;
}

static PyMethodDef EndpointInfoMethods[] =
{
    { STRCAST("type"), reinterpret_cast<PyCFunction>(endpointInfoType), METH_NOARGS,
        PyDoc_STR(STRCAST("type() -> int")) },
    { STRCAST("datagram"), reinterpret_cast<PyCFunction>(endpointInfoDatagram), METH_NOARGS,
        PyDoc_STR(STRCAST("datagram() -> bool")) },
    { STRCAST("secure"), reinterpret_cast<PyCFunction>(endpointInfoSecure), METH_NOARGS,
        PyDoc_STR(STRCAST("secure() -> bool")) },
    { 0, 0 } /* sentinel */
};

static PyGetSetDef EndpointInfoGetters[] =
{
    { STRCAST("timeout"), reinterpret_cast<getter>(endpointInfoGetter), 0,
        PyDoc_STR(STRCAST("timeout in milliseconds")), reinterpret_cast<void*>(MEMBER_TIMEOUT) },
    { STRCAST("compress"), reinterpret_cast<getter>(endpointInfoGetter), 0,
        PyDoc_STR(STRCAST("compression status")), reinterpret_cast<void*>(MEMBER_COMPRESS) },
    { 0, 0 } /* sentinel */
};

static PyGetSetDef TcpEndpointInfoGetters[] =
{
    { STRCAST("host"), reinterpret_cast<getter>(tcpEndpointInfoGetter), 0,
        PyDoc_STR(STRCAST("host name or IP address")), reinterpret_cast<void*>(MEMBER_HOST) },
    { STRCAST("port"), reinterpret_cast<getter>(tcpEndpointInfoGetter), 0,
        PyDoc_STR(STRCAST("TCP port number")), reinterpret_cast<void*>(MEMBER_PORT) },
    { 0, 0 } /* sentinel */
};

static PyGetSetDef UdpEndpointInfoGetters[] =
{
    { STRCAST("host"), reinterpret_cast<getter>(udpEndpointInfoGetter), 0,
        PyDoc_STR(STRCAST("host name or IP address")), reinterpret_cast<void*>(MEMBER_HOST) },
    { STRCAST("port"), reinterpret_cast<getter>(udpEndpointInfoGetter), 0,
        PyDoc_STR(STRCAST("UDP port number")), reinterpret_cast<void*>(MEMBER_PORT) },
    { STRCAST("protocolMajor"), reinterpret_cast<getter>(udpEndpointInfoGetter), 0,
        PyDoc_STR(STRCAST("protocol major version")), reinterpret_cast<void*>(MEMBER_PROTOCOL_MAJOR) },
    { STRCAST("protocolMinor"), reinterpret_cast<getter>(udpEndpointInfoGetter), 0,
        PyDoc_STR(STRCAST("protocol minor version")), reinterpret_cast<void*>(MEMBER_PROTOCOL_MINOR) },
    { STRCAST("encodingMajor"), reinterpret_cast<getter>(udpEndpointInfoGetter), 0,
        PyDoc_STR(STRCAST("encoding major version")), reinterpret_cast<void*>(MEMBER_ENCODING_MAJOR) },
    { STRCAST("encodingMinor"), reinterpret_cast<getter>(udpEndpointInfoGetter), 0,
        PyDoc_STR(STRCAST("encoding minor version")), reinterpret_cast<void*>(MEMBER_ENCODING_MINOR) },
    { STRCAST("mcastInterface"), reinterpret_cast<getter>(udpEndpointInfoGetter), 0,
        PyDoc_STR(STRCAST("multicast interface")), reinterpret_cast<void*>(MEMBER_MCAST_INTERFACE) },
    { STRCAST("mcastTtl"), reinterpret_cast<getter>(udpEndpointInfoGetter), 0,
        PyDoc_STR(STRCAST("multicast time-to-live")), reinterpret_cast<void*>(MEMBER_MCAST_TTL) },
    { 0, 0 } /* sentinel */
};

static PyGetSetDef OpaqueEndpointInfoGetters[] =
{
    { STRCAST("rawBytes"), reinterpret_cast<getter>(opaqueEndpointInfoGetter), 0,
        PyDoc_STR(STRCAST("raw encoding")), reinterpret_cast<void*>(MEMBER_RAW_BYTES) },
    { 0, 0 } /* sentinel */
};

namespace IcePy
{

PyTypeObject EndpointInfoType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyObject_HEAD_INIT(0)
    0,                               /* ob_size */
    STRCAST("IcePy.EndpointInfo"),   /* tp_name */
    sizeof(EndpointInfoObject),      /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    (destructor)endpointInfoDealloc, /* tp_dealloc */
    0,                               /* tp_print */
    0,                               /* tp_getattr */
    0,                               /* tp_setattr */
    0,                               /* tp_compare */
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
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    0,                               /* tp_doc */
    0,                               /* tp_traverse */
    0,                               /* tp_clear */
    0,                               /* tp_richcompare */
    0,                               /* tp_weaklistoffset */
    0,                               /* tp_iter */
    0,                               /* tp_iternext */
    EndpointInfoMethods,             /* tp_methods */
    0,                               /* tp_members */
    EndpointInfoGetters,             /* tp_getset */
    0,                               /* tp_base */
    0,                               /* tp_dict */
    0,                               /* tp_descr_get */
    0,                               /* tp_descr_set */
    0,                               /* tp_dictoffset */
    0,                               /* tp_init */
    0,                               /* tp_alloc */
    (newfunc)endpointInfoNew,        /* tp_new */
    0,                               /* tp_free */
    0,                               /* tp_is_gc */
};

PyTypeObject TcpEndpointInfoType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyObject_HEAD_INIT(0)
    0,                               /* ob_size */
    STRCAST("IcePy.TcpEndpointInfo"),/* tp_name */
    sizeof(EndpointInfoObject),      /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    (destructor)endpointInfoDealloc, /* tp_dealloc */
    0,                               /* tp_print */
    0,                               /* tp_getattr */
    0,                               /* tp_setattr */
    0,                               /* tp_compare */
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
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    0,                               /* tp_doc */
    0,                               /* tp_traverse */
    0,                               /* tp_clear */
    0,                               /* tp_richcompare */
    0,                               /* tp_weaklistoffset */
    0,                               /* tp_iter */
    0,                               /* tp_iternext */
    0,                               /* tp_methods */
    0,                               /* tp_members */
    TcpEndpointInfoGetters,          /* tp_getset */
    0,                               /* tp_base */
    0,                               /* tp_dict */
    0,                               /* tp_descr_get */
    0,                               /* tp_descr_set */
    0,                               /* tp_dictoffset */
    0,                               /* tp_init */
    0,                               /* tp_alloc */
    (newfunc)endpointInfoNew,        /* tp_new */
    0,                               /* tp_free */
    0,                               /* tp_is_gc */
};

PyTypeObject UdpEndpointInfoType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyObject_HEAD_INIT(0)
    0,                               /* ob_size */
    STRCAST("IcePy.UdpEndpointInfo"),/* tp_name */
    sizeof(EndpointInfoObject),      /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    (destructor)endpointInfoDealloc, /* tp_dealloc */
    0,                               /* tp_print */
    0,                               /* tp_getattr */
    0,                               /* tp_setattr */
    0,                               /* tp_compare */
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
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    0,                               /* tp_doc */
    0,                               /* tp_traverse */
    0,                               /* tp_clear */
    0,                               /* tp_richcompare */
    0,                               /* tp_weaklistoffset */
    0,                               /* tp_iter */
    0,                               /* tp_iternext */
    0,                               /* tp_methods */
    0,                               /* tp_members */
    UdpEndpointInfoGetters,          /* tp_getset */
    0,                               /* tp_base */
    0,                               /* tp_dict */
    0,                               /* tp_descr_get */
    0,                               /* tp_descr_set */
    0,                               /* tp_dictoffset */
    0,                               /* tp_init */
    0,                               /* tp_alloc */
    (newfunc)endpointInfoNew,        /* tp_new */
    0,                               /* tp_free */
    0,                               /* tp_is_gc */
};

PyTypeObject OpaqueEndpointInfoType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyObject_HEAD_INIT(0)
    0,                               /* ob_size */
    STRCAST("IcePy.OpaqueEndpointInfo"),/* tp_name */
    sizeof(EndpointInfoObject),      /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    (destructor)endpointInfoDealloc, /* tp_dealloc */
    0,                               /* tp_print */
    0,                               /* tp_getattr */
    0,                               /* tp_setattr */
    0,                               /* tp_compare */
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
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    0,                               /* tp_doc */
    0,                               /* tp_traverse */
    0,                               /* tp_clear */
    0,                               /* tp_richcompare */
    0,                               /* tp_weaklistoffset */
    0,                               /* tp_iter */
    0,                               /* tp_iternext */
    0,                               /* tp_methods */
    0,                               /* tp_members */
    OpaqueEndpointInfoGetters,       /* tp_getset */
    0,                               /* tp_base */
    0,                               /* tp_dict */
    0,                               /* tp_descr_get */
    0,                               /* tp_descr_set */
    0,                               /* tp_dictoffset */
    0,                               /* tp_init */
    0,                               /* tp_alloc */
    (newfunc)endpointInfoNew,        /* tp_new */
    0,                               /* tp_free */
    0,                               /* tp_is_gc */
};

}

bool
IcePy::initEndpointInfo(PyObject* module)
{
    if(PyType_Ready(&EndpointInfoType) < 0)
    {
        return false;
    }
    PyTypeObject* type = &EndpointInfoType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("EndpointInfo"), reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    TcpEndpointInfoType.tp_base = &EndpointInfoType; // Force inheritance from EndpointInfoType.
    if(PyType_Ready(&TcpEndpointInfoType) < 0)
    {
        return false;
    }
    type = &TcpEndpointInfoType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("TcpEndpointInfo"), reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    UdpEndpointInfoType.tp_base = &EndpointInfoType; // Force inheritance from EndpointType.
    if(PyType_Ready(&UdpEndpointInfoType) < 0)
    {
        return false;
    }
    type = &UdpEndpointInfoType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("UdpEndpointInfo"), reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    OpaqueEndpointInfoType.tp_base = &EndpointInfoType; // Force inheritance from EndpointType.
    if(PyType_Ready(&OpaqueEndpointInfoType) < 0)
    {
        return false;
    }
    type = &OpaqueEndpointInfoType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("OpaqueEndpointInfo"), reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    return true;
}

Ice::EndpointInfoPtr
IcePy::getEndpointInfo(PyObject* obj)
{
    assert(PyObject_IsInstance(obj, reinterpret_cast<PyObject*>(&EndpointInfoType)));
    EndpointInfoObject* eobj = reinterpret_cast<EndpointInfoObject*>(obj);
    return *eobj->endpointInfo;
}

PyObject*
IcePy::createEndpointInfo(const Ice::EndpointInfoPtr& endpointInfo)
{
    PyTypeObject* type;
    if(Ice::TcpEndpointInfoPtr::dynamicCast(endpointInfo))
    {
        type = &TcpEndpointInfoType;
    }
    else if(Ice::UdpEndpointInfoPtr::dynamicCast(endpointInfo))
    {
        type = &UdpEndpointInfoType;
    }
    else if(Ice::OpaqueEndpointInfoPtr::dynamicCast(endpointInfo))
    {
        type = &OpaqueEndpointInfoType;
    }
    else
    {
        type = &EndpointInfoType;
    }

    EndpointInfoObject* obj = PyObject_New(EndpointInfoObject, type);
    if(!obj)
    {
        return 0;
    }
    obj->endpointInfo = new Ice::EndpointInfoPtr(endpointInfo);

    return (PyObject*)obj;
}
