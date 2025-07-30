// Copyright (c) ZeroC, Inc.

#ifndef JS_UTIL_H
#define JS_UTIL_H

#include "../Ice/OutputUtil.h"
#include "../Slice/Parser.h"

namespace Slice::JavaScript
{
    [[nodiscard]] std::string getJavaScriptModule(const DefinitionContextPtr& dc);

    [[nodiscard]] std::string typeToJsString(const TypePtr& type, bool definition = false);

    [[nodiscard]] std::string getHelper(const TypePtr& type);

    //
    // Generate code to marshal or unmarshal a type
    //
    void
    writeMarshalUnmarshalCode(IceInternal::Output& out, const TypePtr& type, const std::string& param, bool marshal);
    void writeOptionalMarshalUnmarshalCode(
        IceInternal::Output& out,
        const TypePtr& type,
        const std::string& param,
        std::int32_t tag,
        bool marshal);

    /// Returns a JsDoc formatted link to the provided Slice identifier.
    std::string
    jsLinkFormatter(const std::string& rawLink, const ContainedPtr& source, const SyntaxTreeBasePtr& target);

    void validateJsMetadata(const UnitPtr&);
}

#endif
