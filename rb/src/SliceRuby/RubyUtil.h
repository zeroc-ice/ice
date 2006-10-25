// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SLICE_RUBY_UTIL_H
#define SLICE_RUBY_UTIL_H

#include <Slice/Parser.h>
#include <IceUtil/OutputUtil.h>

namespace Slice
{
namespace Ruby
{

//
// Generate Ruby code for a translation unit.
//
SLICE_API void generate(const Slice::UnitPtr&, bool, bool, const std::vector<std::string>&, IceUtil::Output&);

//
// Split up a string using whitespace delimiters.
//
SLICE_API bool splitString(const std::string&, std::vector<std::string>&, const std::string& = " \t\n\r");

//
// Check the given identifier against Ruby's list of reserved words. If it matches
// a reserved word, then an escaped version is returned with a leading underscore.
//
SLICE_API std::string fixIdent(const std::string&, bool);

//
// Get the fully-qualified name of the given definition. If a prefix is provided,
// it is prepended to the definition's unqualified name.
//
SLICE_API std::string getAbsolute(const Slice::ContainedPtr&, bool, const std::string& = std::string());

//
// Emit a comment header.
//
SLICE_API void printHeader(IceUtil::Output&);

}
}

#endif
