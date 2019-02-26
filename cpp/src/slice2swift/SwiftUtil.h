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

std::string getSwiftModule(const ModulePtr&, std::string&);
std::string getSwiftModule(const ModulePtr&);
ModulePtr getTopLevelModule(const ContainedPtr&);
ModulePtr getTopLevelModule(const TypePtr&);

class SwiftGenerator : private IceUtil::noncopyable
{
public:

    virtual ~SwiftGenerator() {};

    static void validateMetaData(const UnitPtr&);

protected:

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
    void writeDataMembers(IceUtilInternal::Output&, const ContainedPtr&, const DataMemberList&, bool = false);
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
