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

class Gen : public ::IceUtil::noncopyable, public ParserVisitor
{
public:

    Gen(const std::string&, const std::string&, bool, bool);
    virtual ~Gen();

    bool operator!() const; // Returns true if there was a constructor error

    void generate(const UnitPtr&);

    virtual void visitUnitStart(const UnitPtr&);
    virtual void visitUnitEnd(const UnitPtr&);
    virtual void visitModuleStart(const ModulePtr&);
    virtual void visitContainer(const ContainerPtr&);
    virtual void visitClassDefStart(const ClassDefPtr&);
    virtual void visitStructStart(const StructPtr&);

private:

    void printHeader();
    StringList getTagged(const std::string&, std::string&);
    void printComment(const ContainedPtr&);
    void printSummary(const ContainedPtr&);
    void start(const std::string&);
    void start(const std::string&, const std::string&);
    void end();

    std::string scopedToId(const std::string&);
    std::string getScopedMinimized(const ContainedPtr&, const ContainerPtr&);
    std::string toString(const SyntaxTreeBasePtr&, const ContainerPtr&);
    std::string toString(const std::string&, const ContainerPtr&);

    Output O;

    bool _standAlone;
    bool _noGlobals;
    std::string _chapter;
    std::stack<std::string> _elementStack;
    std::string _idPrefix;
    std::map<std::string, int> _idMap;
    int _nextId;
};

}

#endif
