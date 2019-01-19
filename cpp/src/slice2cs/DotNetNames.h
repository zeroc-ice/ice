//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
