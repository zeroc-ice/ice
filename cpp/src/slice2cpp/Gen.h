// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef GEN_H
#define GEN_H

#include <Slice/Parser.h>
#include <Slice/OutputUtil.h>

namespace Slice
{

class Gen : public ::IceUtil::noncopyable
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

private:

    Output H;
    Output C;

    std::string _base;
    std::string _include;
    std::vector<std::string> _includePaths;
    std::string _dllExport;

    class TypesVisitor : public ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

	TypesVisitor(Output&, Output&, const std::string&);

	virtual bool visitModuleStart(const ModulePtr&);
	virtual void visitModuleEnd(const ModulePtr&);
	virtual bool visitClassDefStart(const ClassDefPtr&);
	virtual bool visitExceptionStart(const ExceptionPtr&);
	virtual void visitExceptionEnd(const ExceptionPtr&);
	virtual bool visitStructStart(const StructPtr&);
	virtual void visitStructEnd(const StructPtr&);
	virtual void visitSequence(const SequencePtr&);
	virtual void visitDictionary(const DictionaryPtr&);
	virtual void visitEnum(const EnumPtr&);
	virtual void visitDataMember(const DataMemberPtr&);

    private:

	Output& H;
	Output& C;

	std::string _dllExport;
    };

    class ProxyDeclVisitor : public ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

	ProxyDeclVisitor(Output&, Output&, const std::string&);

	virtual bool visitUnitStart(const UnitPtr&);
	virtual void visitUnitEnd(const UnitPtr&);
	virtual bool visitModuleStart(const ModulePtr&);
	virtual void visitModuleEnd(const ModulePtr&);
	virtual void visitClassDecl(const ClassDeclPtr&);

    private:

	Output& H;
	Output& C;

	std::string _dllExport;
    };

    class ProxyVisitor : public ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

	ProxyVisitor(Output&, Output&, const std::string&);

	virtual bool visitUnitStart(const UnitPtr&);
	virtual void visitUnitEnd(const UnitPtr&);
	virtual bool visitModuleStart(const ModulePtr&);
	virtual void visitModuleEnd(const ModulePtr&);
	virtual bool visitClassDefStart(const ClassDefPtr&);
	virtual void visitClassDefEnd(const ClassDefPtr&);
	virtual void visitOperation(const OperationPtr&);

    private:

	Output& H;
	Output& C;

	std::string _dllExport;
    };

    class DelegateVisitor : public ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

	DelegateVisitor(Output&, Output&, const std::string&);

	virtual bool visitUnitStart(const UnitPtr&);
	virtual void visitUnitEnd(const UnitPtr&);
	virtual bool visitModuleStart(const ModulePtr&);
	virtual void visitModuleEnd(const ModulePtr&);
	virtual bool visitClassDefStart(const ClassDefPtr&);
	virtual void visitClassDefEnd(const ClassDefPtr&);
	virtual void visitOperation(const OperationPtr&);

    private:

	Output& H;
	Output& C;

	std::string _dllExport;
    };

    class DelegateMVisitor : public ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

	DelegateMVisitor(Output&, Output&, const std::string&);

	virtual bool visitUnitStart(const UnitPtr&);
	virtual void visitUnitEnd(const UnitPtr&);
	virtual bool visitModuleStart(const ModulePtr&);
	virtual void visitModuleEnd(const ModulePtr&);
	virtual bool visitClassDefStart(const ClassDefPtr&);
	virtual void visitClassDefEnd(const ClassDefPtr&);
	virtual void visitOperation(const OperationPtr&);

    private:

	Output& H;
	Output& C;

	std::string _dllExport;
    };

    class DelegateDVisitor : public ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

	DelegateDVisitor(Output&, Output&, const std::string&);

	virtual bool visitUnitStart(const UnitPtr&);
	virtual void visitUnitEnd(const UnitPtr&);
	virtual bool visitModuleStart(const ModulePtr&);
	virtual void visitModuleEnd(const ModulePtr&);
	virtual bool visitClassDefStart(const ClassDefPtr&);
	virtual void visitClassDefEnd(const ClassDefPtr&);
	virtual void visitOperation(const OperationPtr&);

    private:

	Output& H;
	Output& C;

	std::string _dllExport;
    };

    class ObjectDeclVisitor : public ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

	ObjectDeclVisitor(Output&, Output&, const std::string&);

	virtual bool visitModuleStart(const ModulePtr&);
	virtual void visitModuleEnd(const ModulePtr&);
	virtual void visitClassDecl(const ClassDeclPtr&);

    private:

	Output& H;
	Output& C;

	std::string _dllExport;
    };

    class ObjectVisitor : public ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

	ObjectVisitor(Output&, Output&, const std::string&);

	virtual bool visitModuleStart(const ModulePtr&);
	virtual void visitModuleEnd(const ModulePtr&);
	virtual bool visitClassDefStart(const ClassDefPtr&);
	virtual void visitClassDefEnd(const ClassDefPtr&);
	virtual bool visitStructStart(const StructPtr&);
	virtual void visitOperation(const OperationPtr&);
	virtual void visitDataMember(const DataMemberPtr&);

    private:

	Output& H;
	Output& C;

	std::string _dllExport;
    };

    class IceVisitor : public ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

	IceVisitor(Output&, Output&, const std::string&);

	virtual bool visitUnitStart(const UnitPtr&);
	virtual void visitUnitEnd(const UnitPtr&);
	virtual void visitClassDecl(const ClassDeclPtr&);
	virtual bool visitClassDefStart(const ClassDefPtr&);

    private:

	Output& H;
	Output& C;

	std::string _dllExport;
    };

    class HandleVisitor : public ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

	HandleVisitor(Output&, Output&, const std::string&);

	virtual bool visitModuleStart(const ModulePtr&);
	virtual void visitModuleEnd(const ModulePtr&);
	virtual void visitClassDecl(const ClassDeclPtr&);
	virtual bool visitClassDefStart(const ClassDefPtr&);

    private:

	Output& H;
	Output& C;

	std::string _dllExport;
    };
};

}

#endif
