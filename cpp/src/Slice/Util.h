// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef _SLICE_UTIL_H
#define _SLICEP_UTIL_H

#include <Slice/Parser.h>

namespace Slice
{

SLICE_API std::string getCwd();
SLICE_API bool isAbsolute(const std::string&);
SLICE_API std::string normalizePath(const std::string&, bool = false);

}

#endif
