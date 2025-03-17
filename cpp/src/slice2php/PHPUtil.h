// Copyright (c) ZeroC, Inc.

#ifndef SLICE_PHP_UTIL_H
#define SLICE_PHP_UTIL_H

#include "Ice/Config.h"
#include <string>

namespace Slice::PHP
{
    //
    // Check the given identifier against PHP's list of reserved words. If it matches
    // a reserved word, then an escaped version is returned with a leading underscore.
    //
    std::string fixIdent(const std::string&);
}

#endif
