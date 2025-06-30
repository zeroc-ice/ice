// Copyright (c) ZeroC, Inc.

#ifndef SLICE_UTIL_H
#define SLICE_UTIL_H

#include "../Ice/OutputUtil.h"
#include "Parser.h"

namespace Slice
{
    std::string fullPath(const std::string& path);
    std::string changeInclude(const std::string& path, const std::vector<std::string>& includePaths);
    std::string removeExtension(const std::string& path);
    void emitError(std::string_view file, int line, std::string_view message);
    void emitWarning(std::string_view file, int line, std::string_view message);
    void emitRaw(const char* message);
    std::vector<std::string> filterMcppWarnings(const std::string& message);
    void printGeneratedHeader(IceInternal::Output& out, std::string_view path, std::string_view commentStyle = "//");
#ifdef _WIN32
    std::vector<std::string> argvToArgs(int argc, wchar_t* argv[]);
#else
    std::vector<std::string> argvToArgs(int argc, char* argv[]);
#endif

    enum EscapeMode
    {
        UCN,
        Octal,
        ShortUCN,
        Matlab,
        EC6UCN
    };

    // Parameters:
    // const string& value: input string provided by Slice Parser
    // const string& nonPrintableEscaped: which of \a, \b, \f, \n, \r, \t, \v, \0 (null), \x20 (\s), \x1b (\e) are
    //                                    escaped in the target language
    //                                    Warning: don't include \0 if the target language recognizes octal escapes
    // const string& printableEscaped: additional printable ASCII characters other than \ and the string delimiter
    //                                 that need to be escaped
    // EscapeMode escapeMode: whether we generate both UCNs, octal escape sequences, only short UCNs (\unnnn),
    //                        Matlab syntax, or ECMAScript 6-style UCNs with \u{...} for astral characters.
    // unsigned char cutOff: characters < cutOff other than the nonPrintableEscaped are generated as
    //                       octal escape sequences, regardless of escapeMode.
    std::string toStringLiteral(
        const std::string& value,
        const std::string& nonPrintableEscaped,
        const std::string& printableEscaped,
        EscapeMode escapeMode,
        unsigned char cutOff);

    void writeDependencies(const std::string& dependencies, const std::string& dependFile);

    std::vector<std::string> splitScopedName(const std::string& scoped, bool allowEmpty = true);

    /// Returns "an" if @p s starts with a vowel and "a" otherwise.
    std::string getArticleFor(const std::string& s);

    /// Returns the 'kindOf' string for the provided Slice definition, but correctly pluralized.
    /// @see Contained::kindOf
    std::string pluralKindOf(const ContainedPtr& p);

    // Checks an identifier for illegal syntax and reports any errors that are present.
    bool checkIdentifier(const std::string& identifier);

    bool isProxyType(const TypePtr& type);

    // Is this the first element defined in its container?
    bool isFirstElement(const ContainedPtr& p);

    /// Returns the first sentence in the provided list of lines.
    std::string getFirstSentence(const StringList& lines);

    /// Checks if @p param collides with any mapped name in @p params. If yes, return param followed by an underscore;
    /// otherwise, return param as is.
    /// @param params The parameter list to check against.
    /// @param param The parameter name to check.
    /// @return The parameter name, possibly modified to avoid collisions.
    std::string getEscapedParamName(const ParameterList& params, std::string_view param);

    /// Gets the name of a Slice type, e.g. "int", "string", "::M::MyClass", etc.
    /// @param type The type.
    /// @return The fully qualified name of the type.
    std::string getTypeScopedName(const TypePtr& type);
}
#endif
