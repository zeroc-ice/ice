// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PATCH_UTIL_H
#define ICE_PATCH_UTIL_H

#include <Ice/Ice.h>

namespace IcePatch
{

std::string identityToPath(const Ice::Identity&);
Ice::Identity pathToIdentity(const std::string&);

Ice::StringSeq readDirectory(const std::string&);

Ice::ByteSeq getMD5(const std::string&);
std::string MD5ToString(const Ice::ByteSeq&);

}

#endif
