// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPY_PROPERTIES_H
#define ICEPY_PROPERTIES_H

#include <Python.h>
#include <Ice/PropertiesF.h>

namespace IcePy
{

struct PropertiesObject
{
    PyObject_HEAD
    Ice::PropertiesPtr* properties;
};

extern PyTypeObject PropertiesType;

bool initProperties(PyObject*);

PyObject* createProperties(const Ice::PropertiesPtr&);

Ice::PropertiesPtr getProperties(PyObject*);

extern "C" PyObject* Ice_createProperties(PyObject*);
extern "C" PyObject* Ice_getDefaultProperties(PyObject*, PyObject*);

}

#endif
