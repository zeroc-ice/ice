// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SLICE_PHP_UTIL_H
#define SLICE_PHP_UTIL_H

#include <IceUtil/Config.h>
#include <string>

namespace Slice
{
namespace PHP
{

//
// Convert a scoped name into a PHP name.
//
std::string scopedToName(const std::string&, bool);

//
// Check the given identifier against PHP's list of reserved words. If it matches
// a reserved word, then an escaped version is returned with a leading underscore.
//
std::string fixIdent(const std::string&);

//
// Since PHP uses the backslash character as the namespace separator, we have
// to escape backslash characters when they appear in a string literal.
//
std::string escapeName(const std::string&);

}
}

#endif
