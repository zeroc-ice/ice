// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef PARSER_H
#define PARSER_H

#include <Ice/Shared.h>
#include <Ice/Handle.h>
#include <list>
#include <stack>
#include <map>

extern int yynerrs;

int yyparse();
int yylex();
void yyerror(const char* s);

// ----------------------------------------------------------------------
// Type_ptr declarations, reference counting, and handle types
// ----------------------------------------------------------------------

namespace IceLang
{

class Token;
class String;
class Parameters;
class Throws;
class SyntaxTreeBase;
class Type;
class Builtin;
class Contained;
class Container;
class Module;
class Constructed;
class ClassDecl;
class ClassDef;
class Proxy;
class Operation;
class DataMember;
class Vector;
class Parser;

}

namespace __Ice
{

void incRef(::IceLang::Token*);
void decRef(::IceLang::Token*);
void incRef(::IceLang::String*);
void decRef(::IceLang::String*);
void incRef(::IceLang::Parameters*);
void decRef(::IceLang::Parameters*);
void incRef(::IceLang::Throws*);
void decRef(::IceLang::Throws*);
void incRef(::IceLang::SyntaxTreeBase*);
void decRef(::IceLang::SyntaxTreeBase*);
void incRef(::IceLang::Type*);
void decRef(::IceLang::Type*);
void incRef(::IceLang::Builtin*);
void decRef(::IceLang::Builtin*);
void incRef(::IceLang::Contained*);
void decRef(::IceLang::Contained*);
void incRef(::IceLang::Container*);
void decRef(::IceLang::Container*);
void incRef(::IceLang::Module*);
void decRef(::IceLang::Module*);
void incRef(::IceLang::Constructed*);
void decRef(::IceLang::Constructed*);
void incRef(::IceLang::ClassDecl*);
void decRef(::IceLang::ClassDecl*);
void incRef(::IceLang::ClassDef*);
void decRef(::IceLang::ClassDef*);
void incRef(::IceLang::Proxy*);
void decRef(::IceLang::Proxy*);
void incRef(::IceLang::Operation*);
void decRef(::IceLang::Operation*);
void incRef(::IceLang::DataMember*);
void decRef(::IceLang::DataMember*);
void incRef(::IceLang::Vector*);
void decRef(::IceLang::Vector*);
void incRef(::IceLang::Parser*);
void decRef(::IceLang::Parser*);

}

namespace IceLang
{

typedef ::__Ice::Handle<Token> Token_ptr;
typedef ::__Ice::Handle<String> String_ptr;
typedef ::__Ice::Handle<Parameters> Parameters_ptr;
typedef ::__Ice::Handle<Throws> Throws_ptr;
typedef ::__Ice::Handle<SyntaxTreeBase> SyntaxTreeBase_ptr;
typedef ::__Ice::Handle<Type> Type_ptr;
typedef ::__Ice::Handle<Builtin> Builtin_ptr;
typedef ::__Ice::Handle<Contained> Contained_ptr;
typedef ::__Ice::Handle<Container> Container_ptr;
typedef ::__Ice::Handle<Module> Module_ptr;
typedef ::__Ice::Handle<Constructed> Constructed_ptr;
typedef ::__Ice::Handle<ClassDecl> ClassDecl_ptr;
typedef ::__Ice::Handle<ClassDef> ClassDef_ptr;
typedef ::__Ice::Handle<Proxy> Proxy_ptr;
typedef ::__Ice::Handle<Operation> Operation_ptr;
typedef ::__Ice::Handle<DataMember> DataMember_ptr;
typedef ::__Ice::Handle<Vector> Vector_ptr;
typedef ::__Ice::Handle<Parser> Parser_ptr;

}

namespace IceLang
{

typedef std::list<Type_ptr> TypeList;
typedef std::pair<Type_ptr, std::string> TypeName;
typedef std::list<TypeName> TypeNameList;

// ----------------------------------------------------------------------
// ParserVisitor
// ----------------------------------------------------------------------

class ParserVisitor
{
public:

    virtual ~ParserVisitor() { }
    virtual void visitUnitStart(const Parser_ptr&) { };
    virtual void visitUnitEnd(const Parser_ptr&) { };
    virtual void visitModuleStart(const Module_ptr&) { };
    virtual void visitModuleEnd(const Module_ptr&) { };
    virtual void visitClassDecl(const ClassDecl_ptr&) { };
    virtual void visitClassDefStart(const ClassDef_ptr&) { };
    virtual void visitClassDefEnd(const ClassDef_ptr&) { };
    virtual void visitOperation(const Operation_ptr&) { };
    virtual void visitDataMember(const DataMember_ptr&) { };
    virtual void visitVector(const Vector_ptr&) { };
};

// ----------------------------------------------------------------------
// Token
// ----------------------------------------------------------------------

class Token : virtual public ::__Ice::SimpleShared
{
};

#define YYSTYPE IceLang::Token_ptr

// ----------------------------------------------------------------------
// String
// ----------------------------------------------------------------------

class String : virtual public Token
{
public:

    String() { }
    std::string v;
};

// ----------------------------------------------------------------------
// Parameters
// ----------------------------------------------------------------------

class Parameters : virtual public Token
{
public:

    Parameters() { }
    TypeNameList v;
};

// ----------------------------------------------------------------------
// Throws
// ----------------------------------------------------------------------

class Throws : virtual public Token
{
public:

    Throws() { }
    TypeList v;
};

// ----------------------------------------------------------------------
// SyntaxTreeBase
// ----------------------------------------------------------------------

class SyntaxTreeBase : virtual public ::__Ice::SimpleShared
{
public:

    virtual void destroy();
    Parser_ptr parser();
    virtual void visit(ParserVisitor*);

protected:

    SyntaxTreeBase(const Parser_ptr&);

    Parser_ptr parser_;
};

// ----------------------------------------------------------------------
// Type
// ----------------------------------------------------------------------

class Type : virtual public SyntaxTreeBase, virtual public Token
{
public:

protected:

    Type(const Parser_ptr&);
};

// ----------------------------------------------------------------------
// Builtin
// ----------------------------------------------------------------------

class Builtin : virtual public Type
{
public:

    enum Kind
    {
	KindByte,
	KindBool,
	KindShort,
	KindInt,
	KindLong,
	KindFloat,
	KindDouble,
	KindString,
	KindWString,
	KindObject,
	KindObjectProxy,
	KindLocalObject
    };
    Kind kind();

protected:

    Builtin(const Parser_ptr&, Kind);
    friend class Parser;

    Kind kind_;
};

// ----------------------------------------------------------------------
// Contained
// ----------------------------------------------------------------------

class Contained : virtual public SyntaxTreeBase
{
public:

    Container_ptr container();
    std::string name();
    std::string scoped();
    std::string scope();

protected:

    Contained(const Container_ptr&,
	      const std::string&);
    friend class Container;

    Container_ptr container_;
    std::string name_;
    std::string scoped_;
};

bool operator<(Contained&, Contained&);
bool operator==(Contained&, Contained&);

// ----------------------------------------------------------------------
// Container
// ----------------------------------------------------------------------

class Container : virtual public SyntaxTreeBase
{
public:

    virtual void destroy();
    Module_ptr createModule(const std::string&);
    ClassDef_ptr createClassDef(const std::string&, const ClassDef_ptr&, bool);
    ClassDecl_ptr createClassDecl(const std::string&, bool);
    Vector_ptr createVector(const std::string&, const Type_ptr&);
    std::vector<Type_ptr> lookupType(const std::string&);
    int includeLevel();
    bool hasProxies();
    bool hasClassDecls();
    bool hasClassDefs();
    bool hasOtherConstructedTypes(); // Other than classes
    std::string thisScope();
    virtual void visit(ParserVisitor*);

protected:

    Container(const Parser_ptr&);

    int includeLevel_;
    std::vector<Contained_ptr> contents_;
};

// ----------------------------------------------------------------------
// Module
// ----------------------------------------------------------------------

class Module : virtual public Container, virtual public Contained
{
public:

    virtual void visit(ParserVisitor*);

protected:

    Module(const Container_ptr&,
	   const std::string&);
    friend class Container;
};

// ----------------------------------------------------------------------
// Constructed
// ----------------------------------------------------------------------

class Constructed : virtual public Type, virtual public Contained
{
public:

protected:

    Constructed(const Container_ptr&,
		const std::string&);
};

// ----------------------------------------------------------------------
// ClassDecl
// ----------------------------------------------------------------------

class ClassDecl : virtual public Constructed
{
public:

    ClassDef_ptr definition();
    bool local();
    virtual void visit(ParserVisitor*);

protected:

    ClassDecl(const Container_ptr&,
	      const std::string&,
	      bool);
    friend class Container;
    friend class ClassDef;

    bool local_;
    ClassDef_ptr definition_;
};

// ----------------------------------------------------------------------
// ClassDef
// ----------------------------------------------------------------------

class ClassDef : virtual public Container, virtual public Contained,
		 virtual public Token 
{
public:

    virtual void destroy();
    Operation_ptr createOperation(const std::string&, const Type_ptr&,
				  const TypeNameList&, const TypeNameList&,
				  const TypeList&);
    DataMember_ptr createDataMember(const std::string&, const Type_ptr&);
    ClassDef_ptr base();
    void base(const ClassDef_ptr&);
    std::vector<Operation_ptr> operations();
    std::vector<DataMember_ptr> dataMembers();
    bool abstract();
    bool local();
    virtual void visit(ParserVisitor*);

protected:

    ClassDef(const Container_ptr&,
	     const std::string&,
	     const ClassDef_ptr&,
	     bool);
    friend class Container;

    ClassDef_ptr base_;
    bool local_;
};

// ----------------------------------------------------------------------
// Proxy
// ----------------------------------------------------------------------

class Proxy : virtual public Type
{
public:

    ClassDecl_ptr _class();

    Proxy(const ClassDecl_ptr&);

protected:

    ClassDecl_ptr class_;
};

// ----------------------------------------------------------------------
// Operation
// ----------------------------------------------------------------------

class Operation : virtual public Contained
{
public:

    Type_ptr returnType();
    TypeNameList inputParameters();
    TypeNameList outputParameters();
    TypeList throws();
    virtual void visit(ParserVisitor*);

protected:

    Operation(const Container_ptr&,
	      const std::string&,
	      const Type_ptr&,
	      const TypeNameList&,
	      const TypeNameList&,
	      const TypeList&);
    friend class ClassDef;

    Type_ptr returnType_;
    TypeNameList inParams_;
    TypeNameList outParams_;
    TypeList throws_;
};

// ----------------------------------------------------------------------
// DataMember
// ----------------------------------------------------------------------

class DataMember : virtual public Contained
{
public:

    Type_ptr type();
    virtual void visit(ParserVisitor*);

protected:
    
    DataMember(const Container_ptr&,
	       const std::string&,
	       const Type_ptr&);
    friend class ClassDef;

    Type_ptr type_;
};

// ----------------------------------------------------------------------
// Vector
// ----------------------------------------------------------------------

class Vector : virtual public Constructed
{
public:

    Type_ptr type();
    virtual void visit(ParserVisitor*);

protected:

    Vector(const Container_ptr&,
	   const std::string&,
	   const Type_ptr&);
    friend class Container;

    Type_ptr type_;
};

// ----------------------------------------------------------------------
// Parser
// ----------------------------------------------------------------------

class Parser : virtual public Container
{
public:

    static Parser_ptr createParser();

    void nextLine();
    void scanPosition(const char*);
    int currentIncludeLevel();

    void error(const char*);
    void warning(const char*);

    Container_ptr currentContainer();
    void pushContainer(const Container_ptr&);
    void popContainer();

    void addContent(const Contained_ptr&);
    std::vector<Contained_ptr> findContents(const std::string&);

    std::vector<std::string> includeFiles();

    int parse(FILE*);

    virtual void destroy();
    virtual void visit(ParserVisitor*);

    Builtin_ptr builtin(Builtin::Kind);

private:

    Parser();

    int currentLine_;
    int currentIncludeLevel_;
    std::string currentFile_;
    std::string topLevelFile_;
    std::vector<std::string> includeFiles_;
    std::stack<Container_ptr> containerStack_;
    std::map<Builtin::Kind, Builtin_ptr> builtins_;
    std::map<std::string, std::vector<Contained_ptr> > contentMap_;
};

extern Parser* parser; // The current parser for bison/flex

}

#endif
