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

#include <IceUtil/Shared.h>
#include <IceUtil/Handle.h>
#include <string>
#include <vector>
#include <list>
#include <stack>
#include <map>
#include <set>

#ifdef WIN32
#   ifdef SLICE_API_EXPORTS
#       define SLICE_API __declspec(dllexport)
#   else
#       define SLICE_API __declspec(dllimport)
#   endif
#else
#   define SLICE_API /**/
#endif

namespace Slice
{

class GrammarBase;
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
class Exception;
class Struct;
class Operation;
class DataMember;
class Sequence;
class Dictionary;
class Enum;
class Enumerator;
class Unit;

typedef ::IceUtil::Handle<GrammarBase> GrammarBasePtr;
typedef ::IceUtil::Handle<SyntaxTreeBase> SyntaxTreeBasePtr;
typedef ::IceUtil::Handle<Type> TypePtr;
typedef ::IceUtil::Handle<Builtin> BuiltinPtr;
typedef ::IceUtil::Handle<Contained> ContainedPtr;
typedef ::IceUtil::Handle<Container> ContainerPtr;
typedef ::IceUtil::Handle<Module> ModulePtr;
typedef ::IceUtil::Handle<Constructed> ConstructedPtr;
typedef ::IceUtil::Handle<ClassDecl> ClassDeclPtr;
typedef ::IceUtil::Handle<ClassDef> ClassDefPtr;
typedef ::IceUtil::Handle<Proxy> ProxyPtr;
typedef ::IceUtil::Handle<Exception> ExceptionPtr;
typedef ::IceUtil::Handle<Struct> StructPtr;
typedef ::IceUtil::Handle<Operation> OperationPtr;
typedef ::IceUtil::Handle<DataMember> DataMemberPtr;
typedef ::IceUtil::Handle<Sequence> SequencePtr;
typedef ::IceUtil::Handle<Dictionary> DictionaryPtr;
typedef ::IceUtil::Handle<Enum> EnumPtr;
typedef ::IceUtil::Handle<Enumerator> EnumeratorPtr;
typedef ::IceUtil::Handle<Unit> UnitPtr;

}

//
// Stuff for flex and bison
//

#define YYSTYPE Slice::GrammarBasePtr
#define YY_DECL int yylex(YYSTYPE* yylvalp)
YY_DECL;
int yyparse();

//
// I must set the initial stack depth to the maximum stack depth to
// disable bison stack resizing. The bison stack resizing routines use
// simple malloc/alloc/memcpy calls, which do not work for the
// YYSTYPE, since YYSTYPE is a C++ type, with constructor, destructor,
// assignment operator, etc.
//
#define YYMAXDEPTH  20000 // 20000 should suffice. Bison default is 10000 as maximum.
#define YYINITDEPTH YYMAXDEPTH // Initial depth is set to max depth, for the reasons described above.

//
// Newer bison versions allow to disable stack resizing by defining
// yyoverflow.
//
#define yyoverflow(a, b, c, d, e, f) yyerror(a)

namespace Slice
{

typedef std::list<TypePtr> TypeList;
typedef std::list<ExceptionPtr> ExceptionList;
typedef std::list<std::string> StringList;
typedef std::pair<TypePtr, std::string> TypeString;
typedef std::list<TypeString> TypeStringList;
typedef std::list<ContainedPtr> ContainedList;
typedef std::list<ModulePtr> ModuleList;
typedef std::list<ConstructedPtr> ConstructedList;
typedef std::list<ClassDefPtr> ClassList;
typedef std::list<ExceptionPtr> ExceptionList;
typedef std::list<StructPtr> StructList;
typedef std::list<SequencePtr> SequenceList;
typedef std::list<DictionaryPtr> DictionaryList;
typedef std::list<EnumPtr> EnumList;
typedef std::list<OperationPtr> OperationList;
typedef std::list<DataMemberPtr> DataMemberList;
typedef std::list<EnumeratorPtr> EnumeratorList;

// ----------------------------------------------------------------------
// ParserVisitor
// ----------------------------------------------------------------------

class SLICE_API ParserVisitor
{
public:

    virtual ~ParserVisitor() { }
    virtual bool visitUnitStart(const UnitPtr&) { return true; }
    virtual void visitUnitEnd(const UnitPtr&) { }
    virtual bool visitModuleStart(const ModulePtr&) { return true; }
    virtual void visitModuleEnd(const ModulePtr&) { }
    virtual void visitClassDecl(const ClassDeclPtr&) { }
    virtual bool visitClassDefStart(const ClassDefPtr&) { return true; }
    virtual void visitClassDefEnd(const ClassDefPtr&) { }
    virtual bool visitExceptionStart(const ExceptionPtr&) { return true; }
    virtual void visitExceptionEnd(const ExceptionPtr&) { }
    virtual bool visitStructStart(const StructPtr&) { return true; }
    virtual void visitStructEnd(const StructPtr&) { }
    virtual void visitOperation(const OperationPtr&) { }
    virtual void visitDataMember(const DataMemberPtr&) { }
    virtual void visitSequence(const SequencePtr&) { }
    virtual void visitDictionary(const DictionaryPtr&) { }
    virtual void visitEnum(const EnumPtr&) { }
};

// ----------------------------------------------------------------------
// GrammarBase
// ----------------------------------------------------------------------

class SLICE_API GrammarBase : public ::IceUtil::SimpleShared
{
};

// ----------------------------------------------------------------------
// SyntaxTreeBase
// ----------------------------------------------------------------------

class SLICE_API SyntaxTreeBase : public GrammarBase
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

class SLICE_API Type : virtual public SyntaxTreeBase
{
public:

protected:

    Type(const UnitPtr&);
};

// ----------------------------------------------------------------------
// Builtin
// ----------------------------------------------------------------------

class SLICE_API Builtin : virtual public Type
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
	KindObject,
	KindObjectProxy,
	KindLocalObject
    };
    Kind kind();

protected:

    Builtin(const UnitPtr&, Kind);
    friend class SLICE_API Unit;

    Kind _kind;
};

// ----------------------------------------------------------------------
// Contained
// ----------------------------------------------------------------------

class SLICE_API Contained : virtual public SyntaxTreeBase
{
public:

    ContainerPtr container();
    std::string name();
    std::string scoped();
    std::string scope();
    std::string file();
    std::string comment();

    std::list<std::string> getMetaData();
    void setMetaData(const std::list<std::string>&);

    enum ContainedType
    {
	ContainedTypeSequence,
	ContainedTypeDictionary,
	ContainedTypeEnum,
	ContainedTypeEnumerator,
	ContainedTypeModule,
	ContainedTypeClass,
	ContainedTypeException,
	ContainedTypeStruct,
	ContainedTypeOperation,
	ContainedTypeDataMember
    };
    virtual ContainedType containedType() = 0;

    virtual bool uses(const ContainedPtr&) = 0;

    bool operator<(const Contained&) const;
    bool operator==(const Contained&) const;

protected:

    Contained(const ContainerPtr&, const std::string&);
    friend class SLICE_API Container;

    ContainerPtr _container;
    std::string _name;
    std::string _scoped;
    std::string _file;
    std::string _comment;
    std::list<std::string> _metaData;
};

// ----------------------------------------------------------------------
// Container
// ----------------------------------------------------------------------

class SLICE_API Container : virtual public SyntaxTreeBase
{
public:

    virtual void destroy();
    ModulePtr createModule(const std::string&);
    ClassDefPtr createClassDef(const std::string&, bool, const ClassList&, bool);
    ClassDeclPtr createClassDecl(const std::string&, bool, bool);
    ExceptionPtr createException(const std::string&, const ExceptionPtr&, bool);
    StructPtr createStruct(const std::string&, bool);
    SequencePtr createSequence(const std::string&, const TypePtr&, bool);
    DictionaryPtr createDictionary(const std::string&, const TypePtr&, const TypePtr&, bool);
    EnumPtr createEnum(const std::string&, bool);
    EnumeratorPtr createEnumerator(const std::string&);
    TypeList lookupType(const std::string&, bool = true);
    TypeList lookupTypeNoBuiltin(const std::string&, bool = true);
    ContainedList lookupContained(const std::string&, bool = true);
    ExceptionPtr lookupException(const std::string&, bool = true);
    ModuleList modules();
    ClassList classes();
    ExceptionList exceptions();
    StructList structs();
    SequenceList sequences();
    DictionaryList dictionaries();
    EnumList enums();
    int includeLevel();
    void updateIncludeLevel();
    bool hasProxies();
    bool hasClassDecls();
    bool hasClassDefs();
    bool hasOtherConstructedOrExceptions(); // Exceptions or constructed types other than classes.
    std::string thisScope();
    void mergeModules();
    void sort();
    void sortContents();
    virtual void visit(ParserVisitor*);
    void containerRecDependencies(std::set<ConstructedPtr>&); // Internal operation, don't use directly.

protected:

    Container(const UnitPtr&);

    bool checkInterfaceAndLocal(const std::string&, bool, bool, bool, bool, bool);

    int _includeLevel;
    ContainedList _contents;
};

// ----------------------------------------------------------------------
// Module
// ----------------------------------------------------------------------

class SLICE_API Module : virtual public Container, virtual public Contained
{
public:

    virtual ContainedType containedType();
    virtual bool uses(const ContainedPtr&);
    virtual void visit(ParserVisitor*);

protected:

    Module(const ContainerPtr&, const std::string&);
    friend class SLICE_API Container;
};

// ----------------------------------------------------------------------
// Constructed
// ----------------------------------------------------------------------

class SLICE_API Constructed : virtual public Type, virtual public Contained
{
public:

    bool isLocal();
    ConstructedList dependencies();
    virtual void recDependencies(std::set<ConstructedPtr>&) = 0; // Internal operation, don't use directly.

protected:

    Constructed(const ContainerPtr&, const std::string&, bool);

    bool _local;
};

// ----------------------------------------------------------------------
// ClassDecl
// ----------------------------------------------------------------------

class SLICE_API ClassDecl : virtual public Constructed
{
public:

    virtual void destroy();
    ClassDefPtr definition();
    bool isInterface();
    virtual ContainedType containedType();
    virtual bool uses(const ContainedPtr&);
    virtual void visit(ParserVisitor*);
    virtual void recDependencies(std::set<ConstructedPtr>&); // Internal operation, don't use directly.

protected:

    ClassDecl(const ContainerPtr&, const std::string&, bool, bool);
    friend class SLICE_API Container;
    friend class SLICE_API ClassDef;

    ClassDefPtr _definition;
    bool _interface;
};

// ----------------------------------------------------------------------
// ClassDef
// ----------------------------------------------------------------------

//
// Note: For the purpose of this parser, a class definition is not
// considered to be a type, but a class declaration is. And each class
// definition has at least one class declaration (but not vice versa),
// so if you need the class as a "constructed type", use the
// declaration() operation to navigate to the class declaration.
//
class SLICE_API ClassDef : virtual public Container, virtual public Contained
{
public:

    virtual void destroy();
    OperationPtr createOperation(const std::string&, const TypePtr&, const TypeStringList&, const TypeStringList&,
				 const ExceptionList&);
    DataMemberPtr createDataMember(const std::string&, const TypePtr&);
    ClassDeclPtr declaration();
    ClassList bases();
    ClassList allBases();
    OperationList operations();
    OperationList allOperations();
    DataMemberList dataMembers();
    bool isAbstract();
    bool isInterface();
    bool isLocal();
    bool hasDataMembers();
    virtual ContainedType containedType();
    virtual bool uses(const ContainedPtr&);
    virtual void visit(ParserVisitor*);

protected:

    ClassDef(const ContainerPtr&, const std::string&, bool, const ClassList&, bool);
    friend class SLICE_API Container;

    ClassDeclPtr _declaration;
    bool _interface;
    bool _hasDataMembers;
    ClassList _bases;
    bool _local;
};

// ----------------------------------------------------------------------
// Proxy
// ----------------------------------------------------------------------

class SLICE_API Proxy : virtual public Type
{
public:

    ClassDeclPtr _class();

    Proxy(const ClassDeclPtr&);

protected:

    ClassDeclPtr __class;
};

// ----------------------------------------------------------------------
// Exception
// ----------------------------------------------------------------------

// No inheritance from Constructed, as this is not a Type
class SLICE_API Exception : virtual public Container, virtual public Contained
{
public:

    virtual void destroy();
    DataMemberPtr createDataMember(const std::string&, const TypePtr&);
    DataMemberList dataMembers();
    ExceptionPtr base();
    ExceptionList allBases();
    bool isLocal();
    virtual ContainedType containedType();
    virtual bool uses(const ContainedPtr&);
    virtual void visit(ParserVisitor*);

protected:

    Exception(const ContainerPtr&, const std::string&, const ExceptionPtr&, bool);
    friend class SLICE_API Container;

    ExceptionPtr _base;
    bool _local;
};

// ----------------------------------------------------------------------
// Struct
// ----------------------------------------------------------------------

class SLICE_API Struct : virtual public Container, virtual public Constructed
{
public:

    DataMemberPtr createDataMember(const std::string&, const TypePtr&);
    DataMemberList dataMembers();
    virtual ContainedType containedType();
    virtual bool uses(const ContainedPtr&);
    virtual void visit(ParserVisitor*);
    virtual void recDependencies(std::set<ConstructedPtr>&); // Internal operation, don't use directly.

protected:

    Struct(const ContainerPtr&, const std::string&, bool);
    friend class SLICE_API Container;
};

// ----------------------------------------------------------------------
// Sequence
// ----------------------------------------------------------------------

class SLICE_API Sequence : virtual public Constructed
{
public:

    TypePtr type();
    virtual ContainedType containedType();
    virtual bool uses(const ContainedPtr&);
    virtual void visit(ParserVisitor*);
    virtual void recDependencies(std::set<ConstructedPtr>&); // Internal operation, don't use directly.

protected:

    Sequence(const ContainerPtr&, const std::string&, const TypePtr&, bool);
    friend class SLICE_API Container;

    TypePtr _type;
};

// ----------------------------------------------------------------------
// Dictionary
// ----------------------------------------------------------------------

class SLICE_API Dictionary : virtual public Constructed
{
public:

    TypePtr keyType();
    TypePtr valueType();
    virtual ContainedType containedType();
    virtual bool uses(const ContainedPtr&);
    virtual void visit(ParserVisitor*);
    virtual void recDependencies(std::set<ConstructedPtr>&); // Internal operation, don't use directly.

protected:

    Dictionary(const ContainerPtr&, const std::string&, const TypePtr&, const TypePtr&, bool);
    friend class SLICE_API Container;

    TypePtr _keyType;
    TypePtr _valueType;
};

// ----------------------------------------------------------------------
// Enum
// ----------------------------------------------------------------------

class SLICE_API Enum : virtual public Constructed
{
public:

    EnumeratorList getEnumerators();
    void setEnumerators(const EnumeratorList&);
    virtual ContainedType containedType();
    virtual bool uses(const ContainedPtr&);
    virtual void visit(ParserVisitor*);
    virtual void recDependencies(std::set<ConstructedPtr>&); // Internal operation, don't use directly.

protected:

    Enum(const ContainerPtr&, const std::string&, bool);
    friend class SLICE_API Container;

    EnumeratorList _enumerators;
};

// ----------------------------------------------------------------------
// Enumerator
// ----------------------------------------------------------------------

class SLICE_API Enumerator : virtual public Contained
{
public:

    virtual bool uses(const ContainedPtr&);
    virtual ContainedType containedType();

protected:

    Enumerator(const ContainerPtr&, const std::string&);
    friend class SLICE_API Container;
};

// ----------------------------------------------------------------------
// Operation
// ----------------------------------------------------------------------

class SLICE_API Operation : virtual public Contained
{
public:

    TypePtr returnType();
    TypeStringList inputParameters();
    TypeStringList outputParameters();
    ExceptionList throws();
    virtual ContainedType containedType();
    virtual bool uses(const ContainedPtr&);
    virtual void visit(ParserVisitor*);

protected:

    Operation(const ContainerPtr&, const std::string&, const TypePtr&, const TypeStringList&, const TypeStringList&,
	      const ExceptionList&);
    friend class SLICE_API ClassDef;

    TypePtr _returnType;
    TypeStringList _inParams;
    TypeStringList _outParams;
    ExceptionList _throws;
};

// ----------------------------------------------------------------------
// DataMember
// ----------------------------------------------------------------------

class SLICE_API DataMember : virtual public Contained
{
public:

    TypePtr type();
    virtual ContainedType containedType();
    virtual bool uses(const ContainedPtr&);
    virtual void visit(ParserVisitor*);

protected:
    
    DataMember(const ContainerPtr&, const std::string&, const TypePtr&);
    friend class SLICE_API ClassDef;
    friend class SLICE_API Struct;
    friend class SLICE_API Exception;

    TypePtr _type;
};

// ----------------------------------------------------------------------
// Unit
// ----------------------------------------------------------------------

class SLICE_API Unit : virtual public Container
{
public:

    static UnitPtr createUnit(bool, bool);

    bool ignRedefs();

    void setComment(const std::string&);
    std::string currentComment();
    std::string currentFile();

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
    ClassList findDerivedClasses(const ClassDefPtr&);
    ExceptionList findDerivedExceptions(const ExceptionPtr&);
    ContainedList findUsedBy(const ContainedPtr&);

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

extern SLICE_API Unit* unit; // The current parser for bison/flex

}

#endif
