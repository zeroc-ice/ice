// Copyright (c) ZeroC, Inc.

#ifndef SLICE_UTIL_H
#define SLICE_UTIL_H

#include "Parser.h"

namespace Slice
{
    /// TODO
    using ValidationFunc = std::function<std::optional<std::string>(const SyntaxTreeBasePtr&, const MetadataPtr&)>;

    class MetadataValidator final : public ParserVisitor
    {
    public:
        MetadataValidator(std::string language, std::map<std::string, ValidationFunc> validators);

        bool visitUnitStart(const UnitPtr&) final;
        bool visitModuleStart(const ModulePtr&) final;
        void visitClassDecl(const ClassDeclPtr&) final;
        bool visitClassDefStart(const ClassDefPtr&) final;
        void visitInterfaceDecl(const InterfaceDeclPtr&) final;
        bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
        bool visitExceptionStart(const ExceptionPtr&) final;
        bool visitStructStart(const StructPtr&) final;
        void visitOperation(const OperationPtr&) final;
        void visitParamDecl(const ParamDeclPtr&) final;
        void visitDataMember(const DataMemberPtr&) final;
        void visitSequence(const SequencePtr&) final;
        void visitDictionary(const DictionaryPtr&) final;
        void visitEnum(const EnumPtr&) final;
        void visitConst(const ConstPtr&) final;

    private:
        MetadataList validateMetadata(const SyntaxTreeBasePtr& p, MetadataList metadata) const;

        /// The language prefix that this visitor is checking ('cpp', 'js', etc.).
        /// Any metadata that doesn't start with the specified language will be ignored by the visitor.
        ///
        /// Setting this to the empty string is a special case for validating language-agnostic metadata.
        std::string _language;

        /// A map of functions that this visitor can call to validate metadata.
        /// Each dictionary entry is of the form ['directive', 'validation function'].
        ///
        /// When this visitor finds metadata that matches it's `language` prefix, it checks this map for a validation
        /// function. If one exists, it will be run to check the metadata, otherwise we report unknown metadata.
        std::map<std::string, ValidationFunc> _validationFunctions;
    };
}
#endif
