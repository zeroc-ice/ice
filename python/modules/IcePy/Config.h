//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICEPY_CONFIG_H
#define ICEPY_CONFIG_H

//
// This file includes <Python.h> and should always be included first,
// see http://www.python.org/doc/api/includes.html for the details.
//

#ifdef _WIN32
// suppress C4100: '_unused_op': unreferenced formal parameter in Python 3.12 cpython/unicodeobject.h
#    pragma warning(disable : 4100)
#endif

#include <Python.h>

#ifdef _WIN32
#    pragma warning(default : 4100)
#endif

#endif
