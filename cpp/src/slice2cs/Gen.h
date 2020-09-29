//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef GEN_H
#define GEN_H

#include <CsUtil.h>

namespace Slice
{

struct CommentInfo
{
    std::vector<std::string> summaryLines;
    std::map<std::string, std::vector<std::string>> params;
    std::map<std::string, std::vector<std::string>> exceptions;
    std::vector<std::string> returnLines;
};

class CsVisitor : public CsGenerator, public ParserVisitor
{
public:

    CsVisitor(::IceUtilInternal::Output&);
    virtual ~CsVisitor();

protected:

    // Write the marshaling code for the operation's params or return type.
    void writeMarshal(const OperationPtr& operation, bool returnType);

    // Write the unmarshaling code for the operation's params or return type - operation's params in the skeleton and
    // return type in the proxy.
    void writeUnmarshal(const OperationPtr& operation, bool returnType);

    void writeMarshalDataMembers(const MemberList&, const std::string&, unsigned int);
    void writeUnmarshalDataMembers(const MemberList&, const std::string&, unsigned int);

    void emitCommonAttributes(); // GeneratedCode and more if needed
    void emitEditorBrowsableNeverAttribute();
    void emitEqualityOperators(const std::string&);
    void emitCustomAttributes(const ContainedPtr&); // attributes specified through metadata
    void emitTypeIdAttribute(const std::string&); // the Ice type ID attribute

    std::string writeValue(const TypePtr&, const std::string&);

    // Generate assignment statements for those data members that have default values.
    void writeDataMemberDefaultValues(const MemberList&, const std::string&, unsigned int);

    // Generate this.X = null! for non-nullable fields.
    void writeSuppressNonNullableWarnings(const MemberList&, unsigned int);

    void writeProxyDocComment(const InterfaceDefPtr&, const std::string&);
    void writeServantDocComment(const InterfaceDefPtr&, const std::string&);

    void writeTypeDocComment(const ContainedPtr&, const std::string&);
    void writeOperationDocComment(const OperationPtr&, const std::string&, bool, bool);

    enum ParamDir { InParam, OutParam };
    void writeParamDocComment(const OperationPtr&, const CommentInfo&, ParamDir);

    // Generates the corresponding namespace. When prefix is empty and the internal namespace stack is empty, lookup
    // the prefix using cs:namespace metadata.
    void openNamespace(const ModulePtr& module, std::string prefix = "");
    void closeNamespace();

    ::IceUtilInternal::Output& _out;

private:

    // Empty means we opened the namespace (and need to close it), non-empty means a saved enclosing namespace.
    std::stack<std::string> _namespaceStack;
};

class Gen : private ::IceUtil::noncopyable
{
public:

    Gen(const std::string&,
        const std::vector<std::string>&,
        const std::string&,
        bool);
    ~Gen();

    void generate(const UnitPtr&);
    void generateImpl(const UnitPtr&);
    void closeOutput();

private:

    IceUtilInternal::Output _out;
    IceUtilInternal::Output _impl;
    std::vector<std::string> _includePaths;

    void printHeader();

    class UnitVisitor : public CsVisitor
    {
    public:

        UnitVisitor(::IceUtilInternal::Output&);

        bool visitUnitStart(const UnitPtr&) override;
    };

    class TypesVisitor : public CsVisitor
    {
    public:

        TypesVisitor(::IceUtilInternal::Output&);

        bool visitModuleStart(const ModulePtr&) override;
        void visitModuleEnd(const ModulePtr&) override;
        bool visitClassDefStart(const ClassDefPtr&) override;
        void visitClassDefEnd(const ClassDefPtr&) override;
        bool visitExceptionStart(const ExceptionPtr&) override;
        void visitExceptionEnd(const ExceptionPtr&) override;
        bool visitStructStart(const StructPtr&) override;
        void visitStructEnd(const StructPtr&) override;
        void visitEnum(const EnumPtr&) override;
        void visitDataMember(const MemberPtr&) override;

    private:

        void writeMarshaling(const ClassDefPtr&);
    };

    class ProxyVisitor : public CsVisitor
    {
    public:

        ProxyVisitor(::IceUtilInternal::Output&);

        bool visitModuleStart(const ModulePtr&) override;
        void visitModuleEnd(const ModulePtr&) override;
        bool visitInterfaceDefStart(const InterfaceDefPtr&) override;
        void visitInterfaceDefEnd(const InterfaceDefPtr&) override;
        void visitOperation(const OperationPtr&) override;

    protected:

        void writeIncomingResponseReader(const OperationPtr&);
        void writeOutgoingRequestWriter(const OperationPtr&);
    };

    class DispatcherVisitor : public CsVisitor
    {
    public:

        DispatcherVisitor(::IceUtilInternal::Output&, bool);

        bool visitModuleStart(const ModulePtr&) override;
        void visitModuleEnd(const ModulePtr&) override;
        bool visitInterfaceDefStart(const InterfaceDefPtr&) override;
        void visitInterfaceDefEnd(const InterfaceDefPtr&) override;
        void visitOperation(const OperationPtr&) override;

    protected:

        void writeReturnValueStruct(const OperationPtr&);
        void writeMethodDeclaration(const OperationPtr&);

        void writeIncomingRequestReader(const OperationPtr&);
        void writeOutgoingResponseWriter(const OperationPtr&);

    private:

        const bool _generateAllAsync;
    };

    class ImplVisitor : public CsVisitor
    {
    public:

        ImplVisitor(::IceUtilInternal::Output&);

        bool visitModuleStart(const ModulePtr&) override;
        void visitModuleEnd(const ModulePtr&) override;
        bool visitInterfaceDefStart(const InterfaceDefPtr&) override;
        void visitInterfaceDefEnd(const InterfaceDefPtr&) override;
        void visitOperation(const OperationPtr&) override;
    };

    class ClassFactoryVisitor : public CsVisitor
    {
    public:

        ClassFactoryVisitor(IceUtilInternal::Output&);
        bool visitModuleStart(const ModulePtr&) override;
        void visitModuleEnd(const ModulePtr&) override;
        bool visitClassDefStart(const ClassDefPtr&) override;
    };

    class CompactIdVisitor : public CsVisitor
    {
    public:

        CompactIdVisitor(IceUtilInternal::Output&);
        bool visitUnitStart(const UnitPtr&) override;
        void visitUnitEnd(const UnitPtr&) override;
        bool visitClassDefStart(const ClassDefPtr&) override;
    };

    class RemoteExceptionFactoryVisitor : public CsVisitor
    {
    public:

        RemoteExceptionFactoryVisitor(IceUtilInternal::Output&);
        bool visitModuleStart(const ModulePtr&) override;
        void visitModuleEnd(const ModulePtr&) override;
        bool visitExceptionStart(const ExceptionPtr&) override;
    };
};

}

#endif
