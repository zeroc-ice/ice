// **********************************************************************
//
// Copyright (c) 2001
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

#include <Slice/Parser.h>
#include <stack>

namespace IceUtil
{

class XMLOutput;

}

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

    virtual bool visitClassDefStart(const ClassDefPtr&);

private:

    void emitMessage(::IceUtil::XMLOutput&, const OperationPtr&);
    void emitOperation(::IceUtil::XMLOutput&, const OperationPtr&);

    void printHeader(::IceUtil::XMLOutput&);

    std::string containedToId(const ContainedPtr&);

    std::string _name;
    std::string _base;
    std::string _include;
    std::string _orgName;
    std::vector<std::string> _includePaths;
    std::string _dir;
};

}

#endif
