// Copyright (c) ZeroC, Inc.

#ifndef CS_UTIL_H
#define CS_UTIL_H

#include "../Ice/OutputUtil.h"
#include "../Slice/DocCommentParser.h"
#include "../Slice/Parser.h"

namespace Slice::Csharp
{
    /// Returns the namespace of a Contained entity.
    [[nodiscard]] std::string getNamespace(const ContainedPtr& p);

    [[nodiscard]] std::string getUnqualified(
        const ContainedPtr& p,
        const std::string& ns,
        const std::string& prefix = "",
        const std::string& suffix = "");

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

    void writeSeeAlso(IceInternal::Output& out, const StringList& seeAlso);

    class CsharpDocCommentFormatter final : public DocCommentFormatter
    {
    public:
        CsharpDocCommentFormatter(
            std::function<
                std::pair<bool, std::string>(const std::string&, const ContainedPtr&, const SyntaxTreeBasePtr&)>
                linkFormatter);

        void preprocess(StringList& rawComment) final;
        std::string formatCode(const std::string& rawText) final;
        std::string formatParamRef(const std::string& param) final;

        std::string
        formatLink(const std::string& rawLink, const ContainedPtr& source, const SyntaxTreeBasePtr& target) final;

        std::string
        formatSeeAlso(const std::string& rawLink, const ContainedPtr& source, const SyntaxTreeBasePtr& target) final;

    private:
        std::function<std::pair<bool, std::string>(const std::string&, const ContainedPtr&, const SyntaxTreeBasePtr&)>
            _linkFormatter;
    };
}

#endif
