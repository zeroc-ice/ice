// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPY_IDENTITY_H
#define ICEPY_IDENTITY_H

#include <Python.h>
#include <Ice/Identity.h>

namespace IcePy
{

PyObject* createIdentity(const Ice::Identity&);
bool checkIdentity(PyObject*);
bool setIdentity(PyObject*, const Ice::Identity&);
bool getIdentity(PyObject*, Ice::Identity&);

extern "C" PyObject* Ice_identityToString(PyObject*, PyObject*);
extern "C" PyObject* Ice_stringToIdentity(PyObject*, PyObject*);

}

#endif
