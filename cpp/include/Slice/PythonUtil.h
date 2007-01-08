// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SLICE_PYTHON_UTIL_H
#define SLICE_PYTHON_UTIL_H

#include <Slice/Parser.h>
#include <IceUtil/OutputUtil.h>

namespace Slice
{
namespace Python
{

//
// Generate Python code for a translation unit.
//
SLICE_API void generate(const Slice::UnitPtr&, bool, bool, const std::vector<std::string>&, IceUtil::Output&);

//
// Split up a string using whitespace delimiters.
//
SLICE_API bool splitString(const std::string&, std::vector<std::string>&, const std::string& = " \t\n\r");

//
// Convert a scoped name into a Python name.
//
SLICE_API std::string scopedToName(const std::string&);

//
// Check the given identifier against Python's list of reserved words. If it matches
// a reserved word, then an escaped version is returned with a leading underscore.
//
SLICE_API std::string fixIdent(const std::string&);

//
// Return the package specified in the global metadata for the given definition,
// or an empty string if no metadata was found.
//
SLICE_API std::string getPackageMetadata(const Slice::ContainedPtr&);

//
// Get the fully-qualified name of the given definition, including any package
// defined via metadata. If a suffix is provided, it is prepended to the
// definition's unqualified name.
//
SLICE_API std::string getAbsolute(const Slice::ContainedPtr&, const std::string& = std::string());

//
// Emit a comment header.
//
SLICE_API void printHeader(IceUtil::Output&);

}
}

#endif
