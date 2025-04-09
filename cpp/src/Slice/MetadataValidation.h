// Copyright (c) ZeroC, Inc.

#ifndef METADATA_VALIDATION_H
#define METADATA_VALIDATION_H

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
        /// Ex: both of these are valid: `["java:buffer"] sequence<string> S;` and `["java:buffer"] StringSeq myField;`.
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
        ///
        /// Note that `ClassDef` and `InterfaceDef` should never appear in this list, since class & interface metadata
        /// is always stored on the corresponding `ClassDecl` and `InterfaceDecl` types, which should be used instead.
        std::list<std::reference_wrapper<const std::type_info>> validOn;

        /// Specifies how many, and what kinds of arguments, this metadata accepts.
        MetadataArgumentKind acceptedArgumentKind;

        /// This field stores the specific values that can be provided as arguments to this metadata.
        ///
        /// If this field is unset, then we perform no validation of the arguments (i.e. arguments can have any value).
        /// This should always be the case if `acceptedArgumentKind` is either of the `...TextArgument` enumerators.
        std::optional<StringList> validArgumentValues = std::nullopt;

        /// Specifies in what contexts the metadata can appear (i.e. can it apply to definitions, references, both?)
        MetadataApplicationContext acceptedContext = MetadataApplicationContext::Definitions;

        /// Indicates whether it's valid and meaningful for this metadata to appear multiple times on the same thing.
        bool mustBeUnique = true;

        /// A function used to run additional validation for this metadata.
        /// If this field is set, it will always be run.
        /// @see ValidationFunc
        ValidationFunc extraValidation = nullptr;
    };

    /// Returns a message of the form "'<directive>' metadata cannot be applied to <type>'" (more or less).
    /// This message is reported to the user when metadata is placed on something for which it is inapplicable.
    std::string misappliedMetadataMessage(const MetadataPtr& metadata, const SyntaxTreeBasePtr& p);

    /// Validates all the metadata in the provided Unit against a map of known metadata.
    /// @param p The unit who's metadata should be validated.
    /// @param prefix Which language's metadata should be validated. Any metadata that starts with a different
    ///               language prefix than this one will be ignored. Note that metadata without any language prefix
    ///               (i.e. parser metadata) is always checked.
    /// @param knownMetadata A map containing the directives that should be validated, and information describing
    ///                      the various constraints and conditions that should be upheld for it and its arguments.
    void validateMetadata(const UnitPtr& p, std::string_view prefix, std::map<std::string, MetadataInfo> knownMetadata);
}
#endif
