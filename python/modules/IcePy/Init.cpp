//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "BatchRequestInterceptor.h"
#include "Communicator.h"
#include "Connection.h"
#include "ConnectionInfo.h"
#include "Current.h"
#include "Dispatcher.h"
#include "Endpoint.h"
#include "EndpointInfo.h"
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

using namespace std;
using namespace IcePy;

extern "C" void IcePy_cleanup(void*);

static PyMethodDef methods[] = {
    {STRCAST("stringVersion"),
     reinterpret_cast<PyCFunction>(IcePy_stringVersion),
     METH_NOARGS,
     PyDoc_STR(STRCAST("stringVersion() -> string"))},
    {STRCAST("intVersion"),
     reinterpret_cast<PyCFunction>(IcePy_intVersion),
     METH_NOARGS,
     PyDoc_STR(STRCAST("intVersion() -> int"))},
    {STRCAST("currentProtocol"),
     reinterpret_cast<PyCFunction>(IcePy_currentProtocol),
     METH_NOARGS,
     PyDoc_STR(STRCAST("currentProtocol() -> Ice.ProtocolVersion"))},
    {STRCAST("currentProtocolEncoding"),
     reinterpret_cast<PyCFunction>(IcePy_currentProtocolEncoding),
     METH_NOARGS,
     PyDoc_STR(STRCAST("currentProtocolEncoding() -> Ice.EncodingVersion"))},
    {STRCAST("currentEncoding"),
     reinterpret_cast<PyCFunction>(IcePy_currentEncoding),
     METH_NOARGS,
     PyDoc_STR(STRCAST("currentEncoding() -> Ice.EncodingVersion"))},
    {STRCAST("stringToProtocolVersion"),
     reinterpret_cast<PyCFunction>(IcePy_stringToProtocolVersion),
     METH_VARARGS,
     PyDoc_STR(STRCAST("stringToProtocolVersion(str) -> Ice.ProtocolVersion"))},
    {STRCAST("protocolVersionToString"),
     reinterpret_cast<PyCFunction>(IcePy_protocolVersionToString),
     METH_VARARGS,
     PyDoc_STR(STRCAST("protocolVersionToString(Ice.ProtocolVersion) -> string"))},
    {STRCAST("stringToEncodingVersion"),
     reinterpret_cast<PyCFunction>(IcePy_stringToEncodingVersion),
     METH_VARARGS,
     PyDoc_STR(STRCAST("stringToEncodingVersion(str) -> Ice.EncodingVersion"))},
    {STRCAST("encodingVersionToString"),
     reinterpret_cast<PyCFunction>(IcePy_encodingVersionToString),
     METH_VARARGS,
     PyDoc_STR(STRCAST("encodingVersionToString(Ice.EncodingVersion) -> string"))},
    {STRCAST("generateUUID"),
     reinterpret_cast<PyCFunction>(IcePy_generateUUID),
     METH_NOARGS,
     PyDoc_STR(STRCAST("generateUUID() -> string"))},
    {STRCAST("createProperties"),
     reinterpret_cast<PyCFunction>(IcePy_createProperties),
     METH_VARARGS,
     PyDoc_STR(STRCAST("createProperties([args]) -> Ice.Properties"))},
    {STRCAST("stringToIdentity"),
     reinterpret_cast<PyCFunction>(IcePy_stringToIdentity),
     METH_O,
     PyDoc_STR(STRCAST("stringToIdentity(string) -> Ice.Identity"))},
    {STRCAST("identityToString"),
     reinterpret_cast<PyCFunction>(IcePy_identityToString),
     METH_VARARGS,
     PyDoc_STR(STRCAST("identityToString(Ice.Identity, Ice.ToStringMode) -> string"))},
    {STRCAST("getProcessLogger"),
     reinterpret_cast<PyCFunction>(IcePy_getProcessLogger),
     METH_NOARGS,
     PyDoc_STR(STRCAST("getProcessLogger() -> Ice.Logger"))},
    {STRCAST("setProcessLogger"),
     reinterpret_cast<PyCFunction>(IcePy_setProcessLogger),
     METH_VARARGS,
     PyDoc_STR(STRCAST("setProcessLogger(logger) -> None"))},
    {STRCAST("defineEnum"),
     reinterpret_cast<PyCFunction>(IcePy_defineEnum),
     METH_VARARGS,
     PyDoc_STR(STRCAST("internal function"))},
    {STRCAST("defineStruct"),
     reinterpret_cast<PyCFunction>(IcePy_defineStruct),
     METH_VARARGS,
     PyDoc_STR(STRCAST("internal function"))},
    {STRCAST("defineSequence"),
     reinterpret_cast<PyCFunction>(IcePy_defineSequence),
     METH_VARARGS,
     PyDoc_STR(STRCAST("internal function"))},
    {STRCAST("defineCustom"),
     reinterpret_cast<PyCFunction>(IcePy_defineCustom),
     METH_VARARGS,
     PyDoc_STR(STRCAST("internal function"))},
    {STRCAST("defineDictionary"),
     reinterpret_cast<PyCFunction>(IcePy_defineDictionary),
     METH_VARARGS,
     PyDoc_STR(STRCAST("internal function"))},
    {STRCAST("declareProxy"),
     reinterpret_cast<PyCFunction>(IcePy_declareProxy),
     METH_VARARGS,
     PyDoc_STR(STRCAST("internal function"))},
    {STRCAST("defineProxy"),
     reinterpret_cast<PyCFunction>(IcePy_defineProxy),
     METH_VARARGS,
     PyDoc_STR(STRCAST("internal function"))},
    {STRCAST("declareClass"),
     reinterpret_cast<PyCFunction>(IcePy_declareClass),
     METH_VARARGS,
     PyDoc_STR(STRCAST("internal function"))},
    {STRCAST("defineClass"),
     reinterpret_cast<PyCFunction>(IcePy_defineClass),
     METH_VARARGS,
     PyDoc_STR(STRCAST("internal function"))},
    {STRCAST("declareValue"),
     reinterpret_cast<PyCFunction>(IcePy_declareValue),
     METH_VARARGS,
     PyDoc_STR(STRCAST("internal function"))},
    {STRCAST("defineValue"),
     reinterpret_cast<PyCFunction>(IcePy_defineValue),
     METH_VARARGS,
     PyDoc_STR(STRCAST("internal function"))},
    {STRCAST("defineException"),
     reinterpret_cast<PyCFunction>(IcePy_defineException),
     METH_VARARGS,
     PyDoc_STR(STRCAST("internal function"))},
    {STRCAST("stringify"),
     reinterpret_cast<PyCFunction>(IcePy_stringify),
     METH_VARARGS,
     PyDoc_STR(STRCAST("internal function"))},
    {STRCAST("stringifyException"),
     reinterpret_cast<PyCFunction>(IcePy_stringifyException),
     METH_VARARGS,
     PyDoc_STR(STRCAST("internal function"))},
    {STRCAST("loadSlice"),
     reinterpret_cast<PyCFunction>(IcePy_loadSlice),
     METH_VARARGS,
     PyDoc_STR(STRCAST("loadSlice(cmd) -> None"))},
    {STRCAST("compile"),
     reinterpret_cast<PyCFunction>(IcePy_compile),
     METH_VARARGS,
     PyDoc_STR(STRCAST("internal function"))},
    {0, 0} /* sentinel */
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
    PyObject* module;

    Ice::registerIceSSL(false);
    Ice::registerIceDiscovery(false);
    Ice::registerIceLocatorDiscovery(false);

#if PY_VERSION_HEX < 0x03090000
    //
    // Notify Python that we are a multi-threaded extension. This method is deprecated
    // and does nothing since Python 3.9
    //
    PyEval_InitThreads();
#endif

    //
    // Create the module.
    //
    module = PyModule_Create(&iceModule);

    //
    // Install built-in Ice types.
    //
    if (!initProxy(module))
    {
        return nullptr;
    }
    if (!initTypes(module))
    {
        return nullptr;
    }
    if (!initProperties(module))
    {
        return nullptr;
    }
    if (!initPropertiesAdmin(module))
    {
        return nullptr;
    }
    if (!initDispatcher(module))
    {
        return nullptr;
    }
    if (!initBatchRequest(module))
    {
        return nullptr;
    }
    if (!initCommunicator(module))
    {
        return nullptr;
    }
    if (!initCurrent(module))
    {
        return nullptr;
    }
    if (!initObjectAdapter(module))
    {
        return nullptr;
    }
    if (!initOperation(module))
    {
        return nullptr;
    }
    if (!initLogger(module))
    {
        return nullptr;
    }
    if (!initConnection(module))
    {
        return nullptr;
    }
    if (!initConnectionInfo(module))
    {
        return nullptr;
    }
    if (!initImplicitContext(module))
    {
        return nullptr;
    }
    if (!initEndpoint(module))
    {
        return nullptr;
    }
    if (!initEndpointInfo(module))
    {
        return nullptr;
    }
    if (!initValueFactoryManager(module))
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
