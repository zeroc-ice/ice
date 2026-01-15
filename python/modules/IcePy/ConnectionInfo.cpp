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
     nullptr,
     PyDoc_STR("ConnectionInfo | None: The information of the underlying transport or ``None`` if there's no "
               "underlying transport."),
     nullptr},
    {"incoming",
     reinterpret_cast<getter>(connectionInfoGetIncoming),
     nullptr,
     PyDoc_STR("bool: ``True`` if this is an incoming connection, ``False`` otherwise."),
     nullptr},
    {"adapterName",
     reinterpret_cast<getter>(connectionInfoGetAdapterName),
     nullptr,
     PyDoc_STR("str: The name of the adapter associated with the connection."),
     nullptr},
    {} /* sentinel */
};

static PyGetSetDef IPConnectionInfoGetters[] = {
    {"localAddress",
     reinterpret_cast<getter>(ipConnectionInfoGetLocalAddress),
     nullptr,
     PyDoc_STR("str: The local address."),
     nullptr},
    {"localPort",
     reinterpret_cast<getter>(ipConnectionInfoGetLocalPort),
     nullptr,
     PyDoc_STR("int: The local port."),
     nullptr},
    {"remoteAddress",
     reinterpret_cast<getter>(ipConnectionInfoGetRemoteAddress),
     nullptr,
     PyDoc_STR("str: The remote address."),
     nullptr},
    {"remotePort",
     reinterpret_cast<getter>(ipConnectionInfoGetRemotePort),
     nullptr,
     PyDoc_STR("int: The remote port."),
     nullptr},
    {} /* sentinel */
};

static PyGetSetDef TCPConnectionInfoGetters[] = {
    {"rcvSize",
     reinterpret_cast<getter>(tcpConnectionInfoGetRcvSize),
     nullptr,
     PyDoc_STR("int: The size of the receive buffer."),
     nullptr},
    {"sndSize",
     reinterpret_cast<getter>(tcpConnectionInfoGetSndSize),
     nullptr,
     PyDoc_STR("int: The size of the send buffer."),
     nullptr},
    {} /* sentinel */
};

static PyGetSetDef UDPConnectionInfoGetters[] = {
    {"mcastAddress",
     reinterpret_cast<getter>(udpConnectionInfoGetMcastAddress),
     nullptr,
     PyDoc_STR("str: The multicast address."),
     nullptr},
    {"mcastPort",
     reinterpret_cast<getter>(udpConnectionInfoGetMcastPort),
     nullptr,
     PyDoc_STR("int: The multicast port."),
     nullptr},
    {"rcvSize",
     reinterpret_cast<getter>(udpConnectionInfoGetRcvSize),
     nullptr,
     PyDoc_STR("int: The size of the receive buffer."),
     nullptr},
    {"sndSize",
     reinterpret_cast<getter>(udpConnectionInfoGetSndSize),
     nullptr,
     PyDoc_STR("int: The size of the send buffer."),
     nullptr},
    {} /* sentinel */
};

static PyGetSetDef WSConnectionInfoGetters[] = {
    {"headers",
     reinterpret_cast<getter>(wsConnectionInfoGetHeaders),
     nullptr,
     PyDoc_STR("dict[str, str]: The headers from the HTTP upgrade request."),
     nullptr},
    {} /* sentinel */
};

static PyGetSetDef SSLConnectionInfoGetters[] = {
    {"peerCertificate",
     reinterpret_cast<getter>(sslConnectionInfoGetPeerCertificate),
     nullptr,
     PyDoc_STR("str: The certificate chain."),
     nullptr},
    {} /* sentinel */
};

namespace IcePy
{
    // clang-format off
    PyTypeObject ConnectionInfoType = {
        .ob_base = PyVarObject_HEAD_INIT(nullptr, 0)
        .tp_name = "IcePy.ConnectionInfo",
        .tp_basicsize = sizeof(ConnectionInfoObject),
        .tp_dealloc = reinterpret_cast<destructor>(connectionInfoDealloc),
        .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        .tp_doc = PyDoc_STR("Base class for all connection info classes."),
        .tp_getset = ConnectionInfoGetters,
        .tp_new = reinterpret_cast<newfunc>(connectionInfoNew),
    };

    PyTypeObject IPConnectionInfoType = {
        .ob_base = PyVarObject_HEAD_INIT(nullptr, 0)
        .tp_name = "IcePy.IPConnectionInfo",
        .tp_basicsize = sizeof(ConnectionInfoObject),
        .tp_dealloc = reinterpret_cast<destructor>(connectionInfoDealloc),
        .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        .tp_doc = PyDoc_STR("Provides access to the connection details of an IP connection."),
        .tp_getset = IPConnectionInfoGetters,
        .tp_new = reinterpret_cast<newfunc>(connectionInfoNew),
    };

    PyTypeObject TCPConnectionInfoType = {
        .ob_base = PyVarObject_HEAD_INIT(nullptr, 0)
        .tp_name = "IcePy.TCPConnectionInfo",
        .tp_basicsize = sizeof(ConnectionInfoObject),
        .tp_dealloc = reinterpret_cast<destructor>(connectionInfoDealloc),
        .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        .tp_doc = PyDoc_STR("Provides access to the connection details of a TCP connection."),
        .tp_getset = TCPConnectionInfoGetters,
        .tp_new = reinterpret_cast<newfunc>(connectionInfoNew),
    };

    PyTypeObject UDPConnectionInfoType = {
        .ob_base = PyVarObject_HEAD_INIT(nullptr, 0)
        .tp_name = "IcePy.UDPConnectionInfo",
        .tp_basicsize = sizeof(ConnectionInfoObject),
        .tp_dealloc = reinterpret_cast<destructor>(connectionInfoDealloc),
        .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        .tp_doc = PyDoc_STR("Provides access to the connection details of a UDP connection."),
        .tp_getset = UDPConnectionInfoGetters,
        .tp_new = reinterpret_cast<newfunc>(connectionInfoNew),
    };

    PyTypeObject WSConnectionInfoType = {
        .ob_base = PyVarObject_HEAD_INIT(nullptr, 0)
        .tp_name = "IcePy.WSConnectionInfo",
        .tp_basicsize = sizeof(ConnectionInfoObject),
        .tp_dealloc = reinterpret_cast<destructor>(connectionInfoDealloc),
        .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        .tp_doc = PyDoc_STR("Provides access to the connection details of a WebSocket connection."),
        .tp_getset = WSConnectionInfoGetters,
        .tp_new = reinterpret_cast<newfunc>(connectionInfoNew),
    };

    static PyTypeObject SSLConnectionInfoType = {
        .ob_base = PyVarObject_HEAD_INIT(nullptr, 0)
        .tp_name = "IcePy.SSLConnectionInfo",
        .tp_basicsize = sizeof(ConnectionInfoObject),
        .tp_dealloc = reinterpret_cast<destructor>(connectionInfoDealloc),
        .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        .tp_doc = PyDoc_STR("Provides access to the connection details of an SSL connection."),
        .tp_getset = SSLConnectionInfoGetters,
        .tp_new = reinterpret_cast<newfunc>(connectionInfoNew),
    };
    // clang-format on
}

bool
IcePy::initConnectionInfo(PyObject* module)
{
    if (PyType_Ready(&ConnectionInfoType) < 0)
    {
        return false;
    }

    if (PyModule_AddObject(module, "ConnectionInfo", reinterpret_cast<PyObject*>(&ConnectionInfoType)) < 0)
    {
        return false;
    }

    IPConnectionInfoType.tp_base = &ConnectionInfoType; // Force inheritance from ConnectionInfoType.
    if (PyType_Ready(&IPConnectionInfoType) < 0)
    {
        return false;
    }

    if (PyModule_AddObject(module, "IPConnectionInfo", reinterpret_cast<PyObject*>(&IPConnectionInfoType)) < 0)
    {
        return false;
    }

    TCPConnectionInfoType.tp_base = &IPConnectionInfoType; // Force inheritance from IPConnectionInfoType.
    if (PyType_Ready(&TCPConnectionInfoType) < 0)
    {
        return false;
    }

    if (PyModule_AddObject(module, "TCPConnectionInfo", reinterpret_cast<PyObject*>(&TCPConnectionInfoType)) < 0)
    {
        return false;
    }

    UDPConnectionInfoType.tp_base = &IPConnectionInfoType; // Force inheritance from IPConnectionType.
    if (PyType_Ready(&UDPConnectionInfoType) < 0)
    {
        return false;
    }

    if (PyModule_AddObject(module, "UDPConnectionInfo", reinterpret_cast<PyObject*>(&UDPConnectionInfoType)) < 0)
    {
        return false;
    }

    WSConnectionInfoType.tp_base = &ConnectionInfoType; // Force inheritance from IPConnectionType.
    if (PyType_Ready(&WSConnectionInfoType) < 0)
    {
        return false;
    }

    if (PyModule_AddObject(module, "WSConnectionInfo", reinterpret_cast<PyObject*>(&WSConnectionInfoType)) < 0)
    {
        return false;
    }

    SSLConnectionInfoType.tp_base = &ConnectionInfoType; // Force inheritance from IPConnectionInfoType.
    if (PyType_Ready(&SSLConnectionInfoType) < 0)
    {
        return false;
    }

    if (PyModule_AddObject(module, "SSLConnectionInfo", reinterpret_cast<PyObject*>(&SSLConnectionInfoType)) < 0)
    {
        return false;
    }

    return true;
}

Ice::ConnectionInfoPtr
IcePy::getConnectionInfo(PyObject* obj)
{
    assert(PyObject_IsInstance(obj, reinterpret_cast<PyObject*>(&ConnectionInfoType)));
    auto* eobj = reinterpret_cast<ConnectionInfoObject*>(obj);
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

    auto* obj = reinterpret_cast<ConnectionInfoObject*>(type->tp_alloc(type, 0));
    if (!obj)
    {
        return nullptr;
    }
    obj->connectionInfo = new Ice::ConnectionInfoPtr(connectionInfo);

    return (PyObject*)obj;
}
