// Copyright (c) ZeroC, Inc.

#include "BatchRequestInterceptor.h"
#include "Communicator.h"
#include "Connection.h"
#include "ConnectionInfo.h"
#include "DocStrings.h"
#include "Endpoint.h"
#include "EndpointInfo.h"
#include "Executor.h"
#include "Ice/Initialize.h"
#include "ImplicitContext.h"
#include "Logger.h"
#include "ObjectAdapter.h"
#include "Operation.h"
#include "Properties.h"
#include "PropertiesAdmin.h"
#include "Proxy.h"
#include "Slice.h"
#include "Types.h"
#include "Util.h"

using namespace std;
using namespace IcePy;

extern "C" void IcePy_cleanup(void*);

namespace
{
    unsigned long mainThreadId;
}

static PyMethodDef methods[] = {
    {"stringVersion",
     reinterpret_cast<PyCFunction>(IcePy_stringVersion),
     METH_NOARGS,
     PyDoc_STR(IcePy_DOC_stringVersion)},
    {"intVersion", reinterpret_cast<PyCFunction>(IcePy_intVersion), METH_NOARGS, PyDoc_STR(IcePy_DOC_intVersion)},
    {"createProperties",
     reinterpret_cast<PyCFunction>(IcePy_createProperties),
     METH_VARARGS,
     PyDoc_STR(IcePy_DOC_createProperties)},
    {"stringToIdentity",
     reinterpret_cast<PyCFunction>(IcePy_stringToIdentity),
     METH_O,
     PyDoc_STR(IcePy_DOC_stringToIdentity)},
    {"identityToString",
     reinterpret_cast<PyCFunction>(IcePy_identityToString),
     METH_VARARGS,
     PyDoc_STR(IcePy_DOC_identityToString)},
    {"getProcessLogger",
     reinterpret_cast<PyCFunction>(IcePy_getProcessLogger),
     METH_NOARGS,
     PyDoc_STR(IcePy_DOC_getProcessLogger)},
    {"setProcessLogger",
     reinterpret_cast<PyCFunction>(IcePy_setProcessLogger),
     METH_VARARGS,
     PyDoc_STR(IcePy_DOC_setProcessLogger)},
    {"defineEnum", reinterpret_cast<PyCFunction>(IcePy_defineEnum), METH_VARARGS, PyDoc_STR(IcePy_DOC_defineEnum)},
    {"defineStruct",
     reinterpret_cast<PyCFunction>(IcePy_defineStruct),
     METH_VARARGS,
     PyDoc_STR(IcePy_DOC_defineStruct)},
    {"defineSequence",
     reinterpret_cast<PyCFunction>(IcePy_defineSequence),
     METH_VARARGS,
     PyDoc_STR(IcePy_DOC_defineSequence)},
    {"defineDictionary",
     reinterpret_cast<PyCFunction>(IcePy_defineDictionary),
     METH_VARARGS,
     PyDoc_STR(IcePy_DOC_defineDictionary)},
    {"declareProxy",
     reinterpret_cast<PyCFunction>(IcePy_declareProxy),
     METH_VARARGS,
     PyDoc_STR(IcePy_DOC_declareProxy)},
    {"defineProxy", reinterpret_cast<PyCFunction>(IcePy_defineProxy), METH_VARARGS, PyDoc_STR(IcePy_DOC_defineProxy)},
    {"declareValue",
     reinterpret_cast<PyCFunction>(IcePy_declareValue),
     METH_VARARGS,
     PyDoc_STR(IcePy_DOC_declareValue)},
    {"defineValue", reinterpret_cast<PyCFunction>(IcePy_defineValue), METH_VARARGS, PyDoc_STR(IcePy_DOC_defineValue)},
    {"defineException",
     reinterpret_cast<PyCFunction>(IcePy_defineException),
     METH_VARARGS,
     PyDoc_STR(IcePy_DOC_defineException)},
    {"loadSlice", reinterpret_cast<PyCFunction>(IcePy_loadSlice), METH_VARARGS, PyDoc_STR(IcePy_DOC_loadSlice)},
    {"compileSlice",
     reinterpret_cast<PyCFunction>(IcePy_compileSlice),
     METH_VARARGS,
     PyDoc_STR(IcePy_DOC_compileSlice)},
    {} /* sentinel */
};

static struct PyModuleDef iceModule = {
    PyModuleDef_HEAD_INIT,
    "IcePy",
    PyDoc_STR(IcePy_DOC_module),
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
    // Create the IcePy module.
    PyObject* module{PyModule_Create(&iceModule)};

    // Initialize the IcePy built-in types.
    if (!initProxy(module) || !initTypes(module) || !initProperties(module) || !initPropertiesAdmin(module) ||
        !initExecutor(module) || !initBatchRequest(module) || !initCommunicator(module) || !initObjectAdapter(module) ||
        !initOperation(module) || !initLogger(module) || !initConnection(module) || !initConnectionInfo(module) ||
        !initImplicitContext(module) || !initEndpoint(module) || !initEndpointInfo(module))
    {
        return nullptr;
    }
    mainThreadId = PyThread_get_thread_ident();
    return module;
}

bool
IcePy::isMainThread()
{
    return PyThread_get_thread_ident() == mainThreadId;
}

extern "C" void
IcePy_cleanup(void* /*self*/)
{
    cleanupLogger();
}
