// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
