// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
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

ICE_API std::ostream& operator<<(std::ostream&, const Identity&);
ICE_API Identity stringToIdentity(const std::string&);
ICE_API std::string identityToString(const Identity&);

}

#endif
