// Copyright (c) ZeroC, Inc.

#ifndef SLICE_RUBY_UTIL_H
#define SLICE_RUBY_UTIL_H

#include "../Ice/OutputUtil.h"
#include "../Slice/Parser.h"

namespace Slice::Ruby
{
    // Generate Ruby code for a translation unit.
    //
    void generate(const Slice::UnitPtr&, bool, const std::vector<std::string>&, IceInternal::Output&);

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
    std::string fixIdent(const std::string&, IdentStyle);

    //
    // Get the fully-qualified name of the given definition. If a prefix is provided,
    // it is prepended to the definition's unqualified name.
    //
    std::string getAbsolute(const Slice::ContainedPtr&, IdentStyle, const std::string& = std::string());

    //
    // Emit a comment header.
    //
    void printHeader(IceInternal::Output&);

    int compile(const std::vector<std::string>&);
}

#endif
