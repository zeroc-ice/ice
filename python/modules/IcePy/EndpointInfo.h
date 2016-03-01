// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPY_ENDPOINT_INFO_H
#define ICEPY_ENDPOINT_INFO_H

#include <Config.h>
#include <Ice/Endpoint.h>

namespace IcePy
{

extern PyTypeObject EndpointInfoType;

bool initEndpointInfo(PyObject*);

PyObject* createEndpointInfo(const Ice::EndpointInfoPtr&);
Ice::EndpointInfoPtr getEndpointInfo(PyObject*);

}

#endif
