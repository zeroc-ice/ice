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
#include <BatchRequestInterceptor.h>
#include <Communicator.h>
#include <Connection.h>
#include <ConnectionInfo.h>
#include <Current.h>
#include <Endpoint.h>
#include <EndpointInfo.h>
#include <ImplicitContext.h>
#include <Logger.h>
#include <ObjectAdapter.h>
#include <Operation.h>
#include <Properties.h>
#include <PropertiesAdmin.h>
#include <Proxy.h>
#include <Slice.h>
#include <Types.h>
#include <Ice/Initialize.h>

using namespace std;
using namespace IcePy;

extern "C" PyObject* IcePy_cleanup(PyObject*);

static PyMethodDef methods[] =
{
    { STRCAST("stringVersion"), reinterpret_cast<PyCFunction>(IcePy_stringVersion), METH_NOARGS,
        PyDoc_STR(STRCAST("stringVersion() -> string")) },
    { STRCAST("intVersion"), reinterpret_cast<PyCFunction>(IcePy_intVersion), METH_NOARGS,
        PyDoc_STR(STRCAST("intVersion() -> int")) },
    { STRCAST("currentProtocol"), reinterpret_cast<PyCFunction>(IcePy_currentProtocol), METH_NOARGS,
        PyDoc_STR(STRCAST("currentProtocol() -> Ice.ProtocolVersion")) },
    { STRCAST("currentProtocolEncoding"), reinterpret_cast<PyCFunction>(IcePy_currentProtocolEncoding), METH_NOARGS,
        PyDoc_STR(STRCAST("currentProtocolEncoding() -> Ice.EncodingVersion")) },
    { STRCAST("currentEncoding"), reinterpret_cast<PyCFunction>(IcePy_currentEncoding), METH_NOARGS,
        PyDoc_STR(STRCAST("currentEncoding() -> Ice.EncodingVersion")) },
    { STRCAST("stringToProtocolVersion"), reinterpret_cast<PyCFunction>(IcePy_stringToProtocolVersion), METH_VARARGS,
        PyDoc_STR(STRCAST("stringToProtocolVersion(str) -> Ice.ProtocolVersion")) },
    { STRCAST("protocolVersionToString"), reinterpret_cast<PyCFunction>(IcePy_protocolVersionToString), METH_VARARGS,
        PyDoc_STR(STRCAST("protocolVersionToString(Ice.ProtocolVersion) -> string")) },
    { STRCAST("stringToEncodingVersion"), reinterpret_cast<PyCFunction>(IcePy_stringToEncodingVersion), METH_VARARGS,
        PyDoc_STR(STRCAST("stringToEncodingVersion(str) -> Ice.EncodingVersion")) },
    { STRCAST("encodingVersionToString"), reinterpret_cast<PyCFunction>(IcePy_encodingVersionToString), METH_VARARGS,
        PyDoc_STR(STRCAST("encodingVersionToString(Ice.EncodingVersion) -> string")) },
    { STRCAST("generateUUID"), reinterpret_cast<PyCFunction>(IcePy_generateUUID), METH_NOARGS,
        PyDoc_STR(STRCAST("generateUUID() -> string")) },
    { STRCAST("createProperties"), reinterpret_cast<PyCFunction>(IcePy_createProperties), METH_VARARGS,
        PyDoc_STR(STRCAST("createProperties([args]) -> Ice.Properties")) },
    { STRCAST("stringToIdentity"), reinterpret_cast<PyCFunction>(IcePy_stringToIdentity), METH_O,
        PyDoc_STR(STRCAST("stringToIdentity(string) -> Ice.Identity")) },
    { STRCAST("identityToString"), reinterpret_cast<PyCFunction>(IcePy_identityToString), METH_O,
        PyDoc_STR(STRCAST("identityToString(Ice.Identity) -> string")) },
    { STRCAST("getProcessLogger"), reinterpret_cast<PyCFunction>(IcePy_getProcessLogger), METH_NOARGS,
        PyDoc_STR(STRCAST("getProcessLogger() -> Ice.Logger")) },
    { STRCAST("setProcessLogger"), reinterpret_cast<PyCFunction>(IcePy_setProcessLogger), METH_VARARGS,
        PyDoc_STR(STRCAST("setProcessLogger(logger) -> None")) },
    { STRCAST("defineEnum"), reinterpret_cast<PyCFunction>(IcePy_defineEnum), METH_VARARGS,
        PyDoc_STR(STRCAST("internal function")) },
    { STRCAST("defineStruct"), reinterpret_cast<PyCFunction>(IcePy_defineStruct), METH_VARARGS,
        PyDoc_STR(STRCAST("internal function")) },
    { STRCAST("defineSequence"), reinterpret_cast<PyCFunction>(IcePy_defineSequence), METH_VARARGS,
        PyDoc_STR(STRCAST("internal function")) },
    { STRCAST("defineCustom"), reinterpret_cast<PyCFunction>(IcePy_defineCustom), METH_VARARGS,
        PyDoc_STR(STRCAST("internal function")) },
    { STRCAST("defineDictionary"), reinterpret_cast<PyCFunction>(IcePy_defineDictionary), METH_VARARGS,
        PyDoc_STR(STRCAST("internal function")) },
    { STRCAST("declareProxy"), reinterpret_cast<PyCFunction>(IcePy_declareProxy), METH_VARARGS,
        PyDoc_STR(STRCAST("internal function")) },
    { STRCAST("defineProxy"), reinterpret_cast<PyCFunction>(IcePy_defineProxy), METH_VARARGS,
        PyDoc_STR(STRCAST("internal function")) },
    { STRCAST("declareClass"), reinterpret_cast<PyCFunction>(IcePy_declareClass), METH_VARARGS,
        PyDoc_STR(STRCAST("internal function")) },
    { STRCAST("defineClass"), reinterpret_cast<PyCFunction>(IcePy_defineClass), METH_VARARGS,
        PyDoc_STR(STRCAST("internal function")) },
    { STRCAST("defineException"), reinterpret_cast<PyCFunction>(IcePy_defineException), METH_VARARGS,
        PyDoc_STR(STRCAST("internal function")) },
    { STRCAST("stringify"), reinterpret_cast<PyCFunction>(IcePy_stringify), METH_VARARGS,
        PyDoc_STR(STRCAST("internal function")) },
    { STRCAST("stringifyException"), reinterpret_cast<PyCFunction>(IcePy_stringifyException), METH_VARARGS,
        PyDoc_STR(STRCAST("internal function")) },
    { STRCAST("loadSlice"), reinterpret_cast<PyCFunction>(IcePy_loadSlice), METH_VARARGS,
        PyDoc_STR(STRCAST("loadSlice(cmd) -> None")) },
    { STRCAST("cleanup"), reinterpret_cast<PyCFunction>(IcePy_cleanup), METH_NOARGS,
        PyDoc_STR(STRCAST("internal function")) },
    { STRCAST("compile"), reinterpret_cast<PyCFunction>(IcePy_compile), METH_VARARGS,
        PyDoc_STR(STRCAST("internal function")) },
    { 0, 0 } /* sentinel */
};

#if PY_VERSION_HEX >= 0x03000000

#   define INIT_RETURN return(0)

static struct PyModuleDef iceModule =
{
    PyModuleDef_HEAD_INIT,
    "IcePy",
    "The Internet Communications Engine.",
    -1,
    methods,
    NULL,
    NULL,
    NULL,
    NULL
};

#else

#   define INIT_RETURN return

PyDoc_STRVAR(moduleDoc, "The Internet Communications Engine.");

#endif

#ifdef ICE_STATIC_LIBS
extern "C"
{
Ice::Plugin* createIceSSL(const Ice::CommunicatorPtr&, const std::string&, const Ice::StringSeq&);
Ice::Plugin* createIceDiscovery(const Ice::CommunicatorPtr&, const string&, const Ice::StringSeq&);
Ice::Plugin* createIceLocatorDiscovery(const Ice::CommunicatorPtr&, const string&, const Ice::StringSeq&);
}
#endif

#if defined(__GNUC__) && PY_VERSION_HEX >= 0x03000000
extern "C" __attribute__((visibility ("default"))) PyObject *
#elif defined(_WIN32) // On Windows, PyMoDINIT_FUNC already defines dllexport
PyMODINIT_FUNC
#else
PyMODINIT_FUNC ICE_DECLSPEC_EXPORT
#endif
#if PY_VERSION_HEX >= 0x03000000
PyInit_IcePy(void)
#else
initIcePy(void)
#endif
{
    PyObject* module;

#ifdef ICE_STATIC_LIBS
    // Register the plugins manually if we're building with static libraries.
    Ice::registerPluginFactory("IceSSL", createIceSSL, false);
    Ice::registerPluginFactory("IceDiscovery", createIceDiscovery, false);
    Ice::registerPluginFactory("IceLocatorDiscovery", createIceLocatorDiscovery, false);
#endif

    //
    // Notify Python that we are a multi-threaded extension.
    //
    PyEval_InitThreads();

#if PY_VERSION_HEX >= 0x03000000
    //
    // Create the module.
    //
    module = PyModule_Create(&iceModule);
#else
    //
    // Initialize the module.
    //
    module = Py_InitModule3(STRCAST("IcePy"), methods, moduleDoc);
#endif

    //
    // Install built-in Ice types.
    //
    if(!initProxy(module))
    {
        INIT_RETURN;
    }
    if(!initTypes(module))
    {
        INIT_RETURN;
    }
    if(!initProperties(module))
    {
        INIT_RETURN;
    }
    if(!initPropertiesAdmin(module))
    {
        INIT_RETURN;
    }
    if(!initBatchRequest(module))
    {
        INIT_RETURN;
    }
    if(!initCommunicator(module))
    {
        INIT_RETURN;
    }
    if(!initCurrent(module))
    {
        INIT_RETURN;
    }
    if(!initObjectAdapter(module))
    {
        INIT_RETURN;
    }
    if(!initOperation(module))
    {
        INIT_RETURN;
    }
    if(!initLogger(module))
    {
        INIT_RETURN;
    }
    if(!initConnection(module))
    {
        INIT_RETURN;
    }
    if(!initConnectionInfo(module))
    {
        INIT_RETURN;
    }
    if(!initImplicitContext(module))
    {
        INIT_RETURN;
    }
    if(!initEndpoint(module))
    {
        INIT_RETURN;
    }
    if(!initEndpointInfo(module))
    {
        INIT_RETURN;
    }

#if PY_VERSION_HEX >= 0x03000000
    return module;
#endif
}

extern "C"
PyObject*
IcePy_cleanup(PyObject* /*self*/)
{
    cleanupLogger();

    Py_INCREF(Py_None);
    return Py_None;
}
