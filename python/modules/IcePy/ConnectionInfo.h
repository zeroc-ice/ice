//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICEPY_CONNECTION_INFO_H
#define ICEPY_CONNECTION_INFO_H

#include <Config.h>
#include <Ice/Connection.h>

namespace IcePy
{

extern PyTypeObject ConnectionInfoType;

bool initConnectionInfo(PyObject*);

PyObject* createConnectionInfo(const Ice::ConnectionInfoPtr&);
Ice::ConnectionInfoPtr getConnectionInfo(PyObject*);

}

#endif
