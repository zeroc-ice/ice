// Copyright (c) ZeroC, Inc.

#ifndef METADATA_VISITOR_H
#define METADATA_VISITOR_H

#include "Parser.h"

#include <functional>
#include <typeinfo>

namespace Slice
{
    enum class MetadataArgumentKind
    {
        /// The metadata takes no arguments.
        NoArguments,
        /// The metadata must have exactly 1 argument.
        SingleArgument,
        /// This metadata can have any number of arguments [0, ∞).
        AnyNumberOfArguments,

        /// This metadata must have an argument, but that argument is raw text and not validated further.
        RequiredTextArgument,
        /// This metadata can optionally have an argument, but that argument is raw text and not validated further.
        OptionalTextArgument,
    };

    enum class MetadataApplicationContext
    {
        /// The metadata can only be applied to definitions and declarations.
        /// Ex: `["deprecated"] enum A {...}` is valid, but `sequence<["deprecated"] A> S;` is not.
        Definitions,

        /// The metadata can be applied to definitions of types and to where those types are referenced.
        /// Ex: both of these are valid: `["java:buffer"] sequence<string> S;` and `["java:buffer"] StringSeq op();`.
        ///
        /// Note that for metadata in this category, if it has been applied to an operation, we validate that metadata
        /// for the operation's **return type**, instead of for the operation itself.
        /// We do the same thing for metadata applied to parameters and data members as well.
        /// Due to the syntax of Slice, it's ambiguous whether metadata is applied to these elements or their types.
        DefinitionsAndTypeReferences,

        /// The metadata can only be applied to the types of operations parameters (including return types).
        /// Ex: `void op(["cpp:array"] StringSeq s);` is valid, but `["cpp:array"] sequence<string> S;` is not.
        ///
        /// Note that for metadata in this category, if it has been applied to an operation, we validate that metadata
        /// for the operation's **return type**, instead of for the operation itself.
        /// We do the same thing for metadata applied to parameters and data members as well.
        /// Due to the syntax of Slice, it's ambiguous whether metadata is applied to these elements or their types.
        ParameterTypeReferences,
    };

    /// Alias for functions which can provide additional validation to certain metadata directives.
    ///
    /// They take a reference to the instance of metadata we're validating, and a reference to what it was applied on.
    /// They should return `nullopt` to signal that the metadata was valid. If the metadata is invalid however, they
    /// should return a string describing the error. This string will be supplied to the user as part of a diagnostic.
    using ValidationFunc = std::function<std::optional<std::string>(const MetadataPtr&, const SyntaxTreeBasePtr&)>;

    struct MetadataInfo
    {
        /// A list of types that this metadata can validly be applied to.
        ///
        /// If this list is empty we don't perform any automatic checking of whether this metadata is validly applied.
        /// Usually, this is because determining validity isn't as straightforward as matching against a list,
        /// and requires a more complex approach, which is achieved through providing an `extraValidation` function.
        std::list<std::reference_wrapper<const std::type_info>> validOn;

        /// Specifies how many, and what kinds of arguments, this metadata accepts.
        MetadataArgumentKind acceptedArguments;

        /// This field stores the specific values that can be provided as arguments to this metadata.
        /// If this field is unset, then we perform no validation of the arguments (i.e. arguments can have any value).
        std::optional<StringList> validArgumentValues = std::nullopt;

        /// Specifies in what contexts the metadata can appear (i.e. can it apply to defintions, references, both?)
        MetadataApplicationContext acceptedContexts = MetadataApplicationContext::Definitions;

        /// Indicates whether it's valid and meaningful for this metadata to appear multiple times on the same thing.
        bool mustBeUnique = true;

        /// A function used to run additional validation for this metadata.
        /// If this field is set, it will always be run.
        /// @see ValidationFunc
        ValidationFunc extraValidation = nullptr;
    };

    class MetadataValidator final : public ParserVisitor
    {
    public:
        MetadataValidator(std::string language, std::map<std::string, MetadataInfo> metadataInfo);

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

        static std::string misappliedMetadataMessage(const MetadataPtr& metadata, const SyntaxTreeBasePtr& p);

    private:
        MetadataList validate(MetadataList metadata, const SyntaxTreeBasePtr& p, bool isTypeContext = false);
        bool isMetadataValid(const MetadataPtr& metadata, const SyntaxTreeBasePtr& p, bool isTypeContext);

        std::string _language;
        std::map<std::string, MetadataInfo> _metadataInfo;
        std::set<std::string> _seenDirectives;
    };
}
#endif
