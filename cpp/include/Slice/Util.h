// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SLICE_UTIL_H
#define SLICE_UTIL_H

#include <Slice/Parser.h>

namespace Slice
{

SLICE_API bool isAbsolute(const std::string&);
SLICE_API std::string fullPath(const std::string&);
SLICE_API std::string changeInclude(const std::string&, const std::vector<std::string>&);

}

#endif
