// Copyright (c) ZeroC, Inc.

#ifndef ICE_RPC_VISITORS_H
#define ICE_RPC_VISITORS_H

#include "CsVisitor.h"

// Visitors for generating C# code for IceRPC.

namespace Slice::IceRpc
{
    /// Generates code for Slice types (including proxies), Slice exceptions, and Slice constants.
    class TypesVisitor final : public CsVisitor
    {
    public:
        TypesVisitor(IceInternal::Output& out);

        bool visitStructStart(const StructPtr&) final;
        void visitStructEnd(const StructPtr&) final;

        bool visitClassDefStart(const ClassDefPtr&) final;
        void visitClassDefEnd(const ClassDefPtr&) final;

        bool visitExceptionStart(const ExceptionPtr&) final;
        void visitExceptionEnd(const ExceptionPtr&) final;

        void visitDataMember(const DataMemberPtr&) final;

        void visitEnum(const EnumPtr&) final;

        bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
        void visitInterfaceDefEnd(const InterfaceDefPtr&) final;
        void visitOperation(const OperationPtr&) final;

        void visitConst(const ConstPtr&) final;

    private:
        bool writePrimaryConstructor(
            const ContainedPtr& p,
            const DataMemberList& fields,
            const DataMemberList& allBaseFields,
            const std::string& kind);

        void writeEncodeDecode(
            int compactId,
            const std::string& ns,
            bool hasBase,
            const DataMemberList& fields,
            const DataMemberList& orderedOptionalFields);

        void writeProxyRequestClass(const InterfaceDefPtr& interface);
        void writeProxyResponseClass(const InterfaceDefPtr& interface);
    };

    // Generates skeleton interfaces.
    class SkeletonVisitor final : public CsVisitor
    {
    public:
        SkeletonVisitor(IceInternal::Output& output);

        bool visitModuleStart(const ModulePtr&) final;

        bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
        void visitInterfaceDefEnd(const InterfaceDefPtr&) final;
        void visitOperation(const OperationPtr&) final;

    private:
        void writeRequestClass(const InterfaceDefPtr& interface);
        void writeResponseClass(const InterfaceDefPtr& interface);
    };
}

#endif
