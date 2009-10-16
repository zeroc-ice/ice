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
#include <structmember.h>

using namespace std;
using namespace IcePy;

namespace IcePy
{

struct EndpointInfoObject
{
    PyObject_HEAD

    // Ice::EndpointInfo
    int timeout;
    PyObject* compress;

    // Ice::TcpEndpointInfo
    // Ice::UdpEndpointInfo
    PyObject* host;
    int port;

    // Ice::UdpEndpointInfo
    unsigned char protocolMajor;
    unsigned char protocolMinor;
    unsigned char encodingMajor;
    unsigned char encodingMinor;
    PyObject* mcastInterface;
    int mcastTtl;

    // Ice::OpaqueEndpointInfo
    PyObject* rawBytes;

    Ice::EndpointInfoPtr* endpointInfo;
};

extern PyTypeObject TcpEndpointInfoType;
extern PyTypeObject UdpEndpointInfoType;
extern PyTypeObject OpaqueEndpointInfoType;

}

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
    if(Ice::TcpEndpointInfoPtr::dynamicCast(*self->endpointInfo))
    {
        Py_DECREF(self->host);
    }
    else if(Ice::UdpEndpointInfoPtr::dynamicCast(*self->endpointInfo))
    {
        Py_DECREF(self->host);
        Py_DECREF(self->mcastInterface);
    }
    else if(Ice::OpaqueEndpointInfoPtr::dynamicCast(*self->endpointInfo))
    {
        Py_DECREF(self->rawBytes);
    }

    delete self->endpointInfo;
    Py_DECREF(self->compress);
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
// Ice::EndpointInfo::datagram
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

static PyMemberDef EndpointInfoMembers[] =
{
    { STRCAST("timeout"), T_INT, offsetof(EndpointInfoObject, timeout), READONLY,
        PyDoc_STR(STRCAST("timeout in milliseconds")) },
    { STRCAST("compress"), T_OBJECT, offsetof(EndpointInfoObject, compress), READONLY,
        PyDoc_STR(STRCAST("compression status")) },
    { 0, 0 } /* sentinel */
};

static PyMemberDef TcpEndpointInfoMembers[] =
{
    { STRCAST("host"), T_OBJECT, offsetof(EndpointInfoObject, host), READONLY,
        PyDoc_STR(STRCAST("host name or ip address")) },
    { STRCAST("port"), T_INT, offsetof(EndpointInfoObject, port), READONLY,
        PyDoc_STR(STRCAST("tcp port number")) },
    { 0, 0 } /* sentinel */
};

static PyMemberDef UdpEndpointInfoMembers[] =
{
    { STRCAST("host"), T_OBJECT, offsetof(EndpointInfoObject, host), READONLY,
        PyDoc_STR(STRCAST("host name or ip address")) },
    { STRCAST("port"), T_INT, offsetof(EndpointInfoObject, port), READONLY,
        PyDoc_STR(STRCAST("udp port number")) },
    { STRCAST("protocolMajor"), T_BYTE, offsetof(EndpointInfoObject, protocolMajor), READONLY,
        PyDoc_STR(STRCAST("protocol major version")) },
    { STRCAST("protocolMinor"), T_BYTE, offsetof(EndpointInfoObject, protocolMinor), READONLY,
        PyDoc_STR(STRCAST("protocol minor version")) },
    { STRCAST("encodingMajor"), T_BYTE, offsetof(EndpointInfoObject, encodingMajor), READONLY,
        PyDoc_STR(STRCAST("encoding major version")) },
    { STRCAST("encodingMinor"), T_BYTE, offsetof(EndpointInfoObject, encodingMinor), READONLY,
        PyDoc_STR(STRCAST("encoding minor version")) },
    { STRCAST("mcastInterface"), T_OBJECT, offsetof(EndpointInfoObject, mcastInterface), READONLY,
        PyDoc_STR(STRCAST("multicast interface")) },
    { STRCAST("mcastTtl"), T_INT, offsetof(EndpointInfoObject, mcastTtl), READONLY,
        PyDoc_STR(STRCAST("mulitcast time-to-live")) },
    { 0, 0 } /* sentinel */
};

static PyMemberDef OpaqueEndpointInfoMembers[] =
{
    { STRCAST("rawBytes"), T_OBJECT, offsetof(EndpointInfoObject, rawBytes), READONLY,
        PyDoc_STR(STRCAST("raw encoding")) },
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
    EndpointInfoMembers,             /* tp_members */
    0,                               /* tp_getset */
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
    TcpEndpointInfoMembers,          /* tp_members */
    0,                               /* tp_getset */
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
    UdpEndpointInfoMembers,          /* tp_members */
    0,                               /* tp_getset */
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
    OpaqueEndpointInfoMembers,       /* tp_members */
    0,                               /* tp_getset */
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
    EndpointInfoObject* obj;
    if(Ice::TcpEndpointInfoPtr::dynamicCast(endpointInfo))
    {
        obj = PyObject_New(EndpointInfoObject, &TcpEndpointInfoType);
        if(!obj)
        {
            return 0;
        }

        Ice::TcpEndpointInfoPtr tcpEndpointInfo = Ice::TcpEndpointInfoPtr::dynamicCast(endpointInfo);
        obj->host = IcePy::createString(tcpEndpointInfo->host);
        obj->port = static_cast<int>(tcpEndpointInfo->port);
    }
    else if(Ice::UdpEndpointInfoPtr::dynamicCast(endpointInfo))
    {
        obj = PyObject_New(EndpointInfoObject, &UdpEndpointInfoType);
        if(!obj)
        {
            return 0;
        }

        Ice::UdpEndpointInfoPtr udpEndpointInfo = Ice::UdpEndpointInfoPtr::dynamicCast(endpointInfo);
        obj->host = IcePy::createString(udpEndpointInfo->host);
        obj->port = static_cast<int>(udpEndpointInfo->port);
        obj->protocolMajor = static_cast<unsigned char>(udpEndpointInfo->protocolMajor);
        obj->protocolMinor = static_cast<unsigned char>(udpEndpointInfo->protocolMinor);
        obj->encodingMajor = static_cast<unsigned char>(udpEndpointInfo->encodingMajor);
        obj->encodingMinor = static_cast<unsigned char>(udpEndpointInfo->encodingMinor);
        obj->mcastInterface = IcePy::createString(udpEndpointInfo->mcastInterface);
        obj->mcastTtl = static_cast<int>(udpEndpointInfo->mcastTtl);
    }
    else if(Ice::OpaqueEndpointInfoPtr::dynamicCast(endpointInfo))
    {
        obj = PyObject_New(EndpointInfoObject, &OpaqueEndpointInfoType);
        if(!obj)
        {
            return 0;
        }

        Ice::OpaqueEndpointInfoPtr opaqueEndpointInfo = Ice::OpaqueEndpointInfoPtr::dynamicCast(endpointInfo);
        Ice::ByteSeq& b = opaqueEndpointInfo->rawBytes;
        obj->rawBytes = PyString_FromStringAndSize(reinterpret_cast<const char*>(&b[0]), static_cast<int>(b.size()));
    }
    else
    {
        obj = PyObject_New(EndpointInfoObject, &EndpointInfoType);
        if(!obj)
        {
            return 0;
        }
    }

    obj->endpointInfo = new Ice::EndpointInfoPtr(endpointInfo);
    obj->timeout = static_cast<int>(endpointInfo->timeout);
    obj->compress = endpointInfo->compress ? getTrue() : getFalse();

    return (PyObject*)obj;
}
