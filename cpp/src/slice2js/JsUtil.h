//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef JS_UTIL_H
#define JS_UTIL_H

#include "../Ice/OutputUtil.h"
#include "../Slice/Parser.h"

namespace Slice
{
    std::string relativePath(const std::string&, const std::string&);
    std::string getJavaScriptModuleForType(const TypePtr& type);
    std::string getJavaScriptModule(const DefinitionContextPtr&);

    class JsGenerator
    {
    public:
        JsGenerator() = default;
        JsGenerator(const JsGenerator&) = delete;
        virtual ~JsGenerator() = default;

        JsGenerator& operator=(const JsGenerator&) = delete;
        static std::string fixDataMemberName(const std::string&, bool, bool);
        static std::string fixId(const std::string&);

        static std::string getUnqualified(const std::string&, const std::string&, const std::string&);

        static std::string typeToJsString(const TypePtr&, bool definition = false);

        static std::string getLocalScope(const std::string&, const std::string& separator = ".");

        static std::string getHelper(const TypePtr&);
        //
        // Generate code to marshal or unmarshal a type
        //
        void writeMarshalUnmarshalCode(IceInternal::Output&, const TypePtr&, const std::string&, bool);
        void writeOptionalMarshalUnmarshalCode(IceInternal::Output&, const TypePtr&, const std::string&, int, bool);
    };
}

#endif
