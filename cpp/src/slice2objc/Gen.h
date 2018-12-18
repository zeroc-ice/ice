// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef GEN_H
#define GEN_H

#include <ObjCUtil.h>

namespace Slice
{

class ObjCVisitor : public ObjCGenerator, public ParserVisitor
{
public:

    ObjCVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);
    virtual ~ObjCVisitor();

protected:

    virtual void writeDispatchAndMarshalling(const ClassDefPtr&);
    virtual void writeMarshalUnmarshalParams(const ParamDeclList&, const OperationPtr&, bool, bool = false);
    virtual std::string getName(const OperationPtr&) const;
    virtual std::string getSelector(const OperationPtr&) const;
    virtual std::string getParams(const OperationPtr&, bool = false) const;
    virtual std::string getBlockParams(const OperationPtr&) const;
    virtual std::string getMarshalParams(const OperationPtr&, bool = false) const;
    virtual std::string getUnmarshalParams(const OperationPtr&, bool = false) const;
    virtual std::string getServerParams(const OperationPtr&) const;
    virtual std::string getResponseCBSig(const OperationPtr&) const;
    virtual std::string getArgs(const OperationPtr&) const;
    virtual std::string getMarshalArgs(const OperationPtr&) const;
    virtual std::string getUnmarshalArgs(const OperationPtr&) const;
    virtual std::string getServerArgs(const OperationPtr&) const;

    ::IceUtilInternal::Output& _H;
    ::IceUtilInternal::Output& _M;
    std::string _dllExport;
};

class Gen : private ::IceUtil::noncopyable
{
public:

    Gen(const std::string&,
        const std::string&,
        const std::string&,
        const std::vector<std::string>&,
        const std::string&,
        const std::string&);
    ~Gen();

    bool operator!() const; // Returns true if there was a constructor error

    void generate(const UnitPtr&);
    void closeOutput();

private:

    IceUtilInternal::Output _H;
    IceUtilInternal::Output _M;

    std::string _base;
    std::string _include;
    std::vector<std::string> _includePaths;
    std::string _dllExport;

    void printHeader(::IceUtilInternal::Output&);

    class UnitVisitor : public ObjCVisitor
    {
    public:

        UnitVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitUnitEnd(const UnitPtr&);

    private:

        std::vector<Slice::ObjCGenerator::ModulePrefix> _prefixes;
    };

    class ObjectDeclVisitor : public ObjCVisitor
    {
    public:

        ObjectDeclVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

        virtual void visitClassDecl(const ClassDeclPtr&);
    };

    class ProxyDeclVisitor : public ObjCVisitor
    {
    public:

        ProxyDeclVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

        virtual void visitClassDecl(const ClassDeclPtr&);
    };

    class TypesVisitor : public ObjCVisitor
    {
    public:

        TypesVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitOperation(const OperationPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual bool visitExceptionStart(const ExceptionPtr&);
        virtual void visitExceptionEnd(const ExceptionPtr&);
        virtual bool visitStructStart(const StructPtr&);
        virtual void visitStructEnd(const StructPtr&);
        virtual void visitSequence(const SequencePtr&);
        virtual void visitDictionary(const DictionaryPtr&);
        virtual void visitEnum(const EnumPtr&);
        virtual void visitConst(const ConstPtr&);

    private:

        enum Escape { NoEscape, WithEscape };
        enum ContainerType { LocalException, Other };

        void writeConstantValue(IceUtilInternal::Output&, const TypePtr&, const SyntaxTreeBasePtr&, const std::string&) const;
        void writeInit(const ContainedPtr&, const DataMemberList&, const DataMemberList&, const DataMemberList&, bool,
                       int, ContainerType) const;
        void writeFactory(const ContainedPtr&, const DataMemberList&, int, ContainerType) const;
        void writeCopyWithZone(const ContainedPtr&, const DataMemberList&, int, ContainerType) const;
        void writeMembers(const DataMemberList&, int) const;
        void writeMemberSignature(const DataMemberList&, int, ContainerType) const;
        void writeMemberCall(const DataMemberList&, int, ContainerType, Escape) const;
        bool requiresMemberInit(const DataMemberList&) const;
        void writeMemberDefaultValueInit(const DataMemberList&, int) const;
        void writeMemberInit(const DataMemberList&, int) const;
        void writeProperties(const DataMemberList&, int) const;
        void writeSynthesize(const DataMemberList&, int) const;
        void writeOptionalDataMemberSelectors(const DataMemberList&, int) const;
        void writeMemberHashCode(const DataMemberList&, int) const;
        void writeMemberEquals(const DataMemberList&, int) const;
        void writeMemberDealloc(const DataMemberList&, int, const std::string& = "") const;
        void writeMemberMarshal(const DataMemberList&, const DataMemberList&, int) const;
        void writeMemberUnmarshal(const DataMemberList&, const DataMemberList&, int) const;
    };

    class ProxyVisitor : public ObjCVisitor
    {
    public:

        ProxyVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual void visitOperation(const OperationPtr&);
    };

    class HelperVisitor : public ObjCVisitor
    {
    public:

        HelperVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitEnum(const EnumPtr&);
        virtual void visitSequence(const SequencePtr&);
        virtual void visitDictionary(const DictionaryPtr&);
        virtual bool visitStructStart(const StructPtr&);
    };

    class DelegateMVisitor : public ObjCVisitor
    {
    public:

        DelegateMVisitor(::IceUtilInternal::Output&, ::IceUtilInternal::Output&, const std::string&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual void visitOperation(const OperationPtr&);
    };
};

}

#endif
