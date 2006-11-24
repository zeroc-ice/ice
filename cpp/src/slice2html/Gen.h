// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef GEN_H
#define GEN_H

#include <Slice/Parser.h>
#include <IceUtil/OutputUtil.h>

namespace Slice
{

void generate(const UnitPtr&, const ::std::string&, const ::std::string&, const ::std::string&, unsigned);

class GeneratorBase : private ::IceUtil::noncopyable
{
public:

    static void setOutputDir(const ::std::string&);
    static void setHeader(const ::std::string&);
    static void setFooter(const ::std::string&);
    static void setIndexCount(const int);

protected:

    GeneratorBase(::IceUtil::XMLOutput&);
    virtual ~GeneratorBase() = 0;

    void openDoc(const ::std::string&, const std::string&);
    void openDoc(const ContainedPtr&);
    void closeDoc();

    void start(const ::std::string&, const ::std::string& = ::std::string());
    void end();

    void printComment(const ContainedPtr&, const ::std::string&, bool = false);
    void printMetaData(const ContainedPtr&);
    void printSummary(const ContainedPtr&, const ContainerPtr&, bool);

    static ::std::string getAnchor(const SyntaxTreeBasePtr&);
    static ::std::string getLinkPath(const SyntaxTreeBasePtr&, const ContainerPtr&, bool);
    static ::std::string toString(const SyntaxTreeBasePtr&, const ContainerPtr&, bool = true, bool = false);
    static ::std::string toString(const ::std::string&, const ContainerPtr&, bool = true, bool = false);
    static ::std::string getComment(const ContainedPtr&, const ContainerPtr&, bool, bool = false);

    ::IceUtil::XMLOutput& _out;

    static unsigned _indexCount;

private:

    void openStream(const ::std::string&);

    static ::std::string containedToId(const ContainedPtr&, bool);
    static StringList getTagged(const ::std::string&, ::std::string&);
    static ::std::string getScopedMinimized(const ContainedPtr&, const ContainerPtr&);
    static StringList getContained(const SyntaxTreeBasePtr&);
    static StringList getContainer(const SyntaxTreeBasePtr&);
    static StringList toStringList(const ContainedPtr&);
    static void makeDir(const ::std::string&);
    static ::std::string readFile(const ::std::string&);
    static void readFile(const ::std::string&, ::std::string&, ::std::string&);

    static ::std::string _dir;
    static ::std::string _header1;
    static ::std::string _header2;
    static ::std::string _footer;
};

class IndexGenerator : private GeneratorBase
{
public:

    IndexGenerator();
    ~IndexGenerator();
    void generate(const ModulePtr&);

private:
    typedef ::std::pair< ::std::string, ::std::string> StringPair;
    typedef ::std::vector<StringPair> ModuleDescriptions;
    ModuleDescriptions _modules;
    ::IceUtil::XMLOutput _out;
};

class IndexVisitor : private ::IceUtil::noncopyable, public ParserVisitor
{
public:

    IndexVisitor();

    virtual bool visitUnitStart(const UnitPtr&);
    virtual bool visitModuleStart(const ModulePtr&);

private:

    IndexGenerator _ig;
};

class ModuleGenerator : private GeneratorBase
{
public:

    ModuleGenerator(::IceUtil::XMLOutput&);
    void generate(const ModulePtr&);

private:

    virtual void visitContainer(const ContainerPtr&);

};

class ExceptionGenerator : private GeneratorBase
{
public:

    ExceptionGenerator(::IceUtil::XMLOutput&);
    void generate(const ExceptionPtr&);
};

class ClassGenerator : private GeneratorBase
{
public:

    ClassGenerator(::IceUtil::XMLOutput&);
    void generate(const ClassDefPtr&);
};

class StructGenerator : private GeneratorBase
{
public:

    StructGenerator(::IceUtil::XMLOutput&);
    void generate(const StructPtr&);
};

class EnumGenerator : private GeneratorBase
{
public:

    EnumGenerator(::IceUtil::XMLOutput&);
    void generate(const EnumPtr&);
};

class Visitor : private ::IceUtil::noncopyable, public ParserVisitor
{
public:

    Visitor();

    virtual bool visitUnitStart(const UnitPtr&);
    virtual bool visitModuleStart(const ModulePtr&);
    virtual bool visitExceptionStart(const ExceptionPtr&);
    virtual bool visitClassDefStart(const ClassDefPtr&);
    virtual bool visitStructStart(const StructPtr&);
    virtual void visitEnum(const EnumPtr&);
};

}

#endif
