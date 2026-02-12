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
        ::IceInternal::Output& out,
        const TypePtr& type,
        const std::string& ns,
        const std::string& param,
        bool marshal,
        const std::string& customStream = "");

    void writeOptionalMarshalUnmarshalCode(
        ::IceInternal::Output& out,
        const TypePtr& type,
        const std::string& ns,
        const std::string& param,
        std::int32_t tag,
        bool marshal,
        const std::string& customStream = "");

    void writeSequenceMarshalUnmarshalCode(
        ::IceInternal::Output& out,
        const SequencePtr& seq,
        const std::string& ns,
        const std::string& param,
        bool marshal,
        bool useHelper,
        const std::string& customStream = "");

    void writeOptionalSequenceMarshalUnmarshalCode(
        ::IceInternal::Output& out,
        const SequencePtr& seq,
        const std::string& ns,
        const std::string& param,
        int tag,
        bool marshal,
        const std::string& customStream = "");

    /// Converts a Slice-formatted link into a C# formatted link.
    /// @param rawLink The reference's raw text, taken verbatim from the doc-comment.
    /// @param source A pointer to the Slice element that the doc-comment (and reference) are written on.
    /// @param target A pointer to the Slice element that is being referenced, or `nullptr` if it doesn't exist.
    /// @returns A pair containing:
    /// - @c false if the link was to a Slice element which isn't mapped in C#; @c true otherwise.
    /// - The C# formatted link.
    std::pair<bool, std::string>
    csLinkFormatter(const std::string& rawLink, const ContainedPtr& source, const SyntaxTreeBasePtr& target);

    class IceDocCommentFormatter final : public DocCommentFormatter
    {
    public:
        void preprocess(StringList& rawComment) final;
        std::string formatCode(const std::string& rawText) final;
        std::string formatParamRef(const std::string& param) final;
        std::string
        formatLink(const std::string& rawLink, const ContainedPtr& source, const SyntaxTreeBasePtr& target) final;
        std::string
        formatSeeAlso(const std::string& rawLink, const ContainedPtr& source, const SyntaxTreeBasePtr& target) final;
    };
}

#endif
