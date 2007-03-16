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

//
// The Ice-E version.
//
#define ICEE_STRING_VERSION "1.2.0" // "A.B.C", with A=major, B=minor, C=patch
#define ICEE_INT_VERSION 10200      // AABBCC, with AA=major, BB=minor, CC=patch

namespace Slice
{

class JavaEOutput : public JavaOutput
{
public:

    virtual void printHeader();
};

class JavaVisitor : public JavaGenerator, public ParserVisitor
{
public:

    virtual ~JavaVisitor();

protected:

    JavaVisitor(const std::string&);

    virtual JavaOutput* createOutput();

    //
    // Compose the parameter lists for an operation.
    //
    std::vector<std::string> getParams(const OperationPtr&, const std::string&);

    //
    // Compose the argument lists for an operation.
    //
    std::vector<std::string> getArgs(const OperationPtr&);

    //
    // Generate a throws clause containing only non-local exceptions.
    //
    void writeThrowsClause(const std::string&, const ExceptionList&);

    //
    // Generate code to compute a hash code for a type.
    //
    void writeHashCode(::IceUtil::Output&, const TypePtr&, const std::string&, int&);

    //
    // Generate dispatch methods for a class or interface.
    //
    void writeDispatch(::IceUtil::Output&, const ClassDefPtr&);
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

    void generate(const UnitPtr&);
    void generateTie(const UnitPtr&);
    void generateImpl(const UnitPtr&);
    void generateImplTie(const UnitPtr&);

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

        TypesVisitor(const std::string&);

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

    class HolderVisitor : public JavaVisitor
    {
    public:

        HolderVisitor(const std::string&);

        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual bool visitStructStart(const StructPtr&);
        virtual void visitSequence(const SequencePtr&);
        virtual void visitDictionary(const DictionaryPtr&);
        virtual void visitEnum(const EnumPtr&);

    private:

        void writeHolder(const TypePtr&);
    };

    class HelperVisitor : public JavaVisitor
    {
    public:

        HelperVisitor(const std::string&);

        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual bool visitStructStart(const StructPtr&);
        virtual void visitSequence(const SequencePtr&);
        virtual void visitDictionary(const DictionaryPtr&);
        virtual void visitEnum(const EnumPtr&);
    };

    class ProxyVisitor : public JavaVisitor
    {
    public:

        ProxyVisitor(const std::string&);

        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual void visitOperation(const OperationPtr&);
    };

    class DispatcherVisitor : public JavaVisitor
    {
    public:

        DispatcherVisitor(const std::string&);

        virtual bool visitClassDefStart(const ClassDefPtr&);
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
};

}

#endif
