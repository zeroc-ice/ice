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
#include <stack>

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
	const ClassDefPtr&);
    virtual ~Gen();

    bool operator!() const; // Returns true if there was a constructor error

    void generate(const UnitPtr&);

    void emitMessage(const OperationPtr&);
    void emitOperation(const OperationPtr&);

private:

    void printHeader();
    void start(const std::string&);
    void end();

    std::string containedToId(const ContainedPtr&);
    std::string toString(const SyntaxTreeBasePtr&);

    Output O;

    std::string _base;
    std::string _include;
    std::string _orgName;
    std::vector<std::string> _includePaths;
    ClassDefPtr _classDef;
    std::stack<std::string> _elementStack;
};

}

#endif
