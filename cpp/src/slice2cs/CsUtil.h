// Copyright (c) ZeroC, Inc.

#ifndef CS_UTIL_H
#define CS_UTIL_H

#include "../Ice/OutputUtil.h"
#include "../Slice/Parser.h"

namespace Slice::Csharp
{
    /// Returns the namespace of a Contained entity.
    [[nodiscard]] std::string getNamespace(const ContainedPtr& p);

    [[nodiscard]] std::string
    getUnqualified(const ContainedPtr& p, const std::string& ns, const std::string& prefix = "");

    /// Removes a leading '@' character from the provided identifier (if one is present).
    [[nodiscard]] std::string removeEscapePrefix(const std::string& identifier);

    /// Returns the namespace prefix of a Contained entity.
    [[nodiscard]] std::string getNamespacePrefix(const ContainedPtr& p);

    /// Writes a one-line XML doc-comment.
    void writeDocLine(
        IceInternal::Output& out,
        const std::string& openTag,
        const std::string& comment,
        const std::optional<std::string>& closeTag = std::nullopt);

    /// Writes an XML doc-comment with one or more lines. If there are multiple lines, the comment is written on
    /// multiple lines. If there is only one line, the comment is written on a single line.
    void writeDocLines(
        IceInternal::Output& out,
        const std::string& openTag,
        const StringList& lines,
        const std::optional<std::string>& closeTag = std::nullopt);
}

#endif
