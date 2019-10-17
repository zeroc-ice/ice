//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef SWIFT_UTIL_H
#define SWIFT_UTIL_H

#include <Slice/Parser.h>
#include <IceUtil/OutputUtil.h>

typedef std::list<std::pair<std:: string, std::string> > StringPairList;

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

struct ParamInfo
{
    std::string name;
    TypePtr type;
    std::string typeStr;
    bool optional;
    int tag;
    ParamDeclPtr param; // 0 == return value
};

typedef std::list<ParamInfo> ParamInfoList;

struct DocElements
{
    StringList overview;
    bool deprecated;
    StringList deprecateReason;
    StringList misc;
    StringList seeAlso;
    StringList returns;
    std::map<std::string, StringList> params;
    std::map<std::string, StringList> exceptions;
};

class SwiftGenerator : private IceUtil::noncopyable
{
public:

    virtual ~SwiftGenerator() {};

    static void validateMetaData(const UnitPtr&);

protected:

    void trimLines(StringList&);
    StringList splitComment(const std::string&);
    bool parseCommentLine(const std::string&, const std::string&, bool, std::string&, std::string&);
    DocElements parseComment(const ContainedPtr&);
    void writeDocLines(IceUtilInternal::Output&, const StringList&, bool commentFirst = true,
                       const std::string& space = " ");
    void writeDocSentence(IceUtilInternal::Output&, const StringList&);
    void writeSeeAlso(IceUtilInternal::Output&, const StringList&, const ContainerPtr&);
    void writeDocSummary(IceUtilInternal::Output&, const ContainedPtr&);
    void writeOpDocSummary(IceUtilInternal::Output&, const OperationPtr&, bool, bool, bool = false);

    void writeProxyDocSummary(IceUtilInternal::Output&, const ClassDefPtr&, const std::string&);
    void writeServantDocSummary(IceUtilInternal::Output&, const ClassDefPtr&, const std::string&);
    void writeMemberDoc(IceUtilInternal::Output&, const DataMemberPtr&);

    std::string paramLabel(const std::string&, const ParamDeclList&);
    std::string operationReturnType(const OperationPtr&, int typeCtx = 0);
    bool operationReturnIsTuple(const OperationPtr&);
    std::string operationReturnDeclaration(const OperationPtr&);
    std::string operationInParamsDeclaration(const OperationPtr&);

    bool operationIsAmd(const OperationPtr&);

    ParamInfoList getAllInParams(const OperationPtr&, int = 0);
    void getInParams(const OperationPtr&, ParamInfoList&, ParamInfoList&);

    ParamInfoList getAllOutParams(const OperationPtr&, int = 0);
    void getOutParams(const OperationPtr&, ParamInfoList&, ParamInfoList&);

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

    static bool isNullableType(const TypePtr&);
    bool isProxyType(const TypePtr&);
    bool isClassType(const TypePtr&);

    bool containsClassMembers(const StructPtr&);

    std::string getValue(const std::string&, const TypePtr&);
    void writeConstantValue(IceUtilInternal::Output& out, const TypePtr&, const SyntaxTreeBasePtr&,
                            const std::string&, const StringList&, const std::string&, bool optional = false);
    void writeDefaultInitializer(IceUtilInternal::Output&,  bool, bool);
    void writeMemberwiseInitializer(IceUtilInternal::Output&, const DataMemberList&, const ContainedPtr&);
    void writeMemberwiseInitializer(IceUtilInternal::Output&, const DataMemberList&, const DataMemberList&,
                                    const DataMemberList&, const ContainedPtr&,
                                    bool local = false,
                                    bool rootClass = false,
                                    const StringPairList& = StringPairList());
    void writeMembers(IceUtilInternal::Output&, const DataMemberList&, const ContainedPtr&, int = 0);

    void writeMarshalUnmarshalCode(::IceUtilInternal::Output&,
                                   const TypePtr&,
                                   const ContainedPtr&,
                                   const std::string&,
                                   bool,
                                   int = -1);

    bool usesMarshalHelper(const TypePtr&);
    void writeMarshalInParams(::IceUtilInternal::Output&, const OperationPtr&);
    void writeMarshalOutParams(::IceUtilInternal::Output&, const OperationPtr&);
    void writeMarshalAsyncOutParams(::IceUtilInternal::Output&, const OperationPtr&);
    void writeUnmarshalInParams(::IceUtilInternal::Output&, const OperationPtr&);
    void writeUnmarshalOutParams(::IceUtilInternal::Output&, const OperationPtr&);
    void writeUnmarshalUserException(::IceUtilInternal::Output& out, const OperationPtr&);
    void writeSwiftAttributes(::IceUtilInternal::Output&, const StringList&);
    void writeProxyOperation(::IceUtilInternal::Output&, const OperationPtr&);
    void writeProxyAsyncOperation(::IceUtilInternal::Output&, const OperationPtr&);
    void writeDispatchOperation(::IceUtilInternal::Output&, const OperationPtr&);
    void writeDispatchAsyncOperation(::IceUtilInternal::Output&, const OperationPtr&);

private:

    class MetaDataVisitor : public ParserVisitor
    {
    public:

        virtual bool visitModuleStart(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitOperation(const OperationPtr&);
        virtual bool visitExceptionStart(const ExceptionPtr&);
        virtual bool visitStructStart(const StructPtr&);
        virtual void visitSequence(const SequencePtr&);
        virtual void visitDictionary(const DictionaryPtr&);
        virtual void visitEnum(const EnumPtr&);
        virtual void visitConst(const ConstPtr&);

    private:

        StringList validate(const SyntaxTreeBasePtr&, const StringList&,
                            const std::string&, const std::string&,
                            bool local = false,
                            bool operationParameter = false);

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
