// Copyright (c) ZeroC, Inc.

#ifndef C_PLUS_PLUS_UTIL_H
#define C_PLUS_PLUS_UTIL_H

#include "../Ice/OutputUtil.h"
#include "../Slice/Parser.h"
#include "TypeContext.h"

namespace Slice
{
    extern std::string paramPrefix;

    struct ToIfdef
    {
        char operator()(char);
    };

    void printHeader(IceInternal::Output& out);
    void printVersionCheck(IceInternal::Output& out);
    void printDllExportStuff(IceInternal::Output& out, const std::string& dllExport);

    bool isMovable(const TypePtr& type);

    std::string getUnqualified(const std::string& type, const std::string& scope);

    /// Gets the C++ type for a Slice parameter or field.
    std::string typeToString(
        const TypePtr& type,
        bool optional,
        const std::string& scope = "",
        const MetadataList& metadata = MetadataList(),
        TypeContext typeCtx = TypeContext::None);

    // TODO: find a better name.
    /// Gets the C++ type for a Slice parameter to be marshaled.
    std::string inputTypeToString(
        const TypePtr& type,
        bool optional,
        const std::string& scope = "",
        const MetadataList& metadata = MetadataList(),
        TypeContext typeCtx = TypeContext::None);

    // TODO: find a better name.
    /// Gets the C++ type for a Slice out parameter when mapped to a C++ out parameter.
    std::string outputTypeToString(
        const TypePtr& type,
        bool optional,
        const std::string& scope = "",
        const MetadataList& metadata = MetadataList(),
        TypeContext typeCtx = TypeContext::None);

    std::string operationModeToString(Operation::Mode mode);
    std::string opFormatTypeToString(const OperationPtr& op);

    void writeMarshalCode(IceInternal::Output& out, const ParameterList& params, const OperationPtr& op);
    void writeUnmarshalCode(IceInternal::Output& out, const ParameterList& params, const OperationPtr& op);
    void writeAllocateCode(
        IceInternal::Output& out,
        const ParameterList& params,
        const OperationPtr& op,
        const std::string& clScope,
        TypeContext typeCtx);

    /// Writes the StreamReader specialization for a struct.
    void writeStreamReader(IceInternal::Output& out, const StructPtr& p, const DataMemberList& dataMembers);

    /// Reads or writes the data members of a class or exception slice.
    void readDataMembers(IceInternal::Output& out, const DataMemberList& dataMembers);
    void writeDataMembers(IceInternal::Output& out, const DataMemberList& dataMembers);

    void writeIceTuple(IceInternal::Output& out, const DataMemberList& dataMembers, TypeContext typeCtx);

    std::string findMetadata(const MetadataList& metadata, TypeContext typeCtx = TypeContext::None);
    bool inWstringModule(const SequencePtr& seq);

    /// Returns a doxygen formatted link to the provided Slice identifier.
    std::string
    cppLinkFormatter(const std::string& rawLink, const ContainedPtr& source, const SyntaxTreeBasePtr& target);

    void validateCppMetadata(const UnitPtr& unit);
}

#endif
