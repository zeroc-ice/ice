// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef SLICE_PARSER_H
#define SLICE_PARSER_H

#include <IceUtil/Shared.h>
#include <IceUtil/Handle.h>
#include <IceUtil/InputUtil.h>
#include <string>
#include <vector>
#include <list>
#include <stack>
#include <map>
#include <set>

#ifdef _WIN32
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

#if defined(_WIN32)
    typedef double Double;
    const IceUtil::Int64 Int32Max =  0x7fffffffi64;
    const IceUtil::Int64 Int32Min = -Int32Max - 1i64;
#elif(__linux__) && defined(i386)
    typedef double Double;
    const IceUtil::Int64 Int32Max =  0x7fffffffLL;
    const IceUtil::Int64 Int32Min = -Int32Max - 1LL;
#else
#   error "Unsupported operating system or platform!"
#endif

const IceUtil::Int64 Int16Max =  0x7fff;
const IceUtil::Int64 Int16Min = -Int16Max - 1;
const IceUtil::Int64 ByteMax = 0xff;
const IceUtil::Int64 ByteMin = 0x00;

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
class ParamDecl;
class DataMember;
class Sequence;
class Dictionary;
class Enum;
class Enumerator;
class ConstDef;
class Unit;
class CICompare;

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
typedef ::IceUtil::Handle<ParamDecl> ParamDeclPtr;
typedef ::IceUtil::Handle<DataMember> DataMemberPtr;
typedef ::IceUtil::Handle<Sequence> SequencePtr;
typedef ::IceUtil::Handle<Dictionary> DictionaryPtr;
typedef ::IceUtil::Handle<Enum> EnumPtr;
typedef ::IceUtil::Handle<Enumerator> EnumeratorPtr;
typedef ::IceUtil::Handle<ConstDef> ConstDefPtr;
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
typedef std::set<std::string> StringSet;
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
typedef std::list<ParamDeclPtr> ParamDeclList;
typedef std::list<EnumeratorPtr> EnumeratorList;
typedef std::pair<SyntaxTreeBasePtr, std::string> SyntaxTreeBaseString;

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
    virtual void visitParamDecl(const ParamDeclPtr&) { }
    virtual void visitDataMember(const DataMemberPtr&) { }
    virtual void visitSequence(const SequencePtr&) { }
    virtual void visitDictionary(const DictionaryPtr&) { }
    virtual void visitEnum(const EnumPtr&) { }
    virtual void visitConstDef(const ConstDefPtr&) { }
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
    UnitPtr unit() const;
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

    virtual bool isLocal() const = 0;

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

    virtual bool isLocal() const;

    Kind kind() const;
    std::string kindAsString() const;

    static const char* builtinTable[];

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

    ContainerPtr container() const;
    std::string name() const;
    std::string scoped() const;
    std::string scope() const;
    std::string file() const;
    std::string comment() const;

    int includeLevel() const;
    void updateIncludeLevel();

    std::list<std::string> getMetaData() const;
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
	ContainedTypeParamDecl,
	ContainedTypeDataMember,
	ContainedTypeConstant
    };
    virtual ContainedType containedType() const = 0;

    virtual bool uses(const ContainedPtr&) const = 0;
    virtual std::string kindOf() const = 0;

    bool operator<(const Contained&) const;
    bool operator==(const Contained&) const;
    bool operator!=(const Contained&) const;

protected:

    Contained(const ContainerPtr&, const std::string&);
    friend class SLICE_API Container;

    ContainerPtr _container;
    std::string _name;
    std::string _scoped;
    std::string _file;
    std::string _comment;
    int _includeLevel;
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
    ConstDefPtr createConstDef(const std::string, const TypePtr&, const SyntaxTreeBasePtr&, const std::string&);
    TypeList lookupType(const std::string&, bool = true);
    TypeList lookupTypeNoBuiltin(const std::string&, bool = true);
    ContainedList lookupContained(const std::string&, bool = true);
    ExceptionPtr lookupException(const std::string&, bool = true);
    ModuleList modules() const;
    ClassList classes() const;
    ExceptionList exceptions() const;
    StructList structs() const;
    SequenceList sequences() const;
    DictionaryList dictionaries() const;
    EnumList enums() const;
    bool hasNonLocalClassDecls() const;
    bool hasClassDecls() const;
    bool hasClassDefs() const;
    bool hasOtherConstructedOrExceptions() const; // Exceptions or constructed types other than classes.
    std::string thisScope() const;
    void mergeModules();
    void sort();
    void sortContents();
    virtual void visit(ParserVisitor*);
    void containerRecDependencies(std::set<ConstructedPtr>&); // Internal operation, don't use directly.

    bool checkIntroduced(const std::string&, ContainedPtr = 0);

protected:

    Container(const UnitPtr&);

    void checkPrefix(const std::string&) const;
    bool checkInterfaceAndLocal(const std::string&, bool, bool, bool, bool, bool);

    ContainedList _contents;
    std::map<std::string, ContainedPtr> _introducedMap;
};

// ----------------------------------------------------------------------
// Module
// ----------------------------------------------------------------------

class SLICE_API Module : virtual public Container, virtual public Contained
{
public:

    virtual ContainedType containedType() const;
    virtual bool uses(const ContainedPtr&) const;
    virtual std::string kindOf() const;
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

    bool isLocal() const;
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
    ClassDefPtr definition() const;
    bool isInterface() const;
    virtual ContainedType containedType() const;
    virtual bool uses(const ContainedPtr&) const;
    virtual void visit(ParserVisitor*);
    virtual std::string kindOf() const;
    virtual void recDependencies(std::set<ConstructedPtr>&); // Internal operation, don't use directly.

    static void checkBasesAreLegal(const std::string&, bool, const ClassList&, const UnitPtr&);

protected:

    ClassDecl(const ContainerPtr&, const std::string&, bool, bool);
    friend class SLICE_API Container;
    friend class SLICE_API ClassDef;

    ClassDefPtr _definition;
    bool _interface;

private:

    typedef std::list<ClassList> GraphPartitionList;
    typedef std::list<StringList> StringPartitionList;

    static bool isInList(const GraphPartitionList&, const ClassDefPtr);
    static void addPartition(GraphPartitionList&, GraphPartitionList::reverse_iterator, const ClassDefPtr);
    static StringPartitionList toStringPartitionList(const GraphPartitionList&);
    static void checkPairIntersections(const StringPartitionList&, const std::string&, const UnitPtr&);
};

// ----------------------------------------------------------------------
// Operation
// ----------------------------------------------------------------------

class SLICE_API Operation : virtual public Contained, virtual public Container
{
public:
    
    //
    // Note: The order of definitions here *must* match
    //       the order of definitions of ::Ice::OperationMode
    //       in Ice/Current.ice!
    //
    enum Mode { Normal, Nonmutating, Idempotent };

    TypePtr returnType() const;
    Mode mode() const;
    ParamDeclPtr createParamDecl(const std::string&, const TypePtr&, bool);
    ParamDeclList parameters() const;
    ExceptionList throws() const;
    void setExceptionList(const ExceptionList&);
    virtual ContainedType containedType() const;
    virtual bool uses(const ContainedPtr&) const;
    virtual std::string kindOf() const;
    virtual void visit(ParserVisitor*);

protected:

    Operation(const ContainerPtr&, const std::string&, const TypePtr&, Mode);
    friend class SLICE_API ClassDef;

    TypePtr _returnType;
    ExceptionList _throws;
    Mode _mode;
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
    OperationPtr createOperation(const std::string&, const TypePtr&, Operation::Mode = Operation::Normal);
    DataMemberPtr createDataMember(const std::string&, const TypePtr&);
    ClassDeclPtr declaration() const;
    ClassList bases() const;
    ClassList allBases() const;
    OperationList operations() const;
    OperationList allOperations() const;
    DataMemberList dataMembers() const;
    DataMemberList allDataMembers() const;
    bool isAbstract() const;
    bool isInterface() const;
    bool isLocal() const;
    bool hasDataMembers() const;
    virtual ContainedType containedType() const;
    virtual bool uses(const ContainedPtr&) const;
    virtual std::string kindOf() const;
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

    virtual bool isLocal() const;

    ClassDeclPtr _class() const;

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
    DataMemberList dataMembers() const;
    ExceptionPtr base() const;
    ExceptionList allBases() const;
    bool isLocal() const;
    virtual ContainedType containedType() const;
    virtual bool uses(const ContainedPtr&) const;
    virtual std::string kindOf() const;
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
    DataMemberList dataMembers() const;
    virtual ContainedType containedType() const;
    virtual bool uses(const ContainedPtr&) const;
    virtual std::string kindOf() const;
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

    TypePtr type() const;
    virtual ContainedType containedType() const;
    virtual bool uses(const ContainedPtr&) const;
    virtual std::string kindOf() const;
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

    TypePtr keyType() const;
    TypePtr valueType() const;
    virtual ContainedType containedType() const;
    virtual bool uses(const ContainedPtr&) const;
    virtual std::string kindOf() const;
    virtual void visit(ParserVisitor*);
    virtual void recDependencies(std::set<ConstructedPtr>&); // Internal operation, don't use directly.

    static bool legalKeyType(const TypePtr&);

protected:

    Dictionary(const ContainerPtr&, const std::string&, const TypePtr&, const TypePtr&, bool);
    friend class SLICE_API Container;

    TypePtr _keyType;
    TypePtr _valueType;

private:

    static bool legalSimpleKeyType(const TypePtr&);
};

// ----------------------------------------------------------------------
// Enum
// ----------------------------------------------------------------------

class SLICE_API Enum : virtual public Constructed
{
public:

    EnumeratorList getEnumerators();
    void setEnumerators(const EnumeratorList&);
    virtual ContainedType containedType() const;
    virtual bool uses(const ContainedPtr&) const;
    virtual std::string kindOf() const;
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

    virtual bool uses(const ContainedPtr&) const;
    virtual ContainedType containedType() const;
    virtual std::string kindOf() const;

protected:

    Enumerator(const ContainerPtr&, const std::string&);
    friend class SLICE_API Container;
};

// ----------------------------------------------------------------------
// ConstDef
// ----------------------------------------------------------------------

class SLICE_API ConstDef : virtual public Contained
{
public:

    TypePtr type() const;
    std::string value() const;
    virtual bool uses(const ContainedPtr&) const;
    virtual ContainedType containedType() const;
    virtual std::string kindOf() const;
    virtual void visit(ParserVisitor*);

    static bool isLegalType(const std::string&, const TypePtr&, const UnitPtr&);
    static bool typesAreCompatible(const std::string&, const TypePtr&,
	                           const SyntaxTreeBasePtr&, const std::string&, const UnitPtr&);
    static bool isInRange(const std::string&, const TypePtr&, const std::string&, const UnitPtr&);

protected:

    ConstDef(const ContainerPtr&, const std::string&, const TypePtr&, const std::string&);
    friend class SLICE_API Container;

    TypePtr _type;
    std::string _value;
};

// ----------------------------------------------------------------------
// ParamDecl
// ----------------------------------------------------------------------

class SLICE_API ParamDecl : virtual public Contained
{
public:

    TypePtr type() const;
    bool isOutParam() const;
    virtual ContainedType containedType() const;
    virtual bool uses(const ContainedPtr&) const;
    virtual std::string kindOf() const;
    virtual void visit(ParserVisitor*);

protected:

    ParamDecl(const ContainerPtr&, const std::string&, const TypePtr&, bool isOutParam);
    friend class SLICE_API Operation;

    TypePtr _type;
    bool _isOutParam;
};

// ----------------------------------------------------------------------
// DataMember
// ----------------------------------------------------------------------

class SLICE_API DataMember : virtual public Contained
{
public:

    TypePtr type() const;
    virtual ContainedType containedType() const;
    virtual bool uses(const ContainedPtr&) const;
    virtual std::string kindOf() const;
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

    static UnitPtr createUnit(bool, bool, bool);

    bool ignRedefs() const;

    bool allowIcePrefix() const;

    void setComment(const std::string&);
    std::string currentComment(); // Not const, as this function removes the current comment.
    std::string currentFile() const;
    int currentLine() const;

    void nextLine();
    void scanPosition(const char*);
    int currentIncludeLevel() const;

    void error(const char*); // Not const, because error count is increased.
    void error(const std::string&); // Ditto.

    void warning(const char*) const;
    void warning(const std::string&) const;

    ContainerPtr currentContainer() const;
    void pushContainer(const ContainerPtr&);
    void popContainer();

    void addContent(const ContainedPtr&);
    void removeContent(const ContainedPtr&);
    ContainedList findContents(const std::string&) const;
    ClassList findDerivedClasses(const ClassDefPtr&) const;
    ExceptionList findDerivedExceptions(const ExceptionPtr&) const;
    ContainedList findUsedBy(const ContainedPtr&) const;

    bool usesProxies() const;
    bool usesNonLocals() const;
    bool usesConsts() const;

    StringList includeFiles() const;

    int parse(FILE*, bool);

    virtual void destroy();
    virtual void visit(ParserVisitor*);

    BuiltinPtr builtin(Builtin::Kind); // Not const, as builtins are created on the fly. (Lazy initialization.)

private:

    Unit(bool, bool, bool);

    bool _ignRedefs;
    bool _all;
    bool _allowIcePrefix;
    int _errors;
    std::string _currentComment;
    int _currentLine;
    int _currentIncludeLevel;
    std::string _currentFile;
    std::string _topLevelFile;
    StringList _includeFiles;
    std::stack<ContainerPtr> _containerStack;
    std::map<Builtin::Kind, BuiltinPtr> _builtins;
    std::map<std::string, ContainedList> _contentMap;
};

extern SLICE_API Unit* unit;		// The current parser for bison/flex

// ----------------------------------------------------------------------
// CICompare -- function object to do case-insensitive string comparison.
// ----------------------------------------------------------------------

class CICompare : public std::binary_function<std::string, std::string, bool>
{
public:

    bool operator()(const std::string&, const std::string&) const;
};

}

#endif
