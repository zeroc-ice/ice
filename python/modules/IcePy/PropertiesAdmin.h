// Copyright (c) ZeroC, Inc.

#ifndef ICEPY_PROPERTIES_ADMIN_H
#define ICEPY_PROPERTIES_ADMIN_H

#include "Config.h"
#include "Ice/NativePropertiesAdmin.h"

namespace IcePy
{
    extern PyTypeObject NativePropertiesAdminType;

    bool initPropertiesAdmin(PyObject*);

    PyObject* createNativePropertiesAdmin(const Ice::NativePropertiesAdminPtr&);
}

#endif
