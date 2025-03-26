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

    //
    // Get the fully-qualified name of the given definition.
    //

    // TODO Comment on why we aren't using mappedName
    std::string getMappedName(const Slice::ContainedPtr& p, IdentStyle style = IdentNormal);
    std::string getAbsolute(const Slice::ContainedPtr& p);
    std::string getMetaTypeName(const Slice::ContainedPtr& p);
    std::string getMetaTypeReference(const Slice::ContainedPtr& p);

    //
    // Emit a comment header.
    //
    void printHeader(IceInternal::Output&);

    int compile(const std::vector<std::string>&);
}

#endif
