// **********************************************************************
//
// Copyright (c) 2002
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
#include <IceUtil/OutputUtil.h>
#include <stack>

namespace Slice
{

class Gen : public ::IceUtil::noncopyable, public ParserVisitor
{
public:

    Gen(const std::string&, const std::string&, bool, bool, bool);
    virtual ~Gen();

    bool operator!() const; // Returns true if there was a constructor error

    void generate(const UnitPtr&);

    virtual bool visitUnitStart(const UnitPtr&);
    virtual void visitUnitEnd(const UnitPtr&);
    virtual bool visitModuleStart(const ModulePtr&);
    virtual void visitContainer(const ContainerPtr&);
    virtual bool visitClassDefStart(const ClassDefPtr&);
    virtual bool visitExceptionStart(const ExceptionPtr&);
    virtual bool visitStructStart(const StructPtr&);
    virtual void visitEnum(const EnumPtr&);
    virtual void visitConstDef(const ConstDefPtr&);

private:

    void printHeader();
    std::string getComment(const ContainedPtr&, const ContainerPtr&, bool);
    StringList getTagged(const std::string&, std::string&);
    void printMetaData(const ContainedPtr&);
    void printComment(const ContainedPtr&);
    void printSummary(const ContainedPtr&);
    void start(const std::string&);
    void start(const std::string&, const std::string&);
    void end();

    std::string containedToId(const ContainedPtr&);
    std::string getScopedMinimized(const ContainedPtr&, const ContainerPtr&);
    std::string toString(const SyntaxTreeBasePtr&, const ContainerPtr&, bool = true);
    std::string toString(const std::string&, const ContainerPtr&, bool = true);

    ::IceUtil::XMLOutput O;

    bool _standAlone;
    bool _noGlobals;
    std::string _chapter;
};

}

#endif
