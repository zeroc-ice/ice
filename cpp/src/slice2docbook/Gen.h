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

#include <Parser.h>
#include <OutputUtil.h>
#include <stack>

namespace Slice
{

class Gen : ::__Ice::noncopyable, public ParserVisitor
{
public:

    Gen(const std::string&, const std::string&);
    virtual ~Gen();

    bool operator!() const; // Returns true if there was a constructor error

    void generate(const Unit_ptr&);

    virtual void visitUnitStart(const Unit_ptr&);
    virtual void visitUnitEnd(const Unit_ptr&);
    virtual void visitModuleStart(const Module_ptr&);
    virtual void visitContainer(const Container_ptr&);
    virtual void visitClassDefStart(const ClassDef_ptr&);

private:

    void printHeader();
    StringList getTagged(const std::string&, std::string&);
    void printComment(const Contained_ptr&);
    void printSummary(const Contained_ptr&);
    void start(const std::string&);
    void start(const std::string&, const std::string&);
    void end();

    Output O;

    std::stack<std::string> elementStack_;
};

}

#endif
