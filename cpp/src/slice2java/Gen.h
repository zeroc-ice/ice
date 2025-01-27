// Copyright (c) ZeroC, Inc.

#ifndef GEN_H
#define GEN_H

#include "../Slice/Parser.h"
#include "JavaUtil.h"

namespace Slice
{
    class JavaVisitor : public JavaGenerator, public ParserVisitor
    {
    public:
        ~JavaVisitor() override;

    protected:
        JavaVisitor(const std::string&);

        enum ParamDir
        {
            InParam,
            OutParam
        };

        std::string getResultType(const OperationPtr&, const std::string&, bool, bool);
        void writeResultType(
            ::IceInternal::Output&,
            const OperationPtr&,
            const std::string&,
            const std::optional<DocComment>&);
        void writeMarshaledResultType(
            ::IceInternal::Output&,
            const OperationPtr&,
            const std::string&,
            const std::optional<DocComment>&);

        void allocatePatcher(::IceInternal::Output&, const TypePtr&, const std::string&, const std::string&);
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

        void writeMarshalProxyParams(::IceInternal::Output&, const std::string&, const OperationPtr&, bool);
        void writeUnmarshalProxyResults(::IceInternal::Output&, const std::string&, const OperationPtr&);
        void
        writeMarshalServantResults(::IceInternal::Output&, const std::string&, const OperationPtr&, const std::string&);

        //
        // Generate a throws clause containing only checked exceptions.
        // op is provided only when we want to check for the java:UserException metadata
        //
        void writeThrowsClause(const std::string&, const ExceptionList&, const OperationPtr& op = nullptr);
        //
        // Marshal/unmarshal a data member.
        //
        void
        writeMarshalDataMember(::IceInternal::Output&, const std::string&, const DataMemberPtr&, int&, bool = false);
        void
        writeUnmarshalDataMember(::IceInternal::Output&, const std::string&, const DataMemberPtr&, int&, bool = false);

        //
        // Generate dispatch methods for an interface.
        //
        void writeDispatch(::IceInternal::Output&, const InterfaceDefPtr&);

        //
        // Generate marshaling methods for a class.
        //
        void writeMarshaling(::IceInternal::Output&, const ClassDefPtr&);

        //
        // Write a constant or default value initializer.
        //
        void writeConstantValue(
            ::IceInternal::Output&,
            const TypePtr&,
            const SyntaxTreeBasePtr&,
            const std::string&,
            const std::string&);

        //
        // Generate assignment statements for those data members that have default values.
        //
        void writeDataMemberInitializers(::IceInternal::Output&, const DataMemberList&, const std::string&);

        //
        // Handle doc comments.
        //
        void writeHiddenDocComment(::IceInternal::Output&);
        void writeDocCommentLines(::IceInternal::Output&, const StringList&);
        void writeDocCommentLines(::IceInternal::Output&, const std::string&);
        void writeDocComment(::IceInternal::Output&, const UnitPtr&, const std::optional<DocComment>&);
        void writeDocComment(::IceInternal::Output&, const std::string&);
        void writeProxyDocComment(
            ::IceInternal::Output&,
            const OperationPtr&,
            const std::string&,
            const std::optional<DocComment>&,
            bool,
            const std::string&);
        void writeHiddenProxyDocComment(::IceInternal::Output&, const OperationPtr&);
        void writeServantDocComment(::IceInternal::Output&, const OperationPtr&, const std::string&, bool);
        void writeSeeAlso(::IceInternal::Output&, const UnitPtr&, const std::string&);
    };

    class Gen final
    {
    public:
        Gen(const std::string&, std::string, const std::vector<std::string>&, std::string);
        Gen(const Gen&) = delete;
        ~Gen();

        Gen& operator=(const Gen&) = delete;

        void generate(const UnitPtr&);

    private:
        std::string _base;
        std::vector<std::string> _includePaths;
        std::string _dir;

        class PackageVisitor final : public JavaVisitor
        {
        public:
            PackageVisitor(const std::string&);

            bool visitModuleStart(const ModulePtr&) final;
        };

        class TypesVisitor final : public JavaVisitor
        {
        public:
            TypesVisitor(const std::string&);

            bool visitClassDefStart(const ClassDefPtr&) final;
            void visitClassDefEnd(const ClassDefPtr&) final;
            bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
            void visitInterfaceDefEnd(const InterfaceDefPtr&) final;
            void visitOperation(const OperationPtr&) final;
            bool visitExceptionStart(const ExceptionPtr&) final;
            void visitExceptionEnd(const ExceptionPtr&) final;
            bool visitStructStart(const StructPtr&) final;
            void visitStructEnd(const StructPtr&) final;
            void visitDataMember(const DataMemberPtr&) final;
            void visitEnum(const EnumPtr&) final;
            void visitConst(const ConstPtr&) final;
        };

        class CompactIdVisitor final : public JavaVisitor
        {
        public:
            CompactIdVisitor(const std::string&);

            bool visitClassDefStart(const ClassDefPtr&) final;
        };

        class HelperVisitor final : public JavaVisitor
        {
        public:
            HelperVisitor(const std::string&);

            void visitSequence(const SequencePtr&) final;
            void visitDictionary(const DictionaryPtr&) final;
        };

        class ProxyVisitor final : public JavaVisitor
        {
        public:
            ProxyVisitor(const std::string&);

            bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
            void visitInterfaceDefEnd(const InterfaceDefPtr&) final;
            void visitOperation(const OperationPtr&) final;
        };
    };
}

#endif
