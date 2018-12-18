// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPY_PROPERTIES_ADMIN_H
#define ICEPY_PROPERTIES_ADMIN_H

#include <Config.h>
#include <Ice/NativePropertiesAdmin.h>

namespace IcePy
{

extern PyTypeObject NativePropertiesAdminType;

bool initPropertiesAdmin(PyObject*);

PyObject* createNativePropertiesAdmin(const Ice::NativePropertiesAdminPtr&);

}

#endif
