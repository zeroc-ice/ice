// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_IDENTITY_UTIL_H
#define ICE_IDENTITY_UTIL_H

#include <Ice/Identity.h>

namespace Ice
{

std::ostream& operator<<(std::ostream&, const Identity&);
Identity stringToIdentity(const std::string&);
std::string identityToString(const Identity&);

}

#endif
