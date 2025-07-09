// Copyright (c) ZeroC, Inc.

#ifndef CS_UTIL_H
#define CS_UTIL_H

#include "../Ice/OutputUtil.h"
#include "../Slice/Parser.h"

namespace Slice::Csharp
{
    /// Convert a dimension-less array declaration to one with a dimension.
    [[nodiscard]] std::string toArrayAlloc(const std::string& decl, const std::string& size);

    /// Returns the namespace of a Contained entity.
    [[nodiscard]] std::string getNamespace(const ContainedPtr& p);

    [[nodiscard]] std::string getUnqualified(const ContainedPtr& p, const std::string& package);

    /// Removes a leading '@' character from the provided identifier (if one is present).
    [[nodiscard]] std::string removeEscapePrefix(const std::string& identifier);

    [[nodiscard]] std::string typeToString(const TypePtr& type, const std::string& package, bool optional = false);

    /// Returns the namespace prefix of a Contained entity.
    [[nodiscard]] std::string getNamespacePrefix(const ContainedPtr& p);

    [[nodiscard]] std::string
    resultStructName(const std::string& className, const std::string& opName, bool marshaledResult = false);
    [[nodiscard]] std::string resultType(const OperationPtr& op, const std::string& package, bool dispatch = false);
    [[nodiscard]] std::string taskResultType(const OperationPtr& op, const std::string& scope, bool dispatch = false);
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
        const std::string& package,
        const std::string& param,
        bool marshal,
        const std::string& customStream = "");
    void writeOptionalMarshalUnmarshalCode(
        ::IceInternal::Output& out,
        const TypePtr& type,
        const std::string& scope,
        const std::string& param,
        std::int32_t tag,
        bool marshal,
        const std::string& customStream = "");
    void writeSequenceMarshalUnmarshalCode(
        ::IceInternal::Output& out,
        const SequencePtr& seq,
        const std::string& scope,
        const std::string& param,
        bool marshal,
        bool useHelper,
        const std::string& customStream = "");
    void writeOptionalSequenceMarshalUnmarshalCode(
        ::IceInternal::Output& out,
        const SequencePtr& seq,
        const std::string& scope,
        const std::string& param,
        int tag,
        bool marshal,
        const std::string& customStream = "");

    /// Returns a C# formatted link to the provided Slice identifier.
    std::string
    csLinkFormatter(const std::string& rawLink, const ContainedPtr& source, const SyntaxTreeBasePtr& target);
}

#endif
