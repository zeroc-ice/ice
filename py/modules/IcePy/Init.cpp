// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifdef _WIN32
#   include <IceUtil/Config.h>
#endif
#include <Communicator.h>
#include <Connection.h>
#include <Current.h>
#include <Logger.h>
#include <ObjectAdapter.h>
#include <Operation.h>
#include <Properties.h>
#include <Proxy.h>
#include <Slice.h>
#include <Types.h>

using namespace std;
using namespace IcePy;

extern "C" PyObject* Ice_registerTypes(PyObject*, PyObject*);

static PyMethodDef methods[] =
{
    { STRCAST("initialize"), (PyCFunction)IcePy_initialize, METH_VARARGS,
        PyDoc_STR(STRCAST("initialize([args]) -> Ice.Communicator")) },
    { STRCAST("initializeWithProperties"), (PyCFunction)IcePy_initializeWithProperties, METH_VARARGS,
        PyDoc_STR(STRCAST("initializeWithProperties(args, properties) -> Ice.Communicator")) },
    { STRCAST("identityToString"), (PyCFunction)IcePy_identityToString, METH_VARARGS,
        PyDoc_STR(STRCAST("identityToString(id) -> string")) },
    { STRCAST("stringToIdentity"), (PyCFunction)IcePy_stringToIdentity, METH_VARARGS,
        PyDoc_STR(STRCAST("stringToIdentity(str) -> Ice.Identity")) },
    { STRCAST("generateUUID"), (PyCFunction)IcePy_generateUUID, METH_NOARGS,
        PyDoc_STR(STRCAST("generateUUID() -> string")) },
    { STRCAST("createProperties"), (PyCFunction)IcePy_createProperties, METH_VARARGS,
        PyDoc_STR(STRCAST("createProperties([args]) -> Ice.Properties")) },
    { STRCAST("getDefaultProperties"), (PyCFunction)IcePy_getDefaultProperties, METH_VARARGS,
        PyDoc_STR(STRCAST("getDefaultProperties([args]) -> Ice.Properties")) },
    { STRCAST("defineEnum"), (PyCFunction)IcePy_defineEnum, METH_VARARGS,
        PyDoc_STR(STRCAST("internal function")) },
    { STRCAST("defineStruct"), (PyCFunction)IcePy_defineStruct, METH_VARARGS,
        PyDoc_STR(STRCAST("internal function")) },
    { STRCAST("defineSequence"), (PyCFunction)IcePy_defineSequence, METH_VARARGS,
        PyDoc_STR(STRCAST("internal function")) },
    { STRCAST("defineDictionary"), (PyCFunction)IcePy_defineDictionary, METH_VARARGS,
        PyDoc_STR(STRCAST("internal function")) },
    { STRCAST("declareProxy"), (PyCFunction)IcePy_declareProxy, METH_VARARGS,
        PyDoc_STR(STRCAST("internal function")) },
    { STRCAST("defineProxy"), (PyCFunction)IcePy_defineProxy, METH_VARARGS,
        PyDoc_STR(STRCAST("internal function")) },
    { STRCAST("declareClass"), (PyCFunction)IcePy_declareClass, METH_VARARGS,
        PyDoc_STR(STRCAST("internal function")) },
    { STRCAST("defineClass"), (PyCFunction)IcePy_defineClass, METH_VARARGS,
        PyDoc_STR(STRCAST("internal function")) },
    { STRCAST("defineException"), (PyCFunction)IcePy_defineException, METH_VARARGS,
        PyDoc_STR(STRCAST("internal function")) },
    { STRCAST("stringify"), (PyCFunction)IcePy_stringify, METH_VARARGS,
        PyDoc_STR(STRCAST("internal function")) },
    { STRCAST("stringifyException"), (PyCFunction)IcePy_stringifyException, METH_VARARGS,
        PyDoc_STR(STRCAST("internal function")) },
    { STRCAST("loadSlice"), (PyCFunction)IcePy_loadSlice, METH_VARARGS,
        PyDoc_STR(STRCAST("loadSlice(cmd) -> None")) },
    { NULL, NULL} /* sentinel */
};

PyDoc_STRVAR(moduleDoc, "The Internet Communications Engine.");

#if defined(__SUNPRO_CC) && (__SUNPRO_CC >= 0x550)
extern "C" __global void
#else
PyMODINIT_FUNC
#endif
initIcePy(void)
{
    PyObject* module;

    //
    // Notify Python that we are a multi-threaded extension.
    //
    PyEval_InitThreads();

    //
    // Initialize the module.
    //
    module = Py_InitModule3(STRCAST("IcePy"), methods, moduleDoc);

    //
    // Install built-in Ice types.
    //
    if(!initProxy(module))
    {
        return;
    }
    if(!initTypes(module))
    {
        return;
    }
    if(!initProperties(module))
    {
        return;
    }
    if(!initCommunicator(module))
    {
        return;
    }
    if(!initCurrent(module))
    {
        return;
    }
    if(!initObjectAdapter(module))
    {
        return;
    }
    if(!initOperation(module))
    {
        return;
    }
    if(!initLogger(module))
    {
        return;
    }
    if(!initConnection(module))
    {
        return;
    }
}
