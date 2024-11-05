// Copyright (c) ZeroC, Inc.

#ifndef SLICE_UTIL_H
#define SLICE_UTIL_H

#include "Parser.h"

#include <functional>

namespace Slice
{
    class MetadataValidator final : public ParserVisitor
    {
    public:
        /// This function performs the actual validation of metadata.
        /// If the metadata pass validation, implementations should return `nullopt`.
        /// If there is a problem with the metadata, implementations should return a message explaining the problem.
        ///
        /// The implementation in this class validates parser metadata (metadata without a language prefix).
        /// So subclasses which override this should:
        ///   1) Perform their own language specific validation
        ///   2) Call this implementation with `MetadataValidator::validateMetadata`
        virtual std::optional<std::string> validateMetadata(const MetadataPtr& metadata, const SyntaxTreeBasePtr& p);

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
        MetadataList validate(MetadataList metadata, const SyntaxTreeBasePtr& p);
    };
}
#endif
