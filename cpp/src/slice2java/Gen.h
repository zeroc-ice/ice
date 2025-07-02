// Copyright (c) ZeroC, Inc.

#ifndef GEN_H
#define GEN_H

#include "../Slice/Parser.h"
#include "JavaUtil.h"

namespace Slice
{
    //
    // Generate code to marshal or unmarshal a type.
    //
    enum OptionalMode
    {
        OptionalNone,
        OptionalInParam,
        OptionalOutParam,
        OptionalReturnParam,
        OptionalMember
    };

    class JavaVisitor : public JavaGenerator, public ParserVisitor
    {
    public:
        ~JavaVisitor() override;

        static void writeMarshalUnmarshalCode(
            IceInternal::Output& out,
            const std::string& package,
            const TypePtr& type,
            OptionalMode mode,
            bool optionalMapping,
            std::int32_t tag,
            const std::string& param,
            bool marshal,
            int& iter,
            const std::string& customStream = "",
            const MetadataList& metadata = MetadataList(),
            const std::string& patchParams = "");

        /// Generate code to marshal or unmarshal a dictionary type.
        static void writeDictionaryMarshalUnmarshalCode(
            IceInternal::Output& out,
            const std::string& package,
            const DictionaryPtr& dict,
            const std::string& param,
            bool marshal,
            int& iter,
            bool useHelper,
            const std::string& customStream = "",
            const MetadataList& metadata = MetadataList());

        /// Generate code to marshal or unmarshal a sequence type.
        static void writeSequenceMarshalUnmarshalCode(
            IceInternal::Output& out,
            const std::string& package,
            const SequencePtr& seq,
            const std::string& param,
            bool marshal,
            int& iter,
            bool useHelper,
            const std::string& customStream = "",
            const MetadataList& metadata = MetadataList());

        static void writeResultTypeMarshalUnmarshalCode(
            IceInternal::Output& out,
            const OperationPtr& op,
            const std::string& package,
            const std::string& streamName,
            const std::string& paramPrefix,
            bool isMarshalling);

        static void writeResultType(
            IceInternal::Output& out,
            const OperationPtr& op,
            const std::string& package,
            const std::optional<DocComment>& comment);
        static void writeMarshaledResultType(
            IceInternal::Output& out,
            const OperationPtr& op,
            const std::string& package,
            const std::optional<DocComment>& comment);

        static void allocatePatcher(
            IceInternal::Output& out,
            const TypePtr& type,
            const std::string& package,
            const std::string& name);
        static std::string getPatcher(const TypePtr& type, const std::string& package, const std::string& dest);

        static void writeSyncIceInvokeMethods(
            IceInternal::Output& out,
            const OperationPtr& p,
            const std::vector<std::string>& params,
            const ExceptionList& throws,
            const std::optional<DocComment>& dc);

        static void writeAsyncIceInvokeMethods(
            IceInternal::Output& out,
            const OperationPtr& p,
            const std::vector<std::string>& params,
            const ExceptionList& throws,
            const std::optional<DocComment>& dc,
            bool optionalMapping);

        static void writeMarshalProxyParams(
            IceInternal::Output& out,
            const std::string& package,
            const OperationPtr& op,
            bool optionalMapping);
        static void
        writeUnmarshalProxyResults(IceInternal::Output& out, const std::string& package, const OperationPtr& op);
        static void writeMarshalServantResults(
            IceInternal::Output& out,
            const std::string& package,
            const OperationPtr& op,
            const std::string& param);

        /// Generate a throws clause containing only checked exceptions.
        /// @remark \p op is provided only when we want to check for the 'java:UserException' metadata.
        static void writeThrowsClause(
            IceInternal::Output& out,
            const std::string& package,
            const ExceptionList& throws,
            const OperationPtr& op = nullptr);

        //
        // Marshal/unmarshal a data member.
        //
        static void
        writeMarshalDataMember(IceInternal::Output&, const std::string&, const DataMemberPtr&, int&, bool = false);
        static void
        writeUnmarshalDataMember(IceInternal::Output&, const std::string&, const DataMemberPtr&, int&, bool = false);

        /// Write a constant or default value initializer.
        static void writeConstantValue(
            IceInternal::Output&,
            const TypePtr&,
            const SyntaxTreeBasePtr&,
            const std::string&,
            const std::string&);

        /// Generate assignment statements for those data members that have default values.
        static void writeDataMemberInitializers(IceInternal::Output&, const DataMemberList&, const std::string&);

        //
        // Handle doc comments.
        //
        static void writeExceptionDocComment(IceInternal::Output& out, const OperationPtr& op, const DocComment& dc);
        static void writeRemarksDocComment(IceInternal::Output& out, const DocComment& comment);
        static void writeHiddenDocComment(IceInternal::Output&);
        static void writeDocCommentLines(IceInternal::Output&, const StringList&);
        static void writeDocCommentLines(IceInternal::Output&, const std::string&);
        static void writeDocComment(IceInternal::Output&, const UnitPtr&, const std::optional<DocComment>&);
        static void writeDocComment(IceInternal::Output&, const std::string&);
        static void writeProxyOpDocComment(
            IceInternal::Output&,
            const OperationPtr&,
            const std::string&,
            const std::optional<DocComment>&,
            bool,
            const std::string&);
        static void writeHiddenProxyDocComment(IceInternal::Output&, const OperationPtr&);
        static void writeServantOpDocComment(IceInternal::Output&, const OperationPtr&, const std::string&, bool);
        static void writeSeeAlso(IceInternal::Output&, const UnitPtr&, const std::string&);
        static void writeParamDocComments(IceInternal::Output& out, const DataMemberList& members);

    protected:
        JavaVisitor(const std::string&);
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
