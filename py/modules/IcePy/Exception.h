// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPY_EXCEPTION_H
#define ICEPY_EXCEPTION_H

#include <Python.h>
#include <Ice/Exception.h>

namespace IcePy
{

PyObject* getPythonException(bool = true);

PyObject* createExceptionInstance(PyObject*);

void setPythonException(const Ice::Exception&);

void throwPythonException(PyObject* = NULL);

void handleSystemExit();

}

#endif
