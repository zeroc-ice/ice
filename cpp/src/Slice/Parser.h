//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef SLICE_PARSER_H
#define SLICE_PARSER_H

#include <IceUtil/Shared.h>
#include <IceUtil/Handle.h>
#include <IceUtil/Exception.h>
#include <string>
#include <vector>
#include <list>
#include <stack>
#include <map>
#include <set>
#include <stdio.h>

namespace Slice
{

class CompilerException : public ::IceUtil::Exception
{
public:

    CompilerException(const char*, int, const std::string&);
#ifndef ICE_CPP11_COMPILER
    ~CompilerException() throw();
#endif
    virtual std::string ice_id() const;
    virtual void ice_print(std::ostream&) const;
#ifndef ICE_CPP11_MAPPING
    virtual CompilerException* ice_clone() const;
#endif
    virtual void ice_throw() const;

    std::string reason() const;

private:

    static const char* _name;
    const std::string _reason;
};

#if defined(_WIN32)

const IceUtil::Int64 Int32Max =  0x7fffffffi64;
const IceUtil::Int64 Int32Min = -Int32Max - 1i64;

#else

#   if defined(INT32_MIN) && defined(INT32_MAX)

const IceUtil::Int64 Int32Max =  INT32_MAX;
const IceUtil::Int64 Int32Min =  INT32_MIN;

#   else

const IceUtil::Int64 Int32Max =  0x7fffffffLL;
const IceUtil::Int64 Int32Min = -Int32Max - 1LL;

#   endif

#endif

const IceUtil::Int64 Int16Max =  0x7fff;
const IceUtil::Int64 Int16Min = -Int16Max - 1;
const IceUtil::Int64 ByteMax = 0xff;
const IceUtil::Int64 ByteMin = 0x00;

enum NodeType
{
    Dummy,
    Real
};

//
// Format preference for classes and exceptions.
//
enum FormatType
{
    DefaultFormat,    // No preference was specified.
    CompactFormat,    // Minimal format.
    SlicedFormat      // Full format.
};

enum WarningCategory
{
    All,
    Deprecated,
    InvalidMetaData
};

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
class Const;
class Unit;
class CICompare;
class DerivedToBaseCompare;
class ModulePartialCompare;

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
typedef ::IceUtil::Handle<Const> ConstPtr;
typedef ::IceUtil::Handle<Unit> UnitPtr;

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
typedef std::list<ConstPtr> ConstList;
typedef std::list<OperationPtr> OperationList;
typedef std::list<DataMemberPtr> DataMemberList;
typedef std::list<ParamDeclPtr> ParamDeclList;
typedef std::list<EnumeratorPtr> EnumeratorList;

struct ConstDef
{
    TypePtr type;
    SyntaxTreeBasePtr value;
    std::string valueAsString;
    std::string valueAsLiteral;
};

struct OptionalDef
{
    TypePtr type;
    std::string name;
    bool optional;
    int tag;
};

// ----------------------------------------------------------------------
// CICompare -- function object to do case-insensitive string comparison.
// ----------------------------------------------------------------------

class CICompare : public std::binary_function<std::string, std::string, bool>
{
public:

    bool operator()(const std::string&, const std::string&) const;
};

#if defined(__SUNPRO_CC)
bool cICompare(const std::string&, const std::string&);
#endif

// ----------------------------------------------------------------------
// DerivedToBaseCompare -- function object to do sort exceptions into
// most-derived to least-derived order.
// ----------------------------------------------------------------------

class DerivedToBaseCompare : public std::binary_function<std::string, std::string, bool>
{
public:

    bool operator()(const ExceptionPtr&, const ExceptionPtr&) const;
};

#if defined(__SUNPRO_CC)
bool derivedToBaseCompare(const ExceptionPtr&, const ExceptionPtr&);
#endif

// ----------------------------------------------------------------------
// ParserVisitor
// ----------------------------------------------------------------------

class ParserVisitor
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
    virtual void visitConst(const ConstPtr&) { }
};

// ----------------------------------------------------------------------
// DefinitionContext
// ----------------------------------------------------------------------

class DefinitionContext : public ::IceUtil::SimpleShared
{
public:

    DefinitionContext(int, const StringList&);

    std::string filename() const;
    int includeLevel() const;
    bool seenDefinition() const;

    void setFilename(const std::string&);
    void setSeenDefinition();

    bool hasMetaData() const;
    void setMetaData(const StringList&);
    std::string findMetaData(const std::string&) const;
    StringList getMetaData() const;

    //
    // Emit warning unless filtered out by [["suppress-warning"]]
    //
    void warning(WarningCategory, const std::string&, int, const std::string&) const;
    void warning(WarningCategory, const std::string&, const std::string&, const std::string&) const;

    void error(const std::string&, int, const std::string&) const;
    void error(const std::string&, const std::string&, const std::string&) const;

private:

    bool suppressWarning(WarningCategory) const;
    void initSuppressedWarnings();

    int _includeLevel;
    StringList _metaData;
    std::string _filename;
    bool _seenDefinition;
    std::set<WarningCategory> _suppressedWarnings;
};
typedef ::IceUtil::Handle<DefinitionContext> DefinitionContextPtr;

// ----------------------------------------------------------------------
// Comment
// ----------------------------------------------------------------------

class Comment : public ::IceUtil::SimpleShared
{
public:

    bool isDeprecated() const;
    StringList deprecated() const;

    StringList overview() const;  // Contains all introductory lines up to the first tag.
    StringList misc() const;      // Contains unrecognized tags.
    StringList seeAlso() const;   // Targets of @see tags.

    StringList returns() const;                           // Description of an operation's return value.
    std::map<std::string, StringList> parameters() const; // Parameter descriptions for an op. Key is parameter name.
    std::map<std::string, StringList> exceptions() const; // Exception descriptions for an op. Key is exception name.

private:

    Comment();

    bool _isDeprecated;
    StringList _deprecated;
    StringList _overview;
    StringList _misc;
    StringList _seeAlso;

    StringList _returns;
    std::map<std::string, StringList> _parameters;
    std::map<std::string, StringList> _exceptions;

    friend class Contained;
};
typedef ::IceUtil::Handle<Comment> CommentPtr;

// ----------------------------------------------------------------------
// GrammarBase
// ----------------------------------------------------------------------

class GrammarBase : public ::IceUtil::SimpleShared
{
};

// ----------------------------------------------------------------------
// SyntaxTreeBase
// ----------------------------------------------------------------------

class SyntaxTreeBase : public GrammarBase
{
public:

    virtual void destroy();
    UnitPtr unit() const;
    DefinitionContextPtr definitionContext() const; // May be nil
    virtual void visit(ParserVisitor*, bool);

protected:

    SyntaxTreeBase(const UnitPtr&, const DefinitionContextPtr& = 0);

    UnitPtr _unit;
    DefinitionContextPtr _definitionContext;
};

// ----------------------------------------------------------------------
// Type
// ----------------------------------------------------------------------

class Type : public virtual SyntaxTreeBase
{
public:

    virtual std::string typeId() const = 0;
    virtual bool usesClasses() const = 0;
    virtual size_t minWireSize() const = 0;
    virtual bool isVariableLength() const = 0;

protected:

    Type(const UnitPtr&);
};

// ----------------------------------------------------------------------
// Builtin
// ----------------------------------------------------------------------

class Builtin : public virtual Type
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
        KindValue
    };

    virtual std::string typeId() const;
    virtual bool usesClasses() const;
    virtual size_t minWireSize() const;
    virtual bool isVariableLength() const;

    Kind kind() const;
    std::string kindAsString() const;

    static const char* builtinTable[];

protected:

    Builtin(const UnitPtr&, Kind);
    friend class Unit;

    Kind _kind;
};

// ----------------------------------------------------------------------
// Contained
// ----------------------------------------------------------------------

class Contained : public virtual SyntaxTreeBase
{
public:

    ContainerPtr container() const;
    std::string name() const;
    std::string scoped() const;
    std::string scope() const;
    std::string flattenedScope() const;
    std::string file() const;
    std::string line() const;
    std::string comment() const;
    CommentPtr parseComment(bool) const;

    int includeLevel() const;
    void updateIncludeLevel();

    bool hasMetaData(const std::string&) const;
    bool findMetaData(const std::string&, std::string&) const;
    std::list<std::string> getMetaData() const;
    void setMetaData(const std::list<std::string>&);
    void addMetaData(const std::string&); // TODO: remove this method once "cs:" and "vb:" are hard errors.

    static FormatType parseFormatMetaData(const std::list<std::string>&);

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

protected:

    Contained(const ContainerPtr&, const std::string&);
    friend class Container;

    ContainerPtr _container;
    std::string _name;
    std::string _scoped;
    std::string _file;
    std::string _line;
    std::string _comment;
    int _includeLevel;
    std::list<std::string> _metaData;
};

// ----------------------------------------------------------------------
// Container
// ----------------------------------------------------------------------

class Container : public virtual SyntaxTreeBase
{
public:

    virtual void destroy();
    ModulePtr createModule(const std::string&);
    ClassDefPtr createClassDef(const std::string&, int, bool, const ClassList&);
    ClassDeclPtr createClassDecl(const std::string&, bool);
    ExceptionPtr createException(const std::string&, const ExceptionPtr&, NodeType = Real);
    StructPtr createStruct(const std::string&, NodeType = Real);
    SequencePtr createSequence(const std::string&, const TypePtr&, const StringList&, NodeType = Real);
    DictionaryPtr createDictionary(const std::string&, const TypePtr&, const StringList&, const TypePtr&,
                                   const StringList&, NodeType = Real);
    EnumPtr createEnum(const std::string&, NodeType = Real);
    EnumeratorPtr createEnumerator(const std::string&);
    EnumeratorPtr createEnumerator(const std::string&, int);
    ConstPtr createConst(const std::string, const TypePtr&, const StringList&, const SyntaxTreeBasePtr&,
                         const std::string&, const std::string&, NodeType = Real);
    TypeList lookupType(const std::string&, bool = true);
    TypeList lookupTypeNoBuiltin(const std::string&, bool = true, bool = false);
    ContainedList lookupContained(const std::string&, bool = true);
    ExceptionPtr lookupException(const std::string&, bool = true);
    UnitPtr unit() const;
    ModuleList modules() const;
    ClassList classes() const;
    ExceptionList exceptions() const;
    StructList structs() const;
    SequenceList sequences() const;
    DictionaryList dictionaries() const;
    EnumList enums() const;
    EnumeratorList enumerators() const;
    EnumeratorList enumerators(const std::string&) const;
    ConstList consts() const;
    ContainedList contents() const;
    bool hasNonLocalClassDecls() const;
    bool hasNonLocalClassDefs() const;
    bool hasNonLocalSequences() const;
    bool hasNonLocalExceptions() const;
    bool hasStructs() const;
    bool hasExceptions() const;
    bool hasDictionaries() const;
    bool hasOnlyDictionaries(DictionaryList&) const;
    bool hasClassDecls() const;
    bool hasClassDefs() const;
    bool hasNonLocalInterfaceDefs() const;
    bool hasValueDefs() const;
    bool hasOnlyClassDecls() const;
    bool hasOperations() const; // interfaces or classes with operations
    bool hasNonLocalAbstractClassDefs() const; // interfaces or abstract classes
    bool hasNonLocalDataOnlyClasses() const;
    bool hasOtherConstructedOrExceptions() const; // Exceptions or constructed types other than classes.
    bool hasContentsWithMetaData(const std::string&) const;
    bool hasAsyncOps() const;
    bool hasNonLocalContained(Contained::ContainedType) const;
    std::string thisScope() const;
    void mergeModules();
    void sort();
    void sortContents(bool);
    virtual void visit(ParserVisitor*, bool);
    void containerRecDependencies(std::set<ConstructedPtr>&); // Internal operation, don't use directly.

    bool checkIntroduced(const std::string&, ContainedPtr = 0);
    bool nameIsLegal(const std::string&, const char *);
    bool checkForGlobalDef(const std::string&, const char *);

protected:

    Container(const UnitPtr&);

    bool checkInterfaceAndLocal(const std::string&, bool, bool, bool);
    bool checkGlobalMetaData(const StringList&, const StringList&);
    bool validateConstant(const std::string&, const TypePtr&, SyntaxTreeBasePtr&, const std::string&, bool);
    EnumeratorPtr validateEnumerator(const std::string&);

    ContainedList _contents;
    std::map<std::string, ContainedPtr, CICompare> _introducedMap;
};

// ----------------------------------------------------------------------
// Module
// ----------------------------------------------------------------------

class Module : public virtual Container, public virtual Contained
{
public:

    virtual ContainedType containedType() const;
    virtual bool uses(const ContainedPtr&) const;
    virtual std::string kindOf() const;
    virtual void visit(ParserVisitor*, bool);

protected:

    Module(const ContainerPtr&, const std::string&);
    friend class Container;
};

// ----------------------------------------------------------------------
// Constructed
// ----------------------------------------------------------------------

class Constructed : public virtual Type, public virtual Contained
{
public:

    virtual std::string typeId() const;
    virtual bool isVariableLength() const = 0;
    ConstructedList dependencies();
    virtual void recDependencies(std::set<ConstructedPtr>&) = 0; // Internal operation, don't use directly.

protected:

    Constructed(const ContainerPtr&, const std::string&);
};

// ----------------------------------------------------------------------
// ClassDecl
// ----------------------------------------------------------------------

class ClassDecl : public virtual Constructed
{
public:

    virtual void destroy();
    ClassDefPtr definition() const;
    bool isInterface() const;
    virtual ContainedType containedType() const;
    virtual bool uses(const ContainedPtr&) const;
    virtual bool usesClasses() const;
    virtual size_t minWireSize() const;
    virtual bool isVariableLength() const;
    virtual void visit(ParserVisitor*, bool);
    virtual std::string kindOf() const;
    virtual void recDependencies(std::set<ConstructedPtr>&); // Internal operation, don't use directly.

    static void checkBasesAreLegal(const std::string&, const ClassList&, const UnitPtr&);

protected:

    ClassDecl(const ContainerPtr&, const std::string&, bool);
    friend class Container;
    friend class ClassDef;

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

class Operation : public virtual Contained, public virtual Container
{
public:

    //
    // Note: The order of definitions here *must* match the order of
    // definitions of ::Ice::OperationMode in Ice/Current.h
    //
    enum Mode
    {
        Normal,
        Nonmutating,
        Idempotent
    };

    TypePtr returnType() const;
    bool returnIsOptional() const;
    int returnTag() const;
    Mode mode() const;
    Mode sendMode() const;
    bool hasMarshaledResult() const;
    ParamDeclPtr createParamDecl(const std::string&, const TypePtr&, bool, bool, int);
    ParamDeclList parameters() const;
    ParamDeclList inParameters() const;
    void inParameters(ParamDeclList&, ParamDeclList&) const;
    ParamDeclList outParameters() const;
    void outParameters(ParamDeclList&, ParamDeclList&) const;
    ExceptionList throws() const;
    void setExceptionList(const ExceptionList&);
    virtual ContainedType containedType() const;
    virtual bool uses(const ContainedPtr&) const;
    bool sendsClasses(bool) const;
    bool returnsClasses(bool) const;
    bool returnsData() const;
    bool returnsMultipleValues() const;
    bool sendsOptionals() const;
    int attributes() const;
    FormatType format() const;
    virtual std::string kindOf() const;
    virtual void visit(ParserVisitor*, bool);

protected:

    Operation(const ContainerPtr&, const std::string&, const TypePtr&, bool, int, Mode);
    friend class ClassDef;

    TypePtr _returnType;
    bool _returnIsOptional;
    int _returnTag;
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
class ClassDef : public virtual Container, public virtual Contained
{
public:

    virtual void destroy();
    OperationPtr createOperation(const std::string&, const TypePtr&, bool, int, Operation::Mode = Operation::Normal);
    DataMemberPtr createDataMember(const std::string&, const TypePtr&, bool, int, const SyntaxTreeBasePtr&,
                                   const std::string&, const std::string&);
    ClassDeclPtr declaration() const;
    ClassList bases() const;
    ClassList allBases() const;
    OperationList operations() const;
    OperationList allOperations() const;
    DataMemberList dataMembers() const;
    DataMemberList orderedOptionalDataMembers() const;
    DataMemberList allDataMembers() const;
    DataMemberList classDataMembers() const;
    DataMemberList allClassDataMembers() const;
    bool canBeCyclic() const;
    bool isAbstract() const;
    bool isInterface() const;
    bool isA(const std::string&) const;
    bool hasDataMembers() const;
    bool hasOperations() const;
    bool hasDefaultValues() const;
    bool inheritsMetaData(const std::string&) const;
    bool hasBaseDataMembers() const;
    virtual ContainedType containedType() const;
    virtual bool uses(const ContainedPtr&) const;
    virtual std::string kindOf() const;
    virtual void visit(ParserVisitor*, bool);
    int compactId() const;

protected:

    ClassDef(const ContainerPtr&, const std::string&, int, bool, const ClassList&);
    friend class Container;

    ClassDeclPtr _declaration;
    bool _interface;
    bool _hasDataMembers;
    bool _hasOperations;
    ClassList _bases;
    int _compactId;
};

// ----------------------------------------------------------------------
// Proxy
// ----------------------------------------------------------------------

class Proxy : public virtual Type
{
public:

    virtual std::string typeId() const;
    virtual bool usesClasses() const;
    virtual size_t minWireSize() const;
    virtual bool isVariableLength() const;

    ClassDeclPtr _class() const;

    Proxy(const ClassDeclPtr&);

protected:

    ClassDeclPtr _classDecl;
};

// ----------------------------------------------------------------------
// Exception
// ----------------------------------------------------------------------

// No inheritance from Constructed, as this is not a Type
class Exception : public virtual Container, public virtual Contained
{
public:

    virtual void destroy();
    DataMemberPtr createDataMember(const std::string&, const TypePtr&, bool, int, const SyntaxTreeBasePtr&,
                                   const std::string&, const std::string&);
    DataMemberList dataMembers() const;
    DataMemberList orderedOptionalDataMembers() const;
    DataMemberList allDataMembers() const;
    DataMemberList classDataMembers() const;
    DataMemberList allClassDataMembers() const;
    ExceptionPtr base() const;
    ExceptionList allBases() const;
    virtual bool isBaseOf(const ExceptionPtr&) const;
    virtual ContainedType containedType() const;
    virtual bool uses(const ContainedPtr&) const;
    bool usesClasses(bool) const;
    bool hasDefaultValues() const;
    bool inheritsMetaData(const std::string&) const;
    bool hasBaseDataMembers() const;
    virtual std::string kindOf() const;
    virtual void visit(ParserVisitor*, bool);

protected:

    Exception(const ContainerPtr&, const std::string&, const ExceptionPtr&);
    friend class Container;

    ExceptionPtr _base;
};

// ----------------------------------------------------------------------
// Struct
// ----------------------------------------------------------------------

class Struct : public virtual Container, public virtual Constructed
{
public:

    DataMemberPtr createDataMember(const std::string&, const TypePtr&, bool, int, const SyntaxTreeBasePtr&,
                                   const std::string&, const std::string&);
    DataMemberList dataMembers() const;
    DataMemberList classDataMembers() const;
    virtual ContainedType containedType() const;
    virtual bool uses(const ContainedPtr&) const;
    virtual bool usesClasses() const;
    virtual size_t minWireSize() const;
    virtual bool isVariableLength() const;
    bool hasDefaultValues() const;
    virtual std::string kindOf() const;
    virtual void visit(ParserVisitor*, bool);
    virtual void recDependencies(std::set<ConstructedPtr>&); // Internal operation, don't use directly.

protected:

    Struct(const ContainerPtr&, const std::string&);
    friend class Container;
};

// ----------------------------------------------------------------------
// Sequence
// ----------------------------------------------------------------------

class Sequence : public virtual Constructed
{
public:

    TypePtr type() const;
    StringList typeMetaData() const;
    virtual ContainedType containedType() const;
    virtual bool uses(const ContainedPtr&) const;
    virtual bool usesClasses() const;
    virtual size_t minWireSize() const;
    virtual bool isVariableLength() const;
    virtual std::string kindOf() const;
    virtual void visit(ParserVisitor*, bool);
    virtual void recDependencies(std::set<ConstructedPtr>&); // Internal operation, don't use directly.

protected:

    Sequence(const ContainerPtr&, const std::string&, const TypePtr&, const StringList&);
    friend class Container;

    TypePtr _type;
    StringList _typeMetaData;
};

// ----------------------------------------------------------------------
// Dictionary
// ----------------------------------------------------------------------

class Dictionary : public virtual Constructed
{
public:

    TypePtr keyType() const;
    TypePtr valueType() const;
    StringList keyMetaData() const;
    StringList valueMetaData() const;
    virtual ContainedType containedType() const;
    virtual bool uses(const ContainedPtr&) const;
    virtual bool usesClasses() const;
    virtual size_t minWireSize() const;
    virtual bool isVariableLength() const;
    virtual std::string kindOf() const;
    virtual void visit(ParserVisitor*, bool);
    virtual void recDependencies(std::set<ConstructedPtr>&); // Internal operation, don't use directly.

    static bool legalKeyType(const TypePtr&, bool&);

protected:

    Dictionary(const ContainerPtr&, const std::string&, const TypePtr&, const StringList&, const TypePtr&,
               const StringList&);
    friend class Container;

    TypePtr _keyType;
    TypePtr _valueType;
    StringList _keyMetaData;
    StringList _valueMetaData;
};

// ----------------------------------------------------------------------
// Enum
// ----------------------------------------------------------------------

class Enum : public virtual Container, public virtual Constructed
{
public:

    virtual void destroy();
    bool explicitValue() const;
    int minValue() const;
    int maxValue() const;
    virtual ContainedType containedType() const;
    virtual bool uses(const ContainedPtr&) const;
    virtual bool usesClasses() const;
    virtual size_t minWireSize() const;
    virtual bool isVariableLength() const;
    virtual std::string kindOf() const;
    virtual void visit(ParserVisitor*, bool);
    virtual void recDependencies(std::set<ConstructedPtr>&); // Internal operation, don't use directly.

protected:

    Enum(const ContainerPtr&, const std::string&);
    int newEnumerator(const EnumeratorPtr&);

    friend class Container;
    friend class Enumerator;

    bool _explicitValue;
    IceUtil::Int64 _minValue;
    IceUtil::Int64 _maxValue;
    int _lastValue;
};

// ----------------------------------------------------------------------
// Enumerator
// ----------------------------------------------------------------------

class Enumerator : public virtual Contained
{
public:

    EnumPtr type() const;
    virtual bool uses(const ContainedPtr&) const;
    virtual ContainedType containedType() const;
    virtual std::string kindOf() const;

    bool explicitValue() const;
    int value() const;

protected:

    Enumerator(const ContainerPtr&, const std::string&);
    Enumerator(const ContainerPtr&, const std::string&, int);
    friend class Container;

    bool _explicitValue;
    int _value;
};

// ----------------------------------------------------------------------
// Const
// ----------------------------------------------------------------------

class Const : public virtual Contained
{
public:

    TypePtr type() const;
    StringList typeMetaData() const;
    SyntaxTreeBasePtr valueType() const;
    std::string value() const;
    std::string literal() const;
    virtual bool uses(const ContainedPtr&) const;
    virtual ContainedType containedType() const;
    virtual std::string kindOf() const;
    virtual void visit(ParserVisitor*, bool);

protected:

    Const(const ContainerPtr&, const std::string&, const TypePtr&, const StringList&, const SyntaxTreeBasePtr&,
          const std::string&, const std::string&);
    friend class Container;

    TypePtr _type;
    StringList _typeMetaData;
    SyntaxTreeBasePtr _valueType;
    std::string _value;
    std::string _literal;
};

// ----------------------------------------------------------------------
// ParamDecl
// ----------------------------------------------------------------------

class ParamDecl : public virtual Contained
{
public:

    TypePtr type() const;
    bool isOutParam() const;
    bool optional() const;
    int tag() const;
    virtual ContainedType containedType() const;
    virtual bool uses(const ContainedPtr&) const;
    virtual std::string kindOf() const;
    virtual void visit(ParserVisitor*, bool);

protected:

    ParamDecl(const ContainerPtr&, const std::string&, const TypePtr&, bool, bool, int);
    friend class Operation;

    TypePtr _type;
    bool _isOutParam;
    bool _optional;
    int _tag;
};

// ----------------------------------------------------------------------
// DataMember
// ----------------------------------------------------------------------

class DataMember : public virtual Contained
{
public:

    TypePtr type() const;
    bool optional() const;
    int tag() const;
    std::string defaultValue() const;
    std::string defaultLiteral() const;
    SyntaxTreeBasePtr defaultValueType() const;
    virtual ContainedType containedType() const;
    virtual bool uses(const ContainedPtr&) const;
    virtual std::string kindOf() const;
    virtual void visit(ParserVisitor*, bool);

protected:

    DataMember(const ContainerPtr&, const std::string&, const TypePtr&, bool, int, const SyntaxTreeBasePtr&,
               const std::string&, const std::string&);
    friend class ClassDef;
    friend class Struct;
    friend class Exception;

    TypePtr _type;
    bool _optional;
    int _tag;
    SyntaxTreeBasePtr _defaultValueType;
    std::string _defaultValue;
    std::string _defaultLiteral;
};

// ----------------------------------------------------------------------
// Unit
// ----------------------------------------------------------------------

class Unit : public virtual Container
{
public:

    static UnitPtr createUnit(bool, bool, bool, bool, const StringList& = StringList());

    bool ignRedefs() const;
    bool allowIcePrefix() const;
    bool allowUnderscore() const;

    void setComment(const std::string&);
    std::string currentComment(); // Not const, as this function removes the current comment.
    std::string currentFile() const;
    std::string topLevelFile() const;
    int currentLine() const;

    void nextLine();
    bool scanPosition(const char*);
    int currentIncludeLevel() const;

    void addGlobalMetaData(const StringList&);

    void setSeenDefinition();

    void error(const std::string&); // Not const because error count is increased
    void warning(WarningCategory, const std::string&) const;

    ContainerPtr currentContainer() const;
    void pushContainer(const ContainerPtr&);
    void popContainer();

    DefinitionContextPtr currentDefinitionContext() const;
    void pushDefinitionContext();
    void popDefinitionContext();
    DefinitionContextPtr findDefinitionContext(const std::string&) const;

    void addContent(const ContainedPtr&);
    void removeContent(const ContainedPtr&);
    ContainedList findContents(const std::string&) const;
    ClassList findDerivedClasses(const ClassDefPtr&) const;
    ExceptionList findDerivedExceptions(const ExceptionPtr&) const;
    ContainedList findUsedBy(const ContainedPtr&) const;

    void addTypeId(int, const std::string&);
    std::string getTypeId(int) const;
    bool hasCompactTypeId() const;

    bool usesNonLocals() const;
    bool usesConsts() const;

    //
    // Returns the path names of the files included directly by the top-level file.
    //
    StringList includeFiles() const;

    //
    // Returns the path names of all files parsed by this unit.
    //
    StringList allFiles() const;

    int parse(const std::string&, FILE*, bool);

    virtual void destroy();
    virtual void visit(ParserVisitor*, bool);

    BuiltinPtr builtin(Builtin::Kind); // Not const, as builtins are created on the fly. (Lazy initialization.)

    void addTopLevelModule(const std::string&, const std::string&);
    std::set<std::string> getTopLevelModules(const std::string&) const;

private:

    Unit(bool, bool, bool, bool, const StringList&);
    static void eraseWhiteSpace(::std::string&);

    bool _ignRedefs;
    bool _all;
    bool _allowIcePrefix;
    bool _allowUnderscore;
    StringList _defaultGlobalMetaData;
    int _errors;
    std::string _currentComment;
    int _currentLine;
    int _currentIncludeLevel;
    std::string _currentFile;
    std::string _topLevelFile;
    std::stack<DefinitionContextPtr> _definitionContextStack;
    StringList _includeFiles;
    std::stack<ContainerPtr> _containerStack;
    std::map<Builtin::Kind, BuiltinPtr> _builtins;
    std::map<std::string, ContainedList> _contentMap;
    std::map<std::string, DefinitionContextPtr> _definitionContextMap;
    std::map<int, std::string> _typeIds;
    std::map< std::string, std::set<std::string> > _fileTopLevelModules;
};

extern Unit* unit; // The current parser for bison/flex

}

#endif
