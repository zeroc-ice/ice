// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

// Ice-E version 1.0.0
// Generated from file `ConnectionF.ice'

#ifndef __Ice_ConnectionF_h__
#define __Ice_ConnectionF_h__

#include <IceE/LocalObjectF.h>
#include <IceE/ProxyF.h>
#include <IceE/ObjectF.h>
#include <IceE/Exception.h>
#include <IceE/LocalObject.h>
#include <IceE/UndefSysMacros.h>

#ifndef ICEE_IGNORE_VERSION
#   if ICEE_INT_VERSION / 100 != 100
#       error Ice version mismatch!
#   endif
#   if ICEE_INT_VERSION % 100 < 0
#       error Ice patch level mismatch!
#   endif
#endif

#ifndef ICEE_API
#   ifdef ICEE_API_EXPORTS
#       define ICEE_API ICEE_DECLSPEC_EXPORT
#   else
#       define ICEE_API ICEE_DECLSPEC_IMPORT
#   endif
#endif

namespace Ice
{

class Connection;
ICEE_API bool operator==(const Connection&, const Connection&);
ICEE_API bool operator!=(const Connection&, const Connection&);
ICEE_API bool operator<(const Connection&, const Connection&);

}

namespace IceInternal
{

ICEE_API void incRef(::Ice::Connection*);
ICEE_API void decRef(::Ice::Connection*);

}

namespace Ice
{

typedef ::IceInternal::Handle< ::Ice::Connection> ConnectionPtr;

}

namespace Ice
{

}

#endif
