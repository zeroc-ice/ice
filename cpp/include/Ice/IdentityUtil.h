// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

ICE_API ICE_DEPRECATED_API Identity stringToIdentity(const std::string&);
ICE_API ICE_DEPRECATED_API std::string identityToString(const Identity&);

}

#endif
