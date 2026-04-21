// Copyright (c) ZeroC, Inc.

#ifndef SWIFT_UTIL_H
#define SWIFT_UTIL_H

#include "../Ice/OutputUtil.h"
#include "../Slice/Parser.h"

using StringPairList = std::list<std::pair<std::string, std::string>>;

namespace Slice::Swift
{
    std::string getSwiftModule(const ModulePtr& module, std::string& swiftPrefix);
    std::string getSwiftModule(const ModulePtr& module);

    /// Returns a DocC formatted link for the given Slice identifier.
    std::string
    swiftLinkFormatter(const std::string& rawLink, const ContainedPtr& source, const SyntaxTreeBasePtr& target);

    void validateSwiftMetadata(const UnitPtr& unit);

    // Swift only allows 1 package per file, so this function checks that if there are multiple top-level-modules
    // within a single Slice file, that they all map to the same Swift package.
    void validateSwiftModuleMappings(const UnitPtr& unit);

    /// Removes any Swift escaping from the provided identifier (any leading or trailing backticks will be removed).
    std::string removeEscaping(std::string ident);

    void writeDocSummary(
        IceInternal::Output& out,
        const ContainedPtr& p,
        const std::optional<std::string>& generatedType = std::nullopt);

    void writeOpDocSummary(IceInternal::Output& out, const OperationPtr& p, bool dispatch);

    std::string paramLabel(const std::string& param, const ParameterList& params);
    std::string operationReturnType(const OperationPtr& op);
    std::string operationReturnDeclaration(const OperationPtr& op);

    std::string typeToString(const TypePtr& type, const ContainedPtr& usedBy, bool optional = false);

    std::string getUnqualified(const std::string& type, const std::string& localModule);
    std::string modeToString(Operation::Mode opMode);
    std::string getOptionalFormat(const TypePtr& type);

    bool isNullableType(const TypePtr& type);

    /// Returns a string representing the Swift type `contained` maps to.
    ///
    /// If `currentModule` is _not_ provided, the type-string is fully qualified.
    /// Otherwise, the type-string is qualified relative to `currentModule`.
    std::string getRelativeTypeString(const ContainedPtr& contained, const std::string& currentModule = "");

    std::string getValue(const std::string& swiftModule, const TypePtr& type);
    void writeConstantValue(
        IceInternal::Output& out,
        const TypePtr& type,
        const SyntaxTreeBasePtr& valueType,
        const std::string& value,
        const std::string& swiftModule,
        bool optional = false);
    void writeDefaultInitializer(IceInternal::Output& out, bool required, bool rootClass);
    void writeMemberwiseInitializer(IceInternal::Output& out, const DataMemberList& members, const ContainedPtr& p);
    void writeMemberwiseInitializer(
        IceInternal::Output& out,
        const DataMemberList& members,
        const DataMemberList& baseMembers,
        const DataMemberList& allMembers,
        const ContainedPtr& p,
        bool rootClass);
    void writeMembers(IceInternal::Output& out, const DataMemberList& members, const ContainedPtr& p);

    void writeMarshalUnmarshalCode(
        IceInternal::Output& out,
        const TypePtr& type,
        const ContainedPtr& p,
        const std::string& param,
        bool marshal,
        std::int32_t tag = -1);

    bool usesMarshalHelper(const TypePtr& type);
    void writeMarshalInParams(IceInternal::Output& out, const OperationPtr& op);
    void writeMarshalAsyncOutParams(IceInternal::Output& out, const OperationPtr& op);
    void writeUnmarshalInParams(IceInternal::Output& out, const OperationPtr& op);
    void writeUnmarshalOutParams(IceInternal::Output& out, const OperationPtr& op);
    void writeUnmarshalUserException(IceInternal::Output& out, const OperationPtr& op);
    void writeSwiftAttributes(IceInternal::Output& out, const MetadataList& metadata);
}

#endif
