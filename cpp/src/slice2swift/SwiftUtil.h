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

class SwiftGenerator : private IceUtil::noncopyable
{
public:

    virtual ~SwiftGenerator() {};

    static void validateMetaData(const UnitPtr&);

protected:

    struct ModulePrefix
    {
        ModulePtr m;
        std::string module;
        std::string name;
    };

    static bool addModule(const ModulePtr&, const std::string&, const std::string&);
    static ModulePrefix modulePrefix(const ModulePtr&);
    static std::string moduleName(const ModulePtr&);
    static ModulePtr findModule(const ContainedPtr&);
    static void modulePrefixError(const ModulePtr&, const std::string&);

    std::string getLocalScope(const std::string&, const std::string& = ".");
    std::string typeToString(const TypePtr&, const ContainedPtr& = 0);
    std::string typeToProxyImpl(const TypePtr&);
    std::string fixIdent(const std::string& ident);
    std::string fixName(const ContainedPtr& cont);
    std::string getAbsolute(const ContainedPtr&,
                            const std::string& = std::string(),
                            const std::string& = std::string());

    std::string modeToString(Operation::Mode);

    bool isObjcRepresentable(const TypePtr&);
    bool isObjcRepresentable(const DataMemberList&);
    bool isValueType(const TypePtr&);
    bool isProxyType(const TypePtr&);

    void writeTuple(IceUtilInternal::Output&, const StringList&);
    void writeDataMembers(IceUtilInternal::Output&, const DataMemberList&, bool = false);
    void writeInitializer(IceUtilInternal::Output&, const DataMemberList&, const DataMemberList& = DataMemberList());
    void writeInitializerMembers(IceUtilInternal::Output&, const DataMemberList&, bool = true);
    void writeOperation(IceUtilInternal::Output&, const OperationPtr&, bool);
    void writeOperationsParameters(IceUtilInternal::Output&, const ParamDeclList&);
    void writeCastFuncs(IceUtilInternal::Output&, const ClassDefPtr&);
    void writeStaticId(IceUtilInternal::Output&, const ClassDefPtr&);
    void writeMarshalUnmarshalCode(IceUtilInternal::Output&, const ClassDefPtr&, const OperationPtr&);

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
        static void modulePrefixError(const ModulePtr&, const std::string&);

        static const std::string _msg;
    };

    //
    // Map of module scoped name to ModulePtr. Used to verify that objc:prefix metadata directives are consistent.
    //
    typedef std::map<std::string, ModulePrefix> ModuleMap;
    static ModuleMap _modules;
};

}

#endif
