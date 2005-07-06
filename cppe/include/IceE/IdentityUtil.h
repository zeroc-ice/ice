// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_IDENTITY_UTIL_H
#define ICEE_IDENTITY_UTIL_H

#include <IceE/Identity.h>

namespace Ice
{

ICEE_API Identity stringToIdentity(const std::string&);
ICEE_API std::string identityToString(const Identity&);

}

#endif
