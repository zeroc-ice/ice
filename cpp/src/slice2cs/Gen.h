//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef GEN_H
#define GEN_H

#include <CsUtil.h>

namespace Slice
{

struct CommentInfo
{
    std::vector<std::string> summaryLines;
    std::map<std::string, std::vector<std::string>> params;
    std::map<std::string, std::vector<std::string>> exceptions;
    std::vector<std::string> returnLines;
};

class CsVisitor : public CsGenerator, public ParserVisitor
{
public:

    CsVisitor(::IceUtilInternal::Output&);
    virtual ~CsVisitor();

protected:

    void writeMarshalParams(const OperationPtr&, const std::list<ParamInfo>&, const std::list<ParamInfo>&,
                            const std::string& stream = "ostr", const std::string& obj = "");
    void writeUnmarshalParams(const OperationPtr&, const std::list<ParamInfo>&, const std::list<ParamInfo>&,
                              const std::string& stream = "istr");
    void writeMarshalDataMember(const DataMemberPtr&, const std::string&, const std::string&,
                                const std::string& stream = "ostr");
    void writeUnmarshalDataMember(const DataMemberPtr&, const std::string&, const std::string&,
                                  const std::string& stream = "istr");

    virtual void writeMarshaling(const ClassDefPtr&);

    void emitAttributes(const ContainedPtr&);
    void emitComVisibleAttribute();
    void emitGeneratedCodeAttribute();
    void emitTypeIdAttribute(const std::string&);
    void emitPartialTypeAttributes();

    std::string writeValue(const TypePtr&, const std::string&);

    void writeConstantValue(const TypePtr&, const SyntaxTreeBasePtr&, const std::string&);

    //
    // Generate assignment statements for those data members that have default values.
    //
    bool requiresDataMemberInitializers(const DataMemberList&);
    void writeDataMemberInitializers(const DataMemberList&, const std::string&, unsigned int = 0);

    void writeProxyDocComment(const ClassDefPtr&, const std::string&);
    void writeServantDocComment(const ClassDefPtr&, const std::string&);

    void writeTypeDocComment(const ContainedPtr&, const std::string&);
    void writeOperationDocComment(const OperationPtr&, const std::string&, bool, bool);

    enum ParamDir { InParam, OutParam };
    void writeParamDocComment(const OperationPtr&, const CommentInfo&, ParamDir);

    void moduleStart(const ModulePtr&);
    void moduleEnd(const ModulePtr&);

    ::IceUtilInternal::Output& _out;
};

class Gen : private ::IceUtil::noncopyable
{
public:

    Gen(const std::string&,
        const std::vector<std::string>&,
        const std::string&,
        bool);
    ~Gen();

    void generate(const UnitPtr&);
    void generateImpl(const UnitPtr&);
    void closeOutput();

private:

    IceUtilInternal::Output _out;
    IceUtilInternal::Output _impl;
    std::vector<std::string> _includePaths;

    void printHeader();

    class UnitVisitor : public CsVisitor
    {
    public:

        UnitVisitor(::IceUtilInternal::Output&);

        virtual bool visitUnitStart(const UnitPtr&);
    };

    class CompactIdVisitor : public CsVisitor
    {
    public:

        CompactIdVisitor(IceUtilInternal::Output&);
        virtual bool visitUnitStart(const UnitPtr&);
        virtual void visitUnitEnd(const UnitPtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
    };

    class TypeIdVisitor : public CsVisitor
    {
    public:

        TypeIdVisitor(IceUtilInternal::Output&);
        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual bool visitExceptionStart(const ExceptionPtr&);

    private:
        void generateHelperClass(const ContainedPtr&);
    };

    class TypesVisitor : public CsVisitor
    {
    public:

        TypesVisitor(::IceUtilInternal::Output&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual bool visitExceptionStart(const ExceptionPtr&);
        virtual void visitExceptionEnd(const ExceptionPtr&);
        virtual bool visitStructStart(const StructPtr&);
        virtual void visitStructEnd(const StructPtr&);
        virtual void visitEnum(const EnumPtr&);
        virtual void visitConst(const ConstPtr&);
        virtual void visitDataMember(const DataMemberPtr&);
    };

    class ProxyVisitor : public CsVisitor
    {
    public:

        ProxyVisitor(::IceUtilInternal::Output&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual void visitOperation(const OperationPtr&);

        void writeOutgoingRequestReader(const OperationPtr&);
        void writeOutgoingRequestWriter(const OperationPtr&);
    };

    class HelperVisitor : public CsVisitor
    {
    public:

        HelperVisitor(::IceUtilInternal::Output&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual void visitSequence(const SequencePtr&);
        virtual void visitDictionary(const DictionaryPtr&);
    };

    class DispatcherVisitor : public CsVisitor
    {
    public:

        DispatcherVisitor(::IceUtilInternal::Output&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitOperation(const OperationPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);

    protected:

        void writeReturnValueStruct(const OperationPtr&);
        void writeMethodDeclaration(const OperationPtr&);
    };

    class ImplVisitor : public CsVisitor
    {
    public:

        ImplVisitor(::IceUtilInternal::Output&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitOperation(const OperationPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
    };
};

}

#endif
