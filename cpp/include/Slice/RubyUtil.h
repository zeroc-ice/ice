// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
SLICE_API void generate(const Slice::UnitPtr&, bool, bool, const std::vector<std::string>&, IceUtilInternal::Output&);

//
// Check the given identifier against Ruby's list of reserved words. If it matches
// a reserved word, then an escaped version is returned with a leading underscore.
//
enum IdentStyle
{
    IdentNormal,
    IdentToUpper, // Mapped identifier must begin with an upper-case letter.
    IdentToLower  // Mapped identifier must begin with a lower-case letter.
};
SLICE_API std::string fixIdent(const std::string&, IdentStyle);

//
// Get the fully-qualified name of the given definition. If a prefix is provided,
// it is prepended to the definition's unqualified name.
//
SLICE_API std::string getAbsolute(const Slice::ContainedPtr&, IdentStyle, const std::string& = std::string());

//
// Emit a comment header.
//
SLICE_API void printHeader(IceUtilInternal::Output&);

SLICE_API int compile(const std::vector<std::string>&);

}
}

#endif
