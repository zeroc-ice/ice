// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPY_CURRENT_H
#define ICEPY_CURRENT_H

#include <Config.h>
#include <Ice/Current.h>

namespace IcePy
{

extern PyTypeObject CurrentType;

bool initCurrent(PyObject*);

PyObject* createCurrent(const Ice::Current&);

}

#endif
