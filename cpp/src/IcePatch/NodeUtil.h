// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PATCH_NODE_UTIL_H
#define ICE_PATCH_NODE_UTIL_H

#include <Ice/Ice.h>
#include <IcePatch/Node.h>

namespace IcePatch
{

Ice::ByteSeq getMD5(const std::string&);
std::string MD5ToString(const Ice::ByteSeq&);

}

#endif
