// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef GEN_H
#define GEN_H

#include <Slice/CsUtil.h>

namespace Slice
{

class CsVisitor : public CsGenerator, public ParserVisitor
{
public:

    CsVisitor(::IceUtil::Output&);
    virtual ~CsVisitor();

protected:

    ::IceUtil::Output& _out;
};

class Gen : public ::IceUtil::noncopyable
{
public:

    Gen(const std::string&,
        const std::string&,
        const std::vector<std::string>&);
    ~Gen();

    bool operator!() const; // Returns true if there was a constructor error

    void generate(const UnitPtr&);

private:

    IceUtil::Output _out;

    std::string _base;
    std::vector<std::string> _includePaths;

    void printHeader();

    class TypesVisitor : public CsVisitor
    {
    public:

        TypesVisitor(::IceUtil::Output&);

	virtual bool visitModuleStart(const ModulePtr&);
	virtual void visitModuleEnd(const ModulePtr&);
	virtual bool visitExceptionStart(const ExceptionPtr&);
	virtual void visitExceptionEnd(const ExceptionPtr&);
	virtual bool visitStructStart(const StructPtr&);
	virtual void visitStructEnd(const StructPtr&);
	virtual void visitSequence(const SequencePtr&);
	virtual void visitDictionary(const DictionaryPtr&);
	virtual void visitEnum(const EnumPtr&);
	virtual void visitConst(const ConstPtr&);
	virtual void visitDataMember(const DataMemberPtr&);
    };
};

}

#endif
