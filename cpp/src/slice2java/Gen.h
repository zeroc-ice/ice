// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef GEN_H
#define GEN_H

#include <Slice/Parser.h>
#include <IceUtil/OutputUtil.h>
//#include <JavaUtil.h>

namespace Slice
{

class JavaVisitor : public ::IceUtil::noncopyable, public ParserVisitor
{
public:

    virtual ~JavaVisitor();

protected:

    JavaVisitor(const std::string&, const std::string&);

    //
    // Given the fully-scoped Java class name, create any intermediate
    // package directories and open the class file
    //
    bool open(const std::string&);
    void close();

    ::IceUtil::Output& output() const;

    //
    // Check a symbol against any of the Java keywords. If a
    // match is found, return the symbol with a leading underscore.
    //
    std::string fixKwd(const std::string&) const;

    //
    // Convert a scoped name into a Java class name. If an optional
    // scope is provided, the scope will be removed from the result.
    //
    std::string getAbsolute(const std::string&,
                            const std::string& = std::string(),
                            const std::string& = std::string(),
                            const std::string& = std::string()) const;

    //
    // Get the Java name for a type. If an optional scope is provided,
    // the scope will be removed from the result if possible.
    //
    enum TypeMode
    {
        TypeModeIn,
        TypeModeOut,
        TypeModeMember,
        TypeModeReturn
    };
    std::string typeToString(const TypePtr&, TypeMode mode,
                             const std::string& = std::string()) const;

    //
    // Compose the parameter list for an operation
    //
    std::string getParams(const OperationPtr&, const std::string&);

    //
    // Compose the argument list for an operation
    //
    std::string getArgs(const OperationPtr&, const std::string&);

    //
    // Generate a throws clause containing only non-local exceptions
    //
    void writeThrowsClause(const std::string&, const ExceptionList&);

    //
    // Generate a throws clause for delegate operations containing only
    // non-local exceptions
    //
    void writeDelegateThrowsClause(const std::string&, const ExceptionList&);

    //
    // Generate code to marshal or unmarshal a type
    //
    void writeMarshalUnmarshalCode(::IceUtil::Output&, const std::string&, const TypePtr&,
                                   const std::string&, bool, int&,
                                   bool = false);

    //
    // Generate generic code to marshal or unmarshal a type
    //
    void writeGenericMarshalUnmarshalCode(::IceUtil::Output&, const std::string&, const TypePtr&,
                                          const std::string&, const std::string&, bool, int&,
                                          bool = false);

    //
    // Generate code to compute a hash code for a type
    //
    void writeHashCode(::IceUtil::Output&, const TypePtr&, const std::string&, int&);

    //
    // Generate dispatch methods for a class or interface
    //
    void writeDispatch(::IceUtil::Output&, const ClassDefPtr&);

private:

    void printHeader();

    std::string _dir;
    std::string _package;

    ::IceUtil::Output* _out;
};

class Gen : public ::IceUtil::noncopyable
{
public:

    Gen(const std::string&,
        const std::string&,
        const std::vector<std::string>&,
        const std::string&,
        const std::string&);
    ~Gen();

    bool operator!() const; // Returns true if there was a constructor error

    void generate(const UnitPtr&);
    void generateImpl(const UnitPtr&);

private:

    std::string _base;
    std::vector<std::string> _includePaths;
    std::string _package;
    std::string _dir;

    class TypesVisitor : public JavaVisitor
    {
    public:

        TypesVisitor(const std::string&, const std::string&);

        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual void visitOperation(const OperationPtr&);
        virtual bool visitExceptionStart(const ExceptionPtr&);
        virtual void visitExceptionEnd(const ExceptionPtr&);
        virtual bool visitStructStart(const StructPtr&);
        virtual void visitStructEnd(const StructPtr&);
        virtual void visitEnum(const EnumPtr&);
        virtual void visitDataMember(const DataMemberPtr&);
    };

    class HolderVisitor : public JavaVisitor
    {
    public:

        HolderVisitor(const std::string&, const std::string&);

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

        HelperVisitor(const std::string&, const std::string&);

        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitSequence(const SequencePtr&);
        virtual void visitDictionary(const DictionaryPtr&);
    };

    class ProxyVisitor : public JavaVisitor
    {
    public:

        ProxyVisitor(const std::string&, const std::string&);

        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual void visitOperation(const OperationPtr&);
    };

    class DelegateVisitor : public JavaVisitor
    {
    public:

        DelegateVisitor(const std::string&, const std::string&);

        virtual bool visitClassDefStart(const ClassDefPtr&);
    };

    class DelegateMVisitor : public JavaVisitor
    {
    public:

        DelegateMVisitor(const std::string&, const std::string&);

        virtual bool visitClassDefStart(const ClassDefPtr&);
    };

    class DelegateDVisitor : public JavaVisitor
    {
    public:

        DelegateDVisitor(const std::string&, const std::string&);

        virtual bool visitClassDefStart(const ClassDefPtr&);
    };

    class DispatcherVisitor : public JavaVisitor
    {
    public:

        DispatcherVisitor(const std::string&, const std::string&);

        virtual bool visitClassDefStart(const ClassDefPtr&);
    };

    class ImplVisitor : public JavaVisitor
    {
        //
        // Generate code to assign a value
        //
        void writeAssign(::IceUtil::Output&, const std::string&, const TypePtr&,
                         const std::string&, int&);

    public:

        ImplVisitor(const std::string&, const std::string&);

        virtual bool visitClassDefStart(const ClassDefPtr&);
    };
};

}

#endif
