// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
