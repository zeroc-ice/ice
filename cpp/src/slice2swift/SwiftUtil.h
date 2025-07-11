// Copyright (c) ZeroC, Inc.

#ifndef SWIFT_UTIL_H
#define SWIFT_UTIL_H

#include "../Ice/OutputUtil.h"
#include "../Slice/Parser.h"

using StringPairList = std::list<std::pair<std::string, std::string>>;

namespace Slice::Swift
{
    std::string getSwiftModule(const ModulePtr&, std::string&);
    std::string getSwiftModule(const ModulePtr&);

    /// Returns a DocC formatted link for the given Slice identifier.
    std::string
    swiftLinkFormatter(const std::string& rawLink, const ContainedPtr& source, const SyntaxTreeBasePtr& target);

    void validateSwiftMetadata(const UnitPtr&);

    // Swift only allows 1 package per file, so this function checks that if there are multiple top-level-modules
    // within a single Slice file, that they all map to the same Swift package.
    void validateSwiftModuleMappings(const UnitPtr&);

    /// Removes any Swift escaping from the provided identifier (any leading or trailing backticks will be removed).
    std::string removeEscaping(std::string ident);

    void writeDocSummary(IceInternal::Output&, const ContainedPtr&);
    void writeOpDocSummary(IceInternal::Output&, const OperationPtr&, bool);

    void writeProxyDocSummary(IceInternal::Output&, const InterfaceDefPtr&, const std::string&);
    void writeServantDocSummary(IceInternal::Output&, const InterfaceDefPtr&, const std::string&);

    std::string paramLabel(const std::string&, const ParameterList&);
    std::string operationReturnType(const OperationPtr&);
    std::string operationReturnDeclaration(const OperationPtr&);

    std::string typeToString(const TypePtr&, const ContainedPtr&, bool = false);

    std::string getUnqualified(const std::string&, const std::string&);
    std::string modeToString(Operation::Mode);
    std::string getOptionalFormat(const TypePtr&);

    bool isNullableType(const TypePtr&);

    /// Returns a string representing the Swift type `contained` maps to.
    ///
    /// If `currentModule` is _not_ provided, the type-string is fully qualified.
    /// Otherwise, the type-string is qualified relative to `currentModule`.
    std::string getRelativeTypeString(const ContainedPtr& contained, const std::string& currentModule = "");

    std::string getValue(const std::string&, const TypePtr&);
    void writeConstantValue(
        IceInternal::Output& out,
        const TypePtr&,
        const SyntaxTreeBasePtr&,
        const std::string&,
        const std::string&,
        bool optional = false);
    void writeDefaultInitializer(IceInternal::Output&, bool, bool);
    void writeMemberwiseInitializer(IceInternal::Output&, const DataMemberList&, const ContainedPtr&);
    void writeMemberwiseInitializer(
        IceInternal::Output&,
        const DataMemberList&,
        const DataMemberList&,
        const DataMemberList&,
        const ContainedPtr&,
        bool rootClass);
    void writeMembers(IceInternal::Output&, const DataMemberList&, const ContainedPtr&);

    void writeMarshalUnmarshalCode(
        ::IceInternal::Output&,
        const TypePtr&,
        const ContainedPtr&,
        const std::string&,
        bool,
        std::int32_t = -1);

    bool usesMarshalHelper(const TypePtr&);
    void writeMarshalInParams(::IceInternal::Output&, const OperationPtr&);
    void writeMarshalAsyncOutParams(::IceInternal::Output&, const OperationPtr&);
    void writeUnmarshalInParams(::IceInternal::Output&, const OperationPtr&);
    void writeUnmarshalOutParams(::IceInternal::Output&, const OperationPtr&);
    void writeUnmarshalUserException(::IceInternal::Output& out, const OperationPtr&);
    void writeSwiftAttributes(::IceInternal::Output&, const MetadataList&);
}

#endif
