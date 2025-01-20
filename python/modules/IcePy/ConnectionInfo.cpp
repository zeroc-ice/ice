// Copyright (c) ZeroC, Inc.

#include "ConnectionInfo.h"
#include "../../cpp/src/Ice/SSL/SSLUtil.h"
#include "EndpointInfo.h"
#include "Ice/Ice.h"
#include "Util.h"

using namespace std;
using namespace IcePy;

namespace IcePy
{
    struct ConnectionInfoObject
    {
        PyObject_HEAD Ice::ConnectionInfoPtr* connectionInfo;
    };
}

extern "C" ConnectionInfoObject*
connectionInfoNew(PyTypeObject* /*type*/, PyObject* /*args*/, PyObject* /*kwds*/)
{
    PyErr_Format(PyExc_RuntimeError, "A connection info cannot be created directly");
    return nullptr;
}

extern "C" void
connectionInfoDealloc(ConnectionInfoObject* self)
{
    delete self->connectionInfo;
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

extern "C" PyObject*
connectionInfoGetUnderlying(ConnectionInfoObject* self, PyObject* /*args*/)
{
    return createConnectionInfo((*self->connectionInfo)->underlying);
}

extern "C" PyObject*
connectionInfoGetIncoming(ConnectionInfoObject* self, PyObject* /*args*/)
{
    return (*self->connectionInfo)->incoming ? Py_True : Py_False;
}

extern "C" PyObject*
connectionInfoGetAdapterName(ConnectionInfoObject* self, PyObject* /*args*/)
{
    return createString((*self->connectionInfo)->adapterName);
}

extern "C" PyObject*
ipConnectionInfoGetLocalAddress(ConnectionInfoObject* self, PyObject* /*args*/)
{
    auto info = dynamic_pointer_cast<Ice::IPConnectionInfo>(*self->connectionInfo);
    assert(info);
    return createString(info->localAddress);
}

extern "C" PyObject*
ipConnectionInfoGetLocalPort(ConnectionInfoObject* self, PyObject* /*args*/)
{
    auto info = dynamic_pointer_cast<Ice::IPConnectionInfo>(*self->connectionInfo);
    assert(info);
    return PyLong_FromLong(info->localPort);
}

extern "C" PyObject*
ipConnectionInfoGetRemoteAddress(ConnectionInfoObject* self, PyObject* /*args*/)
{
    auto info = dynamic_pointer_cast<Ice::IPConnectionInfo>(*self->connectionInfo);
    assert(info);
    return createString(info->remoteAddress);
}

extern "C" PyObject*
ipConnectionInfoGetRemotePort(ConnectionInfoObject* self, PyObject* /*args*/)
{
    auto info = dynamic_pointer_cast<Ice::IPConnectionInfo>(*self->connectionInfo);
    assert(info);
    return PyLong_FromLong(info->remotePort);
}

extern "C" PyObject*
tcpConnectionInfoGetRcvSize(ConnectionInfoObject* self, PyObject* /*args*/)
{
    auto info = dynamic_pointer_cast<Ice::TCPConnectionInfo>(*self->connectionInfo);
    assert(info);
    return PyLong_FromLong(info->rcvSize);
}

extern "C" PyObject*
tcpConnectionInfoGetSndSize(ConnectionInfoObject* self, PyObject* /*args*/)
{
    auto info = dynamic_pointer_cast<Ice::TCPConnectionInfo>(*self->connectionInfo);
    assert(info);
    return PyLong_FromLong(info->sndSize);
}

extern "C" PyObject*
udpConnectionInfoGetMcastAddress(ConnectionInfoObject* self, PyObject* /*args*/)
{
    auto info = dynamic_pointer_cast<Ice::UDPConnectionInfo>(*self->connectionInfo);
    assert(info);
    return createString(info->mcastAddress);
}

extern "C" PyObject*
udpConnectionInfoGetMcastPort(ConnectionInfoObject* self, PyObject* /*args*/)
{
    auto info = dynamic_pointer_cast<Ice::UDPConnectionInfo>(*self->connectionInfo);
    assert(info);
    return PyLong_FromLong(info->mcastPort);
}

extern "C" PyObject*
udpConnectionInfoGetRcvSize(ConnectionInfoObject* self, PyObject* /*args*/)
{
    auto info = dynamic_pointer_cast<Ice::UDPConnectionInfo>(*self->connectionInfo);
    assert(info);
    return PyLong_FromLong(info->rcvSize);
}

extern "C" PyObject*
udpConnectionInfoGetSndSize(ConnectionInfoObject* self, PyObject* /*args*/)
{
    auto info = dynamic_pointer_cast<Ice::UDPConnectionInfo>(*self->connectionInfo);
    assert(info);
    return PyLong_FromLong(info->sndSize);
}

extern "C" PyObject*
wsConnectionInfoGetHeaders(ConnectionInfoObject* self, PyObject* /*args*/)
{
    auto info = dynamic_pointer_cast<Ice::WSConnectionInfo>(*self->connectionInfo);
    assert(info);

    PyObjectHandle result{PyDict_New()};
    if (result.get())
    {
        for (const auto& header : info->headers)
        {
            PyObjectHandle key{createString(header.first)};
            PyObjectHandle val{createString(header.second)};
            if (!val.get() || PyDict_SetItem(result.get(), key.get(), val.get()) < 0)
            {
                return nullptr;
            }
        }
    }

    return result.release();
}

extern "C" PyObject*
sslConnectionInfoGetPeerCertificate(ConnectionInfoObject* self, PyObject* /*args*/)
{
    auto info = dynamic_pointer_cast<Ice::SSL::ConnectionInfo>(*self->connectionInfo);
    assert(info);
    string encoded;
    if (info->peerCertificate)
    {
        encoded = Ice::SSL::encodeCertificate(info->peerCertificate);
    }
    return createString(encoded);
}

static PyGetSetDef ConnectionInfoGetters[] = {
    {"underlying",
     reinterpret_cast<getter>(connectionInfoGetUnderlying),
     0,
     PyDoc_STR("get underlying connection information"),
     0},
    {"incoming",
     reinterpret_cast<getter>(connectionInfoGetIncoming),
     0,
     PyDoc_STR("whether connection is incoming"),
     0},
    {"adapterName",
     reinterpret_cast<getter>(connectionInfoGetAdapterName),
     0,
     PyDoc_STR("adapter associated the connection"),
     0},
    {0, 0} /* sentinel */
};

static PyGetSetDef IPConnectionInfoGetters[] = {
    {"localAddress", reinterpret_cast<getter>(ipConnectionInfoGetLocalAddress), 0, PyDoc_STR("local address"), 0},
    {"localPort", reinterpret_cast<getter>(ipConnectionInfoGetLocalPort), 0, PyDoc_STR("local port"), 0},
    {"remoteAddress", reinterpret_cast<getter>(ipConnectionInfoGetRemoteAddress), 0, PyDoc_STR("remote address"), 0},
    {"remotePort", reinterpret_cast<getter>(ipConnectionInfoGetRemotePort), 0, PyDoc_STR("remote port"), 0},
    {0, 0} /* sentinel */
};

static PyGetSetDef TCPConnectionInfoGetters[] = {
    {"rcvSize", reinterpret_cast<getter>(tcpConnectionInfoGetRcvSize), 0, PyDoc_STR("receive buffer size"), 0},
    {"sndSize", reinterpret_cast<getter>(tcpConnectionInfoGetSndSize), 0, PyDoc_STR("send buffer size"), 0},
    {0, 0} /* sentinel */
};

static PyGetSetDef UDPConnectionInfoGetters[] = {
    {"mcastAddress", reinterpret_cast<getter>(udpConnectionInfoGetMcastAddress), 0, PyDoc_STR("multicast address"), 0},
    {"mcastPort", reinterpret_cast<getter>(udpConnectionInfoGetMcastPort), 0, PyDoc_STR("multicast port"), 0},
    {"rcvSize", reinterpret_cast<getter>(udpConnectionInfoGetRcvSize), 0, PyDoc_STR("receive buffer size"), 0},
    {"sndSize", reinterpret_cast<getter>(udpConnectionInfoGetSndSize), 0, PyDoc_STR("send buffer size"), 0},
    {0, 0} /* sentinel */
};

static PyGetSetDef WSConnectionInfoGetters[] = {
    {"headers", reinterpret_cast<getter>(wsConnectionInfoGetHeaders), 0, PyDoc_STR("request headers"), 0},
    {0, 0} /* sentinel */
};

static PyGetSetDef SSLConnectionInfoGetters[] = {
    {"peerCertificate",
     reinterpret_cast<getter>(sslConnectionInfoGetPeerCertificate),
     0,
     PyDoc_STR("peer certificate"),
     0},
    {0, 0} /* sentinel */
};

namespace IcePy
{
    PyTypeObject ConnectionInfoType = {
        /* The ob_type field must be initialized in the module init function
         * to be portable to Windows without using C++. */
        PyVarObject_HEAD_INIT(0, 0) "IcePy.ConnectionInfo", /* tp_name */
        sizeof(ConnectionInfoObject),                       /* tp_basicsize */
        0,                                                  /* tp_itemsize */
        /* methods */
        (destructor)connectionInfoDealloc,        /* tp_dealloc */
        0,                                        /* tp_print */
        0,                                        /* tp_getattr */
        0,                                        /* tp_setattr */
        0,                                        /* tp_reserved */
        0,                                        /* tp_repr */
        0,                                        /* tp_as_number */
        0,                                        /* tp_as_sequence */
        0,                                        /* tp_as_mapping */
        0,                                        /* tp_hash */
        0,                                        /* tp_call */
        0,                                        /* tp_str */
        0,                                        /* tp_getattro */
        0,                                        /* tp_setattro */
        0,                                        /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
        0,                                        /* tp_doc */
        0,                                        /* tp_traverse */
        0,                                        /* tp_clear */
        0,                                        /* tp_richcompare */
        0,                                        /* tp_weaklistoffset */
        0,                                        /* tp_iter */
        0,                                        /* tp_iternext */
        0,                                        /* tp_methods */
        0,                                        /* tp_members */
        ConnectionInfoGetters,                    /* tp_getset */
        0,                                        /* tp_base */
        0,                                        /* tp_dict */
        0,                                        /* tp_descr_get */
        0,                                        /* tp_descr_set */
        0,                                        /* tp_dictoffset */
        0,                                        /* tp_init */
        0,                                        /* tp_alloc */
        (newfunc)connectionInfoNew,               /* tp_new */
        0,                                        /* tp_free */
        0,                                        /* tp_is_gc */
    };

    PyTypeObject IPConnectionInfoType = {
        /* The ob_type field must be initialized in the module init function
         * to be portable to Windows without using C++. */
        PyVarObject_HEAD_INIT(0, 0) "IcePy.IPConnectionInfo", /* tp_name */
        sizeof(ConnectionInfoObject),                         /* tp_basicsize */
        0,                                                    /* tp_itemsize */
        /* methods */
        (destructor)connectionInfoDealloc,        /* tp_dealloc */
        0,                                        /* tp_print */
        0,                                        /* tp_getattr */
        0,                                        /* tp_setattr */
        0,                                        /* tp_reserved */
        0,                                        /* tp_repr */
        0,                                        /* tp_as_number */
        0,                                        /* tp_as_sequence */
        0,                                        /* tp_as_mapping */
        0,                                        /* tp_hash */
        0,                                        /* tp_call */
        0,                                        /* tp_str */
        0,                                        /* tp_getattro */
        0,                                        /* tp_setattro */
        0,                                        /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
        0,                                        /* tp_doc */
        0,                                        /* tp_traverse */
        0,                                        /* tp_clear */
        0,                                        /* tp_richcompare */
        0,                                        /* tp_weaklistoffset */
        0,                                        /* tp_iter */
        0,                                        /* tp_iternext */
        0,                                        /* tp_methods */
        0,                                        /* tp_members */
        IPConnectionInfoGetters,                  /* tp_getset */
        0,                                        /* tp_base */
        0,                                        /* tp_dict */
        0,                                        /* tp_descr_get */
        0,                                        /* tp_descr_set */
        0,                                        /* tp_dictoffset */
        0,                                        /* tp_init */
        0,                                        /* tp_alloc */
        (newfunc)connectionInfoNew,               /* tp_new */
        0,                                        /* tp_free */
        0,                                        /* tp_is_gc */
    };

    PyTypeObject TCPConnectionInfoType = {
        /* The ob_type field must be initialized in the module init function
         * to be portable to Windows without using C++. */
        PyVarObject_HEAD_INIT(0, 0) "IcePy.TCPConnectionInfo", /* tp_name */
        sizeof(ConnectionInfoObject),                          /* tp_basicsize */
        0,                                                     /* tp_itemsize */
        /* methods */
        (destructor)connectionInfoDealloc,        /* tp_dealloc */
        0,                                        /* tp_print */
        0,                                        /* tp_getattr */
        0,                                        /* tp_setattr */
        0,                                        /* tp_reserved */
        0,                                        /* tp_repr */
        0,                                        /* tp_as_number */
        0,                                        /* tp_as_sequence */
        0,                                        /* tp_as_mapping */
        0,                                        /* tp_hash */
        0,                                        /* tp_call */
        0,                                        /* tp_str */
        0,                                        /* tp_getattro */
        0,                                        /* tp_setattro */
        0,                                        /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
        0,                                        /* tp_doc */
        0,                                        /* tp_traverse */
        0,                                        /* tp_clear */
        0,                                        /* tp_richcompare */
        0,                                        /* tp_weaklistoffset */
        0,                                        /* tp_iter */
        0,                                        /* tp_iternext */
        0,                                        /* tp_methods */
        0,                                        /* tp_members */
        TCPConnectionInfoGetters,                 /* tp_getset */
        0,                                        /* tp_base */
        0,                                        /* tp_dict */
        0,                                        /* tp_descr_get */
        0,                                        /* tp_descr_set */
        0,                                        /* tp_dictoffset */
        0,                                        /* tp_init */
        0,                                        /* tp_alloc */
        (newfunc)connectionInfoNew,               /* tp_new */
        0,                                        /* tp_free */
        0,                                        /* tp_is_gc */
    };

    PyTypeObject UDPConnectionInfoType = {
        /* The ob_type field must be initialized in the module init function
         * to be portable to Windows without using C++. */
        PyVarObject_HEAD_INIT(0, 0) "IcePy.UDPConnectionInfo", /* tp_name */
        sizeof(ConnectionInfoObject),                          /* tp_basicsize */
        0,                                                     /* tp_itemsize */
        /* methods */
        (destructor)connectionInfoDealloc,        /* tp_dealloc */
        0,                                        /* tp_print */
        0,                                        /* tp_getattr */
        0,                                        /* tp_setattr */
        0,                                        /* tp_reserved */
        0,                                        /* tp_repr */
        0,                                        /* tp_as_number */
        0,                                        /* tp_as_sequence */
        0,                                        /* tp_as_mapping */
        0,                                        /* tp_hash */
        0,                                        /* tp_call */
        0,                                        /* tp_str */
        0,                                        /* tp_getattro */
        0,                                        /* tp_setattro */
        0,                                        /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
        0,                                        /* tp_doc */
        0,                                        /* tp_traverse */
        0,                                        /* tp_clear */
        0,                                        /* tp_richcompare */
        0,                                        /* tp_weaklistoffset */
        0,                                        /* tp_iter */
        0,                                        /* tp_iternext */
        0,                                        /* tp_methods */
        0,                                        /* tp_members */
        UDPConnectionInfoGetters,                 /* tp_getset */
        0,                                        /* tp_base */
        0,                                        /* tp_dict */
        0,                                        /* tp_descr_get */
        0,                                        /* tp_descr_set */
        0,                                        /* tp_dictoffset */
        0,                                        /* tp_init */
        0,                                        /* tp_alloc */
        (newfunc)connectionInfoNew,               /* tp_new */
        0,                                        /* tp_free */
        0,                                        /* tp_is_gc */
    };

    PyTypeObject WSConnectionInfoType = {
        /* The ob_type field must be initialized in the module init function
         * to be portable to Windows without using C++. */
        PyVarObject_HEAD_INIT(0, 0) "IcePy.WSConnectionInfo", /* tp_name */
        sizeof(ConnectionInfoObject),                         /* tp_basicsize */
        0,                                                    /* tp_itemsize */
        /* methods */
        (destructor)connectionInfoDealloc,        /* tp_dealloc */
        0,                                        /* tp_print */
        0,                                        /* tp_getattr */
        0,                                        /* tp_setattr */
        0,                                        /* tp_reserved */
        0,                                        /* tp_repr */
        0,                                        /* tp_as_number */
        0,                                        /* tp_as_sequence */
        0,                                        /* tp_as_mapping */
        0,                                        /* tp_hash */
        0,                                        /* tp_call */
        0,                                        /* tp_str */
        0,                                        /* tp_getattro */
        0,                                        /* tp_setattro */
        0,                                        /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
        0,                                        /* tp_doc */
        0,                                        /* tp_traverse */
        0,                                        /* tp_clear */
        0,                                        /* tp_richcompare */
        0,                                        /* tp_weaklistoffset */
        0,                                        /* tp_iter */
        0,                                        /* tp_iternext */
        0,                                        /* tp_methods */
        0,                                        /* tp_members */
        WSConnectionInfoGetters,                  /* tp_getset */
        0,                                        /* tp_base */
        0,                                        /* tp_dict */
        0,                                        /* tp_descr_get */
        0,                                        /* tp_descr_set */
        0,                                        /* tp_dictoffset */
        0,                                        /* tp_init */
        0,                                        /* tp_alloc */
        (newfunc)connectionInfoNew,               /* tp_new */
        0,                                        /* tp_free */
        0,                                        /* tp_is_gc */
    };

    PyTypeObject SSLConnectionInfoType = {
        /* The ob_type field must be initialized in the module init function
         * to be portable to Windows without using C++. */
        PyVarObject_HEAD_INIT(0, 0) "IcePy.SSLConnectionInfo", /* tp_name */
        sizeof(ConnectionInfoObject),                          /* tp_basicsize */
        0,                                                     /* tp_itemsize */
        /* methods */
        (destructor)connectionInfoDealloc,        /* tp_dealloc */
        0,                                        /* tp_print */
        0,                                        /* tp_getattr */
        0,                                        /* tp_setattr */
        0,                                        /* tp_reserved */
        0,                                        /* tp_repr */
        0,                                        /* tp_as_number */
        0,                                        /* tp_as_sequence */
        0,                                        /* tp_as_mapping */
        0,                                        /* tp_hash */
        0,                                        /* tp_call */
        0,                                        /* tp_str */
        0,                                        /* tp_getattro */
        0,                                        /* tp_setattro */
        0,                                        /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
        0,                                        /* tp_doc */
        0,                                        /* tp_traverse */
        0,                                        /* tp_clear */
        0,                                        /* tp_richcompare */
        0,                                        /* tp_weaklistoffset */
        0,                                        /* tp_iter */
        0,                                        /* tp_iternext */
        0,                                        /* tp_methods */
        0,                                        /* tp_members */
        SSLConnectionInfoGetters,                 /* tp_getset */
        0,                                        /* tp_base */
        0,                                        /* tp_dict */
        0,                                        /* tp_descr_get */
        0,                                        /* tp_descr_set */
        0,                                        /* tp_dictoffset */
        0,                                        /* tp_init */
        0,                                        /* tp_alloc */
        (newfunc)connectionInfoNew,               /* tp_new */
        0,                                        /* tp_free */
        0,                                        /* tp_is_gc */
    };
}

bool
IcePy::initConnectionInfo(PyObject* module)
{
    if (PyType_Ready(&ConnectionInfoType) < 0)
    {
        return false;
    }
    PyTypeObject* type = &ConnectionInfoType; // Necessary to prevent GCC's strict-alias warnings.
    if (PyModule_AddObject(module, "ConnectionInfo", reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    IPConnectionInfoType.tp_base = &ConnectionInfoType; // Force inheritance from ConnectionInfoType.
    if (PyType_Ready(&IPConnectionInfoType) < 0)
    {
        return false;
    }
    type = &IPConnectionInfoType; // Necessary to prevent GCC's strict-alias warnings.
    if (PyModule_AddObject(module, "IPConnectionInfo", reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    TCPConnectionInfoType.tp_base = &IPConnectionInfoType; // Force inheritance from IPConnectionInfoType.
    if (PyType_Ready(&TCPConnectionInfoType) < 0)
    {
        return false;
    }
    type = &TCPConnectionInfoType; // Necessary to prevent GCC's strict-alias warnings.
    if (PyModule_AddObject(module, "TCPConnectionInfo", reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    UDPConnectionInfoType.tp_base = &IPConnectionInfoType; // Force inheritance from IPConnectionType.
    if (PyType_Ready(&UDPConnectionInfoType) < 0)
    {
        return false;
    }
    type = &UDPConnectionInfoType; // Necessary to prevent GCC's strict-alias warnings.
    if (PyModule_AddObject(module, "UDPConnectionInfo", reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    WSConnectionInfoType.tp_base = &ConnectionInfoType; // Force inheritance from IPConnectionType.
    if (PyType_Ready(&WSConnectionInfoType) < 0)
    {
        return false;
    }
    type = &WSConnectionInfoType; // Necessary to prevent GCC's strict-alias warnings.
    if (PyModule_AddObject(module, "WSConnectionInfo", reinterpret_cast<PyObject*>(type)) < 0)
    {
        return false;
    }

    SSLConnectionInfoType.tp_base = &ConnectionInfoType; // Force inheritance from IPConnectionInfoType.
    if (PyType_Ready(&SSLConnectionInfoType) < 0)
    {
        return false;
    }
    type = &SSLConnectionInfoType; // Necessary to prevent GCC's strict-alias warnings.
    if (PyModule_AddObject(module, "SSLConnectionInfo", reinterpret_cast<PyObject*>(type)) < 0)
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
    if (!connectionInfo)
    {
        return Py_None;
    }

    PyTypeObject* type;
    if (dynamic_pointer_cast<Ice::WSConnectionInfo>(connectionInfo))
    {
        type = &WSConnectionInfoType;
    }
    else if (dynamic_pointer_cast<Ice::TCPConnectionInfo>(connectionInfo))
    {
        type = &TCPConnectionInfoType;
    }
    else if (dynamic_pointer_cast<Ice::UDPConnectionInfo>(connectionInfo))
    {
        type = &UDPConnectionInfoType;
    }
    else if (dynamic_pointer_cast<Ice::SSL::ConnectionInfo>(connectionInfo))
    {
        type = &SSLConnectionInfoType;
    }
    else if (dynamic_pointer_cast<Ice::IPConnectionInfo>(connectionInfo))
    {
        type = &IPConnectionInfoType;
    }
    else
    {
        type = &ConnectionInfoType;
    }

    ConnectionInfoObject* obj = reinterpret_cast<ConnectionInfoObject*>(type->tp_alloc(type, 0));
    if (!obj)
    {
        return nullptr;
    }
    obj->connectionInfo = new Ice::ConnectionInfoPtr(connectionInfo);

    return (PyObject*)obj;
}
