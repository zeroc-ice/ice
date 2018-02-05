// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPY_CONNECTION_H
#define ICEPY_CONNECTION_H

#include <Config.h>
#include <Ice/ConnectionF.h>
#include <Ice/CommunicatorF.h>

namespace IcePy
{

bool initConnection(PyObject*);

PyObject* createConnection(const Ice::ConnectionPtr&, const Ice::CommunicatorPtr&);

bool checkConnection(PyObject*);
bool getConnectionArg(PyObject*, const std::string&, const std::string&, Ice::ConnectionPtr&);

}

#endif
