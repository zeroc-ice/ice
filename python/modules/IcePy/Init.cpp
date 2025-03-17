// Copyright (c) ZeroC, Inc.

#include "BatchRequestInterceptor.h"
#include "Communicator.h"
#include "Connection.h"
#include "ConnectionInfo.h"
#include "Endpoint.h"
#include "EndpointInfo.h"
#include "Executor.h"
#include "Ice/Initialize.h"
#include "Ice/RegisterPlugins.h"
#include "ImplicitContext.h"
#include "Logger.h"
#include "ObjectAdapter.h"
#include "Operation.h"
#include "Properties.h"
#include "PropertiesAdmin.h"
#include "Proxy.h"
#include "Slice.h"
#include "Types.h"
#include "ValueFactoryManager.h"

// Link with IceDiscovery and IceLocatorDiscovery on Windows for "shared" builds.
#if defined(_MSC_VER) && !defined(ICE_DISABLE_PRAGMA_COMMENT)
#    pragma comment(lib, ICE_LIBNAME("IceDiscovery"))
#    pragma comment(lib, ICE_LIBNAME("IceLocatorDiscovery"))
#endif

using namespace std;
using namespace IcePy;

extern "C" void IcePy_cleanup(void*);

static PyMethodDef methods[] = {
    {"stringVersion",
     reinterpret_cast<PyCFunction>(IcePy_stringVersion),
     METH_NOARGS,
     PyDoc_STR("stringVersion() -> string")},
    {"intVersion", reinterpret_cast<PyCFunction>(IcePy_intVersion), METH_NOARGS, PyDoc_STR("intVersion() -> int")},
    {"currentProtocol",
     reinterpret_cast<PyCFunction>(IcePy_currentProtocol),
     METH_NOARGS,
     PyDoc_STR("currentProtocol() -> Ice.ProtocolVersion")},
    {"currentProtocolEncoding",
     reinterpret_cast<PyCFunction>(IcePy_currentProtocolEncoding),
     METH_NOARGS,
     PyDoc_STR("currentProtocolEncoding() -> Ice.EncodingVersion")},
    {"currentEncoding",
     reinterpret_cast<PyCFunction>(IcePy_currentEncoding),
     METH_NOARGS,
     PyDoc_STR("currentEncoding() -> Ice.EncodingVersion")},
    {"stringToProtocolVersion",
     reinterpret_cast<PyCFunction>(IcePy_stringToProtocolVersion),
     METH_VARARGS,
     PyDoc_STR("stringToProtocolVersion(str) -> Ice.ProtocolVersion")},
    {"protocolVersionToString",
     reinterpret_cast<PyCFunction>(IcePy_protocolVersionToString),
     METH_VARARGS,
     PyDoc_STR("protocolVersionToString(Ice.ProtocolVersion) -> string")},
    {"stringToEncodingVersion",
     reinterpret_cast<PyCFunction>(IcePy_stringToEncodingVersion),
     METH_VARARGS,
     PyDoc_STR("stringToEncodingVersion(str) -> Ice.EncodingVersion")},
    {"encodingVersionToString",
     reinterpret_cast<PyCFunction>(IcePy_encodingVersionToString),
     METH_VARARGS,
     PyDoc_STR("encodingVersionToString(Ice.EncodingVersion) -> string")},
    {"createProperties",
     reinterpret_cast<PyCFunction>(IcePy_createProperties),
     METH_VARARGS,
     PyDoc_STR("createProperties([args]) -> Ice.Properties")},
    {"stringToIdentity",
     reinterpret_cast<PyCFunction>(IcePy_stringToIdentity),
     METH_O,
     PyDoc_STR("stringToIdentity(string) -> Ice.Identity")},
    {"identityToString",
     reinterpret_cast<PyCFunction>(IcePy_identityToString),
     METH_VARARGS,
     PyDoc_STR("identityToString(Ice.Identity, Ice.ToStringMode) -> string")},
    {"getProcessLogger",
     reinterpret_cast<PyCFunction>(IcePy_getProcessLogger),
     METH_NOARGS,
     PyDoc_STR("getProcessLogger() -> Ice.Logger")},
    {"setProcessLogger",
     reinterpret_cast<PyCFunction>(IcePy_setProcessLogger),
     METH_VARARGS,
     PyDoc_STR("setProcessLogger(logger) -> None")},
    {"defineEnum", reinterpret_cast<PyCFunction>(IcePy_defineEnum), METH_VARARGS, PyDoc_STR("internal function")},
    {"defineStruct", reinterpret_cast<PyCFunction>(IcePy_defineStruct), METH_VARARGS, PyDoc_STR("internal function")},
    {"defineSequence",
     reinterpret_cast<PyCFunction>(IcePy_defineSequence),
     METH_VARARGS,
     PyDoc_STR("internal function")},
    {"defineDictionary",
     reinterpret_cast<PyCFunction>(IcePy_defineDictionary),
     METH_VARARGS,
     PyDoc_STR("internal function")},
    {"declareProxy", reinterpret_cast<PyCFunction>(IcePy_declareProxy), METH_VARARGS, PyDoc_STR("internal function")},
    {"defineProxy", reinterpret_cast<PyCFunction>(IcePy_defineProxy), METH_VARARGS, PyDoc_STR("internal function")},
    {"declareValue", reinterpret_cast<PyCFunction>(IcePy_declareValue), METH_VARARGS, PyDoc_STR("internal function")},
    {"defineValue", reinterpret_cast<PyCFunction>(IcePy_defineValue), METH_VARARGS, PyDoc_STR("internal function")},
    {"defineException",
     reinterpret_cast<PyCFunction>(IcePy_defineException),
     METH_VARARGS,
     PyDoc_STR("internal function")},
    {"stringify", reinterpret_cast<PyCFunction>(IcePy_stringify), METH_VARARGS, PyDoc_STR("internal function")},
    {"stringifyException",
     reinterpret_cast<PyCFunction>(IcePy_stringifyException),
     METH_VARARGS,
     PyDoc_STR("internal function")},
    {"loadSlice", reinterpret_cast<PyCFunction>(IcePy_loadSlice), METH_VARARGS, PyDoc_STR("loadSlice(cmd) -> None")},
    {"compile", reinterpret_cast<PyCFunction>(IcePy_compile), METH_VARARGS, PyDoc_STR("internal function")},
    {nullptr, nullptr, METH_NOARGS, nullptr} // Sentinel
};

static struct PyModuleDef iceModule = {
    PyModuleDef_HEAD_INIT,
    "IcePy",
    "The Internet Communications Engine.",
    -1,
    methods,
    nullptr,
    nullptr,
    nullptr,
    IcePy_cleanup};

#if defined(__GNUC__)
extern "C" __attribute__((visibility("default"))) PyObject*
#elif defined(_WIN32) // On Windows, PyMoDINIT_FUNC already defines dllexport
PyMODINIT_FUNC
#else
PyMODINIT_FUNC ICE_DECLSPEC_EXPORT
#endif
PyInit_IcePy(void)
{
    Ice::registerIceDiscovery(false);
    Ice::registerIceLocatorDiscovery(false);

    // Create the IcePy  module.
    PyObject* module{PyModule_Create(&iceModule)};

    // Initialize the IcePy built-in types.
    if (!initProxy(module) || !initTypes(module) || !initProperties(module) || !initPropertiesAdmin(module) ||
        !initExecutor(module) || !initBatchRequest(module) || !initCommunicator(module) || !initObjectAdapter(module) ||
        !initOperation(module) || !initLogger(module) || !initConnection(module) || !initConnectionInfo(module) ||
        !initImplicitContext(module) || !initEndpoint(module) || !initEndpointInfo(module) ||
        !initValueFactoryManager(module))
    {
        return nullptr;
    }
    return module;
}

extern "C" void
IcePy_cleanup(void* /*self*/)
{
    cleanupLogger();
}
