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

    Gen(const std::string&);
    virtual ~Gen();

    bool operator!() const; // Returns true if there was a constructor error

    void generate(const Unit_ptr&);

    virtual void visitUnitStart(const Unit_ptr&);
    virtual void visitUnitEnd(const Unit_ptr&);
    virtual void visitModuleStart(const Module_ptr&);
    virtual void visitModuleEnd(const Module_ptr&);
    virtual void visitClassDefStart(const ClassDef_ptr&);
    virtual void visitClassDefEnd(const ClassDef_ptr&);
    virtual void visitOperation(const Operation_ptr&);
    virtual void visitDataMember(const DataMember_ptr&);
    virtual void visitVector(const Vector_ptr&);
    virtual void visitNative(const Native_ptr&);

private:

    void printHeader(Output&);

    std::string name_;
    std::stack<Output*> outputStack_;
};

}

#endif
