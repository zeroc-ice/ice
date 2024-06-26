//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef JS_UTIL_H
#define JS_UTIL_H

#include "../IceUtil/OutputUtil.h"
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
        virtual ~JsGenerator() {};

        JsGenerator& operator=(const JsGenerator&) = delete;
        static std::string fixId(const std::string&);
        static bool findMetaData(const std::string&, const StringList&, std::string&);

        static std::string getUnqualified(const std::string&, const std::string&, const std::string&);

        static std::string typeToJsString(const TypePtr&, bool definition = false);

        static std::string getLocalScope(const std::string&, const std::string& separator = ".");

        static std::string getHelper(const TypePtr&);
        //
        // Generate code to marshal or unmarshal a type
        //
        void writeMarshalUnmarshalCode(IceUtilInternal::Output&, const TypePtr&, const std::string&, bool);
        void writeOptionalMarshalUnmarshalCode(IceUtilInternal::Output&, const TypePtr&, const std::string&, int, bool);
    };
}

#endif
