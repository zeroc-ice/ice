// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
#include <ImplicitContext.h>
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
    { STRCAST("identityToString"), reinterpret_cast<PyCFunction>(IcePy_identityToString), METH_VARARGS,
        PyDoc_STR(STRCAST("identityToString(id) -> string")) },
    { STRCAST("stringToIdentity"), reinterpret_cast<PyCFunction>(IcePy_stringToIdentity), METH_VARARGS,
        PyDoc_STR(STRCAST("stringToIdentity(str) -> Ice.Identity")) },
    { STRCAST("generateUUID"), reinterpret_cast<PyCFunction>(IcePy_generateUUID), METH_NOARGS,
        PyDoc_STR(STRCAST("generateUUID() -> string")) },
    { STRCAST("createProperties"), reinterpret_cast<PyCFunction>(IcePy_createProperties), METH_VARARGS,
        PyDoc_STR(STRCAST("createProperties([args]) -> Ice.Properties")) },
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
    { 0, 0 } /* sentinel */
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
    if(!initImplicitContext(module))
    {
        return;
    }
}
