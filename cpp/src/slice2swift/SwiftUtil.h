// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SWIFT_UTIL_H
#define SWIFT_UTIL_H

#include <Slice/Parser.h>
#include <IceUtil/OutputUtil.h>

namespace Slice
{

const int TypeContextInParam = 1;
const int TypeContextProtocol = 2;
const int TypeContextLocal = 32;

std::string getSwiftModule(const ModulePtr&, std::string&);
std::string getSwiftModule(const ModulePtr&);
ModulePtr getTopLevelModule(const ContainedPtr&);
ModulePtr getTopLevelModule(const TypePtr&);

std::string fixIdent(const std::string&);
StringList splitScopedName(const std::string&);

class SwiftGenerator : private IceUtil::noncopyable
{
public:

    virtual ~SwiftGenerator() {};

    static void validateMetaData(const UnitPtr&);

protected:

    std::string typeToString(const TypePtr&, const ContainedPtr&, const StringList& = StringList(), bool = false,
                             int = 0);

    std::string getAbsolute(const TypePtr&);
    std::string getAbsolute(const ProxyPtr&);
    std::string getAbsolute(const ClassDeclPtr&);
    std::string getAbsolute(const ClassDefPtr&);
    std::string getAbsolute(const StructPtr&);
    std::string getAbsolute(const ExceptionPtr&);
    std::string getAbsolute(const EnumPtr&);
    std::string getAbsolute(const ConstPtr&);
    std::string getAbsolute(const SequencePtr&);
    std::string getAbsolute(const DictionaryPtr&);

    std::string getUnqualified(const std::string&, const std::string&);
    std::string modeToString(Operation::Mode);
    std::string getOptionalFormat(const TypePtr&);

    bool isNullableType(const TypePtr&);
    bool isObjcRepresentable(const TypePtr&);
    bool isObjcRepresentable(const DataMemberList&);
    bool isProxyType(const TypePtr&);

    void writeConstantValue(IceUtilInternal::Output& out, const TypePtr&, const SyntaxTreeBasePtr&,
                            const std::string&, const StringList&, const std::string&);
    void writeDefaultInitializer(IceUtilInternal::Output&, const DataMemberList&, const ContainedPtr&, bool = true,
                                 bool = false);
    void writeMemberwiseInitializer(IceUtilInternal::Output&, const DataMemberList&, const ContainedPtr&);
    void writeMemberwiseInitializer(IceUtilInternal::Output&, const DataMemberList&, const DataMemberList&,
                                    const DataMemberList&, const ContainedPtr&, bool rootClass = false);
    void writeMembers(IceUtilInternal::Output&, const DataMemberList&, const ContainedPtr&, int = 0);

    void writeMarshalUnmarshalCode(::IceUtilInternal::Output&,
                                   const TypePtr&,
                                   const std::string&,
                                   const std::string&,
                                   bool,
                                   bool,
                                   bool,
                                   int = -1);
    void writeMarshalUnmarshalCode(::IceUtilInternal::Output&,
                                   const DataMemberPtr&,
                                   const ContainedPtr&,
                                   bool,
                                   bool,
                                   bool,
                                   int = -1);

private:

    class MetaDataVisitor : public ParserVisitor
    {
    public:

        virtual bool visitModuleStart(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual bool visitExceptionStart(const ExceptionPtr&);
        virtual bool visitStructStart(const StructPtr&);
        virtual void visitSequence(const SequencePtr&);
        virtual void visitDictionary(const DictionaryPtr&);
        virtual void visitEnum(const EnumPtr&);
        virtual void visitConst(const ConstPtr&);

    private:

        void validate(const ContainedPtr&);

        typedef std::map<std::string, std::string> ModuleMap;
        typedef std::map<std::string, ModuleMap> ModulePrefix;

        //
        // Each Slice unit has to map all top-level modules to a single Swift module
        //
        ModuleMap _modules;

        //
        // With a given Swift module a Slice module has to map to a single prefix
        //
        ModulePrefix _prefixes;

        static const std::string _msg;
    };
};

}

#endif
