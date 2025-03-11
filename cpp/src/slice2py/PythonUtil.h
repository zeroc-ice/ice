// Copyright (c) ZeroC, Inc.

#ifndef SLICE_PYTHON_UTIL_H
#define SLICE_PYTHON_UTIL_H

#include "../Ice/OutputUtil.h"
#include "../Slice/Parser.h"

namespace Slice::Python
{
    /// Get the package directory from metadata (if any).
    std::string getPackageDirectory(const std::string&, const Slice::UnitPtr&);

    /// Determine the name of a Python source file for use in an import statement.
    /// The return value does not include the .py extension.
    std::string getImportFileName(const std::string&, const Slice::UnitPtr&, const std::vector<std::string>&);

    /// Generate Python code for a translation unit.
    void generate(const Slice::UnitPtr&, bool, const std::vector<std::string>&, IceInternal::Output&);

    /// Return the package specified by metadata for the given definition, or an empty string if no metadata was found.
    std::string getPackageMetadata(const Slice::ContainedPtr&);

    /// Get the fully-qualified name of the given definition, including any package defined via metadata.
    std::string getAbsolute(const Slice::ContainedPtr& p);

    /// Get the fully-qualified name of the given definition, including any package defined via metadata,
    /// but "_M_" is prepended to the first name segment, indicating that this is a an explicit reference.
    std::string getTypeReference(const Slice::ContainedPtr& p);

    /// Get the fully-qualified name of the given definition, including any package defined via metadata,
    /// but "_M_" is prepended to the first name segment, and "_t_" is prepended to the final name segment,
    /// indicating that this is a an explicit reference to a type.
    std::string getMetaTypeReference(const Slice::ContainedPtr& p);

    /// Emit a comment header.
    void printHeader(IceInternal::Output&);

    int compile(const std::vector<std::string>&);

    void validateMetadata(const UnitPtr&);
}

#endif
