// Copyright (c) ZeroC, Inc.

#ifndef ICEPY_COMMUNICATOR_H
#define ICEPY_COMMUNICATOR_H

#include "Config.h"
#include "Ice/CommunicatorF.h"
#include "Ice/SliceLoader.h"

namespace IcePy
{
    extern PyTypeObject CommunicatorType;

    bool initCommunicator(PyObject*);

    Ice::CommunicatorPtr getCommunicator(PyObject*);
    Ice::SliceLoaderPtr getSliceLoader(const Ice::CommunicatorPtr&);

    PyObject* createCommunicator(const Ice::CommunicatorPtr&);
    PyObject* getCommunicatorWrapper(const Ice::CommunicatorPtr&);
}

extern "C" PyObject* IcePy_identityToString(PyObject*, PyObject*);
extern "C" PyObject* IcePy_stringToIdentity(PyObject*, PyObject*);

#endif
