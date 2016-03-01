// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
