// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

void generate(const UnitPtr&, const ::std::string&, const ::std::string&, const ::std::string&, const std::string&,
              const ::std::string&, const ::std::string&, const ::std::string&, const ::std::string&,
              unsigned, unsigned);

typedef ::std::set< ::std::string> Files;

class GeneratorBase : private ::IceUtil::noncopyable
{
public:

    static void setOutputDir(const ::std::string&);
    static void setHeader(const ::std::string&);
    static void setFooter(const ::std::string&);
    static void setImageDir(const ::std::string&);
    static void setLogoURL(const ::std::string&);
    static void setSearchAction(const ::std::string&);
    static void setIndexCount(int);
    static void warnSummary(int);
    static void setSymbols(const ContainedList&);

    void closeStream();

protected:

    GeneratorBase(::IceUtilInternal::XMLOutput&, const Files&);
    virtual ~GeneratorBase() = 0;

    void openDoc(const ::std::string&, const std::string&, const std::string& = "", const std::string& = "");
    void openDoc(const ContainedPtr&);
    void closeDoc(const std::string& = "");

    void start(const ::std::string&, const ::std::string& = ::std::string());
    void end();

    void printComment(const ContainedPtr&, const SyntaxTreeBasePtr&, const ::std::string&, bool = false);
    void printMetaData(const ContainedPtr&);
    void printSummary(const ContainedPtr&, const ContainerPtr&, bool, bool);

    void printHeaderFooter(const ContainedPtr&);
    void printSearch();
    void printLogo(const ContainedPtr&);

    bool checkFile(const SyntaxTreeBasePtr&);
    ContainedPtr getPageElement(const SyntaxTreeBasePtr&);
    ::std::string getPath(const SyntaxTreeBasePtr&, const SyntaxTreeBasePtr&);
    ::std::string getURL(const SyntaxTreeBasePtr&, const SyntaxTreeBasePtr&, bool, bool);
    ::std::string getURL(const ::std::string&, const SyntaxTreeBasePtr&, bool);

    ::std::string getComment(const ContainedPtr&, const SyntaxTreeBasePtr&, bool, bool = false);

    static ::std::string getAnchor(const SyntaxTreeBasePtr&);
    static StringList getTarget(const SyntaxTreeBasePtr&);

    static ::std::string getImageDir();
    static ::std::string getLogoURL();

    static ::std::string getFooter(const ::std::string&);

    ::IceUtilInternal::XMLOutput& _out;

    static size_t _indexCount;
    static size_t _warnSummary;

private:

    void openStream(const ::std::string&);

    static ::std::string containedToId(const ContainedPtr&, bool);
    static StringList getTagged(const ::std::string&, ::std::string&);
    static ::std::string getScopedMinimized(const SyntaxTreeBasePtr&, const SyntaxTreeBasePtr&, bool);
    static StringList getContained(const SyntaxTreeBasePtr&);
    static StringList getContainer(const SyntaxTreeBasePtr&);
    static StringList toStringList(const ContainedPtr&);
    static StringList toStringList(const ::std::string&);
    static void makeDir(const ::std::string&);
    static ::std::string readFile(const ::std::string&);
    static void readFile(const ::std::string&, ::std::string&, ::std::string&);
    static void getHeaders(const ::std::string&, ::std::string&, ::std::string&);

    ::std::string _indexFooter;
    const Files _files;

    static ::std::string _dir;
    static ::std::string _header1;
    static ::std::string _header2;
    static ::std::string _footer;
    static ::std::string _imageDir;
    static ::std::string _logoURL;
    static ::std::string _searchAction;
    static ContainedList _symbols;

    //
    // TODO:
    // Members below exist to emit warnings for old-style javadoc comments (using [...] instead of {@link ...}),
    // and to emit warnings for old-style scoped names (X::Y::Z instead of X.Y#Z).
    // Once we remove support for the old style comments, we also need to remove these members.
    //
    ::std::set< ::std::string> _warnOldCommentFiles;
    ::std::string toSliceID(const ::std::string&, const ::std::string&);
    void warnOldStyleIdent(const ::std::string&, const ::std::string&);
};

class StartPageGenerator : private GeneratorBase
{
public:

    StartPageGenerator(const Files&);
    ~StartPageGenerator();
    void generate(const ModulePtr&);

private:

    using GeneratorBase::printHeaderFooter;
    void printHeaderFooter();

    typedef ::std::pair< ::std::string, ::std::string> StringPair;
    typedef ::std::vector<StringPair> ModuleDescriptions;
    ModuleDescriptions _modules;
    ::IceUtilInternal::XMLOutput _out;
};

class FileVisitor : private ::IceUtil::noncopyable, public ParserVisitor
{
public:

    FileVisitor(Files&);

    virtual bool visitUnitStart(const UnitPtr&);
    virtual bool visitModuleStart(const ModulePtr&);
    virtual bool visitExceptionStart(const ExceptionPtr&);
    virtual bool visitClassDefStart(const ClassDefPtr&);
    virtual void visitClassDecl(const ClassDeclPtr&);
    virtual bool visitStructStart(const StructPtr&);
    virtual void visitSequence(const SequencePtr&);
    virtual void visitDictionary(const DictionaryPtr&);
    virtual void visitEnum(const EnumPtr&);

private:

    Files& _files;
};

class StartPageVisitor : private ::IceUtil::noncopyable, public ParserVisitor
{
public:

    StartPageVisitor(const Files&);

    virtual bool visitUnitStart(const UnitPtr&);
    virtual bool visitModuleStart(const ModulePtr&);

private:

    StartPageGenerator _spg;
};

class TOCGenerator : private GeneratorBase
{
public:

    TOCGenerator(const Files&, const ::std::string&, const ::std::string&);
    void generate(const ModulePtr&);
    void writeTOC();
    const ContainedList& symbols() const;

private:

    void writeEntry(const ContainedPtr&);

    ::std::string _footer;
    ModuleList _modules;
    ContainedList _symbols;
    ::IceUtilInternal::XMLOutput _out;
};

class TOCVisitor : private ::IceUtil::noncopyable, public ParserVisitor
{
public:

    TOCVisitor(const Files&, const ::std::string&, const ::std::string&);

    virtual bool visitUnitStart(const UnitPtr&);
    virtual bool visitModuleStart(const ModulePtr&);
    void generate();
    const ContainedList& symbols() const;

private:

    TOCGenerator _tg;
};

class ModuleGenerator : private GeneratorBase
{
public:

    ModuleGenerator(::IceUtilInternal::XMLOutput&, const Files&);
    void generate(const ModulePtr&);

private:

    virtual void visitContainer(const ContainerPtr&);
};

class ExceptionGenerator : private GeneratorBase
{
public:

    ExceptionGenerator(::IceUtilInternal::XMLOutput&, const Files&);
    void generate(const ExceptionPtr&);
};

class ClassGenerator : private GeneratorBase
{
public:

    ClassGenerator(::IceUtilInternal::XMLOutput&, const Files&);
    void generate(const ClassDefPtr&);
};

class StructGenerator : private GeneratorBase
{
public:

    StructGenerator(::IceUtilInternal::XMLOutput&, const Files&);
    void generate(const StructPtr&);
};

class EnumGenerator : private GeneratorBase
{
public:

    EnumGenerator(::IceUtilInternal::XMLOutput&, const Files&);
    void generate(const EnumPtr&);
};

class PageVisitor : private ::IceUtil::noncopyable, public ParserVisitor
{
public:

    PageVisitor(const Files&);

    virtual bool visitUnitStart(const UnitPtr&);
    virtual bool visitModuleStart(const ModulePtr&);
    virtual bool visitExceptionStart(const ExceptionPtr&);
    virtual bool visitClassDefStart(const ClassDefPtr&);
    virtual bool visitStructStart(const StructPtr&);
    virtual void visitEnum(const EnumPtr&);

private:

    const Files& _files;
};

}

#endif
