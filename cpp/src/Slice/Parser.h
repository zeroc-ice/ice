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

namespace Slice
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

void ICE_API incRef(::Slice::Token*);
void ICE_API decRef(::Slice::Token*);
void ICE_API incRef(::Slice::String*);
void ICE_API decRef(::Slice::String*);
void ICE_API incRef(::Slice::Parameters*);
void ICE_API decRef(::Slice::Parameters*);
void ICE_API incRef(::Slice::Throws*);
void ICE_API decRef(::Slice::Throws*);
void ICE_API incRef(::Slice::SyntaxTreeBase*);
void ICE_API decRef(::Slice::SyntaxTreeBase*);
void ICE_API incRef(::Slice::Type*);
void ICE_API decRef(::Slice::Type*);
void ICE_API incRef(::Slice::Builtin*);
void ICE_API decRef(::Slice::Builtin*);
void ICE_API incRef(::Slice::Contained*);
void ICE_API decRef(::Slice::Contained*);
void ICE_API incRef(::Slice::Container*);
void ICE_API decRef(::Slice::Container*);
void ICE_API incRef(::Slice::Module*);
void ICE_API decRef(::Slice::Module*);
void ICE_API incRef(::Slice::Constructed*);
void ICE_API decRef(::Slice::Constructed*);
void ICE_API incRef(::Slice::ClassDecl*);
void ICE_API decRef(::Slice::ClassDecl*);
void ICE_API incRef(::Slice::ClassDef*);
void ICE_API decRef(::Slice::ClassDef*);
void ICE_API incRef(::Slice::Proxy*);
void ICE_API decRef(::Slice::Proxy*);
void ICE_API incRef(::Slice::Operation*);
void ICE_API decRef(::Slice::Operation*);
void ICE_API incRef(::Slice::DataMember*);
void ICE_API decRef(::Slice::DataMember*);
void ICE_API incRef(::Slice::Vector*);
void ICE_API decRef(::Slice::Vector*);
void ICE_API incRef(::Slice::Parser*);
void ICE_API decRef(::Slice::Parser*);

}

namespace Slice
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

namespace Slice
{

typedef std::list<Type_ptr> TypeList;
typedef std::pair<Type_ptr, std::string> TypeName;
typedef std::list<TypeName> TypeNameList;

// ----------------------------------------------------------------------
// ParserVisitor
// ----------------------------------------------------------------------

class ICE_API ParserVisitor
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

class ICE_API Token : virtual public ::__Ice::SimpleShared
{
};

#define YYSTYPE Slice::Token_ptr

// ----------------------------------------------------------------------
// String
// ----------------------------------------------------------------------

class ICE_API String : virtual public Token
{
public:

    String() { }
    std::string v;
};

// ----------------------------------------------------------------------
// Parameters
// ----------------------------------------------------------------------

class ICE_API Parameters : virtual public Token
{
public:

    Parameters() { }
    TypeNameList v;
};

// ----------------------------------------------------------------------
// Throws
// ----------------------------------------------------------------------

class ICE_API Throws : virtual public Token
{
public:

    Throws() { }
    TypeList v;
};

// ----------------------------------------------------------------------
// SyntaxTreeBase
// ----------------------------------------------------------------------

class ICE_API SyntaxTreeBase : virtual public ::__Ice::SimpleShared
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

class ICE_API Type : virtual public SyntaxTreeBase, virtual public Token
{
public:

protected:

    Type(const Parser_ptr&);
};

// ----------------------------------------------------------------------
// Builtin
// ----------------------------------------------------------------------

class ICE_API Builtin : virtual public Type
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
    friend class ICE_API Parser;

    Kind kind_;
};

// ----------------------------------------------------------------------
// Contained
// ----------------------------------------------------------------------

class ICE_API Contained : virtual public SyntaxTreeBase
{
public:

    Container_ptr container();
    std::string name();
    std::string scoped();
    std::string scope();

protected:

    Contained(const Container_ptr&,
	      const std::string&);
    friend class ICE_API Container;

    Container_ptr container_;
    std::string name_;
    std::string scoped_;
};

bool ICE_API operator<(Contained&, Contained&);
bool ICE_API operator==(Contained&, Contained&);

// ----------------------------------------------------------------------
// Container
// ----------------------------------------------------------------------

class ICE_API Container : virtual public SyntaxTreeBase
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

class ICE_API Module : virtual public Container, virtual public Contained
{
public:

    virtual void visit(ParserVisitor*);

protected:

    Module(const Container_ptr&,
	   const std::string&);
    friend class ICE_API Container;
};

// ----------------------------------------------------------------------
// Constructed
// ----------------------------------------------------------------------

class ICE_API Constructed : virtual public Type, virtual public Contained
{
public:

protected:

    Constructed(const Container_ptr&,
		const std::string&);
};

// ----------------------------------------------------------------------
// ClassDecl
// ----------------------------------------------------------------------

class ICE_API ClassDecl : virtual public Constructed
{
public:

    ClassDef_ptr definition();
    bool local();
    virtual void visit(ParserVisitor*);

protected:

    ClassDecl(const Container_ptr&,
	      const std::string&,
	      bool);
    friend class ICE_API Container;
    friend class ICE_API ClassDef;

    bool local_;
    ClassDef_ptr definition_;
};

// ----------------------------------------------------------------------
// ClassDef
// ----------------------------------------------------------------------

class ICE_API ClassDef : virtual public Container, virtual public Contained,
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
    friend class ICE_API Container;

    ClassDef_ptr base_;
    bool local_;
};

// ----------------------------------------------------------------------
// Proxy
// ----------------------------------------------------------------------

class ICE_API Proxy : virtual public Type
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

class ICE_API Operation : virtual public Contained
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
    friend class ICE_API ClassDef;

    Type_ptr returnType_;
    TypeNameList inParams_;
    TypeNameList outParams_;
    TypeList throws_;
};

// ----------------------------------------------------------------------
// DataMember
// ----------------------------------------------------------------------

class ICE_API DataMember : virtual public Contained
{
public:

    Type_ptr type();
    virtual void visit(ParserVisitor*);

protected:
    
    DataMember(const Container_ptr&,
	       const std::string&,
	       const Type_ptr&);
    friend class ICE_API ClassDef;

    Type_ptr type_;
};

// ----------------------------------------------------------------------
// Vector
// ----------------------------------------------------------------------

class ICE_API Vector : virtual public Constructed
{
public:

    Type_ptr type();
    virtual void visit(ParserVisitor*);

protected:

    Vector(const Container_ptr&,
	   const std::string&,
	   const Type_ptr&);
    friend class ICE_API Container;

    Type_ptr type_;
};

// ----------------------------------------------------------------------
// Parser
// ----------------------------------------------------------------------

class ICE_API Parser : virtual public Container
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

    int parse(FILE*, bool);

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
