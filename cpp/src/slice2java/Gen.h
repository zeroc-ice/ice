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

        void writeResultTypeMarshalUnmarshalCode(
            IceInternal::Output& out,
            const OperationPtr& op,
            const std::string& package,
            const std::string& streamName,
            const std::string& paramPrefix,
            bool isMarshalling);

        void writeResultType(
            IceInternal::Output&,
            const OperationPtr&,
            const std::string&,
            const std::optional<DocComment>&);
        void writeMarshaledResultType(
            IceInternal::Output&,
            const OperationPtr&,
            const std::string&,
            const std::optional<DocComment>&);

        void allocatePatcher(IceInternal::Output&, const TypePtr&, const std::string&, const std::string&);
        std::string getPatcher(const TypePtr&, const std::string&, const std::string&);

        /// Returns a vector of this operation's parameters with each of them formatted as 'paramType paramName'.
        /// If 'internal' is true, the names will be prefixed with "iceP_".
        std::vector<std::string>
        getParamsProxy(const OperationPtr& op, const std::string& package, bool optionalMapping, bool internal = false);

        /// Returns a vector of this operation's parameter's names in order.
        /// If 'internal' is true, the names will be prefixed with "iceP_".
        std::vector<std::string> getInArgs(const OperationPtr& op, bool internal = false);

        void writeSyncIceInvokeMethods(
            IceInternal::Output& out,
            const OperationPtr& p,
            const std::vector<std::string>& params,
            const ExceptionList& throws,
            const std::optional<DocComment>& dc);

        void writeAsyncIceInvokeMethods(
            IceInternal::Output& out,
            const OperationPtr& p,
            const std::vector<std::string>& params,
            const ExceptionList& throws,
            const std::optional<DocComment>& dc,
            bool optionalMapping);

        void writeMarshalProxyParams(IceInternal::Output&, const std::string&, const OperationPtr&, bool);
        void writeUnmarshalProxyResults(IceInternal::Output&, const std::string&, const OperationPtr&);
        void
        writeMarshalServantResults(IceInternal::Output&, const std::string&, const OperationPtr&, const std::string&);

        //
        // Generate a throws clause containing only checked exceptions.
        // op is provided only when we want to check for the java:UserException metadata
        //
        void writeThrowsClause(const std::string&, const ExceptionList&, const OperationPtr& op = nullptr);
        //
        // Marshal/unmarshal a data member.
        //
        void writeMarshalDataMember(IceInternal::Output&, const std::string&, const DataMemberPtr&, int&, bool = false);
        void
        writeUnmarshalDataMember(IceInternal::Output&, const std::string&, const DataMemberPtr&, int&, bool = false);

        //
        // Write a constant or default value initializer.
        //
        void writeConstantValue(
            IceInternal::Output&,
            const TypePtr&,
            const SyntaxTreeBasePtr&,
            const std::string&,
            const std::string&);

        //
        // Generate assignment statements for those data members that have default values.
        //
        void writeDataMemberInitializers(IceInternal::Output&, const DataMemberList&, const std::string&);

        //
        // Handle doc comments.
        //
        void writeExceptionDocComment(IceInternal::Output& out, const OperationPtr& op, const DocComment& dc);
        void writeRemarksDocComment(IceInternal::Output& out, const DocComment& comment);
        void writeHiddenDocComment(IceInternal::Output&);
        void writeDocCommentLines(IceInternal::Output&, const StringList&);
        void writeDocCommentLines(IceInternal::Output&, const std::string&);
        void writeDocComment(IceInternal::Output&, const UnitPtr&, const std::optional<DocComment>&);
        void writeDocComment(IceInternal::Output&, const std::string&);
        void writeProxyOpDocComment(
            IceInternal::Output&,
            const OperationPtr&,
            const std::string&,
            const std::optional<DocComment>&,
            bool,
            const std::string&);
        void writeHiddenProxyDocComment(IceInternal::Output&, const OperationPtr&);
        void writeServantOpDocComment(IceInternal::Output&, const OperationPtr&, const std::string&, bool);
        void writeSeeAlso(IceInternal::Output&, const UnitPtr&, const std::string&);
        void writeParamDocComments(IceInternal::Output& out, const DataMemberList& members);
    };

    class Gen final
    {
    public:
        Gen(std::string, const std::vector<std::string>&, std::string);
        Gen(const Gen&) = delete;
        ~Gen();

        Gen& operator=(const Gen&) = delete;

        void generate(const UnitPtr&);

    private:
        std::string _base;
        std::vector<std::string> _includePaths;
        std::string _dir;

        class TypesVisitor final : public JavaVisitor
        {
        public:
            TypesVisitor(const std::string&);

            bool visitModuleStart(const ModulePtr&) final;

            bool visitClassDefStart(const ClassDefPtr&) final;
            void visitClassDefEnd(const ClassDefPtr&) final;
            bool visitExceptionStart(const ExceptionPtr&) final;
            void visitExceptionEnd(const ExceptionPtr&) final;
            bool visitStructStart(const StructPtr&) final;
            void visitStructEnd(const StructPtr&) final;
            void visitDataMember(const DataMemberPtr&) final;
            void visitEnum(const EnumPtr&) final;
            void visitSequence(const SequencePtr&) final;
            void visitDictionary(const DictionaryPtr&) final;

            void visitConst(const ConstPtr&) final;

            // Generate proxy classes
            bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
            void visitInterfaceDefEnd(const InterfaceDefPtr&) final;
            void visitOperation(const OperationPtr&) final;
        };

        class ServantVisitor final : public JavaVisitor
        {
        public:
            ServantVisitor(const std::string& dir);

            bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
            void visitInterfaceDefEnd(const InterfaceDefPtr&) final;
            void visitOperation(const OperationPtr&) final;
        };
    };
}

#endif
