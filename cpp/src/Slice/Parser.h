//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef SLICE_PARSER_H
#define SLICE_PARSER_H

#include <IceUtil/Shared.h>
#include <IceUtil/Handle.h>
#include <IceUtil/Exception.h>
#include <array>
#include <string>
#include <vector>
#include <list>
#include <stack>
#include <map>
#include <optional>
#include <set>
#include <string_view>
#include <stdio.h>

namespace Slice
{

class CompilerException : public ::IceUtil::Exception
{
public:

    CompilerException(const char*, int, const std::string&);
    std::string ice_id() const override;
    void ice_print(std::ostream&) const override;
    CompilerException* ice_cloneImpl() const override;
    void ice_throw() const override;

    std::string reason() const;

private:

    static const char* _name;
    const std::string _reason;
};

enum NodeType
{
    Dummy,
    Real
};

// Format preference for classes and exceptions.
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
    InvalidMetadata,
    ReservedIdentifier
};

class GrammarBase;
class SyntaxTreeBase;
class Type;
class Builtin;
class Contained;
class Container;
class DataMemberContainer;
class Module;
class ClassDecl;
class ClassDef;
class InterfaceDecl;
class InterfaceDef;
class Exception;
class Optional;
class Struct;
class Operation;
class Member;
class TypeAlias;
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
typedef ::IceUtil::Handle<DataMemberContainer> DataMemberContainerPtr;
typedef ::IceUtil::Handle<Module> ModulePtr;
typedef ::IceUtil::Handle<ClassDecl> ClassDeclPtr;
typedef ::IceUtil::Handle<ClassDef> ClassDefPtr;
typedef ::IceUtil::Handle<InterfaceDecl> InterfaceDeclPtr;
typedef ::IceUtil::Handle<InterfaceDef> InterfaceDefPtr;
typedef ::IceUtil::Handle<Optional> OptionalPtr;
typedef ::IceUtil::Handle<Exception> ExceptionPtr;
typedef ::IceUtil::Handle<Struct> StructPtr;
typedef ::IceUtil::Handle<Operation> OperationPtr;
typedef ::IceUtil::Handle<Member> MemberPtr;
typedef ::IceUtil::Handle<TypeAlias> TypeAliasPtr;
typedef ::IceUtil::Handle<Sequence> SequencePtr;
typedef ::IceUtil::Handle<Dictionary> DictionaryPtr;
typedef ::IceUtil::Handle<Enum> EnumPtr;
typedef ::IceUtil::Handle<Enumerator> EnumeratorPtr;
typedef ::IceUtil::Handle<Const> ConstPtr;
typedef ::IceUtil::Handle<Unit> UnitPtr;

typedef std::list<TypePtr> TypeList;
typedef std::set<std::string> StringSet;
typedef std::list<std::string> StringList;
typedef std::list<ContainedPtr> ContainedList;
typedef std::list<ModulePtr> ModuleList;
typedef std::list<ClassDefPtr> ClassList;
typedef std::list<InterfaceDefPtr> InterfaceList;
typedef std::list<ExceptionPtr> ExceptionList;
typedef std::list<StructPtr> StructList;
typedef std::list<SequencePtr> SequenceList;
typedef std::list<DictionaryPtr> DictionaryList;
typedef std::list<EnumPtr> EnumList;
typedef std::list<ConstPtr> ConstList;
typedef std::list<OperationPtr> OperationList;
typedef std::list<MemberPtr> MemberList;
typedef std::list<EnumeratorPtr> EnumeratorList;

// ----------------------------------------------------------------------
// CICompare -- function object to do case-insensitive string comparison.
// ----------------------------------------------------------------------

class CICompare
{
public:

    bool operator()(const std::string&, const std::string&) const;
};

// ----------------------------------------------------------------------
// DerivedToBaseCompare -- function object to do sort exceptions into
// most-derived to least-derived order.
// ----------------------------------------------------------------------

class DerivedToBaseCompare
{
public:

    bool operator()(const ExceptionPtr&, const ExceptionPtr&) const;
};

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
    virtual void visitInterfaceDecl(const InterfaceDeclPtr&) { }
    virtual bool visitInterfaceDefStart(const InterfaceDefPtr&) { return true; }
    virtual void visitInterfaceDefEnd(const InterfaceDefPtr&) { }
    virtual bool visitExceptionStart(const ExceptionPtr&) { return true; }
    virtual void visitExceptionEnd(const ExceptionPtr&) { }
    virtual bool visitStructStart(const StructPtr&) { return true; }
    virtual void visitStructEnd(const StructPtr&) { }
    virtual void visitTypeAlias(const TypeAliasPtr&) { }
    virtual void visitOperation(const OperationPtr&) { }
    virtual void visitParameter(const MemberPtr&) { }
    virtual void visitDataMember(const MemberPtr&) { }
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

    void setFilename(const std::string&);

    void setMetadata(const StringList&);
    std::string findMetadata(const std::string&) const;
    StringList getAllMetadata() const;

    // When parsing Slice definitions, apply 3.7 or 4.0 semantics for class parameters, Object etc.
    bool compatMode() const;

    // Emit warning unless filtered out by [["suppress-warning"]]
    void warning(WarningCategory, const std::string&, int, const std::string&) const;

    void error(const std::string&, int, const std::string&) const;

private:

    bool suppressWarning(WarningCategory) const;
    void initSuppressedWarnings();

    int _includeLevel;
    StringList _metadata;
    std::string _filename;
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
    std::map<std::string, StringList> params() const; // Parameter descriptions for an op. Key is parameter name.
    std::map<std::string, StringList> exceptions() const; // Exception descriptions for an op. Key is exception name.

private:

    Comment();

    bool _isDeprecated;
    StringList _deprecated;
    StringList _overview;
    StringList _misc;
    StringList _seeAlso;

    StringList _returns;
    std::map<std::string, StringList> _params;
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

    SyntaxTreeBase(const UnitPtr&, const DefinitionContextPtr& = nullptr);

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
    virtual bool isClassType() const { return false; }
    virtual bool isInterfaceType() const { return false; }
    virtual size_t minWireSize() const = 0;
    virtual std::string getTagFormat() const = 0;
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
        KindBool,
        KindByte,
        KindShort,
        KindUShort,
        KindInt,
        KindUInt,
        KindVarInt,
        KindVarUInt,
        KindLong,
        KindULong,
        KindVarLong,
        KindVarULong,
        KindFloat,
        KindDouble,
        KindString,
        KindObject, // the implicit base for all proxies
        KindAnyClass
    };

    std::string typeId() const override;
    bool usesClasses() const override;
    bool isClassType() const override { return _kind == KindAnyClass; }
    bool isInterfaceType() const override { return _kind == KindObject; }
    size_t minWireSize() const override;
    std::string getTagFormat() const override;
    bool isVariableLength() const override;

    bool isNumericType() const;
    bool isNumericTypeOrBool() const;
    bool isIntegralType() const;
    bool isUnsignedType() const;
    std::pair<std::int64_t, std::uint64_t> integralRange() const;

    Kind kind() const;
    std::string kindAsString() const;
    static std::optional<Kind> kindFromString(std::string_view);

    inline static const std::array<std::string, 17> builtinTable =
    {
        "bool",
        "byte",
        "short",
        "ushort",
        "int",
        "uint",
        "varint",
        "varuint",
        "long",
        "ulong",
        "varlong",
        "varulong",
        "float",
        "double",
        "string",
        "Object",
        "AnyClass"
    };

protected:

    Builtin(const UnitPtr&, Kind);

    friend class Unit;

    const Kind _kind;
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
    int line() const;
    std::string comment() const;
    CommentPtr parseComment(bool) const;

    int includeLevel() const;

    bool hasMetadata(const std::string&) const;
    bool hasMetadataWithPrefix(const std::string&) const;
    std::optional<std::string> findMetadata(const std::string&) const;
    bool findMetadata(const std::string&, std::string&) const;
    std::string findMetadataWithPrefix(const std::string&) const;
    std::list<std::string> getAllMetadata() const;
    void setMetadata(const std::list<std::string>&);

    FormatType parseFormatMetadata() const;

    virtual bool uses(const ContainedPtr&) const = 0;
    virtual std::string kindOf() const = 0;

    bool operator<(const Contained&) const;
    bool operator==(const Contained&) const;

protected:

    Contained(const ContainerPtr&, const std::string&);

    ContainerPtr _container;
    std::string _name;
    std::string _scoped;
    std::string _file;
    int _line;
    std::string _comment;
    int _includeLevel;
    std::list<std::string> _metadata;
};

// ----------------------------------------------------------------------
// Container
// ----------------------------------------------------------------------

class Container : public virtual SyntaxTreeBase
{
public:

    void destroy() override;
    TypeList lookupType(const std::string&, bool = true);
    TypeList lookupTypeNoBuiltin(const std::string&, bool = true, bool = false);
    ContainedList lookupContained(const std::string&, bool = true);
    ExceptionPtr lookupException(const std::string&, bool = true);
    // Finds enumerators using the deprecated unscoped enumerators lookup
    EnumeratorList enumerators(const std::string&) const;
    virtual ContainedList contents() const = 0;
    bool hasContentsWithMetadata(const std::string&) const;
    std::string thisScope() const;

    bool checkIntroduced(const std::string&, ContainedPtr = nullptr);

protected:

    Container(const UnitPtr&);

    bool validateConstant(const std::string&, const TypePtr&, SyntaxTreeBasePtr&, const std::string&, bool);

    std::map<std::string, ContainedPtr> _introducedMap;
};

// ----------------------------------------------------------------------
// DataMemberContainer
// ----------------------------------------------------------------------

class DataMemberContainer : public virtual Container, public virtual Contained
{
public:

    void destroy() override;
    virtual MemberPtr createDataMember(const std::string&, const TypePtr&, bool, int, const SyntaxTreeBasePtr& = nullptr,
                                       const std::string& = "", const std::string& = "");
    MemberList dataMembers() const;
    std::pair<MemberList, MemberList> sortedDataMembers() const;
    bool hasDataMembers() const;
    virtual bool hasBaseDataMembers() const;
    ContainedList contents() const override;
    bool uses(const ContainedPtr&) const override;

protected:

    DataMemberContainer(const ContainerPtr&, const std::string&);

    MemberList _dataMembers;
};

// ----------------------------------------------------------------------
// Module
// ----------------------------------------------------------------------

class Module : public virtual Container, public virtual Contained
{
public:

    void destroy() override;
    ContainedList contents() const override;
    bool uses(const ContainedPtr&) const override;
    std::string kindOf() const override;
    void visit(ParserVisitor*, bool) override;
    ModulePtr createModule(const std::string&);
    ClassDefPtr createClassDef(const std::string&, int, const ClassDefPtr&);
    ClassDeclPtr createClassDecl(const std::string&);
    InterfaceDefPtr createInterfaceDef(const std::string&, const InterfaceList&);
    InterfaceDeclPtr createInterfaceDecl(const std::string&);
    ExceptionPtr createException(const std::string&, const ExceptionPtr&, NodeType = Real);
    StructPtr createStruct(const std::string&, NodeType = Real);
    TypeAliasPtr createTypeAlias(const std::string& name, const TypePtr& type, const StringList& metadata);
    SequencePtr createSequence(const std::string&, const TypePtr&, const StringList&);
    DictionaryPtr createDictionary(const std::string&, const TypePtr&, const StringList&, const TypePtr&,
                                   const StringList&);
    EnumPtr createEnum(const std::string&, bool, NodeType = Real);
    ConstPtr createConst(const std::string, const TypePtr&, const StringList&, const SyntaxTreeBasePtr&,
                         const std::string&, const std::string&, NodeType = Real);
    EnumList enums() const;
    ConstList consts() const;
    bool hasConsts() const;
    bool hasStructs() const;
    bool hasExceptions() const;
    bool hasEnums() const;
    bool hasClassDecls() const;
    bool hasClassDefs() const;
    bool hasInterfaceDecls() const;
    bool hasInterfaceDefs() const;
    bool hasOnlyClassDecls() const;
    bool hasOnlyInterfaces() const;
    bool hasOnlySubModules() const;
    bool hasNonClassTypes() const;

protected:

    Module(const ContainerPtr&, const std::string&);

    friend class Unit;

    ContainedList _contents;
};

// ----------------------------------------------------------------------
// ClassDecl
// ----------------------------------------------------------------------

class ClassDecl : public virtual Contained, public virtual Type
{
public:

    void destroy() override;
    ClassDefPtr definition() const;
    std::string typeId() const override;
    bool uses(const ContainedPtr&) const override;
    bool usesClasses() const override;
    bool isClassType() const override { return true; }
    size_t minWireSize() const override;
    std::string getTagFormat() const override;
    bool isVariableLength() const override;
    void visit(ParserVisitor*, bool) override;
    std::string kindOf() const override;

protected:

    ClassDecl(const ContainerPtr&, const std::string&);

    friend class Module;

    ClassDefPtr _definition;
};

// ----------------------------------------------------------------------
// ClassDef
// ----------------------------------------------------------------------

// Note: For the purpose of this parser, a class definition is not
// considered to be a type, but a class declaration is. And each class
// definition has at least one class declaration (but not vice versa),
// so if you need the class as a "constructed type", use the
// declaration() operation to navigate to the class declaration.
class ClassDef : public virtual DataMemberContainer, public virtual Contained
{
public:

    void destroy() override;
    MemberPtr createDataMember(const std::string&, const TypePtr&, bool, int, const SyntaxTreeBasePtr& = nullptr,
                               const std::string& = "", const std::string& = "") override;
    ClassDeclPtr declaration() const;
    ClassDefPtr base() const;
    ClassList allBases() const;
    MemberList allDataMembers() const;
    bool isA(const std::string&) const;
    bool inheritsMetadata(const std::string&) const;
    bool hasBaseDataMembers() const override;
    std::string kindOf() const override;
    void visit(ParserVisitor*, bool) override;
    int compactId() const;
    StringList ids() const;

protected:

    ClassDef(const ContainerPtr&, const std::string&, int, const ClassDefPtr&);

    friend class Module;

    ClassDeclPtr _declaration;
    ClassDefPtr _base;
    int _compactId;
};

// ----------------------------------------------------------------------
// InterfaceDecl
// ----------------------------------------------------------------------

class InterfaceDecl : public virtual Contained, public virtual Type
{
public:

    void destroy() override;
    InterfaceDefPtr definition() const;
    std::string typeId() const override;
    bool uses(const ContainedPtr&) const override;
    bool usesClasses() const override;
    bool isInterfaceType() const override { return true; }
    size_t minWireSize() const override;
    std::string getTagFormat() const override;
    bool isVariableLength() const override;
    void visit(ParserVisitor*, bool) override;
    std::string kindOf() const override;

    static void checkBasesAreLegal(const std::string&, const InterfaceList&, const UnitPtr&);

protected:

    InterfaceDecl(const ContainerPtr&, const std::string&);

    friend class Module;

    InterfaceDefPtr _definition;

private:

    typedef std::list<InterfaceList> GraphPartitionList;
    typedef std::list<StringList> StringPartitionList;

    static bool isInList(const GraphPartitionList&, const InterfaceDefPtr&);
    static void addPartition(GraphPartitionList&, GraphPartitionList::reverse_iterator, const InterfaceDefPtr&);
    static StringPartitionList toStringPartitionList(const GraphPartitionList&);
    static void checkPairIntersections(const StringPartitionList&, const std::string&, const UnitPtr&);
};

// ----------------------------------------------------------------------
// Operation
// ----------------------------------------------------------------------

class Operation : public virtual Contained, public virtual Container
{
public:

    // Note: The order of definitions here *must* match the order of
    // definitions of ::Ice::OperationMode in Ice/Current.h
    // TODO: remove from parser
    enum Mode
    {
        Normal,
        Nonmutating,
        Idempotent
    };

    InterfaceDefPtr interface() const;

    // The "in" bit sequence length. It corresponds to the number of in-parameters with optional types that are not
    // class/proxy and that are not tagged.
    size_t paramsBitSequenceSize() const;

    // The "return" bit sequence length. It corresponds to the number of return members with optional types that are
    // not class/proxy and that are not tagged.
    size_t returnBitSequenceSize() const;

    void destroy() override;
    TypePtr deprecatedReturnType() const; //TODO remove this once the compilers have been updated to use return-tuples.
    bool returnIsTagged() const; //TODO remove this once the compilers have been updated to use return-tuples.
    int returnTag() const; //TODO remove this once the compilers have been updated to use return-tuples.
    Mode mode() const; // TODO: remove
    Mode sendMode() const; // TODO: remove
    bool isIdempotent() const { return _mode == Idempotent; }
    bool hasMarshaledResult() const;
    MemberPtr createParameter(const std::string& name, const TypePtr& type, bool isOutParam, bool tagged, int tag,
                              bool stream);
    MemberPtr createReturnMember(const std::string& name, const TypePtr& type, bool tagged, int tag, bool stream);
    MemberList params() const;
    MemberList outParameters() const; //TODO remove this once the compilers have been updated to use return-tuples.
    MemberList returnType() const;
    MemberList allMembers() const;
    ExceptionList throws() const;
    void setExceptionList(const ExceptionList&);
    ContainedList contents() const override;
    bool uses(const ContainedPtr&) const override;
    bool sendsClasses(bool) const;
    bool returnsClasses(bool) const;
    bool returnsData() const;
    bool returnsMultipleValues() const;
    bool hasReturnAndOut() const;
    bool hasSingleReturnType() const;
    FormatType format() const;
    std::string kindOf() const override;
    void visit(ParserVisitor*, bool) override;

protected:

    Operation(const ContainerPtr& container, const std::string& name, Mode mode);

    friend class InterfaceDef;

    MemberList _params;
    MemberList _returnType;
    bool _usesOutParams;
    bool _hasReturnType;
    std::list<ExceptionPtr> _throws;
    Mode _mode;
};

// ----------------------------------------------------------------------
// InterfaceDef
// ----------------------------------------------------------------------

// Note: For the purpose of this parser, an interface definition is not
// considered to be a type, but an interface declaration is. And each interface
// definition has at least one interface declaration (but not vice versa),
// so if you need the interface as a "constructed type", use the
// declaration() function to navigate to the interface declaration.
class InterfaceDef : public virtual Container, public virtual Contained
{
public:

    void destroy() override;
    OperationPtr createOperation(const std::string&, Operation::Mode = Operation::Normal);

    InterfaceDeclPtr declaration() const;
    InterfaceList bases() const;
    InterfaceList allBases() const;
    OperationList operations() const;
    OperationList allOperations() const;
    bool isA(const std::string&) const;
    bool inheritsMetadata(const std::string&) const;
    ContainedList contents() const override;
    bool uses(const ContainedPtr&) const override;
    std::string kindOf() const override;
    void visit(ParserVisitor*, bool) override;
    StringList ids() const;

protected:

    InterfaceDef(const ContainerPtr&, const std::string&, const InterfaceList&);

    friend class Module;

    InterfaceDeclPtr _declaration;
    InterfaceList _bases;
    std::list<OperationPtr> _operations;
};

// ----------------------------------------------------------------------
// Optional (for T? types)
// ----------------------------------------------------------------------

class Optional : public Type
{
public:

    Optional(const TypePtr& underlying);

    std::string typeId() const override;
    bool usesClasses() const override;
    bool isClassType() const override;
    bool isInterfaceType() const override;
    size_t minWireSize() const override;
    std::string getTagFormat() const override;
    bool isVariableLength() const override;
    TypePtr underlying() const { return _underlying; }
    bool encodedUsingBitSequence() const { return minWireSize() == 0; }

private:

    const TypePtr _underlying;
};

// ----------------------------------------------------------------------
// Exception
// ----------------------------------------------------------------------

class Exception : public virtual DataMemberContainer, public virtual Contained
{
public:

    void destroy() override;
    MemberPtr createDataMember(const std::string&, const TypePtr&, bool, int, const SyntaxTreeBasePtr& = nullptr,
                               const std::string& = "", const std::string& = "") override;
    MemberList allDataMembers() const;
    ExceptionPtr base() const;
    ExceptionList allBases() const;
    bool isBaseOf(const ExceptionPtr&) const;
    bool usesClasses(bool) const;
    bool inheritsMetadata(const std::string&) const;
    bool hasBaseDataMembers() const override;
    std::string kindOf() const override;
    void visit(ParserVisitor*, bool) override;

protected:

    Exception(const ContainerPtr&, const std::string&, const ExceptionPtr&);

    friend class Container;
    friend class Module;

    ExceptionPtr _base;
};

// ----------------------------------------------------------------------
// Struct
// ----------------------------------------------------------------------

class Struct : public virtual DataMemberContainer, public virtual Contained, public virtual Type
{
public:
    MemberPtr createDataMember(const std::string&, const TypePtr&, bool, int, const SyntaxTreeBasePtr& = nullptr,
                               const std::string& = "", const std::string& = "") override;
    std::string typeId() const override;
    bool usesClasses() const override;
    size_t minWireSize() const override;
    std::string getTagFormat() const override;
    bool isVariableLength() const override;
    std::string kindOf() const override;
    void visit(ParserVisitor*, bool) override;

protected:

    Struct(const ContainerPtr&, const std::string&);

    friend class Container;
    friend class Module;
};

// ----------------------------------------------------------------------
// TypeAlias
// ----------------------------------------------------------------------

class TypeAlias : public virtual Contained, public virtual Type
{
public:

    void destroy() override;
    TypePtr underlying() const { return _underlying; }
    StringList typeMetadata() const { return _typeMetadata; }

    std::string typeId() const override;
    bool uses(const ContainedPtr& contained) const override;
    bool usesClasses() const override;
    bool isClassType() const override;
    bool isInterfaceType() const override;
    size_t minWireSize() const override;
    std::string getTagFormat() const override;
    bool isVariableLength() const override;
    std::string kindOf() const override;
    void visit(ParserVisitor* visitor, bool all) override;

protected:

    TypeAlias(const ContainerPtr& container, const std::string& name, const TypePtr& underlying,
              const StringList& typeMetadata);

    friend class Module;

    TypePtr _underlying;
    StringList _typeMetadata;
};

// ----------------------------------------------------------------------
// Sequence
// ----------------------------------------------------------------------

class Sequence : public virtual Contained, public virtual Type
{
public:

    TypePtr type() const;
    StringList typeMetadata() const;
    std::string typeId() const override;
    bool uses(const ContainedPtr&) const override;
    bool usesClasses() const override;
    size_t minWireSize() const override;
    std::string getTagFormat() const override;
    bool isVariableLength() const override;
    std::string kindOf() const override;
    void visit(ParserVisitor*, bool) override;

protected:

    Sequence(const ContainerPtr&, const std::string&, const TypePtr&, const StringList&);

    friend class Container;
    friend class Module;

    TypePtr _type;
    StringList _typeMetadata;
};

// ----------------------------------------------------------------------
// Dictionary
// ----------------------------------------------------------------------

class Dictionary : public virtual Contained, public virtual Type
{
public:

    TypePtr keyType() const;
    TypePtr valueType() const;
    StringList keyMetadata() const;
    StringList valueMetadata() const;
    std::string typeId() const override;
    bool uses(const ContainedPtr&) const override;
    bool usesClasses() const override;
    size_t minWireSize() const override;
    std::string getTagFormat() const override;
    bool isVariableLength() const override;
    std::string kindOf() const override;
    void visit(ParserVisitor*, bool) override;

    static bool legalKeyType(const TypePtr&, bool&);

protected:

    Dictionary(const ContainerPtr&, const std::string&, const TypePtr&, const StringList&, const TypePtr&,
               const StringList&);

    friend class Container;
    friend class Module;

    TypePtr _keyType;
    TypePtr _valueType;
    StringList _keyMetadata;
    StringList _valueMetadata;
};

// ----------------------------------------------------------------------
// Enum
// ----------------------------------------------------------------------

class Enum : public virtual Container, public virtual Contained, public virtual Type
{
public:

    void destroy() override;
    EnumeratorPtr createEnumerator(const std::string&);
    EnumeratorPtr createEnumerator(const std::string&, std::int64_t);
    EnumeratorList enumerators() const;

    // The underlying type. The default is nullptr, which means a range of 0..INT32_MAX encoded as a variable-length
    // size. The only permissible underlying types are byte, short, ushort, int, and uint.
    BuiltinPtr underlying() const;

    // A Slice enum is checked by default: the generated unmarshaling code verifies the value matches one of the enum's
    // enumerators.
    bool isUnchecked() const { return _unchecked; }

    bool explicitValue() const;

    std::int64_t minValue() const;
    std::int64_t maxValue() const;
    ContainedList contents() const override;
    std::string typeId() const override;
    bool uses(const ContainedPtr&) const override;
    bool usesClasses() const override;
    size_t minWireSize() const override;
    std::string getTagFormat() const override;
    bool isVariableLength() const override;
    std::string kindOf() const override;
    void visit(ParserVisitor*, bool) override;

    // Sets the underlying type shortly after construction and before any enumerator is added.
    void initUnderlying(const TypePtr&);

protected:

    Enum(const ContainerPtr&, const std::string&, bool);
    std::int64_t newEnumerator(const EnumeratorPtr&);

    friend class Container;
    friend class Module;
    friend class Enumerator;

    std::list<EnumeratorPtr> _enumerators;
    const bool _unchecked;
    BuiltinPtr _underlying;
    bool _explicitValue;
    std::int64_t _minValue;
    std::int64_t _maxValue;
    std::int64_t _lastValue;
};

// ----------------------------------------------------------------------
// Enumerator
// ----------------------------------------------------------------------

class Enumerator : public virtual Contained
{
public:

    EnumPtr type() const;
    bool uses(const ContainedPtr&) const override;
    std::string kindOf() const override;

    bool explicitValue() const;
    std::int64_t value() const;

protected:

    Enumerator(const EnumPtr&, const std::string&);
    Enumerator(const EnumPtr&, const std::string&, std::int64_t);

    friend class Enum;

    bool _explicitValue;
    std::int64_t _value;
};

// ----------------------------------------------------------------------
// Const
// ----------------------------------------------------------------------

class Const : public virtual Contained
{
public:

    TypePtr type() const;
    StringList typeMetadata() const;
    SyntaxTreeBasePtr valueType() const;
    std::string value() const;
    std::string literal() const;
    bool uses(const ContainedPtr&) const override;
    std::string kindOf() const override;
    void visit(ParserVisitor*, bool) override;

protected:

    Const(const ContainerPtr&, const std::string&, const TypePtr&, const StringList&, const SyntaxTreeBasePtr&,
          const std::string&, const std::string&);

    friend class Container;
    friend class Module;

    TypePtr _type;
    StringList _typeMetadata;
    SyntaxTreeBasePtr _valueType;
    std::string _value;
    std::string _literal;
};

// ----------------------------------------------------------------------
// Member
// ----------------------------------------------------------------------

class Member : public virtual Contained
{
public:

    TypePtr type() const;
    bool tagged() const;
    int tag() const;
    bool stream() const;
    std::string defaultValue() const;
    std::string defaultLiteral() const;
    SyntaxTreeBasePtr defaultValueType() const;
    bool uses(const ContainedPtr&) const override;
    std::string kindOf() const override;
    void visit(ParserVisitor*, bool) override;

    // Returns the enclosing operation when this member is a parameter or return value member. Otherwise, returns null.
    OperationPtr operation() const;

protected:

    Member(const ContainerPtr&, const std::string&, const TypePtr&, bool, int, bool, const SyntaxTreeBasePtr& = nullptr,
               const std::string& = "", const std::string& = "");

    friend class DataMemberContainer;
    friend class Operation;

    TypePtr _type;
    bool _tagged;
    int _tag;
    bool _stream;
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

    static UnitPtr createUnit(bool, const StringList& = StringList());
    ModulePtr createModule(const std::string& name);

    bool compatMode() const;
    void checkType(const TypePtr&);

    void setComment(const std::string&);
    void addToComment(const std::string&);
    std::string currentComment(); // Not const, as this function removes the current comment.
    std::string currentFile() const;
    std::string topLevelFile() const;
    int currentLine() const;

    int setCurrentFile(const std::string&, int);
    int currentIncludeLevel() const;

    void addFileMetadata(const StringList&);

    void error(const std::string&); // Not const because error count is increased
    void warning(WarningCategory, const std::string&) const;
    void note(ContainedPtr, const std::string&) const;

    ContainerPtr currentContainer() const;
    ModulePtr currentModule() const;
    void pushContainer(const ContainerPtr&);
    void popContainer();

    DefinitionContextPtr currentDefinitionContext() const;
    void pushDefinitionContext();
    void popDefinitionContext();
    DefinitionContextPtr findDefinitionContext(const std::string&) const;

    void addContent(const ContainedPtr&);
    void removeContent(const ContainedPtr&);
    ContainedList findContents(const std::string&) const;

    void addTypeId(int, const std::string&);
    std::string getTypeId(int) const;
    bool hasCompactTypeId() const;

    // Returns the path names of the files included directly by the top-level file.
    StringList includeFiles() const;

    // Returns the path names of all files parsed by this unit.
    StringList allFiles() const;

    int parse(const std::string&, FILE*, bool);

    void destroy() override;
    ContainedList contents() const override;
    void visit(ParserVisitor*, bool) override;
    bool hasExceptions() const;
    bool hasClassDecls() const;
    bool hasClassDefs() const;
    bool hasInterfaceDecls() const;
    bool hasInterfaceDefs() const;

    // Not const, as builtins are created on the fly. (Lazy initialization.)
    BuiltinPtr builtin(Builtin::Kind);
    OptionalPtr optionalBuiltin(Builtin::Kind);

    void addTopLevelModule(const std::string&, const std::string&);
    std::set<std::string> getTopLevelModules(const std::string&) const;

private:

    Unit(bool, const StringList&);

    bool _all;
    StringList _defaultFileMetadata;
    int _errors;
    std::string _currentComment;
    int _currentIncludeLevel;
    std::string _topLevelFile;
    std::stack<DefinitionContextPtr> _definitionContextStack;
    StringList _includeFiles;
    ModulePtr _globalModule;
    std::list<ModulePtr> _modules;
    std::stack<ContainerPtr> _containerStack;
    std::map<Builtin::Kind, BuiltinPtr> _builtins;
    std::map<Builtin::Kind, OptionalPtr> _optionalBuiltins;
    std::map<std::string, ContainedList> _contentMap;
    std::map<std::string, DefinitionContextPtr> _definitionContextMap;
    std::map<int, std::string> _typeIds;
    std::map< std::string, std::set<std::string> > _fileTopLevelModules;
};

extern Unit* unit; // The current parser for bison/flex

}

#endif
