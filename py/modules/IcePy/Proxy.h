// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPY_PROXY_H
#define ICEPY_PROXY_H

#include <Config.h>
#include <Ice/ProxyF.h>
#include <Ice/CommunicatorF.h>

namespace IcePy
{

extern PyTypeObject ProxyType;

bool initProxy(PyObject*);

PyObject* createProxy(const Ice::ObjectPrx&, const Ice::CommunicatorPtr&, PyObject* = 0);

bool checkProxy(PyObject*);

Ice::ObjectPrx getProxy(PyObject*);
Ice::CommunicatorPtr getProxyCommunicator(PyObject*);

}

#endif
