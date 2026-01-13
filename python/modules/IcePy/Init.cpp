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

Returns the Ice version in the form ``A.B.C``, where ``A`` indicates the major version,
``B`` indicates the minor version, and ``C`` indicates the patch level.

Returns
-------
str
    The Ice version.)";

    constexpr const char* IcePy_intVersion_doc = R"(intVersion() -> int

Returns the Ice version as an integer in the form ``AABBCC``, where ``AA`` indicates the major version,
``BB`` indicates the minor version, and ``CC`` indicates the patch level.
For example, for Ice 3.9.1, the returned value is 30901.

Returns
-------
int
    The Ice version.)";

    constexpr const char* IcePy_createProperties_doc =
        R"(createProperties(args: list[str] | None = None, defaults: Ice.Properties | None = None) -> Ice.Properties

Creates a property set initialized from command-line arguments and a default property set.

Parameters
----------
args : list[str] | None, optional
    The command-line arguments.
defaults : Properties | None, optional
    Default values for the new property set.

Returns
-------
Ice.Properties
    A new property set.)";

    constexpr const char* IcePy_stringToIdentity_doc = R"(stringToIdentity(str: str) -> Ice.Identity

Converts a stringified identity into an Identity.

Parameters
----------
str : str
    The stringified identity.

Returns
-------
Ice.Identity
    An Identity created from the provided string.

Raises
------
ParseException
    If the string cannot be converted to an object identity.)";

    constexpr const char* IcePy_identityToString_doc =
        R"(identityToString(identity: Ice.Identity, toStringMode: Ice.ToStringMode | None = None) -> str

Converts an Identity into a string using the specified mode.

Parameters
----------
identity : Ice.Identity
    The identity.
toStringMode : Ice.ToStringMode | None, optional
    Specifies how to handle non-ASCII characters and non-printable ASCII characters.
    The default is :const:`Ice.ToStringMode.Unicode`.

Returns
-------
str
    The stringified identity.)";

    constexpr const char* IcePy_getProcessLogger_doc = R"(getProcessLogger() -> Ice.Logger

Gets the per-process logger. This logger is used by all communicators that do not have their own specific logger
configured at the time the communicator is created.

Returns
-------
Ice.Logger
    The current per-process logger instance.)";

    constexpr const char* IcePy_setProcessLogger_doc = R"(setProcessLogger(logger: Ice.Logger) -> None

Sets the per-process logger. This logger is used by all communicators that do not have their own specific logger
configured at the time the communicator is created.

Parameters
----------
logger : Ice.Logger
    The new per-process logger instance.)";

    constexpr const char* IcePy_loadSlice_doc = R"(loadSlice(args: list[str]) -> None

Compiles Slice definitions and loads the generated code directly into the current Python environment.

This function does not generate any Python source files. Instead, the generated Python code is loaded directly into the
running interpreter.

This function does not generate any code for Slice files included by the Slice files being loaded. It is the caller's
responsibility to load all necessary Slice definitions. This can be done in a single call to :func:`loadSlice` by
providing all Slice files (including included files) in the `args` parameter, or by making multiple calls to
:func:`loadSlice`.

When :func:`loadSlice` is called multiple times with the same Slice file, the corresponding Python code is not reloaded.

Parameters
----------
args : list[str]
    The list of command-line arguments for the Slice loader. These arguments may include both compiler options and
    the Slice files to compile.

    Supported compiler options:

        - `-DNAME` Define NAME as 1.
        - `-DNAME=DEF` Define NAME as DEF.
        - `-UNAME` Remove any definition for NAME.
        - `-IDIR` Put DIR in the include file search path.
        - `-d`, `--debug`  Print debug messages.

Raises
------
RuntimeError
    If an error occurs during Slice parsing or compilation.)";

    constexpr const char* IcePy_compileSlice_doc = R"(compileSlice(args: list[str]) -> int

Compiles Slice definitions. The behavior is identical to that of the `slice2py` compiler.

Slice compilation errors and warnings are printed to standard error (stderr).

This is an internal function used in the implementation of the `slice2py` Python script included in the Ice Python package.

Parameters
----------
args : list[str]
    The list of command-line arguments for Slice compilation, following the same syntax as the `slice2py` compiler.

Returns
-------
int
    The exit code: 0 indicates success, and a non-zero value indicates failure.)";

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
