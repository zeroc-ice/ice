// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef GEN_H
#define GEN_H

#include <Slice/Parser.h>
#include <IceUtil/OutputUtil.h>
#include <stack>

namespace Slice
{

class Gen : public ::IceUtil::noncopyable, public ParserVisitor
{
public:

    Gen(const std::string&,
	const std::string&,
	const std::string&,
	const std::vector<std::string>&,
	const std::string&);
    virtual ~Gen();

    bool operator!() const; // Returns true if there was a constructor error

    void generate(const UnitPtr&);

    virtual bool visitClassDefStart(const ClassDefPtr&);
    virtual bool visitExceptionStart(const ExceptionPtr&);
    virtual bool visitStructStart(const StructPtr&);
    virtual void visitOperation(const OperationPtr&);
    virtual void visitEnum(const EnumPtr&);
    virtual void visitConstDef(const ConstDefPtr&);
    virtual void visitSequence(const SequencePtr&);
    virtual void visitDictionary(const DictionaryPtr&);

private:

    void printHeader();
    void annotate(const ::std::string&);
    void emitElement(const ::std::string&, const TypePtr&);

    std::string containedToId(const ContainedPtr&);
    std::string toString(const SyntaxTreeBasePtr&);

    ::IceUtil::XMLOutput O;

    std::string _base;
    std::string _include;
    std::string _orgName;
    std::vector<std::string> _includePaths;
};

}

#endif
