// Copyright (c) ZeroC, Inc.

#ifndef ICEPY_CURRENT_H
#define ICEPY_CURRENT_H

#include "Config.h"
#include "Ice/Current.h"

namespace IcePy
{
    PyObject* createCurrent(const Ice::Current&);
}

#endif
