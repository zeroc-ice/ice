// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Identity.h>
#include <Util.h>
#include <Ice/IdentityUtil.h>

using namespace std;
using namespace IcePy;

static PyObject*
getIdentityType()
{
    PyObject* identityType = lookupType("Ice.Identity");
    assert(identityType);
    assert(PyType_Check(identityType));
    return identityType;
}

PyObject*
IcePy::createIdentity(const Ice::Identity& identity)
{
    PyObject* identityType = getIdentityType();

    PyObjectHandle obj = PyObject_CallObject(identityType, NULL);
    if(obj.get() == NULL)
    {
        return NULL;
    }

    if(!setIdentity(obj.get(), identity))
    {
        return NULL;
    }

    return obj.release();
}

bool
IcePy::checkIdentity(PyObject* p)
{
    PyObject* identityType = getIdentityType();
    return PyObject_IsInstance(p, identityType) == 1;
}

bool
IcePy::setIdentity(PyObject* p, const Ice::Identity& identity)
{
    assert(checkIdentity(p));
    PyObjectHandle name = PyString_FromString(const_cast<char*>(identity.name.c_str()));
    PyObjectHandle category = PyString_FromString(const_cast<char*>(identity.category.c_str()));
    if(name.get() == NULL || category.get() == NULL)
    {
        return false;
    }
    if(PyObject_SetAttrString(p, "name", name.get()) < 0 || PyObject_SetAttrString(p, "category", category.get()) < 0)
    {
        return false;
    }
    return true;
}

bool
IcePy::getIdentity(PyObject* p, Ice::Identity& identity)
{
    assert(checkIdentity(p));
    PyObjectHandle name = PyObject_GetAttrString(p, "name");
    PyObjectHandle category = PyObject_GetAttrString(p, "category");
    if(name.get() != NULL)
    {
        char* s = PyString_AsString(name.get());
        if(s == NULL)
        {
            PyErr_Format(PyExc_ValueError, "identity name must be a string");
            return false;
        }
        identity.name = s;
    }
    if(category.get() != NULL)
    {
        char* s = PyString_AsString(category.get());
        if(s == NULL)
        {
            PyErr_Format(PyExc_ValueError, "identity category must be a string");
            return false;
        }
        identity.category = s;
    }
    return true;
}

extern "C"
PyObject*
Ice_identityToString(PyObject* /*self*/, PyObject* args)
{
    PyObject* identityType = getIdentityType();
    PyObject* p;
    if(!PyArg_ParseTuple(args, "O!", identityType, &p))
    {
        return NULL;
    }

    Ice::Identity id;
    if(!getIdentity(p, id))
    {
        return NULL;
    }

    string s;
    try
    {
        s = Ice::identityToString(id);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }
    return PyString_FromString(const_cast<char*>(s.c_str()));
}

extern "C"
PyObject*
Ice_stringToIdentity(PyObject* /*self*/, PyObject* args)
{
    char* str;
    if(!PyArg_ParseTuple(args, "s", &str))
    {
        return NULL;
    }

    Ice::Identity id;
    try
    {
        id = Ice::stringToIdentity(str);
    }
    catch(const Ice::Exception& ex)
    {
        setPythonException(ex);
        return NULL;
    }

    return createIdentity(id);
}
