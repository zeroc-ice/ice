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

#include <Slice/Parser.h>

namespace Slice
{
namespace PHP
{

//
// Convert a scoped name into a PHP name.
//
SLICE_API std::string scopedToName(const std::string&, bool);

//
// Check the given identifier against PHP's list of reserved words. If it matches
// a reserved word, then an escaped version is returned with a leading underscore.
//
SLICE_API std::string fixIdent(const std::string&);

//
// Get the fully-qualified name of the given definition. If a suffix is provided,
// it is prepended to the definition's unqualified name. If the nameSuffix
// is provided, it is appended to the container's name.
//
// COMPILERFIX: MSVC 6 seems to have a problem with const std::string
// = std::string(), const std::string = std::string().
//
SLICE_API std::string getAbsolute(const Slice::ContainedPtr&, bool, const std::string& = "", const std::string& = "");

//
// Since PHP uses the backslash character as the namespace separator, we have
// to escape backslash characters when they appear in a string literal.
//
SLICE_API std::string escapeName(const std::string&);

}
}

#endif
