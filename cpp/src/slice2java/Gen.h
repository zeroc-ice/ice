// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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

    enum ParamDir { InParam, OutParam };

    std::string getResultType(const OperationPtr&, const std::string&, bool, bool);
    void writeResultType(::IceUtilInternal::Output&, const OperationPtr&, const std::string&, const CommentPtr&);
    void writeMarshaledResultType(::IceUtilInternal::Output&, const OperationPtr&, const std::string&,
                                  const CommentPtr&);

    void allocatePatcher(::IceUtilInternal::Output&, const TypePtr&, const std::string&, const std::string&, bool);
    std::string getPatcher(const TypePtr&, const std::string&, const std::string&);

    std::string getFutureType(const OperationPtr&, const std::string&);
    std::string getFutureImplType(const OperationPtr&, const std::string&);

    //
    // Compose the parameter lists for an operation.
    //
    std::vector<std::string> getParams(const OperationPtr&, const std::string&);
    std::vector<std::string> getParamsProxy(const OperationPtr&, const std::string&, bool, bool = false);

    //
    // Compose the argument lists for an operation.
    //
    std::vector<std::string> getArgs(const OperationPtr&);
    std::vector<std::string> getInArgs(const OperationPtr&, bool = false);

    void writeMarshalProxyParams(::IceUtilInternal::Output&, const std::string&, const OperationPtr&, bool);
    void writeUnmarshalProxyResults(::IceUtilInternal::Output&, const std::string&, const OperationPtr&);
    void writeMarshalServantResults(::IceUtilInternal::Output&, const std::string&, const OperationPtr&,
                                    const std::string&);

    //
    // Generate a throws clause containing only checked exceptions.
    // op is provided only when we want to check for the java:UserException metadata
    //
    void writeThrowsClause(const std::string&, const ExceptionList&, const OperationPtr& op = 0);

    //
    // Generate code to compute a hash code for a type.
    //
    void writeHashCode(::IceUtilInternal::Output&, const TypePtr&, const std::string&, int&,
                       const std::list<std::string>& = std::list<std::string>());

    //
    // Marshal/unmarshal a data member.
    //
    void writeMarshalDataMember(::IceUtilInternal::Output&, const std::string&, const DataMemberPtr&, int&, bool = false);
    void writeUnmarshalDataMember(::IceUtilInternal::Output&, const std::string&, const DataMemberPtr&, int&, bool = false);

    //
    // Generate dispatch methods for a class or interface.
    //
    void writeDispatch(::IceUtilInternal::Output&, const ClassDefPtr&);

    //
    // Generate marshaling methods for a class or interface.
    //
    void writeMarshaling(::IceUtilInternal::Output&, const ClassDefPtr&);

    //
    // Write a constant or default value initializer.
    //
    void writeConstantValue(::IceUtilInternal::Output&, const TypePtr&, const SyntaxTreeBasePtr&, const std::string&,
                            const std::string&);

    //
    // Generate assignment statements for those data members that have default values.
    //
    void writeDataMemberInitializers(::IceUtilInternal::Output&, const DataMemberList&, const std::string&);

    //
    // Handle doc comments.
    //
    static StringList splitComment(const ContainedPtr&);
    void writeHiddenDocComment(::IceUtilInternal::Output&);
    void writeDocCommentLines(::IceUtilInternal::Output&, const StringList&);
    void writeDocCommentLines(::IceUtilInternal::Output&, const std::string&);
    void writeDocComment(::IceUtilInternal::Output&, const UnitPtr&, const CommentPtr&);
    void writeDocComment(::IceUtilInternal::Output&, const std::string&);
    void writeProxyDocComment(::IceUtilInternal::Output&, const OperationPtr&, const std::string&, const CommentPtr&,
                              bool, const std::string&);
    void writeHiddenProxyDocComment(::IceUtilInternal::Output&, const OperationPtr&);
    void writeServantDocComment(::IceUtilInternal::Output&, const OperationPtr&, const std::string&,
                                const CommentPtr&, bool);
    void writeSeeAlso(::IceUtilInternal::Output&, const UnitPtr&, const std::string&);
};

class Gen : private ::IceUtil::noncopyable
{
public:

    Gen(const std::string&,
        const std::string&,
        const std::vector<std::string>&,
        const std::string&);
    ~Gen();

    void generate(const UnitPtr&);
    void generateImpl(const UnitPtr&);

    static void writeChecksumClass(const std::string&, const std::string&, const ChecksumMap&);

private:

    std::string _base;
    std::vector<std::string> _includePaths;
    std::string _dir;

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
        virtual void visitOperation(const OperationPtr&);
        virtual bool visitExceptionStart(const ExceptionPtr&);
        virtual void visitExceptionEnd(const ExceptionPtr&);
        virtual bool visitStructStart(const StructPtr&);
        virtual void visitStructEnd(const StructPtr&);
        virtual void visitDataMember(const DataMemberPtr&);
        virtual void visitEnum(const EnumPtr&);
        virtual void visitConst(const ConstPtr&);

    private:

        //
        // Verifies that a data member method does not conflict with an operation.
        //
        bool validateMethod(const OperationList&, const std::string&, int, const std::string&, const std::string&);
    };

    class CompactIdVisitor : public JavaVisitor
    {
    public:

        CompactIdVisitor(const std::string&);

        virtual bool visitClassDefStart(const ClassDefPtr&);
    };

    class HelperVisitor : public JavaVisitor
    {
    public:

        HelperVisitor(const std::string&);

        virtual void visitSequence(const SequencePtr&);
        virtual void visitDictionary(const DictionaryPtr&);
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

    class ImplVisitor : public JavaVisitor
    {
    public:

        ImplVisitor(const std::string&);

        virtual bool visitClassDefStart(const ClassDefPtr&);

    protected:

        //
        // Returns a default value for the type.
        //
        std::string getDefaultValue(const std::string&, const TypePtr&, bool);

        //
        // Generate code to initialize the operation result.
        //
        bool initResult(::IceUtilInternal::Output&, const std::string&, const OperationPtr&);

        //
        // Generate an operation.
        //
        void writeOperation(::IceUtilInternal::Output&, const std::string&, const OperationPtr&, bool);
    };
};

}

#endif
