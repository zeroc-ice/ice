// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICEPY_OBJECT_ADAPTER_H
#define ICEPY_OBJECT_ADAPTER_H

#include <Config.h>
#include <Ice/ObjectAdapterF.h>

namespace IcePy
{

extern PyTypeObject ObjectAdapterType;

bool initObjectAdapter(PyObject*);

PyObject* createObjectAdapter(const Ice::ObjectAdapterPtr&);
Ice::ObjectAdapterPtr getObjectAdapter(PyObject*);

PyObject* wrapObjectAdapter(const Ice::ObjectAdapterPtr&);
Ice::ObjectAdapterPtr unwrapObjectAdapter(PyObject*);

}

#endif
