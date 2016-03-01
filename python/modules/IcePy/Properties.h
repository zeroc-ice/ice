// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
