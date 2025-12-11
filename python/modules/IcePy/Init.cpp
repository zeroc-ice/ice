// Copyright (c) ZeroC, Inc.

#include "BatchRequestInterceptor.h"
#include "Communicator.h"
#include "Connection.h"
#include "ConnectionInfo.h"
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
    constexpr const char* IcePy_stringVersion_doc = R"(stringVersion() -> str

Returns the Ice version as a string.

Returns
-------
str
    The Ice version in the format 'A.B.C' where A is the major version,
    B is the minor version, and C is the patch version.)";

    constexpr const char* IcePy_intVersion_doc = R"(intVersion() -> int

Returns the Ice version as an integer.

Returns
-------
int
    The Ice version as an integer for version comparison.)";

    constexpr const char* IcePy_createProperties_doc =
        R"(createProperties(args: list[str] | None = None) -> Ice.Properties

Creates a new Properties instance.

Parameters
----------
args : list or None, optional
    Command-line arguments used to set properties. If provided, these
    are parsed to extract property settings.

Returns
-------
Ice.Properties
    A new Properties object.)";

    constexpr const char* IcePy_stringToIdentity_doc = R"(stringToIdentity(s: str) -> Ice.Identity

Converts a string into an identity.

Parameters
----------
s : str
    The string representation of an identity.

Returns
-------
Ice.Identity
    The identity.

Raises
------
IdentityParseException
    If the string is not a valid identity.)";

    constexpr const char* IcePy_identityToString_doc =
        R"(identityToString(ident: Ice.Identity, mode: Ice.ToStringMode = Ice.ToStringMode.Unicode) -> str

Converts an identity into a string.

Parameters
----------
ident : Ice.Identity
    The identity to convert.
mode : Ice.ToStringMode, optional
    Specifies how to encode non-ASCII characters. The default is
    Ice.ToStringMode.Unicode.

Returns
-------
str
    The string representation of the identity.)";

    constexpr const char* IcePy_getProcessLogger_doc = R"(getProcessLogger() -> Ice.Logger

Returns the process logger. This is the logger used by the Ice
run time unless the application sets a different logger in
InitializationData for a communicator.

Returns
-------
Ice.Logger
    The process logger.)";

    constexpr const char* IcePy_setProcessLogger_doc = R"(setProcessLogger(logger: Ice.Logger) -> None

Sets the process logger. This logger is used by the Ice run time
unless the application sets a different logger in
InitializationData for a communicator.

Parameters
----------
logger : Ice.Logger
    The new process logger.)";

    constexpr const char* IcePy_loadSlice_doc = R"(loadSlice(args: list[str]) -> None

Dynamically loads Slice definitions from one or more files and generates the corresponding Python code.

Parameters
----------
args : list[str]
    List of command-line arguments for Slice compilation, following
    the same syntax as the slice2py compiler.)";

    constexpr const char* IcePy_compileSlice_doc = R"(compileSlice(args: list[str]) -> int

Compiles Slice definitions. The behavior is identical to the
slice2py compiler. This function does not generate Python code
dynamically; it only processes Slice files according to the
compilation options provided.

Parameters
----------
args : list[str]
    List of command-line arguments for Slice compilation, following
    the same syntax as the slice2py compiler.

Returns
-------
int
    Exit code. 0 indicates success, non-zero indicates failure.)";

    unsigned long mainThreadId;
}

static PyMethodDef methods[] = {
    {"stringVersion",
     reinterpret_cast<PyCFunction>(IcePy_stringVersion),
     METH_NOARGS,
     PyDoc_STR(IcePy_stringVersion_doc)},
    {"intVersion", reinterpret_cast<PyCFunction>(IcePy_intVersion), METH_NOARGS, PyDoc_STR(IcePy_intVersion_doc)},
    {"createProperties",
     reinterpret_cast<PyCFunction>(IcePy_createProperties),
     METH_VARARGS,
     PyDoc_STR(IcePy_createProperties_doc)},
    {"stringToIdentity",
     reinterpret_cast<PyCFunction>(IcePy_stringToIdentity),
     METH_O,
     PyDoc_STR(IcePy_stringToIdentity_doc)},
    {"identityToString",
     reinterpret_cast<PyCFunction>(IcePy_identityToString),
     METH_VARARGS,
     PyDoc_STR(IcePy_identityToString_doc)},
    {"getProcessLogger",
     reinterpret_cast<PyCFunction>(IcePy_getProcessLogger),
     METH_NOARGS,
     PyDoc_STR(IcePy_getProcessLogger_doc)},
    {"setProcessLogger",
     reinterpret_cast<PyCFunction>(IcePy_setProcessLogger),
     METH_VARARGS,
     PyDoc_STR(IcePy_setProcessLogger_doc)},
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
    {"loadSlice", reinterpret_cast<PyCFunction>(IcePy_loadSlice), METH_VARARGS, PyDoc_STR(IcePy_loadSlice_doc)},
    {"compileSlice",
     reinterpret_cast<PyCFunction>(IcePy_compileSlice),
     METH_VARARGS,
     PyDoc_STR(IcePy_compileSlice_doc)},
    {} /* sentinel */
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
    // Create the IcePy  module.
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
