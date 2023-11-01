//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICEPY_CONFIG_H
#define ICEPY_CONFIG_H

//
// This file includes <Python.h> and should always be included first,
// see http://www.python.org/doc/api/includes.html for the details.
//

//
// COMPILERFIX: This is required to prevent annoying warnings with aCC.
// The aCC -mt option causes the definition of the _POSIX_C_SOURCE macro
// (with another lower value.) and this is causing a warning because of
// the redefinition.
//
#if defined(__HP_aCC) && defined(_POSIX_C_SOURCE)
#    undef _POSIX_C_SOURCE
#endif

#ifdef _WIN32
    // suppress C4100: '_unused_op': unreferenced formal parameter in Python 3.12 cpython/unicodeobject.h
#   pragma warning( disable : 4100)
#endif

#include <Python.h>

#ifdef _WIN32
#   pragma warning( default : 4100)
#endif

#ifdef STRCAST
#   error "STRCAST already defined!"
#endif
#define STRCAST(s) const_cast<char*>(s)

#endif
