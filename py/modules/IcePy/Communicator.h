// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPY_COMMUNICATOR_H
#define ICEPY_COMMUNICATOR_H

#include <Config.h>
#include <Ice/CommunicatorF.h>

namespace IcePy
{

extern PyTypeObject CommunicatorType;

bool initCommunicator(PyObject*);

Ice::CommunicatorPtr getCommunicator(PyObject*);

PyObject* createCommunicator(const Ice::CommunicatorPtr&);

}

extern "C" PyObject* IcePy_initialize(PyObject*, PyObject*);
extern "C" PyObject* IcePy_initializeWithProperties(PyObject*, PyObject*);

#endif
