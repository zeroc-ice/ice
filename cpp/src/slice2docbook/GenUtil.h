// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef GEN_UTIL_H
#define GEN_UTIL_H

#include <Parser.h>
#include <OutputUtil.h>

namespace Slice
{

std::string typeToString(const TypePtr&);
std::string addLink(const std::string&, const ContainerPtr&);
std::string scopedToFile(const std::string&);
std::string scopedToId(const std::string&);

}

#endif
