// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef GEN_H
#define GEN_H

#include <Slice/CsUtil.h>

namespace Slice
{


class CsVisitor : public CsGenerator, public ParserVisitor
{
public:

    CsVisitor(::IceUtilInternal::Output&);
    virtual ~CsVisitor();

protected:

    void writeMarshalUnmarshalParams(const ParamDeclList&, const OperationPtr&, bool);
    void writePostUnmarshalParams(const ParamDeclList&, const OperationPtr&);
    void writeMarshalDataMember(const DataMemberPtr&, const std::string&);
    void writeUnmarshalDataMember(const DataMemberPtr&, const std::string&, bool, int&);
    void writeStreamMarshalDataMember(const DataMemberPtr&, const std::string&);
    void writeStreamUnmarshalDataMember(const DataMemberPtr&, const std::string&, bool, int&);

    virtual void writeInheritedOperations(const ClassDefPtr&);
    virtual void writeDispatchAndMarshalling(const ClassDefPtr&, bool);
    virtual std::vector<std::string> getParams(const OperationPtr&);
    virtual std::vector<std::string> getParamsAsync(const OperationPtr&, bool);
    virtual std::vector<std::string> getParamsAsyncCB(const OperationPtr&, bool, bool);
    virtual std::vector<std::string> getArgs(const OperationPtr&);
    virtual std::vector<std::string> getArgsAsync(const OperationPtr&, bool);

    void emitAttributes(const ContainedPtr&);
    void emitComVisibleAttribute();
    void emitGeneratedCodeAttribute();
    void emitPartialTypeAttributes();

    std::string getParamAttributes(const ParamDeclPtr&);

    std::string writeValue(const TypePtr&);

    void writeConstantValue(const TypePtr&, const SyntaxTreeBasePtr&, const std::string&);

    //
    // Generate assignment statements for those data members that have default values.
    //
    bool requiresDataMemberInitializers(const DataMemberList&);
    void writeDataMemberInitializers(const DataMemberList&, int = 0, bool = false);

    std::string toCsIdent(const std::string&);
    std::string editMarkup(const std::string&);
    StringList splitIntoLines(const std::string&);
    void splitComment(const ContainedPtr&, StringList&, StringList&);
    StringList getSummary(const ContainedPtr&);
    void writeDocComment(const ContainedPtr&, const std::string&, const std::string& = "");
    void writeDocCommentOp(const OperationPtr&);

    enum ParamDir { InParam, OutParam };
    void writeDocCommentAMI(const OperationPtr&, ParamDir, const std::string&, const std::string& = "",
                            const std::string& = "", const std::string& = "");
    void writeDocCommentAMD(const OperationPtr&, ParamDir, const std::string&);
    void writeDocCommentParam(const OperationPtr&, ParamDir, bool);

    ::IceUtilInternal::Output& _out;
};

class Gen : private ::IceUtil::noncopyable
{
public:

    Gen(const std::string&,
        const std::vector<std::string>&,
        const std::string&,
        bool,
        bool,
        bool);
    ~Gen();

    void generate(const UnitPtr&);
    void generateTie(const UnitPtr&);
    void generateImpl(const UnitPtr&);
    void generateImplTie(const UnitPtr&);
    void generateChecksums(const UnitPtr&);
    void closeOutput();

private:

    IceUtilInternal::Output _out;
    IceUtilInternal::Output _impl;

    std::vector<std::string> _includePaths;

    bool _stream;

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

    class TypesVisitor : public CsVisitor
    {
    public:

        TypesVisitor(::IceUtilInternal::Output&, bool);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitOperation(const OperationPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual bool visitExceptionStart(const ExceptionPtr&);
        virtual void visitExceptionEnd(const ExceptionPtr&);
        virtual bool visitStructStart(const StructPtr&);
        virtual void visitStructEnd(const StructPtr&);
        virtual void visitSequence(const SequencePtr&);
        virtual void visitDictionary(const DictionaryPtr&);
        virtual void visitEnum(const EnumPtr&);
        virtual void visitConst(const ConstPtr&);
        virtual void visitDataMember(const DataMemberPtr&);

    private:

        void writeMemberHashCode(const DataMemberList&, int);
        void writeMemberEquals(const DataMemberList&, int);

        bool _stream;
    };

    class AsyncDelegateVisitor : public CsVisitor
    {
    public:

        AsyncDelegateVisitor(::IceUtilInternal::Output&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual void visitOperation(const OperationPtr&);
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
    };

    class OpsVisitor : public CsVisitor
    {
    public:

        OpsVisitor(::IceUtilInternal::Output&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);

    private:
        void writeOperations(const ClassDefPtr&, bool);
    };

    class HelperVisitor : public CsVisitor
    {
    public:

        HelperVisitor(::IceUtilInternal::Output&, bool);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual void visitSequence(const SequencePtr&);
        virtual void visitDictionary(const DictionaryPtr&);

    private:

        bool _stream;
    };

    class DispatcherVisitor : public CsVisitor
    {
    public:

        DispatcherVisitor(::IceUtilInternal::Output&, bool);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);

    private:

        bool _stream;
    };

    class AsyncVisitor : public CsVisitor
    {
    public:

        AsyncVisitor(::IceUtilInternal::Output&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual void visitOperation(const OperationPtr&);
    };

    class TieVisitor : public CsVisitor
    {
    public:

        TieVisitor(::IceUtilInternal::Output&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);

    private:

        typedef std::set<std::string> NameSet;
        void writeInheritedOperationsWithOpNames(const ClassDefPtr&, NameSet&);
    };

    class BaseImplVisitor : public CsVisitor
    {
    public:

        BaseImplVisitor(::IceUtilInternal::Output&);

    protected:

        void writeOperation(const OperationPtr&, bool, bool);
    };

    class ImplVisitor : public BaseImplVisitor
    {
    public:

        ImplVisitor(::IceUtilInternal::Output&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
    };

    class ImplTieVisitor : public BaseImplVisitor
    {
    public:

        ImplTieVisitor(::IceUtilInternal::Output&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
    };
};

}

#endif
