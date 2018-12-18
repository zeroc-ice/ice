// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
