// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPY_ENDPOINT_H
#define ICEPY_ENDPOINT_H

#include <Config.h>
#include <Ice/Endpoint.h>

namespace IcePy
{

extern PyTypeObject EndpointType;

bool initEndpoint(PyObject*);

PyObject* createEndpoint(const Ice::EndpointPtr&);
Ice::EndpointPtr getEndpoint(PyObject*);

bool toEndpointSeq(PyObject*, Ice::EndpointSeq&);

}

#endif
