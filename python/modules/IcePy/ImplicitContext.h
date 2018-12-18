// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICEPY_IMPLICIT_CONTEXT_H
#define ICEPY_IMPLICIT_CONTEXT_H

#include <Config.h>
#include <Ice/ImplicitContext.h>

namespace IcePy
{

extern PyTypeObject ImplicitContextType;

bool initImplicitContext(PyObject*);

PyObject* createImplicitContext(const Ice::ImplicitContextPtr&);

}

#endif
