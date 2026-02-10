// Copyright (c) ZeroC, Inc.

#ifndef GEN_H
#define GEN_H

#include "CsUtil.h"

namespace Slice
{
    /// Base visitor class for C# code generation.
    class CsVisitor : public ParserVisitor
    {
    public:
        CsVisitor(IceInternal::Output&);

    protected:
        void emitNonBrowsableAttribute();

        /// Generates C# attributes from any 'cs:attribute' metadata.
        void emitAttributes(const ContainedPtr&);

        void emitObsoleteAttribute(const ContainedPtr&);

        /// Writes a doc-comment for the given Slice element, using this element's doc-comment, if any.
        /// @param p The Slice element.
        /// @param generatedType The kind of mapped element, used for the remarks. For example, "skeleton interface".
        /// This function does not write any remarks when this argument is empty.
        /// @param notes Optional notes included at the end of the remarks.
        void
        writeDocComment(const ContainedPtr& p, const std::string& generatedType = "", const std::string& notes = "");

        /// Writes a doc-comment for a helper class generated for a Slice element.
        /// @param p The Slice element.
        /// @param comment The summary.
        /// @param generatedType The kind of mapped element, used for the remarks. Must not be empty.
        /// @param notes Optional notes included at the end of the remarks.
        void writeHelperDocComment(
            const ContainedPtr& p,
            const std::string& comment,
            const std::string& generatedType,
            const std::string& notes = "");

        void
        writeOpDocComment(const OperationPtr& operation, const std::vector<std::string>& extraParams, bool isAsync);
        void writeParameterDocComments(const DocComment&, const ParameterList&);

        void namespacePrefixStart(const ModulePtr&);
        void namespacePrefixEnd(const ModulePtr&);

        IceInternal::Output& _out;
    };

    /// Generates code for Slice types (including proxies) and Slice exceptions.
    class TypesVisitor final : public CsVisitor
    {
    public:
        TypesVisitor(IceInternal::Output&);

        bool visitModuleStart(const ModulePtr&) final;
        void visitModuleEnd(const ModulePtr&) final;
        bool visitClassDefStart(const ClassDefPtr&) final;
        void visitClassDefEnd(const ClassDefPtr&) final;
        bool visitExceptionStart(const ExceptionPtr&) final;
        void visitExceptionEnd(const ExceptionPtr&) final;
        bool visitStructStart(const StructPtr&) final;
        void visitStructEnd(const StructPtr&) final;
        void visitSequence(const SequencePtr&) final;
        void visitDictionary(const DictionaryPtr&) final;
        void visitEnum(const EnumPtr&) final;
        void visitConst(const ConstPtr&) final;
        void visitDataMember(const DataMemberPtr&) final;

        // For proxies
        bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
        void visitInterfaceDefEnd(const InterfaceDefPtr&) final;
        void visitOperation(const OperationPtr&) final;

    private:
        void writeConstantValue(const TypePtr&, const SyntaxTreeBasePtr&, const std::string&);
        void writeMarshalDataMember(const DataMemberPtr&, const std::string&, const std::string&, bool = false);
        void writeUnmarshalDataMember(const DataMemberPtr&, const std::string&, const std::string&, bool = false);
        void writeMarshaling(const ClassDefPtr&);

        /// Writes "= null!" for non-nullable fields (Slice class and exception only).
        void writeDataMemberInitializers(const DataMemberList&);
    };

    /// Generates Result record structs for any operation that returns multiple values or a marshaled result; does not
    /// generate anything for other operations.
    class ResultVisitor final : public CsVisitor
    {
    public:
        ResultVisitor(IceInternal::Output&);

        bool visitModuleStart(const ModulePtr&) final;
        void visitModuleEnd(const ModulePtr&) final;
        void visitOperation(const OperationPtr&) final;
    };

    /// Generates the server-side code for Slice interfaces.
    class SkeletonVisitor final : public CsVisitor
    {
    public:
        SkeletonVisitor(IceInternal::Output& output, bool async);

        bool visitModuleStart(const ModulePtr&) final;
        void visitModuleEnd(const ModulePtr&) final;
        bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
        void visitInterfaceDefEnd(const InterfaceDefPtr&) final;
        void visitOperation(const OperationPtr&) final;

    private:
        void writeDispatch(const InterfaceDefPtr&);

        std::string getDispatchParams(
            const OperationPtr&,
            std::string&,
            std::vector<std::string>&,
            std::vector<std::string>&,
            const std::string&);

        [[nodiscard]] std::string skeletonPrefix() const;
        [[nodiscard]] std::string prependSkeletonPrefix(const std::string&) const;
        const bool _async;
    };

    class Gen final
    {
    public:
        Gen(const std::string&, const std::string&, bool);
        Gen(const Gen&) = delete;
        ~Gen();

        void generate(const UnitPtr&);

    private:
        IceInternal::Output _out;
        bool _enableAnalysis;

        void printHeader();
    };
}

#endif
