// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef DOTNETNAMES_H
#define DOTNETNAMES_H

#include <string>

namespace Slice
{

namespace DotNet
{

enum BaseType
{
    Object=1, ICloneable=2, Exception=4, END=8
};

extern const char * manglePrefix;

std::string mangleName(const std::string&, int baseTypes = 0);

}

}

#endif
