// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
// Get the package directory from metadata (if any).
//
std::string getPackageDirectory(const std::string&, const Slice::UnitPtr&);

//
// Determine the name of a Python source file for use in an import statement.
// The return value does not include the .py extension.
//
std::string getImportFileName(const std::string&, const Slice::UnitPtr&, const std::vector<std::string>&);

//
// Generate Python code for a translation unit.
//
void generate(const Slice::UnitPtr&, bool, bool, const std::vector<std::string>&, IceUtilInternal::Output&);

//
// Convert a scoped name into a Python name.
//
std::string scopedToName(const std::string&);

//
// Check the given identifier against Python's list of reserved words. If it matches
// a reserved word, then an escaped version is returned with a leading underscore.
//
std::string fixIdent(const std::string&);

//
// Return the package specified by metadata for the given definition,
// or an empty string if no metadata was found.
//
std::string getPackageMetadata(const Slice::ContainedPtr&);

//
// Get the fully-qualified name of the given definition, including any
// package defined via metadata. If a suffix is provided, it is
// prepended to the definition's unqualified name. If the nameSuffix
// is provided, it is appended to the containers name.
//
// COMPILERFIX: MSVC 6 seems to have a problem with const std::string
// = std::string(), const std::string = std::string().
//
std::string getAbsolute(const Slice::ContainedPtr&, const std::string& = "", const std::string& = "");

//
// Emit a comment header.
//
void printHeader(IceUtilInternal::Output&);

int compile(const std::vector<std::string>&);

}
}

#endif
