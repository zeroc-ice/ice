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
#include <structmember.h>

using namespace std;
using namespace IcePy;

namespace IcePy
{

struct ConnectionInfoObject
{
    PyObject_HEAD

    // Ice::ConnectionInfo
    PyObject* endpoint;

    // Ice::TcpConnectionInfo
    // Ice::UdpConnectionInfo
    PyObject* localAddress;
    int localPort;
    PyObject* remoteAddress;
    int remotePort;

    // Ice::UdpConnectionInfo
    PyObject* mcastAddress;
    int mcastPort;

    Ice::ConnectionInfoPtr* connectionInfo;
};

extern PyTypeObject TcpConnectionInfoType;
extern PyTypeObject UdpConnectionInfoType;

}

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
    if(Ice::TcpConnectionInfoPtr::dynamicCast(*self->connectionInfo))
    {
        Py_DECREF(self->localAddress);
        Py_DECREF(self->remoteAddress);
    }
    else if(Ice::UdpConnectionInfoPtr::dynamicCast(*self->connectionInfo))
    {
        Py_DECREF(self->localAddress);
        Py_DECREF(self->remoteAddress);
        Py_DECREF(self->mcastAddress);
    }

    delete self->connectionInfo;
    Py_DECREF(self->endpoint);
    PyObject_Del(self);
}

static PyMemberDef ConnectionInfoMembers[] =
{
    { STRCAST("endpoint"), T_OBJECT, offsetof(ConnectionInfoObject, endpoint), READONLY,
        PyDoc_STR(STRCAST("endpoint used to establish the connection")) },
    { 0, 0 } /* sentinel */
};

static PyMemberDef TcpConnectionInfoMembers[] =
{
    { STRCAST("localAddress"), T_OBJECT, offsetof(ConnectionInfoObject, localAddress), READONLY,
        PyDoc_STR(STRCAST("local address")) },
    { STRCAST("localPort"), T_INT, offsetof(ConnectionInfoObject, localPort), READONLY,
        PyDoc_STR(STRCAST("local port")) },
    { STRCAST("remoteAddress"), T_OBJECT, offsetof(ConnectionInfoObject, remoteAddress), READONLY,
        PyDoc_STR(STRCAST("remote address")) },
    { STRCAST("remotePort"), T_INT, offsetof(ConnectionInfoObject, remotePort), READONLY,
        PyDoc_STR(STRCAST("remote port")) },
    { 0, 0 } /* sentinel */
};

static PyMemberDef UdpConnectionInfoMembers[] =
{
    { STRCAST("localAddress"), T_OBJECT, offsetof(ConnectionInfoObject, localAddress), READONLY,
        PyDoc_STR(STRCAST("local address")) },
    { STRCAST("localPort"), T_INT, offsetof(ConnectionInfoObject, localPort), READONLY,
        PyDoc_STR(STRCAST("local port")) },
    { STRCAST("remoteAddress"), T_OBJECT, offsetof(ConnectionInfoObject, remoteAddress), READONLY,
        PyDoc_STR(STRCAST("remote address")) },
    { STRCAST("remotePort"), T_INT, offsetof(ConnectionInfoObject, remotePort), READONLY,
        PyDoc_STR(STRCAST("remote port")) },
    { STRCAST("mcastAddress"), T_OBJECT, offsetof(ConnectionInfoObject, mcastAddress), READONLY,
        PyDoc_STR(STRCAST("multicast address")) },
    { STRCAST("mcastPort"), T_INT, offsetof(ConnectionInfoObject, mcastPort), READONLY,
        PyDoc_STR(STRCAST("multicast port")) },
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
    ConnectionInfoMembers,           /* tp_members */
    0,                               /* tp_getset */
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
    TcpConnectionInfoMembers,        /* tp_members */
    0,                               /* tp_getset */
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
    UdpConnectionInfoMembers,        /* tp_members */
    0,                               /* tp_getset */
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
    ConnectionInfoObject* obj;
    if(Ice::TcpConnectionInfoPtr::dynamicCast(connectionInfo))
    {
        obj = PyObject_New(ConnectionInfoObject, &TcpConnectionInfoType);
        if(!obj)
        {
            return 0;
        }

        Ice::TcpConnectionInfoPtr tcpConnectionInfo = Ice::TcpConnectionInfoPtr::dynamicCast(connectionInfo);
        obj->localAddress = IcePy::createString(tcpConnectionInfo->localAddress);
        obj->localPort = static_cast<int>(tcpConnectionInfo->localPort);
        obj->remoteAddress = IcePy::createString(tcpConnectionInfo->remoteAddress);
        obj->remotePort = static_cast<int>(tcpConnectionInfo->remotePort);
    }
    else if(Ice::UdpConnectionInfoPtr::dynamicCast(connectionInfo))
    {
        obj = PyObject_New(ConnectionInfoObject, &UdpConnectionInfoType);
        if(!obj)
        {
            return 0;
        }

        Ice::UdpConnectionInfoPtr udpConnectionInfo = Ice::UdpConnectionInfoPtr::dynamicCast(connectionInfo);
        obj->localAddress = IcePy::createString(udpConnectionInfo->localAddress);
        obj->localPort = static_cast<int>(udpConnectionInfo->localPort);
        obj->remoteAddress = IcePy::createString(udpConnectionInfo->remoteAddress);
        obj->remotePort = static_cast<int>(udpConnectionInfo->remotePort);
        obj->mcastAddress = IcePy::createString(udpConnectionInfo->mcastAddress);
        obj->mcastPort = static_cast<int>(udpConnectionInfo->mcastPort);
    }
    else
    {
        obj = PyObject_New(ConnectionInfoObject, &ConnectionInfoType);
        if(!obj)
        {
            return 0;
        }
    }

    obj->connectionInfo = new Ice::ConnectionInfoPtr(connectionInfo);
    Ice::EndpointInfoPtr info = connectionInfo->endpoint;
    obj->endpoint = IcePy::createEndpointInfo(connectionInfo->endpoint);

    return (PyObject*)obj;
}
