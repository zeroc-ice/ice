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

std::string typeToString(const Type_ptr&);
std::string addLink(const std::string&, const Container_ptr&);
std::string scopedToFile(const std::string&);
std::string scopedToId(const std::string&);

}

#endif
