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

#include <Slice/Parser.h>
#include <Slice/JavaUtil.h>
#include <Slice/Checksum.h>

namespace Slice
{

class JavaVisitor : public JavaGenerator, public ParserVisitor
{
public:

    virtual ~JavaVisitor();

protected:

    JavaVisitor(const std::string&);

    //
    // Compose the parameter lists for an operation.
    //
    std::vector<std::string> getParams(const OperationPtr&, const std::string&, bool = false);
    std::vector<std::string> getParamsAsync(const OperationPtr&, const std::string&, bool);
    std::vector<std::string> getParamsAsyncCB(const OperationPtr&, const std::string&);

    //
    // Compose the argument lists for an operation.
    //
    std::vector<std::string> getArgs(const OperationPtr&);
    std::vector<std::string> getArgsAsync(const OperationPtr&);
    std::vector<std::string> getArgsAsyncCB(const OperationPtr&);

    //
    // Generate a throws clause containing only non-local exceptions.
    //
    void writeThrowsClause(const std::string&, const ExceptionList&);

    //
    // Generate a throws clause for delegate operations containing only
    // non-local exceptions.
    //
    void writeDelegateThrowsClause(const std::string&, const ExceptionList&);

    //
    // Generate code to compute a hash code for a type.
    //
    void writeHashCode(::IceUtil::Output&, const TypePtr&, const std::string&, int&,
                       const std::list<std::string>& = std::list<std::string>());

    //
    // Generate dispatch and marshalling methods for a class or interface.
    //
    void writeDispatchAndMarshalling(::IceUtil::Output&, const ClassDefPtr&, bool);
};

class Gen : private ::IceUtil::noncopyable
{
public:

    Gen(const std::string&,
        const std::string&,
        const std::vector<std::string>&,
        const std::string&);
    ~Gen();

    bool operator!() const; // Returns true if there was a constructor error

    void generate(const UnitPtr&, bool);
    void generateTie(const UnitPtr&);
    void generateImpl(const UnitPtr&);
    void generateImplTie(const UnitPtr&);

    static void writeChecksumClass(const std::string&, const std::string&, const ChecksumMap&, bool);

private:

    std::string _base;
    std::vector<std::string> _includePaths;
    std::string _dir;

    class OpsVisitor : public JavaVisitor
    {
    public:

        OpsVisitor(const std::string&);

        virtual bool visitClassDefStart(const ClassDefPtr&);

    private:
        void writeOperations(const ClassDefPtr&, bool);
    };

    class TieVisitor : public JavaVisitor
    {
    public:

        TieVisitor(const std::string&);

        virtual bool visitClassDefStart(const ClassDefPtr&);
    };

    class PackageVisitor : public JavaVisitor
    {
    public:

        PackageVisitor(const std::string&);

        virtual bool visitModuleStart(const ModulePtr&);
    };

    class TypesVisitor : public JavaVisitor
    {
    public:

        TypesVisitor(const std::string&, bool);

        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual bool visitExceptionStart(const ExceptionPtr&);
        virtual void visitExceptionEnd(const ExceptionPtr&);
        virtual bool visitStructStart(const StructPtr&);
        virtual void visitStructEnd(const StructPtr&);
        virtual void visitDataMember(const DataMemberPtr&);
        virtual void visitEnum(const EnumPtr&);
        virtual void visitConst(const ConstPtr&);

    private:

        //
        // Verifies that a getter/setter method does not conflict with an operation.
        //
        bool validateGetterSetter(const OperationList&, const std::string&, int, const std::string&,
                                  const std::string&);

        bool _stream;
    };

    class HolderVisitor : public JavaVisitor
    {
    public:

        HolderVisitor(const std::string&, bool);

        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual bool visitStructStart(const StructPtr&);
        virtual void visitSequence(const SequencePtr&);
        virtual void visitDictionary(const DictionaryPtr&);
        virtual void visitEnum(const EnumPtr&);

    private:

        void writeHolder(const TypePtr&);

        bool _stream;
    };

    class HelperVisitor : public JavaVisitor
    {
    public:

        HelperVisitor(const std::string&, bool);

        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual bool visitStructStart(const StructPtr&);
        virtual void visitSequence(const SequencePtr&);
        virtual void visitDictionary(const DictionaryPtr&);
        virtual void visitEnum(const EnumPtr&);

    private:

        bool _stream;
    };

    class ProxyVisitor : public JavaVisitor
    {
    public:

        ProxyVisitor(const std::string&);

        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual void visitOperation(const OperationPtr&);
    };

    class DelegateVisitor : public JavaVisitor
    {
    public:

        DelegateVisitor(const std::string&);

        virtual bool visitClassDefStart(const ClassDefPtr&);
    };

    class DelegateMVisitor : public JavaVisitor
    {
    public:

        DelegateMVisitor(const std::string&);

        virtual bool visitClassDefStart(const ClassDefPtr&);
    };

    class DelegateDVisitor : public JavaVisitor
    {
    public:

        DelegateDVisitor(const std::string&);

        virtual bool visitClassDefStart(const ClassDefPtr&);
    };

    class DispatcherVisitor : public JavaVisitor
    {
    public:

        DispatcherVisitor(const std::string&, bool);

        virtual bool visitClassDefStart(const ClassDefPtr&);

    private:

        bool _stream;
    };

    class BaseImplVisitor : public JavaVisitor
    {
    public:

        BaseImplVisitor(const std::string&);

    protected:

        //
        // Generate code to emit a local variable declaration and initialize it
        // if necessary.
        //
        void writeDecl(::IceUtil::Output&, const std::string&, const std::string&, const TypePtr&, const StringList&);

        //
        // Generate code to return a value.
        //
        void writeReturn(::IceUtil::Output&, const TypePtr&);

        //
        // Generate an operation.
        //
        void writeOperation(::IceUtil::Output&, const std::string&, const OperationPtr&, bool);
    };

    class ImplVisitor : public BaseImplVisitor
    {
    public:

        ImplVisitor(const std::string&);

        virtual bool visitClassDefStart(const ClassDefPtr&);
    };

    class ImplTieVisitor : public BaseImplVisitor
    {
    public:

        ImplTieVisitor(const std::string&);

        virtual bool visitClassDefStart(const ClassDefPtr&);
    };

    class AsyncVisitor : public JavaVisitor
    {
    public:

        AsyncVisitor(const std::string&);

        virtual void visitOperation(const OperationPtr&);
    };
};

}

#endif
