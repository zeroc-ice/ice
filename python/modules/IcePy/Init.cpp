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

static PyMethodDef methods[] = {
    {"stringVersion",
     reinterpret_cast<PyCFunction>(IcePy_stringVersion),
     METH_NOARGS,
     PyDoc_STR("stringVersion() -> str\n\n"
               "Returns the Ice version as a string.\n\n"
               "Returns\n"
               "-------\n"
               "str\n"
               "    The Ice version in the format 'A.B.C' where A is the major version,\n"
               "    B is the minor version, and C is the patch version.")},
    {"intVersion",
     reinterpret_cast<PyCFunction>(IcePy_intVersion),
     METH_NOARGS,
     PyDoc_STR("intVersion() -> int\n\n"
               "Returns the Ice version as an integer.\n\n"
               "Returns\n"
               "-------\n"
               "int\n"
               "    The Ice version as an integer for version comparison.")},
    {"currentProtocol",
     reinterpret_cast<PyCFunction>(IcePy_currentProtocol),
     METH_NOARGS,
     PyDoc_STR("currentProtocol() -> Ice.ProtocolVersion\n\n"
               "Returns the latest protocol version.\n\n"
               "Returns\n"
               "-------\n"
               "Ice.ProtocolVersion\n"
               "    The latest protocol version supported by this Ice version.")},
    {"currentProtocolEncoding",
     reinterpret_cast<PyCFunction>(IcePy_currentProtocolEncoding),
     METH_NOARGS,
     PyDoc_STR("currentProtocolEncoding() -> Ice.EncodingVersion\n\n"
               "Returns the latest protocol encoding version.\n\n"
               "Returns\n"
               "-------\n"
               "Ice.EncodingVersion\n"
               "    The latest protocol encoding version.")},
    {"currentEncoding",
     reinterpret_cast<PyCFunction>(IcePy_currentEncoding),
     METH_NOARGS,
     PyDoc_STR("currentEncoding() -> Ice.EncodingVersion\n\n"
               "Returns the latest encoding version.\n\n"
               "Returns\n"
               "-------\n"
               "Ice.EncodingVersion\n"
               "    The latest encoding version.")},
    {"stringToProtocolVersion",
     reinterpret_cast<PyCFunction>(IcePy_stringToProtocolVersion),
     METH_VARARGS,
     PyDoc_STR("stringToProtocolVersion(version: str) -> Ice.ProtocolVersion\n\n"
               "Converts a string into a protocol version.\n\n"
               "Parameters\n"
               "----------\n"
               "version : str\n"
               "    The string containing a stringified protocol version in X.Y format.\n\n"
               "Returns\n"
               "-------\n"
               "Ice.ProtocolVersion\n"
               "    The protocol version.\n\n"
               "Raises\n"
               "------\n"
               "ParseException\n"
               "    If the given string is not in the X.Y format.")},
    {"protocolVersionToString",
     reinterpret_cast<PyCFunction>(IcePy_protocolVersionToString),
     METH_VARARGS,
     PyDoc_STR("protocolVersionToString(version: Ice.ProtocolVersion) -> str\n\n"
               "Converts a protocol version into a string.\n\n"
               "Parameters\n"
               "----------\n"
               "version : Ice.ProtocolVersion\n"
               "    The protocol version.\n\n"
               "Returns\n"
               "-------\n"
               "str\n"
               "    A string representing the protocol version.")},
    {"stringToEncodingVersion",
     reinterpret_cast<PyCFunction>(IcePy_stringToEncodingVersion),
     METH_VARARGS,
     PyDoc_STR("stringToEncodingVersion(version: str) -> Ice.EncodingVersion\n\n"
               "Converts a string into an encoding version.\n\n"
               "Parameters\n"
               "----------\n"
               "version : str\n"
               "    The string containing a stringified encoding version in X.Y format.\n\n"
               "Returns\n"
               "-------\n"
               "Ice.EncodingVersion\n"
               "    The encoding version.\n\n"
               "Raises\n"
               "------\n"
               "ParseException\n"
               "    If the given string is not in the X.Y format.")},
    {"encodingVersionToString",
     reinterpret_cast<PyCFunction>(IcePy_encodingVersionToString),
     METH_VARARGS,
     PyDoc_STR("encodingVersionToString(version: Ice.EncodingVersion) -> str\n\n"
               "Converts an encoding version into a string.\n\n"
               "Parameters\n"
               "----------\n"
               "version : Ice.EncodingVersion\n"
               "    The encoding version.\n\n"
               "Returns\n"
               "-------\n"
               "str\n"
               "    A string representing the encoding version.")},
    {"createProperties",
     reinterpret_cast<PyCFunction>(IcePy_createProperties),
     METH_VARARGS,
     PyDoc_STR("createProperties(args: list[str] | None = None) -> Ice.Properties\n\n"
               "Creates a new Properties instance.\n\n"
               "Parameters\n"
               "----------\n"
               "args : list or None, optional\n"
               "    Command-line arguments used to set properties. If provided, these\n"
               "    are parsed to extract property settings.\n\n"
               "Returns\n"
               "-------\n"
               "Ice.Properties\n"
               "    A new Properties object.")},
    {"stringToIdentity",
     reinterpret_cast<PyCFunction>(IcePy_stringToIdentity),
     METH_O,
     PyDoc_STR("stringToIdentity(s: str) -> Ice.Identity\n\n"
               "Converts a string into an identity.\n\n"
               "Parameters\n"
               "----------\n"
               "s : str\n"
               "    The string representation of an identity.\n\n"
               "Returns\n"
               "-------\n"
               "Ice.Identity\n"
               "    The identity.\n\n"
               "Raises\n"
               "------\n"
               "IdentityParseException\n"
               "    If the string is not a valid identity.")},
    {"identityToString",
     reinterpret_cast<PyCFunction>(IcePy_identityToString),
     METH_VARARGS,
     PyDoc_STR("identityToString(ident: Ice.Identity, mode: Ice.ToStringMode = Ice.ToStringMode.Unicode) -> str\n\n"
               "Converts an identity into a string.\n\n"
               "Parameters\n"
               "----------\n"
               "ident : Ice.Identity\n"
               "    The identity to convert.\n"
               "mode : Ice.ToStringMode, optional\n"
               "    Specifies how to encode non-ASCII characters. The default is\n"
               "    Ice.ToStringMode.Unicode.\n\n"
               "Returns\n"
               "-------\n"
               "str\n"
               "    The string representation of the identity.")},
    {"getProcessLogger",
     reinterpret_cast<PyCFunction>(IcePy_getProcessLogger),
     METH_NOARGS,
     PyDoc_STR("getProcessLogger() -> Ice.Logger\n\n"
               "Returns the process logger. This is the logger used by the Ice\n"
               "run time unless the application sets a different logger in\n"
               "InitializationData for a communicator.\n\n"
               "Returns\n"
               "-------\n"
               "Ice.Logger\n"
               "    The process logger.")},
    {"setProcessLogger",
     reinterpret_cast<PyCFunction>(IcePy_setProcessLogger),
     METH_VARARGS,
     PyDoc_STR("setProcessLogger(logger: Ice.Logger) -> None\n\n"
               "Sets the process logger. This logger is used by the Ice run time\n"
               "unless the application sets a different logger in\n"
               "InitializationData for a communicator.\n\n"
               "Parameters\n"
               "----------\n"
               "logger : Ice.Logger\n"
               "    The new process logger.")},
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
    {"loadSlice",
     reinterpret_cast<PyCFunction>(IcePy_loadSlice),
     METH_VARARGS,
     PyDoc_STR(
         "loadSlice(args: list[str]) -> None\n\n"
         "Dynamically loads Slice definitions from one or more files and generates the corresponding Python code.\n\n"
         "Parameters\n"
         "----------\n"
         "args : list[str]\n"
         "    List of command-line arguments for Slice compilation.")},
    {"compileSlice",
     reinterpret_cast<PyCFunction>(IcePy_compileSlice),
     METH_VARARGS,
     PyDoc_STR("compileSlice(args: list[str]) -> int\n\n"
               "Compiles Slice definitions. The behavior is identical to the\n"
               "slice2py compiler. This function does not generate Python code\n"
               "dynamically; it only processes Slice files according to the\n"
               "compilation options provided.\n\n"
               "Parameters\n"
               "----------\n"
               "args : list[str]\n"
               "    List of command-line arguments for Slice compilation, following\n"
               "    the same syntax as the slice2py compiler.\n\n"
               "Returns\n"
               "-------\n"
               "int\n"
               "    Exit code. 0 indicates success, non-zero indicates failure.")},
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

namespace
{
    unsigned long mainThreadId;
}

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
