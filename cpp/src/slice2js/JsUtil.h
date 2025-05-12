// Copyright (c) ZeroC, Inc.

#ifndef JS_UTIL_H
#define JS_UTIL_H

#include "../Ice/OutputUtil.h"
#include "../Slice/Parser.h"

namespace Slice
{
    std::string relativePath(const std::string&, const std::string&);
    std::string getJavaScriptModuleForType(const TypePtr& type);
    std::string getJavaScriptModule(const DefinitionContextPtr&);

    class JsDocCommentFormatter : public DocCommentFormatter
    {
        /// Returns a JsDoc formatted link to the provided Slice identifier.
        [[nodiscard]] std::string
        formatLink(const std::string& rawLink, const ContainedPtr&, const SyntaxTreeBasePtr&) const final;
    };

    class JsGenerator
    {
    public:
        JsGenerator() = default;
        JsGenerator(const JsGenerator&) = delete;
        virtual ~JsGenerator() = default;

        JsGenerator& operator=(const JsGenerator&) = delete;

        static std::string typeToJsString(const TypePtr&, bool definition = false);

        static std::string getHelper(const TypePtr&);
        //
        // Generate code to marshal or unmarshal a type
        //
        void writeMarshalUnmarshalCode(IceInternal::Output&, const TypePtr&, const std::string&, bool);
        void writeOptionalMarshalUnmarshalCode(IceInternal::Output&, const TypePtr&, const std::string&, int, bool);
    };
}

#endif
