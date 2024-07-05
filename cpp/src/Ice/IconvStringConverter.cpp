//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/IconvStringConverter.h"

#ifndef _WIN32

using namespace std;
using namespace Ice;

const char*
IconvInitializationException::ice_id() const noexcept
{
    return "::Ice::IconvInitializationException";
}

#endif
