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

std::string getScopedMinimized(const ContainedPtr&, const ContainerPtr&);
std::string toString(const SyntaxTreeBasePtr&, const ContainerPtr&);
std::string toString(const std::string&, const ContainerPtr&);
std::string scopedToId(const std::string&);

}

#endif
