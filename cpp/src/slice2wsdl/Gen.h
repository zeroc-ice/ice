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
#include <stack>

namespace Slice
{

class Output;

class Gen : public ::IceUtil::noncopyable, public ParserVisitor
{
public:

    Gen(const std::string&,
	const std::string&,
	const std::string&,
	const std::vector<std::string>&,
	const std::string&);
    virtual ~Gen();

    virtual bool visitClassDefStart(const ClassDefPtr&);

private:

    void emitMessage(Output&, const OperationPtr&);
    void emitOperation(Output&, const OperationPtr&);

    void printHeader(Output&);
    void start(Output&, const std::string&);
    void end(Output&);

    std::string containedToId(const ContainedPtr&);

    std::string _name;
    std::string _base;
    std::string _include;
    std::string _orgName;
    std::vector<std::string> _includePaths;
    std::string _dir;
    std::stack<std::string> _elementStack;
};

}

#endif
