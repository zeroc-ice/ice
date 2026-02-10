// Copyright (c) ZeroC, Inc.

#ifndef JS_UTIL_H
#define JS_UTIL_H

#include "../Ice/OutputUtil.h"
#include "../Slice/Parser.h"

namespace Slice::JavaScript
{
    [[nodiscard]] std::string getJavaScriptModule(const DefinitionContextPtr& dc);

    /// Creates a valid JavaScript identifier from an import path, used as a prefix for imported symbols.
    /// e.g., "../foo/bar/baz.ice" -> "__foo_bar_baz", "@zeroc/ice" -> "_zeroc_ice"
    [[nodiscard]] std::string importPathToIdentifier(const std::string& path);

    [[nodiscard]] std::string typeToJsString(const TypePtr& type, bool definition = false);

    /// Resolves a type to its JavaScript string representation, applying the import alias prefix when the type
    /// originates from a different js:module than currentJsModule.
    [[nodiscard]] std::string resolveJsType(const TypePtr& type, const std::string& currentJsModule);

    /// Resolves a contained element's scoped name, applying the import alias prefix when the element
    /// originates from a different js:module than currentJsModule.
    [[nodiscard]] std::string resolveJsScope(const ContainedPtr& contained, const std::string& currentJsModule);

    [[nodiscard]] std::string getHelper(const TypePtr& type, const std::string& currentJsModule);

    //
    // Generate code to marshal or unmarshal a type
    //
    void writeMarshalUnmarshalCode(
        IceInternal::Output& out,
        const TypePtr& type,
        const std::string& param,
        bool marshal,
        const std::string& currentJsModule);
    void writeOptionalMarshalUnmarshalCode(
        IceInternal::Output& out,
        const TypePtr& type,
        const std::string& param,
        std::int32_t tag,
        bool marshal,
        const std::string& currentJsModule);

    /// Returns a JsDoc formatted link to the provided Slice identifier.
    std::string
    jsLinkFormatter(const std::string& rawLink, const ContainedPtr& source, const SyntaxTreeBasePtr& target);

    void validateJsMetadata(const UnitPtr&);
}

#endif
