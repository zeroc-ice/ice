// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPY_LOGGER_H
#define ICEPY_LOGGER_H

#include <Config.h>
#include <Ice/LoggerF.h>

namespace IcePy
{

bool initLogger(PyObject*);

PyObject* createLogger(const Ice::LoggerPtr&);

Ice::LoggerPtr wrapLogger(PyObject*);

}

#endif
