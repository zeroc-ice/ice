// Copyright (c) ZeroC, Inc.

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

// PY_SSIZE_T_CLEAN is required for all # variants of formats (s#, y#, etc.).
// See https://docs.python.org/3/c-api/arg.html
#define PY_SSIZE_T_CLEAN

#include <Python.h>

#ifdef _WIN32
#    pragma warning(default : 4100)
#endif

#endif
