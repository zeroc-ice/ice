// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Exception.h>
#include <Communicator.h>
#include <Current.h>
#include <Identity.h>
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
    { "createProperties", (PyCFunction)Ice_createProperties, METH_NOARGS,
        PyDoc_STR("createProperties() -> Ice.Properties") },
    { "getDefaultProperties", (PyCFunction)Ice_getDefaultProperties, METH_VARARGS,
        PyDoc_STR("getDefaultProperties([args]) -> Ice.Properties") },
    { "addEnum", (PyCFunction)Ice_addEnum, METH_VARARGS,
        PyDoc_STR("internal function") },
    { "addStruct", (PyCFunction)Ice_addStruct, METH_VARARGS,
        PyDoc_STR("internal function") },
    { "addSequence", (PyCFunction)Ice_addSequence, METH_VARARGS,
        PyDoc_STR("internal function") },
    { "addDictionary", (PyCFunction)Ice_addDictionary, METH_VARARGS,
        PyDoc_STR("internal function") },
    { "addProxy", (PyCFunction)Ice_addProxy, METH_VARARGS,
        PyDoc_STR("internal function") },
    { "defineProxy", (PyCFunction)Ice_defineProxy, METH_VARARGS,
        PyDoc_STR("internal function") },
    { "addClass", (PyCFunction)Ice_addClass, METH_VARARGS,
        PyDoc_STR("internal function") },
    { "defineClass", (PyCFunction)Ice_defineClass, METH_VARARGS,
        PyDoc_STR("internal function") },
    { "addException", (PyCFunction)Ice_addException, METH_VARARGS,
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
