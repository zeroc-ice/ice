// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef UTIL_H
#define UTIL_H

#include <string>

#include <Ice/Ice.h>

namespace IceStormInternal
{

std::string
describeEndpoints(const Ice::ObjectPrx&);

}
#endif
