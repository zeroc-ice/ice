// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef SLICE_PARSER_H
#define SLICE_PARSER_H

#include <Ice/Shared.h>
#include <Ice/Handle.h>
#include <list>
#include <stack>
#include <map>

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
class Struct;
class Operation;
class DataMember;
class Sequence;
class Dictionary;
class Enum;
class Enumerator;
class Native;
class Unit;

}

namespace IceInternal
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
void ICE_API incRef(::Slice::Struct*);
void ICE_API decRef(::Slice::Struct*);
void ICE_API incRef(::Slice::Operation*);
void ICE_API decRef(::Slice::Operation*);
void ICE_API incRef(::Slice::DataMember*);
void ICE_API decRef(::Slice::DataMember*);
void ICE_API incRef(::Slice::Sequence*);
void ICE_API decRef(::Slice::Sequence*);
void ICE_API incRef(::Slice::Dictionary*);
void ICE_API decRef(::Slice::Dictionary*);
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

typedef ::IceInternal::Handle<GrammerBase> GrammerBasePtr;
typedef ::IceInternal::Handle<SyntaxTreeBase> SyntaxTreeBasePtr;
typedef ::IceInternal::Handle<Type> TypePtr;
typedef ::IceInternal::Handle<Builtin> BuiltinPtr;
typedef ::IceInternal::Handle<Contained> ContainedPtr;
typedef ::IceInternal::Handle<Container> ContainerPtr;
typedef ::IceInternal::Handle<Module> ModulePtr;
typedef ::IceInternal::Handle<Constructed> ConstructedPtr;
typedef ::IceInternal::Handle<ClassDecl> ClassDeclPtr;
typedef ::IceInternal::Handle<ClassDef> ClassDefPtr;
typedef ::IceInternal::Handle<Proxy> ProxyPtr;
typedef ::IceInternal::Handle<Struct> StructPtr;
typedef ::IceInternal::Handle<Operation> OperationPtr;
typedef ::IceInternal::Handle<DataMember> DataMemberPtr;
typedef ::IceInternal::Handle<Sequence> SequencePtr;
typedef ::IceInternal::Handle<Dictionary> DictionaryPtr;
typedef ::IceInternal::Handle<Enum> EnumPtr;
typedef ::IceInternal::Handle<Enumerator> EnumeratorPtr;
typedef ::IceInternal::Handle<Native> NativePtr;
typedef ::IceInternal::Handle<Unit> UnitPtr;

}

//
// Stuff for flex and bison
//

#define YYSTYPE Slice::GrammerBasePtr
#define YY_DECL int yylex(YYSTYPE* yylvalp)
YY_DECL;
int yyparse();

//
// I must set the initial stack depth to the maximum stack depth to
// disable bison stack resizing. The bison stack resizing routines use
// simple malloc/alloc/memcpy calls, which do not work for the
// YYSTYPE, since YYSTYPE is a C++ smart pointer, with a default
// constructor and a destructor.
//
#define YYMAXDEPTH  20000 // 20000 should suffice. Bison default is 10000 as maximum.
#define YYINITDEPTH YYMAXDEPTH // Initial depth is set to max depth, for the reasons described above.

namespace Slice
{

typedef std::list<TypePtr> TypeList;
typedef std::list<std::string> StringList;
typedef std::pair<TypePtr, std::string> TypeString;
typedef std::list<TypeString> TypeStringList;
typedef std::list<ContainedPtr> ContainedList;
typedef std::list<ModulePtr> ModuleList;
typedef std::list<ClassDefPtr> ClassList;
typedef std::list<StructPtr> StructList;
typedef std::list<SequencePtr> SequenceList;
typedef std::list<DictionaryPtr> DictionaryList;
typedef std::list<EnumPtr> EnumList;
typedef std::list<NativePtr> NativeList;
typedef std::list<OperationPtr> OperationList;
typedef std::list<DataMemberPtr> DataMemberList;

// ----------------------------------------------------------------------
// ParserVisitor
// ----------------------------------------------------------------------

class ICE_API ParserVisitor
{
public:

    virtual ~ParserVisitor() { }
    virtual void visitUnitStart(const UnitPtr&) { }
    virtual void visitUnitEnd(const UnitPtr&) { }
    virtual void visitModuleStart(const ModulePtr&) { }
    virtual void visitModuleEnd(const ModulePtr&) { }
    virtual void visitClassDecl(const ClassDeclPtr&) { }
    virtual void visitClassDefStart(const ClassDefPtr&) { }
    virtual void visitClassDefEnd(const ClassDefPtr&) { }
    virtual void visitStructStart(const StructPtr&) { }
    virtual void visitStructEnd(const StructPtr&) { }
    virtual void visitOperation(const OperationPtr&) { }
    virtual void visitDataMember(const DataMemberPtr&) { }
    virtual void visitSequence(const SequencePtr&) { }
    virtual void visitDictionary(const DictionaryPtr&) { }
    virtual void visitEnum(const EnumPtr&) { }
    virtual void visitNative(const NativePtr&) { }
};

// ----------------------------------------------------------------------
// GrammerBase
// ----------------------------------------------------------------------

class ICE_API GrammerBase : public ::IceInternal::SimpleShared
{
};

// ----------------------------------------------------------------------
// SyntaxTreeBase
// ----------------------------------------------------------------------

class ICE_API SyntaxTreeBase : public GrammerBase
{
public:

    virtual void destroy();
    UnitPtr unit();
    virtual void visit(ParserVisitor*);

protected:

    SyntaxTreeBase(const UnitPtr&);

    UnitPtr _unit;
};

// ----------------------------------------------------------------------
// Type
// ----------------------------------------------------------------------

class ICE_API Type : virtual public SyntaxTreeBase
{
public:

protected:

    Type(const UnitPtr&);
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

    Builtin(const UnitPtr&, Kind);
    friend class ICE_API Unit;

    Kind _kind;
};

// ----------------------------------------------------------------------
// Contained
// ----------------------------------------------------------------------

class ICE_API Contained : virtual public SyntaxTreeBase
{
public:

    ContainerPtr container();
    std::string name();
    std::string scoped();
    std::string scope();
    std::string comment();

    enum ContainedType
    {
	ContainedTypeSequence,
	ContainedTypeDictionary,
	ContainedTypeEnum,
	ContainedTypeEnumerator,
	ContainedTypeNative,
	ContainedTypeModule,
	ContainedTypeClass,
	ContainedTypeStruct,
	ContainedTypeOperation,
	ContainedTypeDataMember
    };
    virtual ContainedType containedType() = 0;

protected:

    Contained(const ContainerPtr&, const std::string&);
    friend class ICE_API Container;

    ContainerPtr _container;
    std::string _name;
    std::string _scoped;
    std::string _comment;
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
    ModulePtr createModule(const std::string&);
    ClassDefPtr createClassDef(const std::string&, bool, bool, const ClassList&);
    ClassDeclPtr createClassDecl(const std::string&, bool, bool);
    StructPtr createStruct(const std::string&);
    SequencePtr createSequence(const std::string&, const TypePtr&);
    DictionaryPtr createDictionary(const std::string&, const TypePtr&, const TypePtr&);
    EnumPtr createEnum(const std::string&, const StringList&);
    EnumeratorPtr createEnumerator(const std::string&);
    NativePtr createNative(const std::string&);
    TypeList lookupType(const std::string&, bool = true);
    TypeList lookupTypeNoBuiltin(const std::string&, bool = true);
    ContainedList lookupContained(const std::string&, bool = true);
    ModuleList modules();
    ClassList classes();
    StructList structs();
    SequenceList sequences();
    DictionaryList dictionaries();
    EnumList enums();
    NativeList natives();
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

    Container(const UnitPtr&);

    bool checkInterfaceAndLocal(const std::string&, bool, bool, bool, bool, bool);

    int _includeLevel;
    ContainedList _contents;
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

    Module(const ContainerPtr&, const std::string&);
    friend class ICE_API Container;
};

// ----------------------------------------------------------------------
// Constructed
// ----------------------------------------------------------------------

class ICE_API Constructed : virtual public Type, virtual public Contained
{
public:

protected:

    Constructed(const ContainerPtr&, const std::string&);
};

// ----------------------------------------------------------------------
// ClassDecl
// ----------------------------------------------------------------------

class ICE_API ClassDecl : virtual public Constructed
{
public:

    ClassDefPtr definition();
    bool isLocal();
    bool isInterface();
    virtual ContainedType containedType();
    virtual void visit(ParserVisitor*);

protected:

    ClassDecl(const ContainerPtr&, const std::string&, bool, bool);
    friend class ICE_API Container;
    friend class ICE_API ClassDef;

    ClassDefPtr _definition;
    bool _local;
    bool _interface;
};

// ----------------------------------------------------------------------
// ClassDef
// ----------------------------------------------------------------------

class ICE_API ClassDef : virtual public Container, virtual public Contained
{
public:

    OperationPtr createOperation(const std::string&, const TypePtr&, const TypeStringList&, const TypeStringList&,
				 const TypeList&, bool);
    DataMemberPtr createDataMember(const std::string&, const TypePtr&);
    ClassList bases();
    ClassList allBases();
    OperationList operations();
    OperationList allOperations();
    DataMemberList dataMembers();
    bool isAbstract();
    bool isLocal();
    bool isInterface();
    virtual ContainedType containedType();
    virtual void visit(ParserVisitor*);

protected:

    ClassDef(const ContainerPtr&, const std::string&, bool, bool, const ClassList&);
    friend class ICE_API Container;

    bool _local;
    bool _interface;
    ClassList _bases;
};

// ----------------------------------------------------------------------
// Proxy
// ----------------------------------------------------------------------

class ICE_API Proxy : virtual public Type
{
public:

    ClassDeclPtr _class();

    Proxy(const ClassDeclPtr&);

protected:

    ClassDeclPtr __class;
};

// ----------------------------------------------------------------------
// Struct
// ----------------------------------------------------------------------

class ICE_API Struct : virtual public Container, virtual public Constructed
{
public:

    DataMemberPtr createDataMember(const std::string&, const TypePtr&);
    DataMemberList dataMembers();
    virtual ContainedType containedType();
    virtual void visit(ParserVisitor*);

protected:

    Struct(const ContainerPtr&, const std::string&);
    friend class ICE_API Container;
};

// ----------------------------------------------------------------------
// Operation
// ----------------------------------------------------------------------

class ICE_API Operation : virtual public Contained
{
public:

    TypePtr returnType();
    TypeStringList inputParameters();
    TypeStringList outputParameters();
    TypeList throws();
    bool nonmutating();
    virtual ContainedType containedType();
    virtual void visit(ParserVisitor*);

protected:

    Operation(const ContainerPtr&, const std::string&, const TypePtr&, const TypeStringList&, const TypeStringList&,
	      const TypeList&, bool);
    friend class ICE_API ClassDef;

    TypePtr _returnType;
    TypeStringList _inParams;
    TypeStringList _outParams;
    TypeList _throws;
    bool _nonmutating;
};

// ----------------------------------------------------------------------
// DataMember
// ----------------------------------------------------------------------

class ICE_API DataMember : virtual public Contained
{
public:

    TypePtr type();
    virtual ContainedType containedType();
    virtual void visit(ParserVisitor*);

protected:
    
    DataMember(const ContainerPtr&, const std::string&, const TypePtr&);
    friend class ICE_API ClassDef;
    friend class ICE_API Struct;

    TypePtr _type;
};

// ----------------------------------------------------------------------
// Sequence
// ----------------------------------------------------------------------

class ICE_API Sequence : virtual public Constructed
{
public:

    TypePtr type();
    virtual ContainedType containedType();
    virtual void visit(ParserVisitor*);

protected:

    Sequence(const ContainerPtr&, const std::string&, const TypePtr&);
    friend class ICE_API Container;

    TypePtr _type;
};

// ----------------------------------------------------------------------
// Dictionary
// ----------------------------------------------------------------------

class ICE_API Dictionary : virtual public Constructed
{
public:

    TypePtr keyType();
    TypePtr valueType();
    virtual ContainedType containedType();
    virtual void visit(ParserVisitor*);

protected:

    Dictionary(const ContainerPtr&, const std::string&, const TypePtr&, const TypePtr&);
    friend class ICE_API Container;

    TypePtr _keyType;
    TypePtr _valueType;
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

    Enum(const ContainerPtr&, const std::string&, const StringList&);
    friend class ICE_API Container;
    
    StringList _enumerators;
};

// ----------------------------------------------------------------------
// Enumerator
// ----------------------------------------------------------------------

class ICE_API Enumerator : virtual public Contained
{
public:

    virtual ContainedType containedType();

protected:

    Enumerator(const ContainerPtr&, const std::string&);
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

    Native(const ContainerPtr&, const std::string&);
    friend class Container;
};

// ----------------------------------------------------------------------
// Unit
// ----------------------------------------------------------------------

class ICE_API Unit : virtual public Container
{
public:

    static UnitPtr createUnit(bool, bool);

    bool ignRedefs();

    void setComment(const std::string&);
    std::string currentComment();

    void nextLine();
    void scanPosition(const char*);
    int currentIncludeLevel();

    void error(const char*);
    void error(const std::string&);

    void warning(const char*);
    void warning(const std::string&);

    ContainerPtr currentContainer();
    void pushContainer(const ContainerPtr&);
    void popContainer();

    void addContent(const ContainedPtr&);
    void removeContent(const ContainedPtr&);
    ContainedList findContents(const std::string&);

    StringList includeFiles();

    int parse(FILE*, bool);

    virtual void destroy();
    virtual void visit(ParserVisitor*);

    BuiltinPtr builtin(Builtin::Kind);

private:

    Unit(bool, bool);

    bool _ignRedefs;
    bool _all;
    int _errors;
    std::string _currentComment;
    int _currentLine;
    int _currentIncludeLevel;
    std::string _currentFile;
    std::string _topLevelFile;
    StringList _includeFiles;
    std::stack<ContainerPtr> _containerStack;
    std::map<Builtin::Kind, BuiltinPtr> _builtins;
    std::map<std::string, ContainedList > _contentMap;
};

extern Unit* unit; // The current parser for bison/flex

}

#endif
