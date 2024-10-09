// Copyright (c) ZeroC, Inc.

#ifndef SLICE_PARSER_H
#define SLICE_PARSER_H

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
    enum NodeType
    {
        Dummy,
        Real
    };

    //
    // Format to use when marshaling a class instance.
    //
    enum FormatType
    {
        CompactFormat, // Minimal format.
        SlicedFormat   // Full format.
    };

    enum WarningCategory
    {
        All,
        Deprecated,
        InvalidMetadata
    };

    class GrammarBase;
    class SyntaxTreeBase;
    class Metadata;
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

    using GrammarBasePtr = std::shared_ptr<GrammarBase>;
    using SyntaxTreeBasePtr = std::shared_ptr<SyntaxTreeBase>;
    using MetadataPtr = std::shared_ptr<Metadata>;
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

    using StringList = std::list<std::string>;
    using MetadataList = std::list<MetadataPtr>;
    using TypeList = std::list<TypePtr>;
    using ContainedList = std::list<ContainedPtr>;
    using ModuleList = std::list<ModulePtr>;
    using ClassList = std::list<ClassDefPtr>;
    using InterfaceList = std::list<InterfaceDefPtr>;
    using ExceptionList = std::list<ExceptionPtr>;
    using EnumList = std::list<EnumPtr>;
    using OperationList = std::list<OperationPtr>;
    using DataMemberList = std::list<DataMemberPtr>;
    using ParamDeclList = std::list<ParamDeclPtr>;
    using EnumeratorList = std::list<EnumeratorPtr>;

    // ----------------------------------------------------------------------
    // CICompare -- function object to do case-insensitive string comparison.
    // ----------------------------------------------------------------------

    class CICompare
    {
    public:
        bool operator()(const std::string& lhs, const std::string& rhs) const;
    };

    // ----------------------------------------------------------------------
    // DerivedToBaseCompare -- function object to do sort exceptions into
    // most-derived to least-derived order.
    // ----------------------------------------------------------------------

    class DerivedToBaseCompare
    {
    public:
        bool operator()(const ExceptionPtr& lhs, const ExceptionPtr& rhs) const;
    };

    // ----------------------------------------------------------------------
    // ParserVisitor
    // ----------------------------------------------------------------------

    class ParserVisitor
    {
    public:
        virtual ~ParserVisitor() {}
        virtual bool visitUnitStart(const UnitPtr& /*unit*/) { return true; }
        virtual void visitUnitEnd(const UnitPtr& /*unit*/) {}
        virtual bool visitModuleStart(const ModulePtr& /*module*/) { return true; }
        virtual void visitModuleEnd(const ModulePtr& /*module*/) {}
        virtual void visitClassDecl(const ClassDeclPtr& /*classDecl*/) {}
        virtual bool visitClassDefStart(const ClassDefPtr& /*classDef*/) { return true; }
        virtual void visitClassDefEnd(const ClassDefPtr& /*classDef*/) {}
        virtual void visitInterfaceDecl(const InterfaceDeclPtr& /*interfaceDecl*/) {}
        virtual bool visitInterfaceDefStart(const InterfaceDefPtr& /*interfaceDef*/) { return true; }
        virtual void visitInterfaceDefEnd(const InterfaceDefPtr& /*interfaceDef*/) {}
        virtual bool visitExceptionStart(const ExceptionPtr& /*exceptionDef*/) { return true; }
        virtual void visitExceptionEnd(const ExceptionPtr& /*exceptionDef*/) {}
        virtual bool visitStructStart(const StructPtr& /*structDef*/) { return true; }
        virtual void visitStructEnd(const StructPtr& /*structDef*/) {}
        virtual void visitOperation(const OperationPtr& /*operation*/) {}
        virtual void visitParamDecl(const ParamDeclPtr& /*parameter*/) {}
        virtual void visitDataMember(const DataMemberPtr& /*member*/) {}
        virtual void visitSequence(const SequencePtr& /*sequence*/) {}
        virtual void visitDictionary(const DictionaryPtr& /*dictionary*/) {}
        virtual void visitEnum(const EnumPtr& /*enumDef*/) {}
        virtual void visitConst(const ConstPtr& /*constDef*/) {}

        virtual bool shouldVisitIncludedDefinitions() const { return false; }
    };

    // ----------------------------------------------------------------------
    // GrammarBase
    // ----------------------------------------------------------------------

    class GrammarBase
    {
    public:
        virtual ~GrammarBase() = default;
    };

    // ----------------------------------------------------------------------
    // Metadata
    // ----------------------------------------------------------------------

    class Metadata final : public virtual GrammarBase
    {
    public:
        Metadata(const std::string& rawMetadata, const UnitPtr& unit);
        std::string_view directive() const;
        std::string_view arguments() const;

        std::string_view file() const;
        int line() const;

        friend std::ostream& operator<<(std::ostream &out, const Metadata &metadata);

    private:
        /// Parses a metadata string into a pair of (directive, arguments) strings.
        static std::pair<std::string, std::string> parseRawMetadata(const std::string& rawMetadata);

        std::string _directive;
        std::string _arguments;

        std::string _file;
        int _line;
    };

    inline std::ostream& operator<<(std::ostream &ostr, const Metadata &metadata)
    {
        ostr << metadata._directive << ":" << metadata._arguments;
        return ostr;
    }

    // ----------------------------------------------------------------------
    // DefinitionContext
    // ----------------------------------------------------------------------

    class DefinitionContext final
    {
    public:
        DefinitionContext(int includeLevel, const MetadataList& metadata);

        std::string filename() const;
        int includeLevel() const;
        bool seenDefinition() const;

        void setFilename(const std::string& filename);
        void setSeenDefinition();

        MetadataList getMetadata() const;
        void setMetadata(MetadataList metadata);
        bool hasMetadata(std::string_view directive) const;
        std::optional<std::string> getMetadataArgs(std::string_view directive) const;

        /// Emits a warning unless it should be filtered out by a [["suppress-warning"]].
        void warning(WarningCategory category, const std::string& file, int line, const std::string& message) const;

    private:
        bool shouldSuppressWarning(WarningCategory category) const;
        void initSuppressedWarnings();

        int _includeLevel;
        MetadataList _metadata;
        std::string _filename;
        bool _seenDefinition;
        std::set<WarningCategory> _suppressedWarnings;
    };
    using DefinitionContextPtr = std::shared_ptr<DefinitionContext>;

    // ----------------------------------------------------------------------
    // Comment
    // ----------------------------------------------------------------------

    class Comment final
    {
    public:
        bool isDeprecated() const;
        StringList deprecated() const;

        /// Contains all introductory lines up to the first tag.
        StringList overview() const;
        /// Contains unrecognized tags.
        StringList misc() const;
        /// Targets of @see tags.
        StringList seeAlso() const;

        /// Description of an operation's return value.
        StringList returns() const;
        /// Parameter descriptions for an op. Key is parameter name.
        std::map<std::string, StringList> parameters() const;
        /// Exception descriptions for an op. Key is exception name.
        std::map<std::string, StringList> exceptions() const;

    private:
        friend class Contained;

        bool _isDeprecated;
        StringList _deprecated;
        StringList _overview;
        StringList _misc;
        StringList _seeAlso;

        StringList _returns;
        std::map<std::string, StringList> _parameters;
        std::map<std::string, StringList> _exceptions;
    };
    using CommentPtr = std::shared_ptr<Comment>;

    // ----------------------------------------------------------------------
    // SyntaxTreeBase
    // ----------------------------------------------------------------------

    class SyntaxTreeBase : public GrammarBase
    {
    public:
        SyntaxTreeBase(const UnitPtr& unit);
        virtual void destroy();
        UnitPtr unit() const;
        DefinitionContextPtr definitionContext() const; // May be nil
        virtual void visit(ParserVisitor* visitor);

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
        Type(const UnitPtr& unit);
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

    class Builtin final : public virtual Type
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

        Builtin(const UnitPtr& unit, Kind kind);

        std::string typeId() const final;
        bool isClassType() const final;
        size_t minWireSize() const final;
        std::string getOptionalFormat() const final;
        bool isVariableLength() const final;

        bool isNumericType() const;
        bool isIntegralType() const;

        Kind kind() const;
        std::string kindAsString() const;
        static std::optional<Kind> kindFromString(std::string_view str);

        inline static const std::array<std::string, 11> builtinTable =
            {"byte", "bool", "short", "int", "long", "float", "double", "string", "Object", "Object*", "Value"};

    private:
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
        CommentPtr parseComment(bool stripMarkup) const;
        CommentPtr parseComment(const std::string& text, bool stripMarkup) const;

        int includeLevel() const;

        MetadataList getMetadata() const;
        void setMetadata(MetadataList metadata);
        bool hasMetadata(std::string_view directive) const;
        std::optional<std::string> getMetadataArgs(std::string_view directive) const;

        std::optional<FormatType> parseFormatMetadata() const;

        /// Returns true if this item is deprecated, due to the presence of 'deprecated' metadata.
        /// @return True if this item has 'deprecated' metadata on it, false otherwise.
        bool isDeprecated() const;

        /// If this item is deprecated, return its deprecation message (if present).
        /// This is the string argument that can be optionally provided with 'deprecated' metadata.
        /// @return The message provided to the 'deprecated' metadata, if present.
        std::optional<std::string> getDeprecationReason() const;

        virtual std::string kindOf() const = 0;

    protected:
        Contained(const ContainerPtr& container, const std::string& name);

        ContainerPtr _container;
        std::string _name;
        std::string _scoped;
        std::string _file;
        int _line;
        std::string _comment;
        int _includeLevel;
        MetadataList _metadata;
    };

    // ----------------------------------------------------------------------
    // Container
    // ----------------------------------------------------------------------

    class Container : public virtual SyntaxTreeBase, public std::enable_shared_from_this<Container>
    {
    public:
        Container(const UnitPtr& unit);
        void destroy() override;
        ModulePtr createModule(const std::string& name);
        ClassDefPtr createClassDef(const std::string& name, int id, const ClassDefPtr& base);
        ClassDeclPtr createClassDecl(const std::string& name);
        InterfaceDefPtr createInterfaceDef(const std::string& name, const InterfaceList& bases);
        InterfaceDeclPtr createInterfaceDecl(const std::string& name);
        ExceptionPtr createException(const std::string& name, const ExceptionPtr& base, NodeType nodeType = Real);
        StructPtr createStruct(const std::string& name, NodeType nodeType = Real);
        SequencePtr createSequence(
            const std::string& name,
            const TypePtr& type,
            MetadataList metadata,
            NodeType nodeType = Real);
        DictionaryPtr createDictionary(
            const std::string& name,
            const TypePtr& keyType,
            MetadataList keyMetadata,
            const TypePtr& valueType,
            MetadataList valueMetadata,
            NodeType nodeType = Real);
        EnumPtr createEnum(const std::string& name, NodeType nodeType = Real);
        ConstPtr createConst(
            const std::string name,
            const TypePtr& constType,
            MetadataList metadata,
            const SyntaxTreeBasePtr& valueType,
            const std::string& value,
            NodeType nodeType = Real);
        TypeList lookupType(const std::string& identifier);
        TypeList lookupTypeNoBuiltin(const std::string& identifier, bool emitErrors, bool ignoreUndefined = false);
        ContainedList lookupContained(const std::string& identifier, bool emitErrors);
        ExceptionPtr lookupException(const std::string& identifier, bool emitErrors);
        UnitPtr unit() const;
        ModuleList modules() const;
        ClassList classes() const;
        InterfaceList interfaces() const;
        ExceptionList exceptions() const;
        EnumList enums() const;
        EnumeratorList enumerators() const;
        EnumeratorList enumerators(const std::string& identifier) const;
        ContainedList contents() const;
        std::string thisScope() const;
        void visit(ParserVisitor* visitor) override;

        bool checkIntroduced(const std::string& scopedName, ContainedPtr namedThing = 0);

        /// Returns true if this container is the global scope (ie. it's of type `Unit`), and false otherwise.
        /// If false, we emit an error message. So this function should only be called for types which cannot appear at
        /// global scope... so everything except for `Module`s.
        bool checkForGlobalDefinition(const char* definitionKindPlural);

        /// Returns true if this contains elements of the specified type.
        /// This check is recursive, so it will still return true even if the type is only contained indirectly.
        template<typename T> bool contains() const
        {
            for (const auto& p : contents())
            {
                if (std::dynamic_pointer_cast<T>(p))
                {
                    return true;
                }
                ContainerPtr childContainer = std::dynamic_pointer_cast<Container>(p);
                if (childContainer && childContainer->contains<T>())
                {
                    return true;
                }
            }
            return false;
        }

    protected:
        bool validateConstant(
            const std::string& name,
            const TypePtr& type,
            SyntaxTreeBasePtr& valueType,
            const std::string& valueString,
            bool isConstant);

        ContainedList _contents;
        std::map<std::string, ContainedPtr, CICompare> _introducedMap;
    };

    // ----------------------------------------------------------------------
    // Module
    // ----------------------------------------------------------------------

    class Module final : public virtual Container, public virtual Contained
    {
    public:
        Module(const ContainerPtr& container, const std::string& name);
        std::string kindOf() const final;
        void visit(ParserVisitor* visitor) final;
    };

    // ----------------------------------------------------------------------
    // Constructed
    // ----------------------------------------------------------------------

    class Constructed : public virtual Type, public virtual Contained
    {
    public:
        Constructed(const ContainerPtr& container, const std::string& name);
        std::string typeId() const override;
    };

    // ----------------------------------------------------------------------
    // ClassDecl
    // ----------------------------------------------------------------------

    class ClassDecl final : public virtual Constructed, public std::enable_shared_from_this<ClassDecl>
    {
    public:
        ClassDecl(const ContainerPtr& container, const std::string& name);
        void destroy() final;
        ClassDefPtr definition() const;
        bool isClassType() const final;
        size_t minWireSize() const final;
        std::string getOptionalFormat() const final;
        bool isVariableLength() const final;
        void visit(ParserVisitor* visitor) final;
        std::string kindOf() const final;

    private:
        friend class Container;

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
    class ClassDef final : public virtual Container, public virtual Contained
    {
    public:
        ClassDef(const ContainerPtr& container, const std::string& name, int id, const ClassDefPtr& base);
        void destroy() final;
        DataMemberPtr createDataMember(
            const std::string& name,
            const TypePtr& type,
            bool isOptional,
            int tag,
            const SyntaxTreeBasePtr& defaultValueType,
            const std::string& defaultValueString);
        ClassDeclPtr declaration() const;
        ClassDefPtr base() const;
        ClassList allBases() const;
        DataMemberList dataMembers() const;
        DataMemberList orderedOptionalDataMembers() const;
        DataMemberList allDataMembers() const;
        DataMemberList classDataMembers() const;
        DataMemberList allClassDataMembers() const;
        bool canBeCyclic() const;
        bool inheritsMetadata(string_view directive) const;
        bool hasBaseDataMembers() const;
        void visit(ParserVisitor* visitor) final;
        int compactId() const;
        std::string kindOf() const final;

    private:
        friend class Container;

        ClassDeclPtr _declaration;
        ClassDefPtr _base;
        int _compactId;
    };

    // ----------------------------------------------------------------------
    // InterfaceDecl
    // ----------------------------------------------------------------------

    class InterfaceDecl final : public virtual Constructed, public std::enable_shared_from_this<InterfaceDecl>
    {
    public:
        InterfaceDecl(const ContainerPtr& container, const std::string& name);
        void destroy() final;
        InterfaceDefPtr definition() const;
        size_t minWireSize() const final;
        std::string getOptionalFormat() const final;
        bool isVariableLength() const final;
        void visit(ParserVisitor* visitor) final;
        std::string kindOf() const final;

        static void checkBasesAreLegal(const std::string& name, const InterfaceList& bases, const UnitPtr& unit);

    private:
        friend class Container;

        InterfaceDefPtr _definition;

        typedef std::list<InterfaceList> GraphPartitionList;
        typedef std::list<StringList> StringPartitionList;

        static bool isInList(const GraphPartitionList& gpl, const InterfaceDefPtr& interfaceDef);

        static void addPartition(
            GraphPartitionList& partitions,
            GraphPartitionList::reverse_iterator tail,
            const InterfaceDefPtr& base);
        static StringPartitionList toStringPartitionList(const GraphPartitionList& partitions);
        static void checkPairIntersections(
            const StringPartitionList& stringPartitions,
            const std::string& name,
            const UnitPtr& unit);
    };

    // ----------------------------------------------------------------------
    // Operation
    // ----------------------------------------------------------------------

    class Operation final : public virtual Contained, public virtual Container
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

        ParamDeclPtr
        createParamDecl(const std::string& name, const TypePtr& type, bool isOutParam, bool isOptional, int tag);

        ParamDeclList parameters() const;
        ParamDeclList inParameters() const;
        void inParameters(ParamDeclList& required, ParamDeclList& optional) const;
        ParamDeclList outParameters() const;
        void outParameters(ParamDeclList& required, ParamDeclList& optional) const;
        ExceptionList throws() const;
        void setExceptionList(const ExceptionList& exceptions);
        bool sendsClasses() const;
        bool returnsClasses() const;
        bool returnsData() const;
        bool returnsMultipleValues() const;
        bool sendsOptionals() const;
        std::optional<FormatType> format() const;
        std::string kindOf() const final;
        void visit(ParserVisitor* visitor) final;

    private:
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
    class InterfaceDef final : public virtual Container, public virtual Contained
    {
    public:
        InterfaceDef(const ContainerPtr& container, const std::string& name, const InterfaceList& bases);
        void destroy() final;
        OperationPtr createOperation(
            const std::string& name,
            const TypePtr& returnType,
            bool isOptional,
            int tag,
            Operation::Mode mode = Operation::Normal);

        InterfaceDeclPtr declaration() const;
        InterfaceList bases() const;
        InterfaceList allBases() const;
        OperationList operations() const;
        OperationList allOperations() const;
        bool hasOperations() const;
        bool inheritsMetadata(string_view directive) const;
        std::string kindOf() const final;
        void visit(ParserVisitor* visitor) final;

        // Returns the type IDs of all the interfaces in the inheritance tree, in alphabetical order.
        StringList ids() const;

    private:
        friend class Container;

        InterfaceDeclPtr _declaration;
        InterfaceList _bases;
        bool _hasOperations;
    };

    // ----------------------------------------------------------------------
    // Exception
    // ----------------------------------------------------------------------

    // No inheritance from Constructed, as this is not a Type
    class Exception final : public virtual Container, public virtual Contained
    {
    public:
        Exception(const ContainerPtr& container, const std::string& name, const ExceptionPtr& base);
        void destroy() final;
        DataMemberPtr createDataMember(
            const std::string& name,
            const TypePtr& type,
            bool isOptional,
            int tag,
            const SyntaxTreeBasePtr& defaultValueType,
            const std::string& defaultValueString);
        DataMemberList dataMembers() const;
        DataMemberList orderedOptionalDataMembers() const;
        DataMemberList allDataMembers() const;
        DataMemberList classDataMembers() const;
        DataMemberList allClassDataMembers() const;
        ExceptionPtr base() const;
        ExceptionList allBases() const;
        bool isBaseOf(const ExceptionPtr& otherException) const;
        bool usesClasses() const;
        bool inheritsMetadata(string_view metadata) const;
        bool hasBaseDataMembers() const;
        std::string kindOf() const final;
        void visit(ParserVisitor* visitor) final;

    private:
        ExceptionPtr _base;
    };

    // ----------------------------------------------------------------------
    // Struct
    // ----------------------------------------------------------------------

    class Struct final : public virtual Container, public virtual Constructed
    {
    public:
        Struct(const ContainerPtr& container, const std::string& name);
        DataMemberPtr createDataMember(
            const std::string& name,
            const TypePtr& type,
            bool isOptional,
            int tag,
            const SyntaxTreeBasePtr& defaultValueType,
            const std::string& defaultValueString);
        DataMemberList dataMembers() const;
        DataMemberList classDataMembers() const;
        bool usesClasses() const final;
        size_t minWireSize() const final;
        std::string getOptionalFormat() const final;
        bool isVariableLength() const final;
        std::string kindOf() const final;
        void visit(ParserVisitor* visitor) final;
    };

    // ----------------------------------------------------------------------
    // Sequence
    // ----------------------------------------------------------------------

    class Sequence final : public virtual Constructed, public std::enable_shared_from_this<Sequence>
    {
    public:
        Sequence(
            const ContainerPtr& container,
            const std::string& name,
            const TypePtr& type,
            MetadataList typeMetadata);
        TypePtr type() const;
        MetadataList typeMetadata() const;
        bool usesClasses() const final;
        size_t minWireSize() const final;
        std::string getOptionalFormat() const final;
        bool isVariableLength() const final;
        std::string kindOf() const final;
        void visit(ParserVisitor* visitor) final;

    private:
        TypePtr _type;
        MetadataList _typeMetadata;
    };

    // ----------------------------------------------------------------------
    // Dictionary
    // ----------------------------------------------------------------------

    class Dictionary final : public virtual Constructed, public std::enable_shared_from_this<Dictionary>
    {
    public:
        Dictionary(
            const ContainerPtr& container,
            const std::string& name,
            const TypePtr& keyType,
            MetadataList keyMetadata,
            const TypePtr& valueType,
            MetadataList valueMetadata);
        TypePtr keyType() const;
        TypePtr valueType() const;
        MetadataList keyMetadata() const;
        MetadataList valueMetadata() const;
        bool usesClasses() const final;
        size_t minWireSize() const final;
        std::string getOptionalFormat() const final;
        bool isVariableLength() const final;
        std::string kindOf() const final;
        void visit(ParserVisitor* visitor) final;

        static bool isLegalKeyType(const TypePtr& type);

    private:
        TypePtr _keyType;
        TypePtr _valueType;
        MetadataList _keyMetadata;
        MetadataList _valueMetadata;
    };

    // ----------------------------------------------------------------------
    // Enum
    // ----------------------------------------------------------------------

    class Enum final : public virtual Container, public virtual Constructed
    {
    public:
        Enum(const ContainerPtr& container, const std::string& name);
        void destroy() final;
        EnumeratorPtr createEnumerator(const std::string& name, std::optional<int> explicitValue);
        bool hasExplicitValues() const;
        int minValue() const;
        int maxValue() const;
        size_t minWireSize() const final;
        std::string getOptionalFormat() const final;
        bool isVariableLength() const final;
        std::string kindOf() const final;
        void visit(ParserVisitor* visitor) final;

    private:
        bool _hasExplicitValues;
        std::int64_t _minValue;
        std::int64_t _maxValue;
        int _lastValue;
    };

    // ----------------------------------------------------------------------
    // Enumerator
    // ----------------------------------------------------------------------

    class Enumerator final : public virtual Contained
    {
    public:
        Enumerator(const ContainerPtr& container, const std::string& name, int value, bool hasExplicitValue);
        EnumPtr type() const;
        std::string kindOf() const final;

        bool hasExplicitValue() const;
        int value() const;

    private:
        bool _hasExplicitValue;
        int _value;
    };

    // ----------------------------------------------------------------------
    // Const
    // ----------------------------------------------------------------------

    class Const final : public virtual Contained, public std::enable_shared_from_this<Const>
    {
    public:
        Const(
            const ContainerPtr& container,
            const std::string& name,
            const TypePtr& type,
            MetadataList typeMetadata,
            const SyntaxTreeBasePtr& valueType,
            const std::string& valueString);
        TypePtr type() const;
        MetadataList typeMetadata() const;
        SyntaxTreeBasePtr valueType() const;
        std::string value() const;
        std::string kindOf() const final;
        void visit(ParserVisitor* visitor) final;

    private:
        TypePtr _type;
        MetadataList _typeMetadata;
        SyntaxTreeBasePtr _valueType;
        std::string _value;
    };

    // ----------------------------------------------------------------------
    // ParamDecl
    // ----------------------------------------------------------------------

    class ParamDecl final : public virtual Contained, public std::enable_shared_from_this<ParamDecl>
    {
    public:
        ParamDecl(
            const ContainerPtr& container,
            const std::string& name,
            const TypePtr& type,
            bool isOutParam,
            bool isOptional,
            int tag);
        TypePtr type() const;
        bool isOutParam() const;
        bool optional() const;
        int tag() const;
        std::string kindOf() const final;
        void visit(ParserVisitor* visitor) final;

    private:
        TypePtr _type;
        bool _isOutParam;
        bool _optional;
        int _tag;
    };

    // ----------------------------------------------------------------------
    // DataMember
    // ----------------------------------------------------------------------

    class DataMember final : public virtual Contained, public std::enable_shared_from_this<DataMember>
    {
    public:
        DataMember(
            const ContainerPtr& container,
            const std::string& name,
            const TypePtr& type,
            bool isOptional,
            int tag,
            const SyntaxTreeBasePtr& defaultValueType,
            const std::string& defaultValueString);
        TypePtr type() const;
        bool optional() const;
        int tag() const;
        std::string defaultValue() const;
        SyntaxTreeBasePtr defaultValueType() const;
        std::string kindOf() const final;
        void visit(ParserVisitor* visitor) final;

    private:
        TypePtr _type;
        bool _optional;
        int _tag;
        SyntaxTreeBasePtr _defaultValueType;
        std::string _defaultValue;
    };

    // ----------------------------------------------------------------------
    // Unit
    // ----------------------------------------------------------------------

    class Unit final : public virtual Container
    {
    public:
        static UnitPtr createUnit(bool all, const StringList& defaultFileMetadata = StringList());

        void setComment(const std::string& comment);
        void addToComment(const std::string& comment);
        std::string currentComment(); // Not const, as this function removes the current comment.
        std::string currentFile() const;
        std::string topLevelFile() const;
        int currentLine() const;

        int setCurrentFile(const std::string& currentFile, int lineNumber);
        int currentIncludeLevel() const;

        void addFileMetadata(const MetadataList& metadata);

        void setSeenDefinition();

        void error(const std::string& message);
        void error(const std::string& file, int line, const std::string& message);
        void warning(WarningCategory category, const std::string& message) const;

        ContainerPtr currentContainer() const;
        void pushContainer(const ContainerPtr& container);
        void popContainer();

        DefinitionContextPtr currentDefinitionContext() const;
        DefinitionContextPtr findDefinitionContext(const std::string& file) const;

        void addContent(const ContainedPtr& contained);
        ContainedList findContents(const std::string& scopedName) const;

        void addTypeId(int compactId, const std::string& typeId);
        std::string getTypeId(int compactId) const;

        // Returns the path names of the files included directly by the top-level file.
        StringList includeFiles() const;

        // Returns the path names of all files parsed by this unit.
        StringList allFiles() const;

        int parse(const std::string& filename, FILE* file, bool debugMode);

        void destroy() final;
        void visit(ParserVisitor* visitor) final;

        // Not const, as builtins are created on the fly. (Lazy initialization.)
        BuiltinPtr createBuiltin(Builtin::Kind kind);

        void addTopLevelModule(const std::string& file, const std::string& module);
        std::set<std::string> getTopLevelModules(const std::string& file) const;

    private:
        Unit(bool all, const MetadataList& defaultFileMetadata);

        void pushDefinitionContext();
        void popDefinitionContext();

        bool _all;
        MetadataList _defaultFileMetadata;
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
