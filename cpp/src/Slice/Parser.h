// Copyright (c) ZeroC, Inc.

#ifndef SLICE_PARSER_H
#define SLICE_PARSER_H

#include <array>
#include <cstdint>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <ostream>
#include <set>
#include <stack>
#include <stdio.h>
#include <string>
#include <string_view>

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
        InvalidMetadata,
        InvalidComment
    };

    class GrammarBase;
    class SyntaxTreeBase;
    class Metadata;
    class Type;
    class Builtin;
    class Contained;
    class Container;
    class Module;
    class ClassDecl;
    class ClassDef;
    class InterfaceDecl;
    class InterfaceDef;
    class Exception;
    class Struct;
    class Operation;
    class Parameter;
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
    using ClassDeclPtr = std::shared_ptr<ClassDecl>;
    using ClassDefPtr = std::shared_ptr<ClassDef>;
    using InterfaceDeclPtr = std::shared_ptr<InterfaceDecl>;
    using InterfaceDefPtr = std::shared_ptr<InterfaceDef>;
    using ExceptionPtr = std::shared_ptr<Exception>;
    using StructPtr = std::shared_ptr<Struct>;
    using OperationPtr = std::shared_ptr<Operation>;
    using ParameterPtr = std::shared_ptr<Parameter>;
    using DataMemberPtr = std::shared_ptr<DataMember>;
    using SequencePtr = std::shared_ptr<Sequence>;
    using DictionaryPtr = std::shared_ptr<Dictionary>;
    using EnumPtr = std::shared_ptr<Enum>;
    using EnumeratorPtr = std::shared_ptr<Enumerator>;
    using ConstPtr = std::shared_ptr<Const>;
    using UnitPtr = std::shared_ptr<Unit>;

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
    using ParameterList = std::list<ParameterPtr>;
    using EnumeratorList = std::list<EnumeratorPtr>;

    bool containedCompare(const ContainedPtr& lhs, const ContainedPtr& rhs);
    bool containedEqual(const ContainedPtr& lhs, const ContainedPtr& rhs);

    template<typename T> bool compareTag(const T& lhs, const T& rhs) { return lhs->tag() < rhs->tag(); }

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
        virtual ~ParserVisitor() = default;
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
        virtual void visitParameter(const ParameterPtr& /*parameter*/) {}
        virtual void visitDataMember(const DataMemberPtr& /*member*/) {}
        virtual void visitSequence(const SequencePtr& /*sequence*/) {}
        virtual void visitDictionary(const DictionaryPtr& /*dictionary*/) {}
        virtual void visitEnum(const EnumPtr& /*enumDef*/) {}
        virtual void visitConst(const ConstPtr& /*constDef*/) {}

        [[nodiscard]] virtual bool shouldVisitIncludedDefinitions() const { return false; }
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

    class Metadata final : public GrammarBase
    {
    public:
        Metadata(std::string rawMetadata, std::string file, int line);
        [[nodiscard]] const std::string& directive() const;
        [[nodiscard]] const std::string& arguments() const;

        [[nodiscard]] const std::string& file() const;
        [[nodiscard]] int line() const;

        friend std::ostream& operator<<(std::ostream& out, const Metadata& metadata);

    private:
        std::string _directive;
        std::string _arguments;

        std::string _file;
        int _line;
    };

    inline std::ostream& operator<<(std::ostream& ostr, const Metadata& metadata)
    {
        ostr << metadata._directive;
        if (!metadata._arguments.empty())
        {
            ostr << ":" << metadata._arguments;
        }
        return ostr;
    }

    // ----------------------------------------------------------------------
    // DefinitionContext
    // ----------------------------------------------------------------------

    class DefinitionContext final
    {
    public:
        DefinitionContext(int includeLevel);

        /// Returns the file name of the definition context, as reported by the preprocessor.
        /// This may be an absolute or relative path.
        ///
        /// @return A reference to the file name.
        [[nodiscard]] const std::string& filename() const;

        /// Returns the resolved file name of the definition context. This is always an absolute path.
        ///
        /// @return A reference to the resolved file name.
        [[nodiscard]] const std::string& resolvedFilename() const;
        [[nodiscard]] int includeLevel() const;
        [[nodiscard]] bool seenDefinition() const;

        void setFilename(std::string filename, std::string resolvedFilename);
        void setSeenDefinition();

        [[nodiscard]] const MetadataList& getMetadata() const;
        void setMetadata(MetadataList metadata);
        void appendMetadata(MetadataList metadata);
        [[nodiscard]] bool hasMetadata(std::string_view directive) const;
        [[nodiscard]] std::optional<std::string> getMetadataArgs(std::string_view directive) const;

        [[nodiscard]] bool isSuppressed(WarningCategory category) const;

    private:
        void initSuppressedWarnings();

        int _includeLevel;
        MetadataList _metadata;
        std::string _filename;
        std::string _resolvedFilename;
        bool _seenDefinition{false};
        std::set<WarningCategory> _suppressedWarnings;
    };
    using DefinitionContextPtr = std::shared_ptr<DefinitionContext>;

    // ----------------------------------------------------------------------
    // DocComment
    // ----------------------------------------------------------------------

    /// Functions of this type are used to map link tags into each language's link syntax.
    /// In Slice, links are of the form: '{@link <rawLink>}'.
    ///
    /// The first argument (`rawLink`) is the raw link text, taken verbatim from the doc-comment.
    /// The second argument (`source`) is a pointer to the Slice element that the doc comment (and link) are written on.
    /// The third argument (`target`) is a pointer to the Slice element that is being linked to.
    /// If the parser could not resolve the link, this will be `nullptr`.
    ///
    /// This function should return the fully formatted link, which will replace the entire '{@link <rawLink>}'
    /// in a raw doc-comment.
    using DocLinkFormatter =
        std::string (*)(const std::string& rawLink, const ContainedPtr& source, const SyntaxTreeBasePtr& target);

    class DocCommentParser;

    class DocComment final
    {
    public:
        /// Returns an unparsed 'DocComment' instance containing the provided string.
        /// If the provided string is empty or only contains whitespace, this returns 'nullopt' instead.
        [[nodiscard]] static std::optional<DocComment> createUnparsed(const std::string& rawDocComment);

        /// Parses this doc-comment in-place, using the provided formatter to map language-specific elements.
        /// @param p The Slice element that this doc-comment is applied to.
        /// @param linkFormatter This function is used to replace '{@link ...}' tags with their corresponding
        ///                      representation in a target language.
        /// @remark This function must be called on a doc-comment before it's usable for code-gen purposes.
        void parse(const ContainedPtr& p, const DocLinkFormatter& linkFormatter);

        [[nodiscard]] bool isDeprecated() const;
        [[nodiscard]] const StringList& deprecated() const;

        /// Contains all introductory lines up to the first tag.
        [[nodiscard]] const StringList& overview() const;
        /// Returns the contents of any '@remark' or '@remarks' tags.
        [[nodiscard]] const StringList& remarks() const;
        /// Targets of '@see' tags.
        [[nodiscard]] const StringList& seeAlso() const;

        /// Description of an operation's return value.
        [[nodiscard]] const StringList& returns() const;
        /// Parameter descriptions for an op. Key is parameter name.
        [[nodiscard]] const std::map<std::string, StringList>& parameters() const;
        /// Exception descriptions for an op. Key is exception name.
        [[nodiscard]] const std::map<std::string, StringList>& exceptions() const;

    private:
        StringList _rawDocCommentLines;

        bool _isDeprecated{false};
        StringList _deprecated;
        StringList _overview;
        StringList _remarks;
        StringList _seeAlso;

        StringList _returns;
        std::map<std::string, StringList> _parameters;
        std::map<std::string, StringList> _exceptions;
    };

    // ----------------------------------------------------------------------
    // SyntaxTreeBase
    // ----------------------------------------------------------------------

    class SyntaxTreeBase : public GrammarBase
    {
    public:
        [[nodiscard]] virtual UnitPtr unit() const = 0;
    };

    // ----------------------------------------------------------------------
    // Type
    // ----------------------------------------------------------------------

    class Type : public virtual SyntaxTreeBase
    {
    public:
        [[nodiscard]] virtual bool isClassType() const;
        [[nodiscard]] virtual bool usesClasses() const;
        [[nodiscard]] virtual size_t minWireSize() const = 0;
        [[nodiscard]] virtual std::string getOptionalFormat() const = 0;
        [[nodiscard]] virtual bool isVariableLength() const = 0;
    };

    // ----------------------------------------------------------------------
    // Builtin
    // ----------------------------------------------------------------------

    class Builtin final : public Type
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
            KindObjectProxy,
            KindValue
        };

        Builtin(UnitPtr unit, Kind kind);
        void destroy();
        [[nodiscard]] UnitPtr unit() const final;

        [[nodiscard]] bool isClassType() const final;
        [[nodiscard]] size_t minWireSize() const final;
        [[nodiscard]] std::string getOptionalFormat() const final;
        [[nodiscard]] bool isVariableLength() const final;

        [[nodiscard]] bool isNumericType() const;
        [[nodiscard]] bool isIntegralType() const;

        [[nodiscard]] Kind kind() const;
        [[nodiscard]] std::string kindAsString() const;
        static std::optional<Kind> kindFromString(std::string_view str);

        // NOLINTNEXTLINE(cert-err58-cpp)
        inline static const std::array<std::string, 10> builtinTable =
            {"byte", "bool", "short", "int", "long", "float", "double", "string", "Object*", "Value"};

    private:
        const Kind _kind;
        UnitPtr _unit;
    };

    // ----------------------------------------------------------------------
    // Contained
    // ----------------------------------------------------------------------

    class Contained : public virtual SyntaxTreeBase
    {
    public:
        virtual void destroy() {}
        [[nodiscard]] ContainerPtr container() const;
        [[nodiscard]] bool isTopLevel() const;
        [[nodiscard]] virtual ModulePtr getTopLevelModule() const;

        // Functions for getting the Slice name of an element.

        /// Returns the Slice identifier of this element.
        [[nodiscard]] std::string name() const;

        /// Returns the Slice scope that this element is contained within (with a trailing '::').
        [[nodiscard]] std::string scope() const;

        /// Returns the fully-scoped Slice identifier of this element (equivalent to `scope() + name()`).
        [[nodiscard]] std::string scoped() const;

        // Functions for getting the mapped name of an element.

        /// Returns the mapped identifier that this element will use in the target language.
        [[nodiscard]] std::string mappedName() const;

        /// Returns the mapped scope that this element will be generated in in the target language.
        /// @param separator This string will be used to separate scope segments.
        /// @param leading If true, the returned string will have a leading `separator` (defaults to false).
        /// @remark Equivalent to `mappedScope(separator, leading) + mappedName()`.
        [[nodiscard]] std::string mappedScoped(const std::string& separator, bool leading = false) const;

        /// Returns the mapped fully-scoped identifier that this element will use in the target language.
        /// @param separator This string will be used to separate scope segments.
        /// @param leading If true, the returned string will have a leading `separator` (defaults to false).
        [[nodiscard]] std::string mappedScope(const std::string& separator, bool leading = false) const;

        // Functions to get information about where a Slice element was defined at.

        [[nodiscard]] const std::string& file() const;
        [[nodiscard]] int line() const;

        [[nodiscard]] const std::optional<DocComment>& docComment() const;

        [[nodiscard]] int includeLevel() const;
        [[nodiscard]] DefinitionContextPtr definitionContext() const;
        [[nodiscard]] UnitPtr unit() const final;

        [[nodiscard]] virtual const MetadataList& getMetadata() const;
        virtual void setMetadata(MetadataList metadata);
        virtual void appendMetadata(MetadataList metadata);
        [[nodiscard]] bool hasMetadata(std::string_view directive) const;
        [[nodiscard]] std::optional<std::string> getMetadataArgs(std::string_view directive) const;

        [[nodiscard]] std::optional<FormatType> parseFormatMetadata() const;

        /// Returns true if this item is deprecated, due to the presence of 'deprecated' metadata.
        /// @return `true` if this item has 'deprecated' metadata on it, `false` otherwise.
        [[nodiscard]] bool isDeprecated() const;

        /// If this item is deprecated, return its deprecation message (if present).
        /// This is the string argument that can be optionally provided with 'deprecated' metadata.
        /// @return The message provided to the 'deprecated' metadata, if present.
        [[nodiscard]] std::optional<std::string> getDeprecationReason() const;

        virtual void visit(ParserVisitor* visitor) = 0;
        [[nodiscard]] virtual std::string kindOf() const = 0;

    protected:
        friend class DocCommentParser;

        Contained(const ContainerPtr& container, std::string name);

        ContainerPtr _container;
        std::string _name;
        std::string _file;
        int _line;
        std::optional<DocComment> _docComment;
        int _includeLevel;
        DefinitionContextPtr _definitionContext;
        MetadataList _metadata;
    };

    // ----------------------------------------------------------------------
    // Container
    // ----------------------------------------------------------------------

    class Container : public virtual SyntaxTreeBase, public std::enable_shared_from_this<Container>
    {
    public:
        void destroyContents();
        ModulePtr createModule(const std::string& name, bool nestedSyntax);
        [[nodiscard]] ClassDefPtr createClassDef(const std::string& name, std::int32_t id, const ClassDefPtr& base);
        [[nodiscard]] ClassDeclPtr createClassDecl(const std::string& name);
        [[nodiscard]] InterfaceDefPtr createInterfaceDef(const std::string& name, const InterfaceList& bases);
        [[nodiscard]] InterfaceDeclPtr createInterfaceDecl(const std::string& name);
        [[nodiscard]] ExceptionPtr
        createException(const std::string& name, const ExceptionPtr& base, NodeType nodeType = Real);
        [[nodiscard]] StructPtr createStruct(const std::string& name, NodeType nodeType = Real);
        [[nodiscard]] SequencePtr createSequence(const std::string& name, const TypePtr& type, MetadataList metadata);
        [[nodiscard]] DictionaryPtr createDictionary(
            const std::string& name,
            const TypePtr& keyType,
            MetadataList keyMetadata,
            const TypePtr& valueType,
            MetadataList valueMetadata);
        [[nodiscard]] EnumPtr createEnum(const std::string& name);
        [[nodiscard]] ConstPtr createConst(
            const std::string& name,
            const TypePtr& constType,
            MetadataList metadata,
            const SyntaxTreeBasePtr& valueType,
            const std::string& value);
        [[nodiscard]] TypeList lookupType(const std::string& identifier);
        [[nodiscard]] TypeList
        lookupTypeNoBuiltin(const std::string& identifier, bool emitErrors, bool ignoreUndefined = false);
        [[nodiscard]] ContainedList lookupContained(const std::string& identifier, bool emitErrors);
        [[nodiscard]] InterfaceDefPtr lookupInterfaceDef(const std::string& identifier, bool emitErrors);
        [[nodiscard]] ExceptionPtr lookupException(const std::string& identifier, bool emitErrors);
        [[nodiscard]] ModuleList modules() const;
        [[nodiscard]] InterfaceList interfaces() const;
        [[nodiscard]] EnumList enums() const;
        [[nodiscard]] EnumeratorList enumerators() const;
        [[nodiscard]] EnumeratorList enumerators(const std::string& identifier) const;
        [[nodiscard]] ContainedList contents() const;
        void visitContents(ParserVisitor* visitor);

        /// Reports an error when a Slice definition would retroactively change the meaning of an identifier.
        /// For example:
        /// ```
        /// module Outer {
        ///     const int H = 12;
        ///     module Inner {
        ///         const int G = H;
        ///         class H {}       // error: retroactively changes the meaning of 'H'
        ///     }
        /// }
        /// ```
        /// Because the parser parses in order, the meaning of 'H' in 'Inner' is ordering dependent, which is not good.
        /// The user should disambiguate by qualifying the type-reference as 'Outer::H'.
        void checkHasChangedMeaning(const std::string& name, ContainedPtr namedThing = nullptr);

        /// Returns `true` if this container is the global scope (i.e. it's of type `Unit`), and `false` otherwise.
        /// If false, we emit an error message. So this function should only be called for types which cannot appear at
        /// global scope... so everything except for `Module`s.
        bool checkForGlobalDefinition(const char* definitionKindPlural);

        /// Returns true if this contains elements of the specified type.
        /// This check is recursive, so it will still return true even if the type is only contained indirectly.
        template<typename T> [[nodiscard]] bool contains() const
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

        [[nodiscard]] std::string thisScope() const;

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

    class Module final : public Container, public Contained
    {
    public:
        Module(const ContainerPtr& container, const std::string& name, bool nestedSyntax);
        [[nodiscard]] std::string kindOf() const final;
        [[nodiscard]] ModulePtr getTopLevelModule() const final;
        void visit(ParserVisitor* visitor) final;
        void destroy() final;

        const bool usesNestedSyntax;
    };

    // ----------------------------------------------------------------------
    // ClassDecl
    // ----------------------------------------------------------------------

    class ClassDecl final : public Contained, public Type, public std::enable_shared_from_this<ClassDecl>
    {
    public:
        ClassDecl(const ContainerPtr& container, const std::string& name);
        void destroy() final;
        [[nodiscard]] ClassDefPtr definition() const;
        [[nodiscard]] bool isClassType() const final;
        [[nodiscard]] size_t minWireSize() const final;
        [[nodiscard]] std::string getOptionalFormat() const final;
        [[nodiscard]] bool isVariableLength() const final;
        void visit(ParserVisitor* visitor) final;
        [[nodiscard]] std::string kindOf() const final;

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
    // so if you need the class as a "type", use the
    // declaration() operation to navigate to the class declaration.
    //
    class ClassDef final : public Container, public Contained
    {
    public:
        ClassDef(const ContainerPtr& container, const std::string& name, std::int32_t id, ClassDefPtr base);
        void destroy() final;
        DataMemberPtr createDataMember(
            const std::string& name,
            const TypePtr& type,
            bool isOptional,
            std::int32_t tag,
            SyntaxTreeBasePtr defaultValueType,
            std::optional<std::string> defaultValueString);
        [[nodiscard]] ClassDeclPtr declaration() const;
        [[nodiscard]] ClassDefPtr base() const;
        [[nodiscard]] ClassList allBases() const;
        [[nodiscard]] DataMemberList dataMembers() const;
        [[nodiscard]] DataMemberList orderedOptionalDataMembers() const;
        [[nodiscard]] DataMemberList allDataMembers() const;
        [[nodiscard]] DataMemberList classDataMembers() const;
        [[nodiscard]] bool canBeCyclic() const;
        void visit(ParserVisitor* visitor) final;
        [[nodiscard]] std::int32_t compactId() const;
        [[nodiscard]] std::string kindOf() const final;

        // Class metadata is always stored on the underlying decl type, not the definition.
        // So we override these `xMetadata` functions to forward to `_declarations->xMetadata()` instead.
        [[nodiscard]] const MetadataList& getMetadata() const final;
        void setMetadata(MetadataList metadata) final;
        void appendMetadata(MetadataList metadata) final;

    private:
        friend class Container;

        ClassDeclPtr _declaration;
        ClassDefPtr _base;
        std::int32_t _compactId;
    };

    // ----------------------------------------------------------------------
    // InterfaceDecl
    // ----------------------------------------------------------------------

    class InterfaceDecl final : public Contained, public Type, public std::enable_shared_from_this<InterfaceDecl>
    {
    public:
        InterfaceDecl(const ContainerPtr& container, const std::string& name);
        void destroy() final;
        [[nodiscard]] InterfaceDefPtr definition() const;
        [[nodiscard]] size_t minWireSize() const final;
        [[nodiscard]] std::string getOptionalFormat() const final;
        [[nodiscard]] bool isVariableLength() const final;
        void visit(ParserVisitor* visitor) final;
        [[nodiscard]] std::string kindOf() const final;

        static void checkBasesAreLegal(const std::string& name, const InterfaceList& bases, const UnitPtr& unit);

    private:
        friend class Container;

        InterfaceDefPtr _definition;

        using GraphPartitionList = std::list<InterfaceList>;
        using StringPartitionList = std::list<StringList>;

        static bool isInList(const GraphPartitionList& gpl, const InterfaceDefPtr& interfaceDef);

        static void addPartition(
            GraphPartitionList& partitions,
            const GraphPartitionList::reverse_iterator& tail,
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

    class Operation final : public Container, public Contained
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

        Operation(const ContainerPtr&, const std::string&, TypePtr, bool, std::int32_t, Mode);
        [[nodiscard]] InterfaceDefPtr interface() const;
        [[nodiscard]] TypePtr returnType() const;
        [[nodiscard]] bool returnIsOptional() const;
        [[nodiscard]] std::int32_t returnTag() const;
        [[nodiscard]] Mode mode() const;
        [[nodiscard]] bool hasMarshaledResult() const;

        ParameterPtr createParameter(const std::string& name, const TypePtr& type, bool isOptional, std::int32_t tag);

        /// Returns all parameters of this operation.
        [[nodiscard]] ParameterList parameters() const;

        /// Returns a list of all this operation's in-parameters (all parameters not marked with 'out').
        [[nodiscard]] ParameterList inParameters() const;

        /// Returns all of this operation's in-parameters sorted in this order: '(required..., optional...)'.
        /// Required parameters are kept in definition order and optional parameters are sorted by tag.
        [[nodiscard]] ParameterList sortedInParameters() const;

        /// Returns a list of all this operation's out-parameters (all parameters marked with 'out').
        [[nodiscard]] ParameterList outParameters() const;

        /// Creates a temporary Parameter that represents this operation's return value.
        /// @param name The name of the return parameter.
        /// @return A dummy Parameter that represents this operation's return value, or nullptr if this operation is
        /// void.
        /// @remark The returned Parameter is not "saved" by this operation - it's completely ephemeral.
        /// The provided name is not checked for uniqueness: the caller must ensure it's unique enough for its purpose.
        /// Furthermore, there is no way to specify a lang:identifier metadata for this parameter, so the mapped
        /// parameter name is always the default (i.e., simply name).
        [[nodiscard]] ParameterPtr returnParameter(const std::string& name);

        /// Returns this operation's out parameters and return type sorted in this order: '(required..., optional...)'.
        /// If the this operation's return type is non-void and non-optional, it is at the end of the 'required' list.
        /// Otherwise, required parameters are kept in definition order and optional parameters are sorted by tag.
        ///
        /// For convenience, non-void return types are represented by a dummy `Parameter` in this list.
        /// However it's important to note that it is not _actually_ a parameter.
        ///
        // Creating this temporary Parameter doesn't introduce cycles, since nothing from the AST points to it,
        // even if it points back into the AST. So it will be destroyed when the returned list goes out of scope.
        /// @param returnName The name of the return parameter. See #returnParameter for more details.
        /// @return A list of all this operation's out-parameters and return type, sorted in the order described above.
        [[nodiscard]] ParameterList sortedReturnAndOutParameters(const std::string& returnName);

        [[nodiscard]] ExceptionList throws() const;
        void setExceptionList(const ExceptionList& exceptions);
        [[nodiscard]] bool sendsClasses() const;
        [[nodiscard]] bool returnsClasses() const;
        [[nodiscard]] bool returnsData() const;

        /// Returns true if this operation has any out parameters or a non-void return type.
        [[nodiscard]] bool returnsAnyValues() const;
        /// Returns true if this operation's output parameters, plus any non-void return type, is greater than 1.
        [[nodiscard]] bool returnsMultipleValues() const;

        [[nodiscard]] bool sendsOptionals() const;
        [[nodiscard]] bool receivesOptionals() const;
        [[nodiscard]] std::optional<FormatType> format() const;
        [[nodiscard]] std::string kindOf() const final;
        void visit(ParserVisitor* visitor) final;
        void destroy() final;

    private:
        TypePtr _returnType;
        bool _returnIsOptional;
        std::int32_t _returnTag;
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
    // so if you need the interface as a "type", use the
    // declaration() function to navigate to the interface declaration.
    //
    class InterfaceDef final : public Container, public Contained
    {
    public:
        InterfaceDef(const ContainerPtr& container, const std::string& name, InterfaceList bases);
        void destroy() final;
        OperationPtr createOperation(
            const std::string& name,
            const TypePtr& returnType,
            bool isOptional,
            std::int32_t tag,
            Operation::Mode mode = Operation::Normal);

        [[nodiscard]] InterfaceDeclPtr declaration() const;
        [[nodiscard]] InterfaceList bases() const;
        [[nodiscard]] InterfaceList allBases() const;
        [[nodiscard]] OperationList operations() const;
        [[nodiscard]] OperationList allOperations() const;
        [[nodiscard]] std::string kindOf() const final;
        void visit(ParserVisitor* visitor) final;

        // Returns the type IDs of all the interfaces in the inheritance tree, in alphabetical order.
        [[nodiscard]] StringList ids() const;

        // Interface metadata is always stored on the underlying decl type, not the definition.
        // So we override these `xMetadata` functions to forward to `_declarations->xMetadata()` instead.
        [[nodiscard]] const MetadataList& getMetadata() const final;
        void setMetadata(MetadataList metadata) final;
        void appendMetadata(MetadataList metadata) final;

    private:
        friend class Container;

        // Returns true if name does not collide with any base name; otherwise, false.
        bool checkBaseOperationNames(const std::string& name, const std::vector<std::string>& baseNames);

        InterfaceDeclPtr _declaration;
        InterfaceList _bases;
    };

    // ----------------------------------------------------------------------
    // Exception
    // ----------------------------------------------------------------------
    class Exception final : public Container, public Contained
    {
    public:
        Exception(const ContainerPtr& container, const std::string& name, ExceptionPtr base);
        void destroy() final;
        DataMemberPtr createDataMember(
            const std::string& name,
            const TypePtr& type,
            bool isOptional,
            std::int32_t tag,
            SyntaxTreeBasePtr defaultValueType,
            std::optional<std::string> defaultValueString);
        [[nodiscard]] DataMemberList dataMembers() const;
        [[nodiscard]] DataMemberList orderedOptionalDataMembers() const;
        [[nodiscard]] DataMemberList allDataMembers() const;
        [[nodiscard]] DataMemberList classDataMembers() const;
        [[nodiscard]] ExceptionPtr base() const;
        [[nodiscard]] ExceptionList allBases() const;
        [[nodiscard]] bool isBaseOf(const ExceptionPtr& otherException) const;
        [[nodiscard]] bool usesClasses() const;
        [[nodiscard]] std::string kindOf() const final;
        void visit(ParserVisitor* visitor) final;

    private:
        ExceptionPtr _base;
    };

    // ----------------------------------------------------------------------
    // Struct
    // ----------------------------------------------------------------------

    class Struct final : public Container, public Contained, public Type
    {
    public:
        Struct(const ContainerPtr& container, const std::string& name);
        DataMemberPtr createDataMember(
            const std::string& name,
            const TypePtr& type,
            bool isOptional,
            std::int32_t tag,
            SyntaxTreeBasePtr defaultValueType,
            std::optional<std::string> defaultValueString);
        [[nodiscard]] DataMemberList dataMembers() const;
        [[nodiscard]] DataMemberList classDataMembers() const;
        [[nodiscard]] bool usesClasses() const final;
        [[nodiscard]] size_t minWireSize() const final;
        [[nodiscard]] std::string getOptionalFormat() const final;
        [[nodiscard]] bool isVariableLength() const final;
        [[nodiscard]] std::string kindOf() const final;
        void visit(ParserVisitor* visitor) final;
        void destroy() final;
    };

    // ----------------------------------------------------------------------
    // Sequence
    // ----------------------------------------------------------------------

    class Sequence final : public Contained, public Type, public std::enable_shared_from_this<Sequence>
    {
    public:
        Sequence(const ContainerPtr& container, const std::string& name, TypePtr type, MetadataList typeMetadata);
        [[nodiscard]] TypePtr type() const;
        [[nodiscard]] MetadataList typeMetadata() const;
        void setTypeMetadata(MetadataList metadata);
        [[nodiscard]] bool usesClasses() const final;
        [[nodiscard]] size_t minWireSize() const final;
        [[nodiscard]] std::string getOptionalFormat() const final;
        [[nodiscard]] bool isVariableLength() const final;
        [[nodiscard]] std::string kindOf() const final;
        void visit(ParserVisitor* visitor) final;

    private:
        TypePtr _type;
        MetadataList _typeMetadata;
    };

    // ----------------------------------------------------------------------
    // Dictionary
    // ----------------------------------------------------------------------

    class Dictionary final : public Contained, public Type, public std::enable_shared_from_this<Dictionary>
    {
    public:
        Dictionary(
            const ContainerPtr& container,
            const std::string& name,
            TypePtr keyType,
            MetadataList keyMetadata,
            TypePtr valueType,
            MetadataList valueMetadata);
        [[nodiscard]] TypePtr keyType() const;
        [[nodiscard]] TypePtr valueType() const;
        [[nodiscard]] MetadataList keyMetadata() const;
        [[nodiscard]] MetadataList valueMetadata() const;
        void setKeyMetadata(MetadataList metadata);
        void setValueMetadata(MetadataList metadata);
        [[nodiscard]] bool usesClasses() const final;
        [[nodiscard]] size_t minWireSize() const final;
        [[nodiscard]] std::string getOptionalFormat() const final;
        [[nodiscard]] bool isVariableLength() const final;
        [[nodiscard]] std::string kindOf() const final;
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

    class Enum final : public Container, public Contained, public Type
    {
    public:
        Enum(const ContainerPtr& container, const std::string& name);
        EnumeratorPtr createEnumerator(const std::string& name, std::optional<std::int32_t> explicitValue);
        [[nodiscard]] bool hasExplicitValues() const;
        [[nodiscard]] std::int32_t minValue() const;
        [[nodiscard]] std::int32_t maxValue() const;
        [[nodiscard]] size_t minWireSize() const final;
        [[nodiscard]] std::string getOptionalFormat() const final;
        [[nodiscard]] bool isVariableLength() const final;
        [[nodiscard]] std::string kindOf() const final;
        void visit(ParserVisitor* visitor) final;
        void destroy() final;

    private:
        bool _hasExplicitValues{false};
        std::int32_t _minValue;
        std::int32_t _maxValue{0};
        std::int32_t _lastValue{-1};
    };

    // ----------------------------------------------------------------------
    // Enumerator
    // ----------------------------------------------------------------------

    class Enumerator final : public Contained
    {
    public:
        Enumerator(const ContainerPtr& container, const std::string& name, std::int32_t value, bool hasExplicitValue);
        [[nodiscard]] EnumPtr type() const;
        [[nodiscard]] std::string kindOf() const final;

        [[nodiscard]] bool hasExplicitValue() const;
        [[nodiscard]] std::int32_t value() const;

        void visit(ParserVisitor* visitor) final;

    private:
        bool _hasExplicitValue;
        std::int32_t _value;
    };

    // ----------------------------------------------------------------------
    // Const
    // ----------------------------------------------------------------------

    class Const final : public Contained, public std::enable_shared_from_this<Const>
    {
    public:
        Const(
            const ContainerPtr& container,
            const std::string& name,
            TypePtr type,
            MetadataList typeMetadata,
            SyntaxTreeBasePtr valueType,
            std::string valueString);
        [[nodiscard]] TypePtr type() const;
        [[nodiscard]] MetadataList typeMetadata() const;
        void setTypeMetadata(MetadataList metadata);
        [[nodiscard]] SyntaxTreeBasePtr valueType() const;
        [[nodiscard]] std::string value() const;
        [[nodiscard]] std::string kindOf() const final;
        void visit(ParserVisitor* visitor) final;

    private:
        TypePtr _type;
        MetadataList _typeMetadata;
        SyntaxTreeBasePtr _valueType;
        std::string _value;
    };

    // ----------------------------------------------------------------------
    // Parameter
    // ----------------------------------------------------------------------

    class Parameter final : public Contained, public std::enable_shared_from_this<Parameter>
    {
    public:
        Parameter(
            const ContainerPtr& container,
            const std::string& name,
            TypePtr type,
            bool isOptional,
            std::int32_t tag);
        [[nodiscard]] TypePtr type() const;
        [[nodiscard]] bool isOutParam() const;
        void setIsOutParam();
        [[nodiscard]] bool optional() const;
        [[nodiscard]] std::int32_t tag() const;
        [[nodiscard]] std::string kindOf() const final;
        void visit(ParserVisitor* visitor) final;

    private:
        TypePtr _type;
        bool _isOutParam{false};
        bool _optional;
        std::int32_t _tag;
    };

    // ----------------------------------------------------------------------
    // DataMember
    // ----------------------------------------------------------------------

    class DataMember final : public Contained, public std::enable_shared_from_this<DataMember>
    {
    public:
        DataMember(
            const ContainerPtr& container,
            const std::string& name,
            TypePtr type,
            bool isOptional,
            std::int32_t tag,
            SyntaxTreeBasePtr defaultValueType,
            std::optional<std::string> defaultValueString);
        [[nodiscard]] TypePtr type() const;
        [[nodiscard]] bool optional() const;
        [[nodiscard]] std::int32_t tag() const;

        // defaultValue() and defaultValueType() are either both null (no default value) or both non-null.
        [[nodiscard]] std::optional<std::string> defaultValue() const;
        [[nodiscard]] SyntaxTreeBasePtr defaultValueType() const;
        [[nodiscard]] std::string kindOf() const final;
        void visit(ParserVisitor* visitor) final;

    private:
        TypePtr _type;
        bool _optional;
        std::int32_t _tag;
        SyntaxTreeBasePtr _defaultValueType;
        std::optional<std::string> _defaultValue;
    };

    // ----------------------------------------------------------------------
    // Unit
    // ----------------------------------------------------------------------

    class Unit final : public Container
    {
    public:
        static UnitPtr createUnit(std::string languageName, bool all);

        Unit(std::string languageName, bool all);

        [[nodiscard]] std::string languageName() const;

        /// Sets `_currentDocComment` to the provided string, erasing anything currently stored in it.
        /// @param comment The raw comment string. It can span multiple lines and include comment formatting characters
        ///     like '/**' and leading '*'. All such comment formatting will be removed by this function though.
        // Note that this is only called for JavaDoc style comments, where the comment is set in one-shot.
        void setDocComment(const std::string& comment);
        /// Appends the provided comment to `_currentDocComment` with a newline separator.
        /// @param comment The raw comment string. It shouldn't contain newlines, nor any comment formatting characters.
        ///     Unlike `setDocComment`, this function doesn't process the comment string in any way.
        // Note that this is only called for triple-slash style comments, where each line is added separately.
        void appendToDocComment(const std::string& comment);
        /// Returns the current doc-comment string. This function uses move semantics. After calling it, the currently
        /// stored doc-comment will be cleared. This string can span multiple lines, and won't include formatting like
        /// leading '///' or opening '/**' etc.
        std::string currentDocComment();

        [[nodiscard]] std::string currentFile() const;
        [[nodiscard]] const std::string& topLevelFile() const;
        [[nodiscard]] int currentLine() const;

        int setCurrentFile(std::string currentFile, int lineNumber);
        [[nodiscard]] int currentIncludeLevel() const;

        void addFileMetadata(MetadataList metadata);

        void setSeenDefinition();

        void error(std::string_view message);
        void error(std::string_view file, int line, std::string_view message);
        void warning(WarningCategory category, std::string_view message) const;
        void warning(std::string_view file, int line, WarningCategory category, std::string_view message) const;

        [[nodiscard]] ContainerPtr currentContainer() const;
        void pushContainer(const ContainerPtr& container);
        void popContainer();

        [[nodiscard]] DefinitionContextPtr currentDefinitionContext() const;

        /// Returns the definition context for the given file, or nullptr if none exists.
        /// This method only checks `DefinitionContext::filename` and does not consider
        /// `DefinitionContext::resolvedFilename`.
        ///
        /// @param file The file name to search for.
        /// @return A pointer to the matching definition context, or nullptr if not found.
        [[nodiscard]] DefinitionContextPtr findDefinitionContext(std::string_view file) const;

        void addContent(const ContainedPtr& contained);
        [[nodiscard]] ContainedList findContents(const std::string& scopedName) const;

        void addTypeId(std::int32_t compactId, const std::string& typeId);
        [[nodiscard]] std::string getTypeId(std::int32_t compactId) const;

        // Returns the path names of the files included directly by the top-level file.
        [[nodiscard]] StringList includeFiles() const;

        /// Returns the absolute paths of all files parsed by this unit.
        /// The first entry is the top-level file, followed by its direct and transitive dependencies in the order they
        /// were parsed.
        ///
        /// @return A list of absolute paths to the parsed files.
        [[nodiscard]] StringList allFiles() const;

        int parse(const std::string& filename, FILE* file, bool debugMode);

        void destroy();
        void visit(ParserVisitor* visitor);
        [[nodiscard]] UnitPtr unit() const final;

        /// Returns `EXIT_FAILURE` if 1 or more errors have been reported and `EXIT_SUCCESS` otherwise.
        int getStatus() const;

        // Not const, as builtins are created on the fly. (Lazy initialization.)
        BuiltinPtr createBuiltin(Builtin::Kind kind);

        void addTopLevelModule(const std::string& file, const std::string& module);
        [[nodiscard]] std::set<std::string> getTopLevelModules(const std::string& file) const;

    private:
        void pushDefinitionContext();
        void popDefinitionContext();

        const std::string _languageName;
        bool _all;
        int _errors{0};
        std::string _currentDocComment;
        int _currentIncludeLevel{0};
        std::string _topLevelFile;
        std::stack<DefinitionContextPtr> _definitionContextStack;
        StringList _includeFiles;
        std::stack<ContainerPtr> _containerStack;
        std::map<Builtin::Kind, BuiltinPtr> _builtins;
        std::map<std::string, ContainedList> _contentMap;
        std::map<std::string, DefinitionContextPtr, std::less<>> _definitionContextMap;
        std::map<std::int32_t, std::string> _typeIds;
        std::map<std::string, std::set<std::string>> _fileTopLevelModules;
        StringList _allFiles;
    };

    extern Unit* currentUnit; // The current parser for bison/flex
}

#endif
