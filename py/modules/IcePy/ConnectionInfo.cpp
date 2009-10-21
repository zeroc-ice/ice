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
#include <ConnectionInfo.h>
#include <EndpointInfo.h>
#include <Util.h>

using namespace std;
using namespace IcePy;

namespace IcePy
{

struct ConnectionInfoObject
{
    PyObject_HEAD
    Ice::ConnectionInfoPtr* connectionInfo;
};

}

//
// ConnectionInfo members
//
#define MEMBER_ENDPOINT         0

//
// Shared TCP/UDP members
//
#define MEMBER_LOCAL_ADDRESS    1
#define MEMBER_LOCAL_PORT       2
#define MEMBER_REMOTE_ADDRESS   3
#define MEMBER_REMOTE_PORT      4

//
// UdpConnectionInfo members
//
#define MEMBER_MCAST_ADDRESS    5
#define MEMBER_MCAST_PORT       6

#ifdef WIN32
extern "C"
#endif
static ConnectionInfoObject*
connectionInfoNew(PyObject* /*arg*/)
{
    PyErr_Format(PyExc_RuntimeError, STRCAST("An connection info cannot be created directly"));
    return 0;
}

#ifdef WIN32
extern "C"
#endif
static void
connectionInfoDealloc(ConnectionInfoObject* self)
{
    delete self->connectionInfo;
    PyObject_Del(self);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
connectionInfoGetter(ConnectionInfoObject* self, void* closure)
{
    int member = reinterpret_cast<int>(closure);
    PyObject* result = 0;

    switch(member)
    {
    case MEMBER_ENDPOINT:
        result = createEndpointInfo((*self->connectionInfo)->endpoint);
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
tcpConnectionInfoGetter(ConnectionInfoObject* self, void* closure)
{
    int member = reinterpret_cast<int>(closure);
    PyObject* result = 0;
    Ice::TcpConnectionInfoPtr info = Ice::TcpConnectionInfoPtr::dynamicCast(*self->connectionInfo);
    assert(info);

    switch(member)
    {
    case MEMBER_LOCAL_ADDRESS:
        result = createString(info->localAddress);
        break;
    case MEMBER_LOCAL_PORT:
        result = PyInt_FromLong(info->localPort);
        break;
    case MEMBER_REMOTE_ADDRESS:
        result = createString(info->remoteAddress);
        break;
    case MEMBER_REMOTE_PORT:
        result = PyInt_FromLong(info->remotePort);
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
udpConnectionInfoGetter(ConnectionInfoObject* self, void* closure)
{
    int member = reinterpret_cast<int>(closure);
    PyObject* result = 0;
    Ice::UdpConnectionInfoPtr info = Ice::UdpConnectionInfoPtr::dynamicCast(*self->connectionInfo);
    assert(info);

    switch(member)
    {
    case MEMBER_LOCAL_ADDRESS:
        result = createString(info->localAddress);
        break;
    case MEMBER_LOCAL_PORT:
        result = PyInt_FromLong(info->localPort);
        break;
    case MEMBER_REMOTE_ADDRESS:
        result = createString(info->remoteAddress);
        break;
    case MEMBER_REMOTE_PORT:
        result = PyInt_FromLong(info->remotePort);
        break;
    case MEMBER_MCAST_ADDRESS:
        result = createString(info->mcastAddress);
        break;
    case MEMBER_MCAST_PORT:
        result = PyInt_FromLong(info->mcastPort);
        break;
    default:
        assert(false);
    }

    return result;
}

static PyGetSetDef ConnectionInfoGetters[] =
{
    { STRCAST("endpoint"), reinterpret_cast<getter>(connectionInfoGetter), 0,
        PyDoc_STR(STRCAST("endpoint used to establish the connection")), reinterpret_cast<void*>(MEMBER_ENDPOINT) },
    { 0, 0 } /* sentinel */
};

static PyGetSetDef TcpConnectionInfoGetters[] =
{
    { STRCAST("localAddress"), reinterpret_cast<getter>(tcpConnectionInfoGetter), 0,
        PyDoc_STR(STRCAST("local address")), reinterpret_cast<void*>(MEMBER_LOCAL_ADDRESS) },
    { STRCAST("localPort"), reinterpret_cast<getter>(tcpConnectionInfoGetter), 0,
        PyDoc_STR(STRCAST("local port")), reinterpret_cast<void*>(MEMBER_LOCAL_PORT) },
    { STRCAST("remoteAddress"), reinterpret_cast<getter>(tcpConnectionInfoGetter), 0,
        PyDoc_STR(STRCAST("remote address")), reinterpret_cast<void*>(MEMBER_REMOTE_ADDRESS) },
    { STRCAST("remotePort"), reinterpret_cast<getter>(tcpConnectionInfoGetter), 0,
        PyDoc_STR(STRCAST("remote port")), reinterpret_cast<void*>(MEMBER_REMOTE_PORT) },
    { 0, 0 } /* sentinel */
};

static PyGetSetDef UdpConnectionInfoGetters[] =
{
    { STRCAST("localAddress"), reinterpret_cast<getter>(udpConnectionInfoGetter), 0,
        PyDoc_STR(STRCAST("local address")), reinterpret_cast<void*>(MEMBER_LOCAL_ADDRESS) },
    { STRCAST("localPort"), reinterpret_cast<getter>(udpConnectionInfoGetter), 0,
        PyDoc_STR(STRCAST("local port")), reinterpret_cast<void*>(MEMBER_LOCAL_PORT) },
    { STRCAST("remoteAddress"), reinterpret_cast<getter>(udpConnectionInfoGetter), 0,
        PyDoc_STR(STRCAST("remote address")), reinterpret_cast<void*>(MEMBER_REMOTE_ADDRESS) },
    { STRCAST("remotePort"), reinterpret_cast<getter>(udpConnectionInfoGetter), 0,
        PyDoc_STR(STRCAST("remote port")), reinterpret_cast<void*>(MEMBER_REMOTE_PORT) },
    { STRCAST("mcastAddress"), reinterpret_cast<getter>(udpConnectionInfoGetter), 0,
        PyDoc_STR(STRCAST("multicast address")), reinterpret_cast<void*>(MEMBER_MCAST_ADDRESS) },
    { STRCAST("mcastPort"), reinterpret_cast<getter>(udpConnectionInfoGetter), 0,
        PyDoc_STR(STRCAST("multicast port")), reinterpret_cast<void*>(MEMBER_MCAST_PORT) },
    { 0, 0 } /* sentinel */
};

namespace IcePy
{

PyTypeObject ConnectionInfoType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyObject_HEAD_INIT(0)
    0,                               /* ob_size */
    STRCAST("IcePy.ConnectionInfo"), /* tp_name */
    sizeof(ConnectionInfoObject),    /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    (destructor)connectionInfoDealloc, /* tp_dealloc */
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
    ConnectionInfoGetters,           /* tp_getset */
    0,                               /* tp_base */
    0,                               /* tp_dict */
    0,                               /* tp_descr_get */
    0,                               /* tp_descr_set */
    0,                               /* tp_dictoffset */
    0,                               /* tp_init */
    0,                               /* tp_alloc */
    (newfunc)connectionInfoNew,      /* tp_new */
    0,                               /* tp_free */
    0,                               /* tp_is_gc */
};

PyTypeObject TcpConnectionInfoType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyObject_HEAD_INIT(0)
    0,                               /* ob_size */
    STRCAST("IcePy.TcpConnectionInfo"),/* tp_name */
    sizeof(ConnectionInfoObject),    /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    (destructor)connectionInfoDealloc, /* tp_dealloc */
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
    TcpConnectionInfoGetters,        /* tp_getset */
    0,                               /* tp_base */
    0,                               /* tp_dict */
    0,                               /* tp_descr_get */
    0,                               /* tp_descr_set */
    0,                               /* tp_dictoffset */
    0,                               /* tp_init */
    0,                               /* tp_alloc */
    (newfunc)connectionInfoNew,      /* tp_new */
    0,                               /* tp_free */
    0,                               /* tp_is_gc */
};

PyTypeObject UdpConnectionInfoType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyObject_HEAD_INIT(0)
    0,                               /* ob_size */
    STRCAST("IcePy.UdpConnectionInfo"),/* tp_name */
    sizeof(ConnectionInfoObject),    /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    (destructor)connectionInfoDealloc, /* tp_dealloc */
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
    UdpConnectionInfoGetters,        /* tp_getset */
    0,                               /* tp_base */
    0,                               /* tp_dict */
    0,                               /* tp_descr_get */
    0,                               /* tp_descr_set */
    0,                               /* tp_dictoffset */
    0,                               /* tp_init */
    0,                               /* tp_alloc */
    (newfunc)connectionInfoNew,      /* tp_new */
    0,                               /* tp_free */
    0,                               /* tp_is_gc */
};

}

bool
IcePy::initConnectionInfo(PyObject* module)
{
    if(PyType_Ready(&ConnectionInfoType) < 0)
    {
        return false;
    }
    PyTypeObject* type = &ConnectionInfoType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("ConnectionInfo"), reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    TcpConnectionInfoType.tp_base = &ConnectionInfoType; // Force inheritance from ConnectionInfoType.
    if(PyType_Ready(&TcpConnectionInfoType) < 0)
    {
        return false;
    }
    type = &TcpConnectionInfoType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("TcpConnectionInfo"), reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    UdpConnectionInfoType.tp_base = &ConnectionInfoType; // Force inheritance from ConnectionType.
    if(PyType_Ready(&UdpConnectionInfoType) < 0)
    {
        return false;
    }
    type = &UdpConnectionInfoType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("UdpConnectionInfo"), reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    return true;
}

Ice::ConnectionInfoPtr
IcePy::getConnectionInfo(PyObject* obj)
{
    assert(PyObject_IsInstance(obj, reinterpret_cast<PyObject*>(&ConnectionInfoType)));
    ConnectionInfoObject* eobj = reinterpret_cast<ConnectionInfoObject*>(obj);
    return *eobj->connectionInfo;
}

PyObject*
IcePy::createConnectionInfo(const Ice::ConnectionInfoPtr& connectionInfo)
{
    PyTypeObject* type;
    if(Ice::TcpConnectionInfoPtr::dynamicCast(connectionInfo))
    {
        type = &TcpConnectionInfoType;
    }
    else if(Ice::UdpConnectionInfoPtr::dynamicCast(connectionInfo))
    {
        type = &UdpConnectionInfoType;
    }
    else
    {
        type = &ConnectionInfoType;
    }

    ConnectionInfoObject* obj = PyObject_New(ConnectionInfoObject, type);
    if(!obj)
    {
        return 0;
    }
    obj->connectionInfo = new Ice::ConnectionInfoPtr(connectionInfo);

    return (PyObject*)obj;
}
