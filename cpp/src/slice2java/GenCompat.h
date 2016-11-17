// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef GEN_COMPAT_H
#define GEN_COMPAT_H

#include <Slice/Parser.h>
#include <Slice/JavaUtil.h>
#include <Slice/Checksum.h>

namespace Slice
{

class JavaCompatVisitor : public JavaCompatGenerator, public ParserVisitor
{
public:

    virtual ~JavaCompatVisitor();

protected:

    JavaCompatVisitor(const std::string&);

    enum ParamDir { InParam, OutParam };

    ParamDeclList getOutParams(const OperationPtr&);

    //
    // Compose the parameter lists for an operation.
    //
    std::vector<std::string> getParams(const OperationPtr&, const std::string&, bool, bool);
    std::vector<std::string> getParamsProxy(const OperationPtr&, const std::string&, bool, bool, bool = false);
    std::vector<std::string> getInOutParams(const OperationPtr&, const std::string&, ParamDir, bool, bool, bool = false);
    std::vector<std::string> getParamsAsync(const OperationPtr&, const std::string&, bool, bool);
    std::vector<std::string> getParamsAsyncCB(const OperationPtr&, const std::string&, bool, bool);

    std::string getAsyncCallbackInterface(const OperationPtr&, const std::string&);
    std::string getAsyncCallbackBaseClass(const OperationPtr&, bool);
    std::string getLambdaResponseCB(const OperationPtr&, const std::string&);
    std::vector<std::string> getParamsAsyncLambda(const OperationPtr&, const std::string&,
                                                  bool context = false, bool sentCB = false,
                                                  bool optionalMapping = false,
                                                  bool inParams = true,
                                                  bool internal = false);
    std::vector<std::string> getArgsAsyncLambda(const OperationPtr&, const std::string&,
                                                bool context = false,
                                                bool sentCB = false);

    //
    // Compose the argument lists for an operation.
    //
    std::vector<std::string> getArgs(const OperationPtr&);
    std::vector<std::string> getInOutArgs(const OperationPtr&, ParamDir, bool = false);
    std::vector<std::string> getArgsAsync(const OperationPtr&);
    std::vector<std::string> getArgsAsyncCB(const OperationPtr&);

    void writeMarshalUnmarshalParams(::IceUtilInternal::Output&, const std::string&, const ParamDeclList&,
                                     const OperationPtr&, int&, bool, bool, bool, const std::string& = "", bool = false);

    //
    // Generate a throws clause containing only non-local exceptions.
    //
    void writeThrowsClause(const std::string&, const ExceptionList&);

    //
    // Generate code to compute a hash code for a type.
    //
    void writeHashCode(::IceUtilInternal::Output&, const TypePtr&, const std::string&, int&,
                       const std::list<std::string>& = std::list<std::string>());

    //
    // Marshal/unmarshal a data member.
    //
    void writeMarshalDataMember(::IceUtilInternal::Output&, const std::string&, const DataMemberPtr&, int&, bool = false);
    void writeUnmarshalDataMember(::IceUtilInternal::Output&, const std::string&, const DataMemberPtr&, int&,
                                  bool, int&, bool = false);
    void writeStreamMarshalDataMember(::IceUtilInternal::Output&, const std::string&, const DataMemberPtr&, int&);
    void writeStreamUnmarshalDataMember(::IceUtilInternal::Output&, const std::string&, const DataMemberPtr&, int&,
                                        bool, int&);

    //
    // Generate a patcher class.
    //
    void writePatcher(::IceUtilInternal::Output&, const std::string&, const DataMemberList&, const DataMemberList&);

    //
    // Generate dispatch and marshalling methods for a class or interface.
    //
    void writeDispatchAndMarshalling(::IceUtilInternal::Output&, const ClassDefPtr&);

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
    // Write doc comments.
    //
    static StringList splitComment(const ContainedPtr&);
    static void writeDocComment(::IceUtilInternal::Output&, const ContainedPtr&,
                                const std::string&, const std::string& = "");
    static void writeDocComment(::IceUtilInternal::Output&, const std::string&, const std::string&);
    static void writeDocCommentOp(::IceUtilInternal::Output&, const OperationPtr&);

    static void writeDocCommentAsync(::IceUtilInternal::Output&, const OperationPtr&,
                                     ParamDir, const std::string& = "");
    static void writeDocCommentAMI(::IceUtilInternal::Output&, const OperationPtr&, ParamDir, const std::string& = "",
                                   const std::string& = "", const std::string& = "", const std::string& = "",
                                   const std::string& = "");
    static void writeDocCommentParam(::IceUtilInternal::Output&, const OperationPtr&, ParamDir, bool = true);
};

class GenCompat : private ::IceUtil::noncopyable
{
public:

    GenCompat(const std::string&,
              const std::string&,
              const std::vector<std::string>&,
              const std::string&,
              bool);
    ~GenCompat();

    void generate(const UnitPtr&);
    void generateImpl(const UnitPtr&);
    void generateImplTie(const UnitPtr&);

    static void writeChecksumClass(const std::string&, const std::string&, const ChecksumMap&);

private:

    std::string _base;
    std::vector<std::string> _includePaths;
    std::string _dir;
    bool _tie;

    class OpsVisitor : public JavaCompatVisitor
    {
    public:

        OpsVisitor(const std::string&);

        virtual bool visitClassDefStart(const ClassDefPtr&);

    private:
        void writeOperations(const ClassDefPtr&, bool);
    };

    class PackageVisitor : public JavaCompatVisitor
    {
    public:

        PackageVisitor(const std::string&);

        virtual bool visitModuleStart(const ModulePtr&);
    };

    class TypesVisitor : public JavaCompatVisitor
    {
    public:

        TypesVisitor(const std::string&);

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
        // Verifies that a data member method does not conflict with an operation.
        //
        bool validateMethod(const OperationList&, const std::string&, int, const std::string&, const std::string&);
    };

    class CompactIdVisitor : public JavaCompatVisitor
    {
    public:

        CompactIdVisitor(const std::string&);

        virtual bool visitClassDefStart(const ClassDefPtr&);
    };

    class HolderVisitor : public JavaCompatVisitor
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

    class HelperVisitor : public JavaCompatVisitor
    {
    public:

        HelperVisitor(const std::string&);

        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitSequence(const SequencePtr&);
        virtual void visitDictionary(const DictionaryPtr&);

    private:

        void writeOperation(const ClassDefPtr&, const std::string&, const OperationPtr&, bool);
    };

    class ProxyVisitor : public JavaCompatVisitor
    {
    public:

        ProxyVisitor(const std::string&);

        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual void visitOperation(const OperationPtr&);
    };

    class DispatcherVisitor : public JavaCompatVisitor
    {
    public:

        DispatcherVisitor(const std::string&, bool);

        virtual bool visitClassDefStart(const ClassDefPtr&);

    private:
        bool _tie;
    };

    class BaseImplVisitor : public JavaCompatVisitor
    {
    public:

        BaseImplVisitor(const std::string&);

    protected:

        //
        // Generate code to emit a local variable declaration and initialize it
        // if necessary.
        //
        void writeDecl(::IceUtilInternal::Output&, const std::string&, const std::string&, const TypePtr&,
                       const StringList&, bool);

        //
        // Generate code to return a value.
        //
        void writeReturn(::IceUtilInternal::Output&, const TypePtr&, bool);

        //
        // Generate an operation.
        //
        void writeOperation(::IceUtilInternal::Output&, const std::string&, const OperationPtr&, bool);
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

    class AsyncVisitor : public JavaCompatVisitor
    {
    public:

        AsyncVisitor(const std::string&);

        virtual void visitOperation(const OperationPtr&);
    };
};

}

#endif
