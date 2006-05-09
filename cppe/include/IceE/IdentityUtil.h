// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_IDENTITY_UTIL_H
#define ICEE_IDENTITY_UTIL_H

#include <IceE/Identity.h>

namespace Ice
{

ICE_API ICE_DEPRECATED_API Identity stringToIdentity(const std::string&);
ICE_API ICE_DEPRECATED_API std::string identityToString(const Identity&);

}

#endif
