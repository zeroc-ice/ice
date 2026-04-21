// Copyright (c) ZeroC, Inc.

#ifndef ICE_VISITORS_H
#define ICE_VISITORS_H

#include "CsVisitor.h"

// Visitors for generating C# code for Ice.

namespace Slice::Ice
{
    /// Generates code for Slice types (including proxies) and Slice exceptions.
    class TypesVisitor final : public CsVisitor
    {
    public:
        TypesVisitor(IceInternal::Output& out);

        bool visitClassDefStart(const ClassDefPtr&) final;
        void visitClassDefEnd(const ClassDefPtr&) final;
        bool visitExceptionStart(const ExceptionPtr&) final;
        void visitExceptionEnd(const ExceptionPtr&) final;
        bool visitStructStart(const StructPtr&) final;
        void visitStructEnd(const StructPtr&) final;
        void visitSequence(const SequencePtr&) final;
        void visitDictionary(const DictionaryPtr&) final;
        void visitEnum(const EnumPtr&) final;
        void visitConst(const ConstPtr&) final;
        void visitDataMember(const DataMemberPtr&) final;

        // For proxies
        bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
        void visitInterfaceDefEnd(const InterfaceDefPtr&) final;
        void visitOperation(const OperationPtr&) final;

    private:
        void writeMarshalDataMember(
            const DataMemberPtr& member,
            const std::string& name,
            const std::string& ns,
            bool forStruct = false);
        void writeUnmarshalDataMember(
            const DataMemberPtr& member,
            const std::string& name,
            const std::string& ns,
            bool forStruct = false);
        void writeMarshaling(const ClassDefPtr& p);

        /// Writes "= null!" for non-nullable fields (Slice class and exception only).
        void writeDataMemberInitializers(const DataMemberList& dataMembers);
    };

    /// Generates Result record structs for any operation that returns multiple values or a marshaled result; does not
    /// generate anything for other operations.
    class ResultVisitor final : public CsVisitor
    {
    public:
        ResultVisitor(IceInternal::Output& out);

        bool visitModuleStart(const ModulePtr&) final;

        void visitOperation(const OperationPtr&) final;
    };

    /// Generates the server-side code for Slice interfaces.
    class SkeletonVisitor final : public CsVisitor
    {
    public:
        SkeletonVisitor(IceInternal::Output& out, bool async);

        bool visitModuleStart(const ModulePtr&) final;

        bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
        void visitInterfaceDefEnd(const InterfaceDefPtr&) final;
        void visitOperation(const OperationPtr&) final;

    private:
        void writeDispatch(const InterfaceDefPtr& p);

        std::string getDispatchParams(
            const OperationPtr& op,
            std::string& retS,
            std::vector<std::string>& params,
            std::vector<std::string>& args,
            const std::string& ns);

        [[nodiscard]] std::string skeletonPrefix() const;
        [[nodiscard]] std::string prependSkeletonPrefix(const std::string& name) const;
        const bool _async;
    };
}

#endif
