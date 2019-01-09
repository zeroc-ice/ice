// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef JS_UTIL_H
#define JS_UTIL_H

#include <Slice/Parser.h>
#include <IceUtil/OutputUtil.h>

namespace Slice
{

std::string relativePath(const std::string&, const std::string&);

class JsGenerator : private ::IceUtil::noncopyable
{
public:

    virtual ~JsGenerator() {};

    static bool isClassType(const TypePtr&);
    static std::string getModuleMetadata(const TypePtr&);
    static std::string getModuleMetadata(const ContainedPtr&);
    static std::string fixId(const std::string&);
    static std::string fixId(const ContainedPtr&);
    static bool findMetaData(const std::string&, const StringList&, std::string&);
    static std::string importPrefix(const TypePtr&,
                                    const ContainedPtr&,
                                    const std::vector<std::pair<std::string, std::string> >&);

    static std::string importPrefix(const ContainedPtr&,
                                    const ContainedPtr&,
                                    const std::vector<std::pair<std::string, std::string> >&);
    static std::string importPrefix(const std::string&, const ContainedPtr&);

    static std::string getUnqualified(const std::string&, const std::string&, const std::string&);

    static std::string typeToString(const TypePtr&,
                                    const ContainedPtr& = 0,
                                    const std::vector<std::pair<std::string, std::string> >& =
                                        std::vector<std::pair<std::string, std::string> >(),
                                    bool typeScript = false,
                                    bool definition = false);

    static std::string typeToString(const TypePtr&,
                                    const ContainedPtr&,
                                    const std::vector<std::pair<std::string, std::string> >&,
                                    bool typeScript,
                                    bool definition,
                                    bool usealias);

    static std::string getLocalScope(const std::string&, const std::string& separator = ".");

    static std::string getHelper(const TypePtr&);
    //
    // Generate code to marshal or unmarshal a type
    //
    void writeMarshalUnmarshalCode(::IceUtilInternal::Output&, const TypePtr&, const std::string&, bool);
    void writeOptionalMarshalUnmarshalCode(::IceUtilInternal::Output&, const TypePtr&, const std::string&, int, bool);

private:

    std::vector< std::string> _seenProxy;
};

}

#endif
