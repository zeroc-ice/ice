// Copyright (c) ZeroC, Inc.

#include "ConnectionInfo.h"
#include "../../cpp/src/Ice/SSL/SSLUtil.h"
#include "DocStrings.h"
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
connectionInfoGetConnectionId(ConnectionInfoObject* self, PyObject* /*args*/)
{
    return createString((*self->connectionInfo)->connectionId);
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
     PyDoc_STR(IcePy_DOC_ConnectionInfo_underlying),
     nullptr},
    {"incoming",
     reinterpret_cast<getter>(connectionInfoGetIncoming),
     nullptr,
     PyDoc_STR(IcePy_DOC_ConnectionInfo_incoming),
     nullptr},
    {"adapterName",
     reinterpret_cast<getter>(connectionInfoGetAdapterName),
     nullptr,
     PyDoc_STR(IcePy_DOC_ConnectionInfo_adapterName),
     nullptr},
    {"connectionId",
     reinterpret_cast<getter>(connectionInfoGetConnectionId),
     nullptr,
     PyDoc_STR(IcePy_DOC_ConnectionInfo_connectionId),
     nullptr},
    {} /* sentinel */
};

static PyGetSetDef IPConnectionInfoGetters[] = {
    {"localAddress",
     reinterpret_cast<getter>(ipConnectionInfoGetLocalAddress),
     nullptr,
     PyDoc_STR(IcePy_DOC_IPConnectionInfo_localAddress),
     nullptr},
    {"localPort",
     reinterpret_cast<getter>(ipConnectionInfoGetLocalPort),
     nullptr,
     PyDoc_STR(IcePy_DOC_IPConnectionInfo_localPort),
     nullptr},
    {"remoteAddress",
     reinterpret_cast<getter>(ipConnectionInfoGetRemoteAddress),
     nullptr,
     PyDoc_STR(IcePy_DOC_IPConnectionInfo_remoteAddress),
     nullptr},
    {"remotePort",
     reinterpret_cast<getter>(ipConnectionInfoGetRemotePort),
     nullptr,
     PyDoc_STR(IcePy_DOC_IPConnectionInfo_remotePort),
     nullptr},
    {} /* sentinel */
};

static PyGetSetDef TCPConnectionInfoGetters[] = {
    {"rcvSize",
     reinterpret_cast<getter>(tcpConnectionInfoGetRcvSize),
     nullptr,
     PyDoc_STR(IcePy_DOC_TCPConnectionInfo_rcvSize),
     nullptr},
    {"sndSize",
     reinterpret_cast<getter>(tcpConnectionInfoGetSndSize),
     nullptr,
     PyDoc_STR(IcePy_DOC_TCPConnectionInfo_sndSize),
     nullptr},
    {} /* sentinel */
};

static PyGetSetDef UDPConnectionInfoGetters[] = {
    {"mcastAddress",
     reinterpret_cast<getter>(udpConnectionInfoGetMcastAddress),
     nullptr,
     PyDoc_STR(IcePy_DOC_UDPConnectionInfo_mcastAddress),
     nullptr},
    {"mcastPort",
     reinterpret_cast<getter>(udpConnectionInfoGetMcastPort),
     nullptr,
     PyDoc_STR(IcePy_DOC_UDPConnectionInfo_mcastPort),
     nullptr},
    {"rcvSize",
     reinterpret_cast<getter>(udpConnectionInfoGetRcvSize),
     nullptr,
     PyDoc_STR(IcePy_DOC_UDPConnectionInfo_rcvSize),
     nullptr},
    {"sndSize",
     reinterpret_cast<getter>(udpConnectionInfoGetSndSize),
     nullptr,
     PyDoc_STR(IcePy_DOC_UDPConnectionInfo_sndSize),
     nullptr},
    {} /* sentinel */
};

static PyGetSetDef WSConnectionInfoGetters[] = {
    {"headers",
     reinterpret_cast<getter>(wsConnectionInfoGetHeaders),
     nullptr,
     PyDoc_STR(IcePy_DOC_WSConnectionInfo_headers),
     nullptr},
    {} /* sentinel */
};

static PyGetSetDef SSLConnectionInfoGetters[] = {
    {"peerCertificate",
     reinterpret_cast<getter>(sslConnectionInfoGetPeerCertificate),
     nullptr,
     PyDoc_STR(IcePy_DOC_SSLConnectionInfo_peerCertificate),
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
        .tp_doc = PyDoc_STR(IcePy_DOC_ConnectionInfo),
        .tp_getset = ConnectionInfoGetters,
        .tp_new = reinterpret_cast<newfunc>(connectionInfoNew),
    };

    PyTypeObject IPConnectionInfoType = {
        .ob_base = PyVarObject_HEAD_INIT(nullptr, 0)
        .tp_name = "IcePy.IPConnectionInfo",
        .tp_basicsize = sizeof(ConnectionInfoObject),
        .tp_dealloc = reinterpret_cast<destructor>(connectionInfoDealloc),
        .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        .tp_doc = PyDoc_STR(IcePy_DOC_IPConnectionInfo),
        .tp_getset = IPConnectionInfoGetters,
        .tp_new = reinterpret_cast<newfunc>(connectionInfoNew),
    };

    PyTypeObject TCPConnectionInfoType = {
        .ob_base = PyVarObject_HEAD_INIT(nullptr, 0)
        .tp_name = "IcePy.TCPConnectionInfo",
        .tp_basicsize = sizeof(ConnectionInfoObject),
        .tp_dealloc = reinterpret_cast<destructor>(connectionInfoDealloc),
        .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        .tp_doc = PyDoc_STR(IcePy_DOC_TCPConnectionInfo),
        .tp_getset = TCPConnectionInfoGetters,
        .tp_new = reinterpret_cast<newfunc>(connectionInfoNew),
    };

    PyTypeObject UDPConnectionInfoType = {
        .ob_base = PyVarObject_HEAD_INIT(nullptr, 0)
        .tp_name = "IcePy.UDPConnectionInfo",
        .tp_basicsize = sizeof(ConnectionInfoObject),
        .tp_dealloc = reinterpret_cast<destructor>(connectionInfoDealloc),
        .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        .tp_doc = PyDoc_STR(IcePy_DOC_UDPConnectionInfo),
        .tp_getset = UDPConnectionInfoGetters,
        .tp_new = reinterpret_cast<newfunc>(connectionInfoNew),
    };

    PyTypeObject WSConnectionInfoType = {
        .ob_base = PyVarObject_HEAD_INIT(nullptr, 0)
        .tp_name = "IcePy.WSConnectionInfo",
        .tp_basicsize = sizeof(ConnectionInfoObject),
        .tp_dealloc = reinterpret_cast<destructor>(connectionInfoDealloc),
        .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        .tp_doc = PyDoc_STR(IcePy_DOC_WSConnectionInfo),
        .tp_getset = WSConnectionInfoGetters,
        .tp_new = reinterpret_cast<newfunc>(connectionInfoNew),
    };

    static PyTypeObject SSLConnectionInfoType = {
        .ob_base = PyVarObject_HEAD_INIT(nullptr, 0)
        .tp_name = "IcePy.SSLConnectionInfo",
        .tp_basicsize = sizeof(ConnectionInfoObject),
        .tp_dealloc = reinterpret_cast<destructor>(connectionInfoDealloc),
        .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
        .tp_doc = PyDoc_STR(IcePy_DOC_SSLConnectionInfo),
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

    UDPConnectionInfoType.tp_base = &IPConnectionInfoType; // Force inheritance from IPConnectionInfoType.
    if (PyType_Ready(&UDPConnectionInfoType) < 0)
    {
        return false;
    }

    if (PyModule_AddObject(module, "UDPConnectionInfo", reinterpret_cast<PyObject*>(&UDPConnectionInfoType)) < 0)
    {
        return false;
    }

    WSConnectionInfoType.tp_base = &ConnectionInfoType; // Force inheritance from ConnectionInfoType.
    if (PyType_Ready(&WSConnectionInfoType) < 0)
    {
        return false;
    }

    if (PyModule_AddObject(module, "WSConnectionInfo", reinterpret_cast<PyObject*>(&WSConnectionInfoType)) < 0)
    {
        return false;
    }

    SSLConnectionInfoType.tp_base = &ConnectionInfoType; // Force inheritance from ConnectionInfoType.
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
