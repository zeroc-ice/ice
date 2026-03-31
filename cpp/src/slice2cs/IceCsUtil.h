// Copyright (c) ZeroC, Inc.

#ifndef ICE_CS_UTIL_H
#define ICE_CS_UTIL_H

#include "../Ice/OutputUtil.h"
#include "../Slice/DocCommentParser.h"
#include "../Slice/Parser.h"

// Ice-specific helper functions for C# code generation.

namespace Slice::Csharp
{
    /// Convert a dimension-less array declaration to one with a dimension.
    [[nodiscard]] std::string toArrayAlloc(const std::string& decl, const std::string& size);

    [[nodiscard]] std::string typeToString(const TypePtr& type, const std::string& ns, bool optional = false);

    [[nodiscard]] std::string
    resultStructName(const std::string& className, const std::string& opName, bool marshaledResult = false);
    [[nodiscard]] std::string resultType(const OperationPtr& op, const std::string& ns, bool dispatch = false);
    [[nodiscard]] std::string taskResultType(const OperationPtr& op, const std::string& ns, bool dispatch = false);
    [[nodiscard]] std::string getOptionalFormat(const TypePtr& type);
    [[nodiscard]] std::string getStaticId(const TypePtr& type);

    /// Is this Slice type mapped to a C# value type?
    [[nodiscard]] bool isValueType(const TypePtr& type);

    /// Is this Slice struct mapped to a C# class?
    [[nodiscard]] inline bool isMappedToClass(const StructPtr& p) { return !isValueType(p); }

    /// Is the mapped C# type for this field a non-nullable C# reference type?
    [[nodiscard]] bool isMappedToNonNullableReference(const DataMemberPtr& p);

    /// Is the mapped C# type for this field a non-nullable reference type?
    /// string fields are not included since they have a "" default.
    [[nodiscard]] bool isMappedToRequiredField(const DataMemberPtr& p);

    //
    // Generate code to marshal or unmarshal a type
    //
    void writeMarshalUnmarshalCode(
        IceInternal::Output& out,
        const TypePtr& type,
        const std::string& ns,
        const std::string& param,
        bool marshal,
        const std::string& customStream = "");

    void writeOptionalMarshalUnmarshalCode(
        IceInternal::Output& out,
        const TypePtr& type,
        const std::string& ns,
        const std::string& param,
        std::int32_t tag,
        bool marshal,
        const std::string& customStream = "");

    void writeSequenceMarshalUnmarshalCode(
        IceInternal::Output& out,
        const SequencePtr& seq,
        const std::string& ns,
        const std::string& param,
        bool marshal,
        bool useHelper,
        const std::string& customStream = "");

    void writeOptionalSequenceMarshalUnmarshalCode(
        IceInternal::Output& out,
        const SequencePtr& seq,
        const std::string& ns,
        const std::string& param,
        int tag,
        bool marshal,
        const std::string& customStream = "");

    //
    // Doc-comments
    //

    /// Writes a doc-comment for the given Slice element, using this element's doc-comment, if any.
    /// @param p The Slice element.
    /// @param generatedType The kind of mapped element, used for the remarks. For example, "skeleton interface".
    /// This function does not write any remarks when this argument is empty.
    /// @param notes Optional notes included at the end of the remarks.
    void writeIceDocComment(
        IceInternal::Output& out,
        const ContainedPtr& p,
        const std::string& generatedType = "",
        const std::string& notes = "");

    /// Writes a doc-comment for a helper class generated for a Slice element.
    /// @param p The Slice element.
    /// @param comment The summary.
    /// @param generatedType The kind of mapped element, used for the remarks. Must not be empty.
    /// @param notes Optional notes included at the end of the remarks.
    void writeIceHelperDocComment(
        IceInternal::Output& out,
        const ContainedPtr& p,
        const std::string& comment,
        const std::string& generatedType,
        const std::string& notes = "");

    void writeIceOpDocComment(
        IceInternal::Output& out,
        const OperationPtr& operation,
        const std::vector<std::string>& extraParams,
        bool isAsync);

    /// Converts a Slice-formatted link into a C# formatted link.
    /// @param rawLink The reference's raw text, taken verbatim from the doc-comment.
    /// @param source A pointer to the Slice element that the doc-comment (and reference) are written on.
    /// @param target A pointer to the Slice element that is being referenced, or `nullptr` if it doesn't exist.
    /// @returns A pair containing:
    /// - @c false if the link was to a Slice element which isn't mapped in C#; @c true otherwise.
    /// - The C# formatted link.
    std::pair<bool, std::string>
    iceLinkFormatter(const std::string& rawLink, const ContainedPtr& source, const SyntaxTreeBasePtr& target);
}

#endif
