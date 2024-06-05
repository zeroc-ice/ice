//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef SLICE_PARSER_H
#define SLICE_PARSER_H

#include "IceUtil/Exception.h"
#include <array>
#include <cstdint>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <stack>
#include <stdio.h>
#include <string>
#include <string_view>
#include <vector>

namespace Slice
{
    class CompilerException : public ::IceUtil::Exception
    {
    public:
        CompilerException(const char*, int, const std::string&);
        virtual std::string ice_id() const;
        virtual void ice_print(std::ostream&) const;

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

    //
    // Format preference for classes and exceptions.
    //
    enum FormatType
    {
        DefaultFormat, // No preference was specified.
        CompactFormat, // Minimal format.
        SlicedFormat   // Full format.
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
    class InterfaceDecl;
    class InterfaceDef;
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

    using GrammarBasePtr = std::shared_ptr<GrammarBase>;
    using SyntaxTreeBasePtr = std::shared_ptr<SyntaxTreeBase>;
    using TypePtr = std::shared_ptr<Type>;
    using BuiltinPtr = std::shared_ptr<Builtin>;
    using ContainedPtr = std::shared_ptr<Contained>;
    using ContainerPtr = std::shared_ptr<Container>;
    using ModulePtr = std::shared_ptr<Module>;
    using ConstructedPtr = std::shared_ptr<Constructed>;
    using ClassDeclPtr = std::shared_ptr<ClassDecl>;
    using ClassDefPtr = std::shared_ptr<ClassDef>;
    using InterfaceDeclPtr = std::shared_ptr<InterfaceDecl>;
    using InterfaceDefPtr = std::shared_ptr<InterfaceDef>;
    using ExceptionPtr = std::shared_ptr<Exception>;
    using StructPtr = std::shared_ptr<Struct>;
    using OperationPtr = std::shared_ptr<Operation>;
    using ParamDeclPtr = std::shared_ptr<ParamDecl>;
    using DataMemberPtr = std::shared_ptr<DataMember>;
    using SequencePtr = std::shared_ptr<Sequence>;
    using DictionaryPtr = std::shared_ptr<Dictionary>;
    using EnumPtr = std::shared_ptr<Enum>;
    using EnumeratorPtr = std::shared_ptr<Enumerator>;
    using ConstPtr = std::shared_ptr<Const>;
    using UnitPtr = std::shared_ptr<Unit>;

    bool containedCompare(const ContainedPtr& lhs, const ContainedPtr& rhs);
    bool containedEqual(const ContainedPtr& lhs, const ContainedPtr& rhs);

    using TypeList = std::list<TypePtr>;
    using ExceptionList = std::list<ExceptionPtr>;
    using StringSet = std::set<std::string>;
    using StringList = std::list<std::string>;
    using TypeString = std::pair<TypePtr, std::string>;
    using TypeStringList = std::list<TypeString>;
    using ContainedList = std::list<ContainedPtr>;
    using ModuleList = std::list<ModulePtr>;
    using ClassList = std::list<ClassDefPtr>;
    using InterfaceList = std::list<InterfaceDefPtr>;
    using ExceptionList = std::list<ExceptionPtr>;
    using StructList = std::list<StructPtr>;
    using SequenceList = std::list<SequencePtr>;
    using DictionaryList = std::list<DictionaryPtr>;
    using EnumList = std::list<EnumPtr>;
    using ConstList = std::list<ConstPtr>;
    using OperationList = std::list<OperationPtr>;
    using DataMemberList = std::list<DataMemberPtr>;
    using ParamDeclList = std::list<ParamDeclPtr>;
    using EnumeratorList = std::list<EnumeratorPtr>;

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
        virtual ~ParserVisitor() {}
        virtual bool visitUnitStart(const UnitPtr&) { return true; }
        virtual void visitUnitEnd(const UnitPtr&) {}
        virtual bool visitModuleStart(const ModulePtr&) { return true; }
        virtual void visitModuleEnd(const ModulePtr&) {}
        virtual void visitClassDecl(const ClassDeclPtr&) {}
        virtual bool visitClassDefStart(const ClassDefPtr&) { return true; }
        virtual void visitClassDefEnd(const ClassDefPtr&) {}
        virtual void visitInterfaceDecl(const InterfaceDeclPtr&) {}
        virtual bool visitInterfaceDefStart(const InterfaceDefPtr&) { return true; }
        virtual void visitInterfaceDefEnd(const InterfaceDefPtr&) {}
        virtual bool visitExceptionStart(const ExceptionPtr&) { return true; }
        virtual void visitExceptionEnd(const ExceptionPtr&) {}
        virtual bool visitStructStart(const StructPtr&) { return true; }
        virtual void visitStructEnd(const StructPtr&) {}
        virtual void visitOperation(const OperationPtr&) {}
        virtual void visitParamDecl(const ParamDeclPtr&) {}
        virtual void visitDataMember(const DataMemberPtr&) {}
        virtual void visitSequence(const SequencePtr&) {}
        virtual void visitDictionary(const DictionaryPtr&) {}
        virtual void visitEnum(const EnumPtr&) {}
        virtual void visitConst(const ConstPtr&) {}
    };

    // ----------------------------------------------------------------------
    // DefinitionContext
    // ----------------------------------------------------------------------

    class DefinitionContext
    {
    public:
        DefinitionContext(int, const StringList&);

        std::string filename() const;
        int includeLevel() const;
        bool seenDefinition() const;

        void setFilename(const std::string&);
        void setSeenDefinition();

        bool hasMetaData() const;
        bool hasMetaDataDirective(const std::string&) const;
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
    using DefinitionContextPtr = std::shared_ptr<DefinitionContext>;

    // ----------------------------------------------------------------------
    // Comment
    // ----------------------------------------------------------------------

    class Comment
    {
    public:
        bool isDeprecated() const;
        StringList deprecated() const;

        StringList overview() const; // Contains all introductory lines up to the first tag.
        StringList misc() const;     // Contains unrecognized tags.
        StringList seeAlso() const;  // Targets of @see tags.

        StringList returns() const; // Description of an operation's return value.
        std::map<std::string, StringList>
        parameters() const; // Parameter descriptions for an op. Key is parameter name.
        std::map<std::string, StringList>
        exceptions() const; // Exception descriptions for an op. Key is exception name.

    private:
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
    using CommentPtr = std::shared_ptr<Comment>;

    // ----------------------------------------------------------------------
    // GrammarBase
    // ----------------------------------------------------------------------

    class GrammarBase : public virtual std::enable_shared_from_this<GrammarBase>
    {
    public:
        virtual ~GrammarBase() = default;
    };

    // ----------------------------------------------------------------------
    // SyntaxTreeBase
    // ----------------------------------------------------------------------

    class SyntaxTreeBase : public GrammarBase
    {
    public:
        SyntaxTreeBase(const UnitPtr&);
        virtual void destroy();
        UnitPtr unit() const;
        DefinitionContextPtr definitionContext() const; // May be nil
        virtual void visit(ParserVisitor*, bool);

    protected:
        UnitPtr _unit;
        DefinitionContextPtr _definitionContext;
    };

    // ----------------------------------------------------------------------
    // Type
    // ----------------------------------------------------------------------

    class Type : public virtual SyntaxTreeBase
    {
    public:
        Type(const UnitPtr&);
        virtual std::string typeId() const = 0;
        virtual bool isClassType() const;
        virtual bool usesClasses() const;
        virtual size_t minWireSize() const = 0;
        virtual std::string getOptionalFormat() const = 0;
        virtual bool isVariableLength() const = 0;
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

        Builtin(const UnitPtr&, Kind);

        virtual std::string typeId() const;
        virtual bool isClassType() const;
        virtual size_t minWireSize() const;
        virtual std::string getOptionalFormat() const;
        virtual bool isVariableLength() const;

        bool isNumericType() const;
        bool isIntegralType() const;

        Kind kind() const;
        std::string kindAsString() const;
        static std::optional<Kind> kindFromString(std::string_view);

        inline static const std::array<std::string, 11> builtinTable =
            {"byte", "bool", "short", "int", "long", "float", "double", "string", "Object", "Object*", "Value"};

    protected:
        friend class Unit;

        const Kind _kind;
    };

    // ----------------------------------------------------------------------
    // Contained
    // ----------------------------------------------------------------------

    class Contained : public virtual SyntaxTreeBase
    {
    public:
        Contained(const ContainerPtr&, const std::string&);
        virtual void init();
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

        static FormatType parseFormatMetaData(const std::list<std::string>&);

        /// Returns true if this item is deprecated (due to the presence of 'deprecated' metadata).
        /// @param checkParent If true, this item's immediate container will also be checked for 'deprecated' metadata.
        /// @return True if this item (or possibly its container) has 'deprecated' metadata on it, false otherwise.
        bool isDeprecated(bool checkParent) const;

        /// If this item is deprecated, return its deprecation message (if present).
        /// This is the string argument that can be optionally provided with 'deprecated' metadata.
        /// @param checkParent If true, this item's immediate container will also be checked for 'deprecated' messages.
        /// @return The message provided to the 'deprecated' metadata, if present. If 'checkParent' is true, and both
        /// this item and its parent have 'deprecated' messages, the item's message is returned, not its container's.
        std::optional<std::string> getDeprecationReason(bool checkParent) const;

        enum ContainedType
        {
            ContainedTypeSequence,
            ContainedTypeDictionary,
            ContainedTypeEnum,
            ContainedTypeEnumerator,
            ContainedTypeModule,
            ContainedTypeClass,
            ContainedTypeInterface,
            ContainedTypeException,
            ContainedTypeStruct,
            ContainedTypeOperation,
            ContainedTypeParamDecl,
            ContainedTypeDataMember,
            ContainedTypeConstant
        };
        virtual ContainedType containedType() const = 0;

        virtual std::string kindOf() const = 0;

    protected:
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
        Container(const UnitPtr&);
        virtual void destroy();
        ModulePtr createModule(const std::string&);
        ClassDefPtr createClassDef(const std::string&, int, const ClassDefPtr&);
        ClassDeclPtr createClassDecl(const std::string&);
        InterfaceDefPtr createInterfaceDef(const std::string&, const InterfaceList&);
        InterfaceDeclPtr createInterfaceDecl(const std::string&);
        ExceptionPtr createException(const std::string&, const ExceptionPtr&, NodeType = Real);
        StructPtr createStruct(const std::string&, NodeType = Real);
        SequencePtr createSequence(const std::string&, const TypePtr&, const StringList&, NodeType = Real);
        DictionaryPtr createDictionary(
            const std::string&,
            const TypePtr&,
            const StringList&,
            const TypePtr&,
            const StringList&,
            NodeType = Real);
        EnumPtr createEnum(const std::string&, NodeType = Real);
        EnumeratorPtr createEnumerator(const std::string&);
        EnumeratorPtr createEnumerator(const std::string&, int);
        ConstPtr createConst(
            const std::string,
            const TypePtr&,
            const StringList&,
            const SyntaxTreeBasePtr&,
            const std::string&,
            const std::string&,
            NodeType = Real);
        TypeList lookupType(const std::string&, bool = true);
        TypeList lookupTypeNoBuiltin(const std::string&, bool = true, bool = false);
        ContainedList lookupContained(const std::string&, bool = true);
        ExceptionPtr lookupException(const std::string&, bool = true);
        UnitPtr unit() const;
        ModuleList modules() const;
        ClassList classes() const;
        InterfaceList interfaces() const;
        ExceptionList exceptions() const;
        StructList structs() const;
        SequenceList sequences() const;
        DictionaryList dictionaries() const;
        EnumList enums() const;
        EnumeratorList enumerators() const;
        EnumeratorList enumerators(const std::string&) const;
        ConstList consts() const;
        ContainedList contents() const;
        bool hasSequences() const;
        bool hasStructs() const;
        bool hasExceptions() const;
        bool hasDictionaries() const;
        bool hasClassDefs() const;
        bool hasInterfaceDefs() const;
        bool hasValueDefs() const;
        bool hasOperations() const;
        bool hasContained(Contained::ContainedType) const;
        std::string thisScope() const;
        void sort();
        void sortContents(bool);
        virtual void visit(ParserVisitor*, bool);

        bool checkIntroduced(const std::string&, ContainedPtr = 0);
        bool checkForGlobalDef(const std::string&, const char*);

    protected:
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
        Module(const ContainerPtr&, const std::string&);
        virtual ContainedType containedType() const;
        virtual std::string kindOf() const;
        virtual void visit(ParserVisitor*, bool);

        friend class Container;
    };

    // ----------------------------------------------------------------------
    // Constructed
    // ----------------------------------------------------------------------

    class Constructed : public virtual Type, public virtual Contained
    {
    public:
        Constructed(const ContainerPtr&, const std::string&);
        virtual std::string typeId() const;
    };

    // ----------------------------------------------------------------------
    // ClassDecl
    // ----------------------------------------------------------------------

    class ClassDecl : public virtual Constructed
    {
    public:
        ClassDecl(const ContainerPtr&, const std::string&);
        virtual void destroy();
        ClassDefPtr definition() const;
        virtual ContainedType containedType() const;
        virtual bool isClassType() const;
        virtual size_t minWireSize() const;
        virtual std::string getOptionalFormat() const;
        virtual bool isVariableLength() const;
        virtual void visit(ParserVisitor*, bool);
        virtual std::string kindOf() const;

    protected:
        friend class Container;
        friend class ClassDef;

        ClassDefPtr _definition;
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
        ClassDef(const ContainerPtr&, const std::string&, int, const ClassDefPtr&);
        virtual void destroy();
        DataMemberPtr createDataMember(
            const std::string&,
            const TypePtr&,
            bool,
            int,
            const SyntaxTreeBasePtr&,
            const std::string&,
            const std::string&);
        ClassDeclPtr declaration() const;
        ClassDefPtr base() const;
        ClassList allBases() const;
        DataMemberList dataMembers() const;
        DataMemberList orderedOptionalDataMembers() const;
        DataMemberList allDataMembers() const;
        DataMemberList classDataMembers() const;
        DataMemberList allClassDataMembers() const;
        bool canBeCyclic() const;
        bool isA(const std::string&) const;
        bool hasDataMembers() const;
        bool hasDefaultValues() const;
        bool inheritsMetaData(const std::string&) const;
        bool hasBaseDataMembers() const;
        virtual ContainedType containedType() const;
        virtual void visit(ParserVisitor*, bool);
        int compactId() const;
        virtual std::string kindOf() const;

    protected:
        friend class Container;

        ClassDeclPtr _declaration;
        bool _hasDataMembers;
        ClassDefPtr _base;
        int _compactId;
    };

    // ----------------------------------------------------------------------
    // InterfaceDecl
    // ----------------------------------------------------------------------

    class InterfaceDecl : public virtual Constructed
    {
    public:
        InterfaceDecl(const ContainerPtr&, const std::string&);
        virtual void destroy();
        InterfaceDefPtr definition() const;
        virtual ContainedType containedType() const;
        virtual size_t minWireSize() const;
        virtual std::string getOptionalFormat() const;
        virtual bool isVariableLength() const;
        virtual void visit(ParserVisitor*, bool);
        virtual std::string kindOf() const;

        static void checkBasesAreLegal(const std::string&, const InterfaceList&, const UnitPtr&);

    protected:
        friend class Container;
        friend class InterfaceDef;

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
        //
        // Note: The values of enumerators here *must* match the values for
        // enumerators of Ice::OperationMode in slice/Ice/OperationMode.ice!
        //
        enum Mode
        {
            Normal = 0,
            Idempotent = 2
        };

        Operation(const ContainerPtr&, const std::string&, const TypePtr&, bool, int, Mode);
        InterfaceDefPtr interface() const;
        TypePtr returnType() const;
        bool returnIsOptional() const;
        int returnTag() const;
        Mode mode() const;
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
        bool sendsClasses() const;
        bool returnsClasses() const;
        bool returnsData() const;
        bool returnsMultipleValues() const;
        bool sendsOptionals() const;
        FormatType format() const;
        virtual std::string kindOf() const;
        virtual void visit(ParserVisitor*, bool);

    protected:
        friend class InterfaceDef;

        TypePtr _returnType;
        bool _returnIsOptional;
        int _returnTag;
        ExceptionList _throws;
        Mode _mode;
    };

    // ----------------------------------------------------------------------
    // InterfaceDef
    // ----------------------------------------------------------------------

    //
    // Note: For the purpose of this parser, an interface definition is not
    // considered to be a type, but an interface declaration is. And each interface
    // definition has at least one interface declaration (but not vice versa),
    // so if you need the interface as a "constructed type", use the
    // declaration() function to navigate to the interface declaration.
    //
    class InterfaceDef : public virtual Container, public virtual Contained
    {
    public:
        InterfaceDef(const ContainerPtr&, const std::string&, const InterfaceList&);
        virtual void destroy();
        OperationPtr
        createOperation(const std::string&, const TypePtr&, bool, int, Operation::Mode = Operation::Normal);

        InterfaceDeclPtr declaration() const;
        InterfaceList bases() const;
        InterfaceList allBases() const;
        OperationList operations() const;
        OperationList allOperations() const;
        bool isA(const std::string&) const;
        bool hasOperations() const;
        bool inheritsMetaData(const std::string&) const;
        virtual ContainedType containedType() const;
        virtual std::string kindOf() const;
        virtual void visit(ParserVisitor*, bool);

        // Returns the type IDs of all the interfaces in the inheritance tree, in alphabetical order.
        StringList ids() const;

    protected:
        friend class Container;

        InterfaceDeclPtr _declaration;
        InterfaceList _bases;
        bool _hasOperations;
    };

    // ----------------------------------------------------------------------
    // Exception
    // ----------------------------------------------------------------------

    // No inheritance from Constructed, as this is not a Type
    class Exception : public virtual Container, public virtual Contained
    {
    public:
        Exception(const ContainerPtr&, const std::string&, const ExceptionPtr&);
        virtual void destroy();
        DataMemberPtr createDataMember(
            const std::string&,
            const TypePtr&,
            bool,
            int,
            const SyntaxTreeBasePtr&,
            const std::string&,
            const std::string&);
        DataMemberList dataMembers() const;
        DataMemberList orderedOptionalDataMembers() const;
        DataMemberList allDataMembers() const;
        DataMemberList classDataMembers() const;
        DataMemberList allClassDataMembers() const;
        ExceptionPtr base() const;
        ExceptionList allBases() const;
        virtual bool isBaseOf(const ExceptionPtr&) const;
        virtual ContainedType containedType() const;
        bool usesClasses() const;
        bool hasDefaultValues() const;
        bool inheritsMetaData(const std::string&) const;
        bool hasBaseDataMembers() const;
        virtual std::string kindOf() const;
        virtual void visit(ParserVisitor*, bool);

    protected:
        friend class Container;

        ExceptionPtr _base;
    };

    // ----------------------------------------------------------------------
    // Struct
    // ----------------------------------------------------------------------

    class Struct : public virtual Container, public virtual Constructed
    {
    public:
        Struct(const ContainerPtr&, const std::string&);
        DataMemberPtr createDataMember(
            const std::string&,
            const TypePtr&,
            bool,
            int,
            const SyntaxTreeBasePtr&,
            const std::string&,
            const std::string&);
        DataMemberList dataMembers() const;
        DataMemberList classDataMembers() const;
        virtual ContainedType containedType() const;
        virtual bool usesClasses() const;
        virtual size_t minWireSize() const;
        virtual std::string getOptionalFormat() const;
        virtual bool isVariableLength() const;
        bool hasDefaultValues() const;
        virtual std::string kindOf() const;
        virtual void visit(ParserVisitor*, bool);

        friend class Container;
    };

    // ----------------------------------------------------------------------
    // Sequence
    // ----------------------------------------------------------------------

    class Sequence : public virtual Constructed
    {
    public:
        Sequence(const ContainerPtr&, const std::string&, const TypePtr&, const StringList&);
        TypePtr type() const;
        StringList typeMetaData() const;
        virtual ContainedType containedType() const;
        virtual bool usesClasses() const;
        virtual size_t minWireSize() const;
        virtual std::string getOptionalFormat() const;
        virtual bool isVariableLength() const;
        virtual std::string kindOf() const;
        virtual void visit(ParserVisitor*, bool);

    protected:
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
        Dictionary(
            const ContainerPtr&,
            const std::string&,
            const TypePtr&,
            const StringList&,
            const TypePtr&,
            const StringList&);
        TypePtr keyType() const;
        TypePtr valueType() const;
        StringList keyMetaData() const;
        StringList valueMetaData() const;
        virtual ContainedType containedType() const;
        virtual bool usesClasses() const;
        virtual size_t minWireSize() const;
        virtual std::string getOptionalFormat() const;
        virtual bool isVariableLength() const;
        virtual std::string kindOf() const;
        virtual void visit(ParserVisitor*, bool);

        static bool legalKeyType(const TypePtr&);

    protected:
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
        Enum(const ContainerPtr&, const std::string&);
        virtual void destroy();
        bool explicitValue() const;
        int minValue() const;
        int maxValue() const;
        virtual ContainedType containedType() const;
        virtual size_t minWireSize() const;
        virtual std::string getOptionalFormat() const;
        virtual bool isVariableLength() const;
        virtual std::string kindOf() const;
        virtual void visit(ParserVisitor*, bool);

    protected:
        int newEnumerator(const EnumeratorPtr&);

        friend class Container;
        friend class Enumerator;

        bool _explicitValue;
        std::int64_t _minValue;
        std::int64_t _maxValue;
        int _lastValue;
    };

    // ----------------------------------------------------------------------
    // Enumerator
    // ----------------------------------------------------------------------

    class Enumerator : public virtual Contained
    {
    public:
        Enumerator(const ContainerPtr&, const std::string&);
        Enumerator(const ContainerPtr&, const std::string&, int);
        virtual void init();
        EnumPtr type() const;
        virtual ContainedType containedType() const;
        virtual std::string kindOf() const;

        bool explicitValue() const;
        int value() const;

    protected:
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
        Const(
            const ContainerPtr&,
            const std::string&,
            const TypePtr&,
            const StringList&,
            const SyntaxTreeBasePtr&,
            const std::string&,
            const std::string&);
        TypePtr type() const;
        StringList typeMetaData() const;
        SyntaxTreeBasePtr valueType() const;
        std::string value() const;
        std::string literal() const;
        virtual ContainedType containedType() const;
        virtual std::string kindOf() const;
        virtual void visit(ParserVisitor*, bool);

    protected:
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
        ParamDecl(const ContainerPtr&, const std::string&, const TypePtr&, bool, bool, int);
        TypePtr type() const;
        bool isOutParam() const;
        bool optional() const;
        int tag() const;
        virtual ContainedType containedType() const;
        virtual std::string kindOf() const;
        virtual void visit(ParserVisitor*, bool);

    protected:
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
        DataMember(
            const ContainerPtr&,
            const std::string&,
            const TypePtr&,
            bool,
            int,
            const SyntaxTreeBasePtr&,
            const std::string&,
            const std::string&);
        TypePtr type() const;
        bool optional() const;
        int tag() const;
        std::string defaultValue() const;
        std::string defaultLiteral() const;
        SyntaxTreeBasePtr defaultValueType() const;
        virtual ContainedType containedType() const;
        virtual std::string kindOf() const;
        virtual void visit(ParserVisitor*, bool);

    protected:
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
        static UnitPtr createUnit(bool, const StringList& = StringList());

        Unit(bool, const StringList&);

        void setComment(const std::string&);
        void addToComment(const std::string&);
        std::string currentComment(); // Not const, as this function removes the current comment.
        std::string currentFile() const;
        std::string topLevelFile() const;
        int currentLine() const;

        int setCurrentFile(const std::string&, int);
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
        ContainedList findContents(const std::string&) const;

        void addTypeId(int, const std::string&);
        std::string getTypeId(int) const;
        bool hasCompactTypeId() const;

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
        void init();

        bool _all;
        StringList _defaultGlobalMetaData;
        int _errors;
        std::string _currentComment;
        int _currentIncludeLevel;
        std::string _topLevelFile;
        std::stack<DefinitionContextPtr> _definitionContextStack;
        StringList _includeFiles;
        std::stack<ContainerPtr> _containerStack;
        std::map<Builtin::Kind, BuiltinPtr> _builtins;
        std::map<std::string, ContainedList> _contentMap;
        std::map<std::string, DefinitionContextPtr> _definitionContextMap;
        std::map<int, std::string> _typeIds;
        std::map<std::string, std::set<std::string>> _fileTopLevelModules;
    };

    extern Unit* currentUnit; // The current parser for bison/flex
}

#endif
