// Copyright (c) ZeroC, Inc.

#ifndef ICE_RPC_CS_UTIL_H
#define ICE_RPC_CS_UTIL_H

#include "../Ice/OutputUtil.h"
#include "../Slice/DocCommentParser.h"
#include "../Slice/Parser.h"

// IceRPC-specific helper functions for C# code generation.

namespace Slice::Csharp
{
    enum class TypeContext
    {
        Field,
        IncomingParam,
        OutgoingParam
    };

    /// Does this type map to a value type in C#?
    [[nodiscard]] bool isCsValueType(const TypePtr& type);

    /// Maps a Slice type to a C# field type.
    /// @param type The Slice type to map.
    /// @param ns The current C# namespace.
    /// @param optional Whether the field is optional.
    /// @return The C# type for the field.
    [[nodiscard]] std::string csFieldType(const TypePtr& type, const std::string& ns, bool optional = false);

    /// Maps a Slice type to a C# incoming parameter type.
    /// @param type The Slice type to map.
    /// @param ns The current C# namespace.
    /// @param optional Whether the parameter is optional.
    /// @return The C# type for the incoming parameter.
    [[nodiscard]] std::string csIncomingParamType(const TypePtr& type, const std::string& ns, bool optional = false);

    /// Maps a Slice type to a C# outgoing parameter type.
    /// @param type The Slice type to map.
    /// @param ns The current C# namespace.
    /// @param optional Whether the parameter is optional.
    /// @return The C# type for the outgoing parameter.
    [[nodiscard]] std::string csOutgoingParamType(const TypePtr& type, const std::string& ns, bool optional = false);

    /// Maps a Slice type to a C# type based on TypeContext.
    [[nodiscard]] std::string
    csType(const TypePtr& type, const std::string& ns, TypeContext context, bool optional = false);

    /// Returns whether the mapped C# field is required or not.
    [[nodiscard]] bool csRequired(const DataMemberPtr& field);

    /// Encodes a non-optional field.
    void encodeField(
        ::IceInternal::Output& out,
        const std::string& fieldName,
        const TypePtr& type,
        const std::string& ns,
        TypeContext context,
        const std::string& encoderName);

    /// Encodes an optional field.
    void encodeOptionalField(
        ::IceInternal::Output& out,
        int tag,
        const std::string& fieldName,
        const TypePtr& type,
        const std::string& ns,
        TypeContext context,
        const std::string& encoderName);

    /// Decodes a non-optional field.
    void decodeField(::IceInternal::Output& out, const TypePtr& type, const std::string& ns);

    /// Decodes an optional field.
    void decodeOptionalField(
        ::IceInternal::Output& out,
        int tag,
        const TypePtr& type,
        const std::string& ns,
        TypeContext context);

    //
    // Doc-comments
    //

    /// Writes a doc-comment for the given Slice element, using this element's doc-comment, if any.
    /// @param p The Slice element.
    /// @param generatedType The kind of mapped element, used for the remarks. For example, "skeleton interface".
    /// This function does not write any remarks when this argument is empty.
    /// @param notes Optional notes included at the end of the remarks.
    void writeIceRpcDocComment(
        IceInternal::Output& out,
        const ContainedPtr& p,
        const std::string& generatedType = "",
        const std::string& notes = "");

    /// Writes a doc-comment for a helper class generated for a Slice element.
    /// @param p The Slice element.
    /// @param comment The summary.
    /// @param generatedType The kind of mapped element, used for the remarks. Must not be empty.
    /// @param notes Optional notes included at the end of the remarks.
    void writeIceRpcHelperDocComment(
        IceInternal::Output& out,
        const ContainedPtr& p,
        const std::string& comment,
        const std::string& generatedType,
        const std::string& notes = "");

    /// Converts a Slice-formatted link into a C# formatted link.
    /// @param rawLink The reference's raw text, taken verbatim from the doc-comment.
    /// @param source A pointer to the Slice element that the doc-comment (and reference) are written on.
    /// @param target A pointer to the Slice element that is being referenced, or `nullptr` if it doesn't exist.
    /// @returns A pair containing:
    /// - @c false if the link was to a Slice element which isn't mapped in C#; @c true otherwise.
    /// - The C# formatted link.
    std::pair<bool, std::string>
    icerpcLinkFormatter(const std::string& rawLink, const ContainedPtr& source, const SyntaxTreeBasePtr& target);
}

#endif
