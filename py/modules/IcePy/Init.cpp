// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Communicator.h>
#include <Current.h>
#include <ObjectAdapter.h>
#include <Properties.h>
#include <Proxy.h>
#include <Slice.h>
#include <Types.h>

using namespace std;
using namespace IcePy;

static PyMethodDef methods[] =
{
    { "initialize", (PyCFunction)Ice_initialize, METH_VARARGS,
        PyDoc_STR("initialize([args]) -> Ice.Communicator") },
    { "initializeWithProperties", (PyCFunction)Ice_initializeWithProperties, METH_VARARGS,
        PyDoc_STR("initializeWithProperties(args, properties) -> Ice.Communicator") },
    { "identityToString", (PyCFunction)Ice_identityToString, METH_VARARGS,
        PyDoc_STR("identityToString(id) -> string") },
    { "stringToIdentity", (PyCFunction)Ice_stringToIdentity, METH_VARARGS,
        PyDoc_STR("stringToIdentity(str) -> Ice.Identity") },
    { "createProperties", (PyCFunction)Ice_createProperties, METH_VARARGS,
        PyDoc_STR("createProperties([args]) -> Ice.Properties") },
    { "getDefaultProperties", (PyCFunction)Ice_getDefaultProperties, METH_VARARGS,
        PyDoc_STR("getDefaultProperties([args]) -> Ice.Properties") },
    { "defineEnum", (PyCFunction)Ice_defineEnum, METH_VARARGS,
        PyDoc_STR("internal function") },
    { "defineStruct", (PyCFunction)Ice_defineStruct, METH_VARARGS,
        PyDoc_STR("internal function") },
    { "defineSequence", (PyCFunction)Ice_defineSequence, METH_VARARGS,
        PyDoc_STR("internal function") },
    { "defineDictionary", (PyCFunction)Ice_defineDictionary, METH_VARARGS,
        PyDoc_STR("internal function") },
    { "declareProxy", (PyCFunction)Ice_declareProxy, METH_VARARGS,
        PyDoc_STR("internal function") },
    { "defineProxy", (PyCFunction)Ice_defineProxy, METH_VARARGS,
        PyDoc_STR("internal function") },
    { "declareClass", (PyCFunction)Ice_declareClass, METH_VARARGS,
        PyDoc_STR("internal function") },
    { "defineClass", (PyCFunction)Ice_defineClass, METH_VARARGS,
        PyDoc_STR("internal function") },
    { "defineException", (PyCFunction)Ice_defineException, METH_VARARGS,
        PyDoc_STR("internal function") },
    { "loadSlice", (PyCFunction)Ice_loadSlice, METH_VARARGS,
        PyDoc_STR("loadSlice(cmd) -> None") },
    { NULL, NULL} /* sentinel */
};

PyDoc_STRVAR(moduleDoc, "The Internet Communications Engine.");

PyMODINIT_FUNC
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
    module = Py_InitModule3("IcePy", methods, moduleDoc);

    //
    // Install built-in Ice types.
    //
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
    if(!initProxy(module))
    {
        return;
    }
    if(!initTypes(module))
    {
        return;
    }
}
