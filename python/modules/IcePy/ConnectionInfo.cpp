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
#include <ConnectionInfo.h>
#include <EndpointInfo.h>
#include <Util.h>
#include <Ice/Object.h>
#include <IceSSL/ConnectionInfo.h>

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

#ifdef WIN32
extern "C"
#endif
static ConnectionInfoObject*
connectionInfoNew(PyTypeObject* /*type*/, PyObject* /*args*/, PyObject* /*kwds*/)
{
    PyErr_Format(PyExc_RuntimeError, STRCAST("A connection info cannot be created directly"));
    return 0;
}

#ifdef WIN32
extern "C"
#endif
static void
connectionInfoDealloc(ConnectionInfoObject* self)
{
    delete self->connectionInfo;
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
connectionInfoGetIncoming(ConnectionInfoObject* self)
{
    PyObject* result = (*self->connectionInfo)->incoming ? getTrue() : getFalse();
    Py_INCREF(result);
    return result;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
connectionInfoGetAdapterName(ConnectionInfoObject* self)
{
    return createString((*self->connectionInfo)->adapterName);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
connectionInfoGetRcvSize(ConnectionInfoObject* self)
{
    return PyLong_FromLong((*self->connectionInfo)->rcvSize);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
connectionInfoGetSndSize(ConnectionInfoObject* self)
{
    return PyLong_FromLong((*self->connectionInfo)->sndSize);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
ipConnectionInfoGetLocalAddress(ConnectionInfoObject* self)
{
    Ice::IPConnectionInfoPtr info = Ice::IPConnectionInfoPtr::dynamicCast(*self->connectionInfo);
    assert(info);
    return createString(info->localAddress);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
ipConnectionInfoGetLocalPort(ConnectionInfoObject* self)
{
    Ice::IPConnectionInfoPtr info = Ice::IPConnectionInfoPtr::dynamicCast(*self->connectionInfo);
    assert(info);
    return PyLong_FromLong(info->localPort);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
ipConnectionInfoGetRemoteAddress(ConnectionInfoObject* self)
{
    Ice::IPConnectionInfoPtr info = Ice::IPConnectionInfoPtr::dynamicCast(*self->connectionInfo);
    assert(info);
    return createString(info->remoteAddress);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
ipConnectionInfoGetRemotePort(ConnectionInfoObject* self)
{
    Ice::IPConnectionInfoPtr info = Ice::IPConnectionInfoPtr::dynamicCast(*self->connectionInfo);
    assert(info);
    return PyLong_FromLong(info->remotePort);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
udpConnectionInfoGetMcastAddress(ConnectionInfoObject* self)
{
    Ice::UDPConnectionInfoPtr info = Ice::UDPConnectionInfoPtr::dynamicCast(*self->connectionInfo);
    assert(info);
    return createString(info->mcastAddress);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
udpConnectionInfoGetMcastPort(ConnectionInfoObject* self, void* member)
{
    Ice::UDPConnectionInfoPtr info = Ice::UDPConnectionInfoPtr::dynamicCast(*self->connectionInfo);
    assert(info);
    return PyLong_FromLong(info->mcastPort);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
wsConnectionInfoGetHeaders(ConnectionInfoObject* self)
{
    Ice::WSConnectionInfoPtr info = Ice::WSConnectionInfoPtr::dynamicCast(*self->connectionInfo);
    assert(info);

    PyObjectHandle result = PyDict_New();
    if(result.get())
    {
        for(Ice::HeaderDict::iterator p = info->headers.begin(); p != info->headers.end(); ++p)
        {
            PyObjectHandle key = createString(p->first);
            PyObjectHandle val = createString(p->second);
            if(!val.get() || PyDict_SetItem(result.get(), key.get(), val.get()) < 0)
            {
                return 0;
            }
        }
    }

    return result.release();
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
sslConnectionInfoGetCipher(ConnectionInfoObject* self)
{
    IceSSL::ConnectionInfoPtr info = IceSSL::ConnectionInfoPtr::dynamicCast(*self->connectionInfo);
    assert(info);
    return createString(info->cipher);
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
sslConnectionInfoGetCerts(ConnectionInfoObject* self)
{
    IceSSL::ConnectionInfoPtr info = IceSSL::ConnectionInfoPtr::dynamicCast(*self->connectionInfo);
    assert(info);
    PyObject* certs = PyList_New(0);
    stringSeqToList(info->certs, certs);
    return certs;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
sslConnectionInfoGetVerified(ConnectionInfoObject* self)
{
    IceSSL::ConnectionInfoPtr info = IceSSL::ConnectionInfoPtr::dynamicCast(*self->connectionInfo);
    assert(info);
    PyObject* result = info->incoming ? getTrue() : getFalse();
    Py_INCREF(result);
    return result;
}

#ifdef WIN32
extern "C"
#endif
static PyObject*
wssConnectionInfoGetHeaders(ConnectionInfoObject* self)
{
    IceSSL::WSSConnectionInfoPtr info = IceSSL::WSSConnectionInfoPtr::dynamicCast(*self->connectionInfo);
    assert(info);

    PyObjectHandle result = PyDict_New();
    if(result.get())
    {
        for(Ice::HeaderDict::iterator p = info->headers.begin(); p != info->headers.end(); ++p)
        {
            PyObjectHandle key = createString(p->first);
            PyObjectHandle val = createString(p->second);
            if(!val.get() || PyDict_SetItem(result.get(), key.get(), val.get()) < 0)
            {
                return 0;
            }
        }
    }

    return result.release();
}

static PyGetSetDef ConnectionInfoGetters[] =
{
    { STRCAST("incoming"), reinterpret_cast<getter>(connectionInfoGetIncoming), 0,
        PyDoc_STR(STRCAST("whether connection is incoming")), 0 },
    { STRCAST("adapterName"), reinterpret_cast<getter>(connectionInfoGetAdapterName), 0,
        PyDoc_STR(STRCAST("adapter associated the connection")), 0 },
    { STRCAST("rcvSize"), reinterpret_cast<getter>(connectionInfoGetRcvSize), 0,
        PyDoc_STR(STRCAST("receive buffer size")), 0 },
    { STRCAST("sndSize"), reinterpret_cast<getter>(connectionInfoGetSndSize), 0,
        PyDoc_STR(STRCAST("send buffer size")), 0 },
    { 0, 0 } /* sentinel */
};

static PyGetSetDef IPConnectionInfoGetters[] =
{
    { STRCAST("localAddress"), reinterpret_cast<getter>(ipConnectionInfoGetLocalAddress), 0,
        PyDoc_STR(STRCAST("local address")), 0 },
    { STRCAST("localPort"), reinterpret_cast<getter>(ipConnectionInfoGetLocalPort), 0,
        PyDoc_STR(STRCAST("local port")), 0 },
    { STRCAST("remoteAddress"), reinterpret_cast<getter>(ipConnectionInfoGetRemoteAddress), 0,
        PyDoc_STR(STRCAST("remote address")), 0 },
    { STRCAST("remotePort"), reinterpret_cast<getter>(ipConnectionInfoGetRemotePort), 0,
        PyDoc_STR(STRCAST("remote port")), 0 },
    { 0, 0 } /* sentinel */
};

static PyGetSetDef UDPConnectionInfoGetters[] =
{
    { STRCAST("mcastAddress"), reinterpret_cast<getter>(udpConnectionInfoGetMcastAddress), 0,
        PyDoc_STR(STRCAST("multicast address")), 0 },
    { STRCAST("mcastPort"), reinterpret_cast<getter>(udpConnectionInfoGetMcastPort), 0,
        PyDoc_STR(STRCAST("multicast port")), 0 },
    { 0, 0 } /* sentinel */
};

static PyGetSetDef WSConnectionInfoGetters[] =
{
    { STRCAST("headers"), reinterpret_cast<getter>(wsConnectionInfoGetHeaders), 0,
        PyDoc_STR(STRCAST("request headers")), 0 },
    { 0, 0 } /* sentinel */
};

static PyGetSetDef SSLConnectionInfoGetters[] =
{
    { STRCAST("cipher"), reinterpret_cast<getter>(sslConnectionInfoGetCipher), 0,
        PyDoc_STR(STRCAST("negotiated cipher suite")), 0 },
    { STRCAST("certs"), reinterpret_cast<getter>(sslConnectionInfoGetCerts), 0,
        PyDoc_STR(STRCAST("certificate chain")), 0 },
    { STRCAST("verified"), reinterpret_cast<getter>(sslConnectionInfoGetVerified), 0,
        PyDoc_STR(STRCAST("certificate chain verification status")), 0 },
    { 0, 0 } /* sentinel */
};

static PyGetSetDef WSSConnectionInfoGetters[] =
{
    { STRCAST("headers"), reinterpret_cast<getter>(wssConnectionInfoGetHeaders), 0,
        PyDoc_STR(STRCAST("request headers")), 0 },
    { 0, 0 } /* sentinel */
};

namespace IcePy
{

PyTypeObject ConnectionInfoType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyVarObject_HEAD_INIT(0, 0)
    STRCAST("IcePy.ConnectionInfo"), /* tp_name */
    sizeof(ConnectionInfoObject),    /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    (destructor)connectionInfoDealloc, /* tp_dealloc */
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

PyTypeObject IPConnectionInfoType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyVarObject_HEAD_INIT(0, 0)
    STRCAST("IcePy.IPConnectionInfo"), /* tp_name */
    sizeof(ConnectionInfoObject),    /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    (destructor)connectionInfoDealloc, /* tp_dealloc */
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
    IPConnectionInfoGetters,         /* tp_getset */
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

PyTypeObject TCPConnectionInfoType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyVarObject_HEAD_INIT(0, 0)
    STRCAST("IcePy.TCPConnectionInfo"),/* tp_name */
    sizeof(ConnectionInfoObject),    /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    (destructor)connectionInfoDealloc, /* tp_dealloc */
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

PyTypeObject UDPConnectionInfoType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyVarObject_HEAD_INIT(0, 0)
    STRCAST("IcePy.UDPConnectionInfo"),/* tp_name */
    sizeof(ConnectionInfoObject),    /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    (destructor)connectionInfoDealloc, /* tp_dealloc */
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
    UDPConnectionInfoGetters,        /* tp_getset */
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

PyTypeObject WSConnectionInfoType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyVarObject_HEAD_INIT(0, 0)
    STRCAST("IcePy.WSConnectionInfo"),/* tp_name */
    sizeof(ConnectionInfoObject),    /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    (destructor)connectionInfoDealloc, /* tp_dealloc */
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
    WSConnectionInfoGetters,         /* tp_getset */
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

PyTypeObject SSLConnectionInfoType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyVarObject_HEAD_INIT(0, 0)
    STRCAST("IcePy.SSLConnectionInfo"),/* tp_name */
    sizeof(ConnectionInfoObject),    /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    (destructor)connectionInfoDealloc, /* tp_dealloc */
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
    SSLConnectionInfoGetters,        /* tp_getset */
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

PyTypeObject WSSConnectionInfoType =
{
    /* The ob_type field must be initialized in the module init function
     * to be portable to Windows without using C++. */
    PyVarObject_HEAD_INIT(0, 0)
    STRCAST("IcePy.WSSConnectionInfo"),/* tp_name */
    sizeof(ConnectionInfoObject),    /* tp_basicsize */
    0,                               /* tp_itemsize */
    /* methods */
    (destructor)connectionInfoDealloc, /* tp_dealloc */
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
    WSSConnectionInfoGetters,        /* tp_getset */
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

    IPConnectionInfoType.tp_base = &ConnectionInfoType; // Force inheritance from ConnectionInfoType.
    if(PyType_Ready(&IPConnectionInfoType) < 0)
    {
        return false;
    }
    type = &IPConnectionInfoType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("IPConnectionInfo"), reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    TCPConnectionInfoType.tp_base = &IPConnectionInfoType; // Force inheritance from IPConnectionInfoType.
    if(PyType_Ready(&TCPConnectionInfoType) < 0)
    {
        return false;
    }
    type = &TCPConnectionInfoType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("TCPConnectionInfo"), reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    UDPConnectionInfoType.tp_base = &IPConnectionInfoType; // Force inheritance from IPConnectionType.
    if(PyType_Ready(&UDPConnectionInfoType) < 0)
    {
        return false;
    }
    type = &UDPConnectionInfoType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("UDPConnectionInfo"), reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    WSConnectionInfoType.tp_base = &IPConnectionInfoType; // Force inheritance from IPConnectionType.
    if(PyType_Ready(&WSConnectionInfoType) < 0)
    {
        return false;
    }
    type = &WSConnectionInfoType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("WSConnectionInfo"), reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    SSLConnectionInfoType.tp_base = &IPConnectionInfoType; // Force inheritance from IPConnectionInfoType.
    if(PyType_Ready(&SSLConnectionInfoType) < 0)
    {
        return false;
    }
    type = &SSLConnectionInfoType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("SSLConnectionInfo"), reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    WSSConnectionInfoType.tp_base = &SSLConnectionInfoType; // Force inheritance from IPConnectionType.
    if(PyType_Ready(&WSSConnectionInfoType) < 0)
    {
        return false;
    }
    type = &WSSConnectionInfoType; // Necessary to prevent GCC's strict-alias warnings.
    if(PyModule_AddObject(module, STRCAST("WSSConnectionInfo"), reinterpret_cast<PyObject*>(type)) < 0)
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
    if(Ice::WSConnectionInfoPtr::dynamicCast(connectionInfo))
    {
        type = &WSConnectionInfoType;
    }
    else if(Ice::TCPConnectionInfoPtr::dynamicCast(connectionInfo))
    {
        type = &TCPConnectionInfoType;
    }
    else if(Ice::UDPConnectionInfoPtr::dynamicCast(connectionInfo))
    {
        type = &UDPConnectionInfoType;
    }
    else if(IceSSL::WSSConnectionInfoPtr::dynamicCast(connectionInfo))
    {
        type = &WSSConnectionInfoType;
    }
    else if(IceSSL::ConnectionInfoPtr::dynamicCast(connectionInfo))
    {
        type = &SSLConnectionInfoType;
    }
    else if(Ice::IPConnectionInfoPtr::dynamicCast(connectionInfo))
    {
        type = &IPConnectionInfoType;
    }
    else
    {
        type = &ConnectionInfoType;
    }

    ConnectionInfoObject* obj = reinterpret_cast<ConnectionInfoObject*>(type->tp_alloc(type, 0));
    if(!obj)
    {
        return 0;
    }
    obj->connectionInfo = new Ice::ConnectionInfoPtr(connectionInfo);

    return (PyObject*)obj;
}
