//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
