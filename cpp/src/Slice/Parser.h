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

namespace Slice
{

class GrammerBase;
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
class Enum;
class Enumerator;
class Native;
class Unit;

}

namespace __Ice
{

void ICE_API incRef(::Slice::GrammerBase*);
void ICE_API decRef(::Slice::GrammerBase*);
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
void ICE_API incRef(::Slice::Enum*);
void ICE_API decRef(::Slice::Enum*);
void ICE_API incRef(::Slice::Enumerator*);
void ICE_API decRef(::Slice::Enumerator*);
void ICE_API incRef(::Slice::Native*);
void ICE_API decRef(::Slice::Native*);
void ICE_API incRef(::Slice::Unit*);
void ICE_API decRef(::Slice::Unit*);

}

namespace Slice
{

typedef ::__Ice::Handle<GrammerBase> GrammerBase_ptr;
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
typedef ::__Ice::Handle<Enum> Enum_ptr;
typedef ::__Ice::Handle<Enumerator> Enumerator_ptr;
typedef ::__Ice::Handle<Native> Native_ptr;
typedef ::__Ice::Handle<Unit> Unit_ptr;

}

namespace Slice
{

typedef std::list<Type_ptr> TypeList;
typedef std::list<std::string> StringList;
typedef std::pair<Type_ptr, std::string> TypeString;
typedef std::list<TypeString> TypeStringList;
typedef std::list<ClassDef_ptr> ClassList;

// ----------------------------------------------------------------------
// ParserVisitor
// ----------------------------------------------------------------------

class ICE_API ParserVisitor
{
public:

    virtual ~ParserVisitor() { }
    virtual void visitUnitStart(const Unit_ptr&) { };
    virtual void visitUnitEnd(const Unit_ptr&) { };
    virtual void visitModuleStart(const Module_ptr&) { };
    virtual void visitModuleEnd(const Module_ptr&) { };
    virtual void visitClassDecl(const ClassDecl_ptr&) { };
    virtual void visitClassDefStart(const ClassDef_ptr&) { };
    virtual void visitClassDefEnd(const ClassDef_ptr&) { };
    virtual void visitOperation(const Operation_ptr&) { };
    virtual void visitDataMember(const DataMember_ptr&) { };
    virtual void visitVector(const Vector_ptr&) { };
    virtual void visitEnum(const Enum_ptr&) { };
    virtual void visitNative(const Native_ptr&) { };
};

// ----------------------------------------------------------------------
// GrammerBase
// ----------------------------------------------------------------------

class ICE_API GrammerBase : public ::__Ice::SimpleShared
{
};

#define YYSTYPE Slice::GrammerBase_ptr

// ----------------------------------------------------------------------
// SyntaxTreeBase
// ----------------------------------------------------------------------

class ICE_API SyntaxTreeBase : public GrammerBase
{
public:

    virtual void destroy();
    Unit_ptr unit();
    virtual void visit(ParserVisitor*);

protected:

    SyntaxTreeBase(const Unit_ptr&);

    Unit_ptr unit_;
};

// ----------------------------------------------------------------------
// Type
// ----------------------------------------------------------------------

class ICE_API Type : virtual public SyntaxTreeBase
{
public:

protected:

    Type(const Unit_ptr&);
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

    Builtin(const Unit_ptr&, Kind);
    friend class ICE_API Unit;

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
    std::string comment();

    enum ContainedType
    {
	ContainedTypeVector,
	ContainedTypeEnum,
	ContainedTypeEnumerator,
	ContainedTypeNative,
	ContainedTypeModule,
	ContainedTypeClass,
	ContainedTypeOperation,
	ContainedTypeDataMember
    };
    virtual ContainedType containedType() = 0;

protected:

    Contained(const Container_ptr&,
	      const std::string&);
    friend class ICE_API Container;

    Container_ptr container_;
    std::string name_;
    std::string scoped_;
    std::string comment_;
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
    ClassDef_ptr createClassDef(const std::string&, bool, bool,
				const ClassList&);
    ClassDecl_ptr createClassDecl(const std::string&, bool, bool);
    Vector_ptr createVector(const std::string&, const Type_ptr&);
    Enum_ptr createEnum(const std::string&, const StringList&);
    Enumerator_ptr createEnumerator(const std::string&);
    Native_ptr createNative(const std::string&);
    std::list<Type_ptr> lookupType(const std::string&);
    int includeLevel();
    bool hasProxies();
    bool hasClassDecls();
    bool hasClassDefs();
    bool hasOtherConstructedTypes(); // Other than classes
    std::string thisScope();
    void mergeModules();
    void sort();
    virtual void visit(ParserVisitor*);

protected:

    Container(const Unit_ptr&);

    int includeLevel_;
    std::list<Contained_ptr> contents_;
};

// ----------------------------------------------------------------------
// Module
// ----------------------------------------------------------------------

class ICE_API Module : virtual public Container, virtual public Contained
{
public:

    virtual ContainedType containedType();
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
    bool isLocal();
    bool isInterface();
    virtual ContainedType containedType();
    virtual void visit(ParserVisitor*);

protected:

    ClassDecl(const Container_ptr&,
	      const std::string&,
	      bool,
	      bool);
    friend class ICE_API Container;
    friend class ICE_API ClassDef;

    ClassDef_ptr definition_;
    bool local_;
    bool interface_;
};

// ----------------------------------------------------------------------
// ClassDef
// ----------------------------------------------------------------------

class ICE_API ClassDef : virtual public Container, virtual public Contained
{
public:

    virtual void destroy();
    Operation_ptr createOperation(const std::string&, const Type_ptr&,
				  const TypeStringList&, const TypeStringList&,
				  const TypeList&);
    DataMember_ptr createDataMember(const std::string&, const Type_ptr&);
    ClassList bases();
    ClassList allBases();
    std::list<Operation_ptr> operations();
    std::list<DataMember_ptr> dataMembers();
    bool isAbstract();
    bool isLocal();
    bool isInterface();
    virtual ContainedType containedType();
    virtual void visit(ParserVisitor*);

protected:

    ClassDef(const Container_ptr&,
	     const std::string&,
	     bool,
	     bool,
	     const ClassList&);
    friend class ICE_API Container;

    bool local_;
    bool interface_;
    ClassList bases_;
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
    TypeStringList inputParameters();
    TypeStringList outputParameters();
    TypeList throws();
    virtual ContainedType containedType();
    virtual void visit(ParserVisitor*);

protected:

    Operation(const Container_ptr&,
	      const std::string&,
	      const Type_ptr&,
	      const TypeStringList&,
	      const TypeStringList&,
	      const TypeList&);
    friend class ICE_API ClassDef;

    Type_ptr returnType_;
    TypeStringList inParams_;
    TypeStringList outParams_;
    TypeList throws_;
};

// ----------------------------------------------------------------------
// DataMember
// ----------------------------------------------------------------------

class ICE_API DataMember : virtual public Contained
{
public:

    Type_ptr type();
    virtual ContainedType containedType();
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
    virtual ContainedType containedType();
    virtual void visit(ParserVisitor*);

protected:

    Vector(const Container_ptr&,
	   const std::string&,
	   const Type_ptr&);
    friend class ICE_API Container;

    Type_ptr type_;
};

// ----------------------------------------------------------------------
// Enum
// ----------------------------------------------------------------------

class ICE_API Enum : virtual public Constructed
{
public:

    StringList enumerators();
    virtual ContainedType containedType();
    virtual void visit(ParserVisitor*);

protected:

    Enum(const Container_ptr&,
	 const std::string&,
	 const StringList&);
    friend class ICE_API Container;
    
    StringList enumerators_;
};

// ----------------------------------------------------------------------
// Enumerator
// ----------------------------------------------------------------------

class ICE_API Enumerator : virtual public Contained
{
public:

    virtual ContainedType containedType();

protected:

    Enumerator(const Container_ptr&,
	       const std::string&);
    friend class ICE_API Container;
};

// ----------------------------------------------------------------------
// Native
// ----------------------------------------------------------------------

class Native : virtual public Constructed
{
public:

    virtual ContainedType containedType();
    virtual void visit(ParserVisitor*);

protected:

    Native(const Container_ptr&,
	   const std::string&);
    friend class Container;
};

// ----------------------------------------------------------------------
// Unit
// ----------------------------------------------------------------------

class ICE_API Unit : virtual public Container
{
public:

    static Unit_ptr createUnit(bool, bool);

    bool ignRedefs();

    void setComment(const std::string&);
    std::string currentComment();

    void nextLine();
    void scanPosition(const char*);
    int currentIncludeLevel();

    void error(const char*);
    void warning(const char*);

    Container_ptr currentContainer();
    void pushContainer(const Container_ptr&);
    void popContainer();

    void addContent(const Contained_ptr&);
    void removeContent(const Contained_ptr&);
    std::list<Contained_ptr> findContents(const std::string&);

    std::list<std::string> includeFiles();

    int parse(FILE*, bool);

    virtual void destroy();
    virtual void visit(ParserVisitor*);

    Builtin_ptr builtin(Builtin::Kind);

private:

    Unit(bool, bool);

    bool ignRedefs_;
    bool all_;
    std::string currentComment_;
    int currentLine_;
    int currentIncludeLevel_;
    std::string currentFile_;
    std::string topLevelFile_;
    std::list<std::string> includeFiles_;
    std::stack<Container_ptr> containerStack_;
    std::map<Builtin::Kind, Builtin_ptr> builtins_;
    std::map<std::string, std::list<Contained_ptr> > contentMap_;
};

extern Unit* unit; // The current parser for bison/flex

}

#endif
