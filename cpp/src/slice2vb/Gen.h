// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef GEN_H
#define GEN_H

#include <Slice/VbUtil.h>

namespace Slice
{

class VbVisitor : public VbGenerator, public ParserVisitor
{
public:

    VbVisitor(::IceUtil::Output&);
    virtual ~VbVisitor();

protected:

    virtual void writeInheritedOperations(const ClassDefPtr&);
    virtual void writeDispatch(const ClassDefPtr&);
    virtual std::vector<std::string> getParams(const OperationPtr&);
    virtual std::vector<std::string> getParamsAsync(const OperationPtr&, bool);
    virtual std::vector<std::string> getParamsAsyncCB(const OperationPtr&);
    virtual std::vector<std::string> getArgs(const OperationPtr&);
    virtual std::vector<std::string> getArgsAsync(const OperationPtr&);
    virtual std::vector<std::string> getArgsAsyncCB(const OperationPtr&);

    void emitAttributes(const ContainedPtr&);
    ::std::string getParamAttributes(const ParamDeclPtr&);

    ::IceUtil::Output& _out;
};

class Gen : private ::IceUtil::noncopyable
{
public:

    Gen(const std::string&,
        const std::string&,
        const std::vector<std::string>&,
        const std::string&,
        bool,
        bool,
        bool);
    ~Gen();

    bool operator!() const; // Returns true if there was a constructor error

    void generate(const UnitPtr&);
    void generateTie(const UnitPtr&);
    void generateImpl(const UnitPtr&);
    void generateImplTie(const UnitPtr&);
    void generateChecksums(const UnitPtr&);

private:

    IceUtil::Output _out;
    IceUtil::Output _impl;

    std::vector<std::string> _includePaths;

    bool _stream;

    void printHeader();

    class UnitVisitor : public VbVisitor
    {
    public:

        UnitVisitor(::IceUtil::Output&, bool);

        virtual bool visitModuleStart(const ModulePtr&);

    private:

        bool _stream;
        bool _globalMetaDataDone;
    };

    class TypesVisitor : public VbVisitor
    {
    public:

        TypesVisitor(::IceUtil::Output&, bool);

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

        void invokeObjectMethod(const TypePtr&, const std::string&, const std::string&, const std::string&);

        bool _stream;
    };

    class ProxyVisitor : public VbVisitor
    {
    public:

        ProxyVisitor(::IceUtil::Output&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual void visitOperation(const OperationPtr&);
    };

    class OpsVisitor : public VbVisitor
    {
    public:

        OpsVisitor(::IceUtil::Output&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);

    private:
        void writeOperations(const ClassDefPtr&, bool);
    };

    class HelperVisitor : public VbVisitor
    {
    public:

        HelperVisitor(::IceUtil::Output&, bool);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual void visitSequence(const SequencePtr&);
        virtual void visitDictionary(const DictionaryPtr&);

    private:

        bool _stream;
    };

    class DelegateVisitor : public VbVisitor
    {
    public:

        DelegateVisitor(::IceUtil::Output&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
    };

    class DelegateMVisitor : public VbVisitor
    {
    public:

        DelegateMVisitor(::IceUtil::Output&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
    };

    class DelegateDVisitor : public VbVisitor
    {
    public:

        DelegateDVisitor(::IceUtil::Output&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
    };

    class DispatcherVisitor : public VbVisitor
    {
    public:

        DispatcherVisitor(::IceUtil::Output&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
    };

    class AsyncVisitor : public VbVisitor
    {
    public:

        AsyncVisitor(::IceUtil::Output&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual void visitOperation(const OperationPtr&);
    };

    class TieVisitor : public VbVisitor
    {
    public:

        TieVisitor(::IceUtil::Output&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);

    private:

        typedef ::std::set< ::std::string> NameSet;
        void writeInheritedOperationsWithOpNames(const ClassDefPtr&, NameSet&);
    };

    class BaseImplVisitor : public VbVisitor
    {
    public:

        BaseImplVisitor(::IceUtil::Output&);

    protected:

        void writeOperation(const OperationPtr&, bool, bool);

    private:

        ::std::string writeValue(const TypePtr&);
    };

    class ImplVisitor : public BaseImplVisitor
    {
    public:

        ImplVisitor(::IceUtil::Output&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
    };

    class ImplTieVisitor : public BaseImplVisitor
    {
    public:

        ImplTieVisitor(::IceUtil::Output&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
    };
};

}

#endif
