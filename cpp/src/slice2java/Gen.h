//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef GEN_H
#define GEN_H

#include "../Slice/Parser.h"
#include "JavaUtil.h"

namespace Slice
{
    class JavaVisitor : public JavaGenerator, public ParserVisitor
    {
    public:
        virtual ~JavaVisitor();

    protected:
        JavaVisitor(const std::string&);

        enum ParamDir
        {
            InParam,
            OutParam
        };

        std::string getResultType(const OperationPtr&, const std::string&, bool, bool);
        void writeResultType(::IceUtilInternal::Output&, const OperationPtr&, const std::string&, const CommentPtr&);
        void writeMarshaledResultType(
            ::IceUtilInternal::Output&,
            const OperationPtr&,
            const std::string&,
            const CommentPtr&);

        void allocatePatcher(::IceUtilInternal::Output&, const TypePtr&, const std::string&, const std::string&);
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
        void writeMarshalServantResults(
            ::IceUtilInternal::Output&,
            const std::string&,
            const OperationPtr&,
            const std::string&);

        //
        // Generate a throws clause containing only checked exceptions.
        // op is provided only when we want to check for the java:UserException metadata
        //
        void writeThrowsClause(const std::string&, const ExceptionList&, const OperationPtr& op = 0);
        //
        // Marshal/unmarshal a data member.
        //
        void writeMarshalDataMember(
            ::IceUtilInternal::Output&,
            const std::string&,
            const DataMemberPtr&,
            int&,
            bool = false);
        void writeUnmarshalDataMember(
            ::IceUtilInternal::Output&,
            const std::string&,
            const DataMemberPtr&,
            int&,
            bool = false);

        //
        // Generate dispatch methods for an interface.
        //
        void writeDispatch(::IceUtilInternal::Output&, const InterfaceDefPtr&);

        //
        // Generate marshaling methods for a class.
        //
        void writeMarshaling(::IceUtilInternal::Output&, const ClassDefPtr&);

        //
        // Write a constant or default value initializer.
        //
        void writeConstantValue(
            ::IceUtilInternal::Output&,
            const TypePtr&,
            const SyntaxTreeBasePtr&,
            const std::string&,
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
        void writeProxyDocComment(
            ::IceUtilInternal::Output&,
            const OperationPtr&,
            const std::string&,
            const CommentPtr&,
            bool,
            const std::string&);
        void writeHiddenProxyDocComment(::IceUtilInternal::Output&, const OperationPtr&);
        void writeServantDocComment(
            ::IceUtilInternal::Output&,
            const OperationPtr&,
            const std::string&,
            const CommentPtr&,
            bool);
        void writeSeeAlso(::IceUtilInternal::Output&, const UnitPtr&, const std::string&);
    };

    class Gen
    {
    public:
        Gen(const std::string&, const std::string&, const std::vector<std::string>&, const std::string&);
        Gen(const Gen&) = delete;
        ~Gen();

        Gen& operator=(const Gen&) = delete;

        void generate(const UnitPtr&);

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
            virtual bool visitInterfaceDefStart(const InterfaceDefPtr&);
            virtual void visitInterfaceDefEnd(const InterfaceDefPtr&);
            virtual void visitOperation(const OperationPtr&);
            virtual bool visitExceptionStart(const ExceptionPtr&);
            virtual void visitExceptionEnd(const ExceptionPtr&);
            virtual bool visitStructStart(const StructPtr&);
            virtual void visitStructEnd(const StructPtr&);
            virtual void visitDataMember(const DataMemberPtr&);
            virtual void visitEnum(const EnumPtr&);
            virtual void visitConst(const ConstPtr&);
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

            virtual bool visitInterfaceDefStart(const InterfaceDefPtr&);
            virtual void visitInterfaceDefEnd(const InterfaceDefPtr&);
            virtual void visitOperation(const OperationPtr&);
        };
    };
}

#endif
