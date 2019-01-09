// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICEPY_PROPERTIES_H
#define ICEPY_PROPERTIES_H

#include <Config.h>
#include <Ice/PropertiesF.h>

namespace IcePy
{

extern PyTypeObject PropertiesType;

bool initProperties(PyObject*);

PyObject* createProperties(const Ice::PropertiesPtr&);

Ice::PropertiesPtr getProperties(PyObject*);

}

extern "C" PyObject* IcePy_createProperties(PyObject*, PyObject*);

#endif
