// Copyright (c) ZeroC, Inc.

#ifndef SLICE_RUBY_UTIL_H
#define SLICE_RUBY_UTIL_H

#include "../Ice/OutputUtil.h"
#include "../Slice/Parser.h"

namespace Slice::Ruby
{
    /// Generates Ruby code for the provided translation unit.
    void generate(
        const Slice::UnitPtr& unit,
        bool all,
        const std::vector<std::string>& includePaths,
        IceInternal::Output& out);

    /// This enum is used by ::getMappedName to control the casing of the identifier it returns.
    enum IdentStyle
    {
        /// No change will be made to the mapped identifier.
        IdentNormal,

        /// The first letter of the mapped identifier will be upper-cased.
        IdentToUpper,

        /// The first letter of the mapped identifier will be lower-cased.
        IdentToLower,
    };

    /// Returns the mapped name of the provided Slice element.
    /// If the element has 'ruby:identifier' metadata on it, the metadata's argument is returned as-is.
    /// Otherwise, the slice name is returned, after its casing has been fixed according to the provided @p style.
    std::string getMappedName(const Slice::ContainedPtr& p, IdentStyle style = IdentNormal);

    /// Returns the fully-qualified mapped-identifier of the provided Slice element.
    /// This is equivalent to calling ::getMappedName on @p p and all it's containers.
    std::string getAbsolute(const Slice::ContainedPtr& p);

    /// Equivalent to ::getMappedName but with "T_" preprended to the name.
    std::string getMetaTypeName(const Slice::ContainedPtr& p);

    /// Equivalent to ::getAbsolute but with "T_" preprended to the name.
    std::string getMetaTypeReference(const Slice::ContainedPtr& p);

    /// Emits a comment header for a generated file.
    void printHeader(IceInternal::Output& out);

    /// Entry point into 'slice2rb'.
    /// @param argv The arguments provided to 'slice2rb'.
    int compile(const std::vector<std::string>& argv);
}

#endif
