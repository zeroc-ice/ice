// Copyright (c) ZeroC, Inc.

#ifndef SLICE_PYTHON_UTIL_H
#define SLICE_PYTHON_UTIL_H

#include "../Ice/OutputUtil.h"
#include "../Slice/Parser.h"

#include <map>
#include <set>
#include <string>

namespace Slice::Python
{
    // The kind of method being documented or generated.
    enum MethodKind
    {
        SyncInvocation,
        AsyncInvocation,
        Dispatch
    };

    /// Represents the scope of an import statement—either required at runtime or only used for type hints.
    enum ImportScope
    {
        /// The import is required at runtime by the generated Python code.
        RuntimeImport,

        /// The import is only used for type hints and is not needed at runtime.
        TypingImport
    };

    // The context a type will be used in.
    enum TypeContext
    {
        // If the type is an interface, it is used as a servant (base type).
        Servant,
        // If the type is an interface, it is used as a proxy (base type or parameter).
        Proxy,
        // A field of a class or struct
        Field,
        // A parameter
        Parameter,
        // A return type or out parameter
        Return
    };

    using ModuleImportsMap = std::map<std::string, std::set<std::pair<std::string, std::string>>>;

    // Maps import statements per generated Python module.
    // - Key: the generated module name, e.g., "Ice.Locator_ice" (we generate one Python module per each unique Slice
    // module in a Slice file).
    // - Value: a map from imported module name to the set of pairs representing the imported name and its alias.
    using ImportsMap = std::map<std::string, ModuleImportsMap>;

    /// Represents a Python code fragment generated for a Slice definition.
    struct PythonCodeFragment
    {
        /// The Slice file name from which this code fragment was generated.
        std::string sliceFileName;

        /// The Python module for the generated code.
        std::string moduleName;

        /// The Python file name where this code fragment will be written.
        std::string fileName;

        /// Whether this code fragment is a package index file.
        bool isPackageIndex = false;

        /// The generated code.
        std::string code;
    };

    /// Determines the mapped package for a given Slice definition.
    /// @param p The Slice definition to get the mapped package for.
    /// @param packageSeparator Use this character as the separator between package segments.
    /// @return The mapped package name, with the specified separator.
    std::string getMappedPackage(const SyntaxTreeBasePtr& p, char packageSeparator = '.');

    /// Returns the fully qualified name of the Python module that corresponds to the given Slice definition.
    ///
    /// Each Slice module is mapped to a Python package with the same name, but with "::" replaced by ".".
    /// Within that package, each Slice definition is mapped to a Python module with the same name as the definition.
    ///
    /// For example:
    /// - A Slice definition named `Baz` in the module `::Bar::Foo` is mapped to the Python module `"Bar.Foo.Baz"`.
    ///
    /// @param p The Slice definition to map to a Python module.
    /// @return The fully qualified Python module name corresponding to the Slice definition.
    std::string getPythonModuleForDefinition(const SyntaxTreeBasePtr& p);

    /// Returns the fully qualified name of the Python module where the given Slice definition is forward-declared.
    ///
    /// Forward declarations are generated only for classes and interfaces. The corresponding Python module name
    /// is the same as the definition module returned by `getPythonModuleForDefinition`, with an "F" appended to the
    /// end.
    ///
    /// For example, the forward declaration of the class `Bar::MyClass` is placed in the module `"Bar.MyClassF"`.
    ///
    /// @param p The Slice definition to get the forward declaration module name for. Must be a class or interface.
    /// @return The fully qualified Python module name for the forward declaration.
    std::string getPythonModuleForForwardDeclaration(const SyntaxTreeBasePtr& p);

    /// Returns the alias used for importing the given Slice definition in Python.
    ///
    /// The alias follows the pattern `"M1_M2_Xxx"`, where:
    /// - `M1_M2` is the mapped scope of the definition, using underscores (`_`) as separators instead of `::`.
    /// - `Xxx` is the mapped name of the definition (typically the class or interface name).
    ///
    /// If the definition represents an interface, the suffix `"Prx"` is appended to the alias to refer to the proxy
    /// type.
    ///
    /// @param source The Slice definition that is importing the given definition.
    /// @param p The Slice definition to get the Python import alias for.
    /// @return The alias to use when importing the given definition in generated Python code.
    std::string getImportAlias(
        const ContainedPtr& source,
        const std::map<std::string, std::string>& allImports,
        const SyntaxTreeBasePtr& p);

    std::string getImportAlias(
        const ContainedPtr& source,
        const std::map<std::string, std::string>& allImports,
        const std::string& scope,
        const std::string& name);

    /// Gets the name used for the meta-type of the given Slice definition. IcePy creates a meta-type for each Slice
    /// type. The generated code uses these meta-types to call IcePy.
    /// @param p The Slice definition to get the meta-type name for.
    /// @return The name of the meta-type for the given Slice definition.
    std::string getMetaType(const SyntaxTreeBasePtr& p);

    /// Helper method to emit the generated code that format the fields of a type in __repr__ implementation.
    std::string formatFields(const DataMemberList& members);

    /// Checks if the given Slice type corresponds to non-optional type which can be used as default value in Python.
    /// This is really anything that is not a Python dataclass, sequence, or dictionary type. Slice classes and
    /// interfaces are always mapped as optional.
    bool canBeUsedAsDefaultValue(const TypePtr& type);

    PythonCodeFragment createCodeFragmentForPythonModule(const ContainedPtr& contained, const std::string& code);

    // Get a list of all definitions exported for the Python module corresponding to the given Slice definition.
    std::vector<std::string> getAll(const ContainedPtr& definition);

    class BufferedOutputBase
    {
    protected:
        std::ostringstream _outBuffer;
    };

    /// A output class that writes to a stream, used by the CodeVisitor to write Python code fragments.
    class BufferedOutput final : public BufferedOutputBase, public IceInternal::Output
    {
    public:
        BufferedOutput() : Output(_outBuffer) {}

        /// Returns the string representation of the buffered output.
        /// @return the string containing the buffered output.
        std::string str() const { return _outBuffer.str(); }
    };

    /// Creates the package directory structure for a given Python module name.
    ///
    /// For example, if the module name is "Foo.Bar.Baz", this function creates
    /// the directories "Foo/Bar" under the specified output path.
    ///
    /// @param moduleName The name of the Python module (e.g., "Foo.Bar.Baz").
    /// @param outputPath The base directory in which to create the package directories. Must already exist.
    void createPackagePath(const std::string& moduleName, const std::string& outputPath);

    /// Writes the standard header comment to a generated Python source file.
    ///
    /// @param out The output stream to write the header to.
    void writeHeader(IceInternal::Output& out);

    /// Write the package index that exports the given imports.
    void writePackageIndex(const std::map<std::string, std::set<std::string>>& imports, IceInternal::Output& out);

    std::vector<PythonCodeFragment>
    dynamicCompile(const std::vector<std::string>& files, const std::vector<std::string>& cppArgs, bool debug);

    int staticCompile(const std::vector<std::string>& files);

    /// Generate Python code for a translation unit.
    /// @param unit is the Slice unit to generate code for.
    /// @param outputDir The base-directory to write the generated Python files to.
    void generate(const Slice::UnitPtr& unit, const std::string& outputDir);

    /// Returns a DocString formatted link to the provided Slice identifier.
    std::string
    pyLinkFormatter(const std::string& rawLink, const ContainedPtr& source, const SyntaxTreeBasePtr& target);

    /// Validates the Slice Python metadata for the given unit.
    void validatePythonMetadata(const UnitPtr&);

    /// Gets a map with all the import names for the given source module. The map key is the imported definition name,
    /// and the value is the Python module name where the definition is imported from.
    /// @param sourceModule The name of the source module.
    /// @param runtimeImports The map of runtime imports. This map contains the runtime imports for each generated
    /// Python module.
    /// @param typingImports The map of typing imports. This map contains the typing imports for each generated
    /// Python module.
    /// @return A map with all the import names for the given source module.
    std::map<std::string, std::string>
    getAllImports(const std::string& sourceModule, const ImportsMap& runtimeImports, const ImportsMap& typingImports);

    // Collects Python definitions for each generated Python package.
    // Each package corresponds to a Slice module and includes an `__init__.py` file
    // that re-exports selected definitions from the package's internal modules.
    class PackageVisitor final : public ParserVisitor
    {
    public:
        bool visitModuleStart(const ModulePtr&) final;
        bool visitClassDefStart(const ClassDefPtr&) final;
        bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
        bool visitStructStart(const StructPtr&) final;
        bool visitExceptionStart(const ExceptionPtr&) final;
        void visitSequence(const SequencePtr&) final;
        void visitDictionary(const DictionaryPtr&) final;
        void visitEnum(const EnumPtr&) final;
        void visitConst(const ConstPtr&) final;

        [[nodiscard]] const std::map<std::string, std::map<std::string, std::set<std::string>>>& imports()
        {
            return _imports;
        }
        [[nodiscard]] const std::set<std::string>& generatedModules() const { return _generatedModules; }
        [[nodiscard]] const std::set<std::string>& packageIndexFiles() const { return _packageIndexFiles; }

    private:
        void addRuntimeImport(const ContainedPtr& definition, const std::string& prefix = "");
        void addRuntimeImportForMetaType(const ContainedPtr& definition);

        // A map where:
        // - The outer key is the generated Python package name (e.g., "Foo" for the Slice module ::Foo).
        // - The inner key is the generated Python module name (e.g., "Bar" for the Slice class ::Foo::Bar).
        // - The value is the set of symbol names that the generated module contributes to the package.
        //   For example, the generated module for the Slice class "Bar" contributes the symbols "Bar" and
        //   "__Foo_Bar_t", corresponding to the class itself and its associated meta-type.
        std::map<std::string, std::map<std::string, std::set<std::string>>> _imports;

        // The set of generated Python modules.
        std::set<std::string> _generatedModules;

        // The se of generated package index files.
        std::set<std::string> _packageIndexFiles;
    };

    // Collect the import statements required by each generated Python module.
    class ImportVisitor final : public ParserVisitor
    {
    public:
        bool visitClassDefStart(const ClassDefPtr&) final;
        bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
        bool visitStructStart(const StructPtr&) final;
        bool visitExceptionStart(const ExceptionPtr&) final;
        void visitSequence(const SequencePtr&) final;
        void visitDictionary(const DictionaryPtr&) final;
        void visitEnum(const EnumPtr&) final;
        void visitConst(const ConstPtr&) final;

        [[nodiscard]] const ImportsMap& getRuntimeImports() const { return _runtimeImports; }

        [[nodiscard]] const ImportsMap& getTypingImports() const { return _typingImports; }

        [[nodiscard]] const std::map<std::string, std::map<std::string, std::string>> getAllImportNames() const
        {
            return _allImports;
        }

    private:
        void visitDataMembers(const ContainedPtr&, const std::list<DataMemberPtr>&);

        /// Adds a runtime import for the given Slice definition if it comes from a different module.
        /// @param definition is the Slice definition to import.
        /// @param source is the Slice definition that requires the import.
        void addRuntimeImport(
            const SyntaxTreeBasePtr& definition,
            const ContainedPtr& source,
            TypeContext typeContext = Proxy);

        /// Adds a runtime import for the given definition from the specified Python module.
        ///
        /// @param moduleName The fully qualified name of the Python module to import from.
        /// @param definition A pair consisting of the name and alias to use for the imported symbol.
        ///                   If the alias is empty, the name is used as the alias.
        /// @param source The Slice definition that requires this import.
        void addRuntimeImport(const std::string& moduleName, const std::string& definition, const ContainedPtr& source);

        /// Adds a typing import for the given definition from the specified Python module.
        ///
        /// Typing imports are generated inside an `if TYPE_CHECKING:` block, so they are only used for type hints.
        ///
        /// @param moduleName The fully qualified name of the Python module to import from.
        /// @param definition The definition to import, represented as a pair of name and alias.
        /// @param source The Slice definition that requires this import.
        void addTypingImport(const std::string& moduleName, const std::string& definition, const ContainedPtr& source);

        /// Adds a typing import for the package containing the given Slice definition.
        ///
        /// Typing imports are generated inside an `if TYPE_CHECKING:` block, so they are only used for type hints.
        ///
        /// @param definition The definition to import the containing package.
        /// @param source The Slice definition that requires this import.
        /// @param forMarshaling If true, the sequence is used for marshaling (invocation input parameter, or dispatch
        /// output parameter).
        void
        addTypingImport(const SyntaxTreeBasePtr& definition, const ContainedPtr& source, bool forMarshaling = false);

        /// Adds a typing import for the given package.
        ///
        /// Typing imports are generated inside an `if TYPE_CHECKING:` block, so they are only used for type hints.
        ///
        /// @param packageName The name of the package to import.
        /// @param source The Slice definition that requires this import.
        void addTypingImport(const std::string& packageName, const ContainedPtr& source);

        /// Import the meta type for the given Slice definition if it comes from a different module.
        /// @param definition is the Slice definition to import.
        /// @param source is the Slice definition that requires the import.
        void addRuntimeImportForMetaType(const SyntaxTreeBasePtr& definition, const ContainedPtr& source);

        ImportsMap _runtimeImports;
        ImportsMap _typingImports;

        /// A map of all import names for each source module.
        /// - Key: the Python generated module name.
        /// - Value: a map where the key is the imported definition name or alias, and the value is the Python
        /// module name where the definition is imported from.
        std::map<std::string, std::map<std::string, std::string>> _allImports;
    };

    // CodeVisitor generates the Python mapping for a translation unit.
    class CodeVisitor final : public ParserVisitor
    {
    public:
        CodeVisitor(
            ImportsMap runtimeImports,
            ImportsMap typingImports,
            std::map<std::string, std::map<std::string, std::string>> allImports)
            : _runtimeImports(std::move(runtimeImports)),
              _typingImports(std::move(typingImports)),
              _allImports(std::move(allImports))
        {
        }

        bool visitClassDefStart(const ClassDefPtr&) final;
        bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
        bool visitExceptionStart(const ExceptionPtr&) final;
        bool visitStructStart(const StructPtr&) final;
        void visitSequence(const SequencePtr&) final;
        void visitDictionary(const DictionaryPtr&) final;
        void visitEnum(const EnumPtr&) final;
        void visitConst(const ConstPtr&) final;

        [[nodiscard]] const std::vector<PythonCodeFragment>& codeFragments() const { return _codeFragments; }

    private:
        /// Returns a string representation of the type hint for the given Slice type.
        /// @param type The Slice type to convert to a type hint string.
        /// @param optional If true, the type hint will indicate that the type is optional (i.e., it can be `None`).
        /// @param source The Slice definition requesting the type hint.
        /// @param forMarshaling If true, the type is used for marshaling (invocation input parameter, or dispatch
        /// output parameter).
        /// @return The string representation of the type hint for the given Slice type.
        std::string
        typeToTypeHintString(const TypePtr& type, bool optional, const ContainedPtr& source, bool forMarshaling);

        /// Returns a string representation of the return type hint for the given operation.
        /// @param operation The Slice operation to get the return type hint for.
        /// @param methodKind The kind of method being documented or generated (sync, async, or dispatch).
        /// @return The string representation of the return type hint for the given operation.
        std::string returnTypeHint(const OperationPtr& operation, MethodKind methodKind);

        /// Returns a string representation of the operation's return type hint. This is the same as `returnTypeHint`,
        /// but with the " -> " prefix for use in function signatures.
        /// @param operation The Slice operation to get the return type hint for.
        /// @param methodKind The kind of method being documented or generated (sync, async, or dispatch).
        /// @return The string representation of the operation's return type hint, prefixed with " -> ".
        std::string operationReturnTypeHint(const OperationPtr& operation, MethodKind methodKind);

        // Emit Python code for operations
        void writeOperations(const InterfaceDefPtr&, IceInternal::Output&);

        /// Get the default value for initializing a given type.
        /// @param source The Slice definition that is initializing the type.
        /// @param member The data member to initialize.
        /// @param forConstructor If true, the initialization is for a constructor parameter, otherwise it is for a
        /// dataclass field.
        std::string getTypeInitializer(const ContainedPtr& source, const DataMemberPtr& member, bool forConstructor);

        // Write Python metadata as a tuple.
        void writeMetadata(const MetadataList&, IceInternal::Output&);

        // Write the __repr__ method for a generated class or exception.
        void writeRepr(const ContainedPtr& contained, const DataMemberList& members, IceInternal::Output& out);

        // Write the data members meta-info for a meta type declaration.
        void writeMetaTypeDataMembers(
            const ContainedPtr& contained,
            const DataMemberList& members,
            IceInternal::Output& out);

        // Write a member assignment statement for a constructor.
        void writeAssign(const ContainedPtr& source, const DataMemberPtr& member, IceInternal::Output& out);

        // Write a constant value.
        void writeConstantValue(
            const ContainedPtr& source,
            const TypePtr&,
            const SyntaxTreeBasePtr&,
            const std::string&,
            IceInternal::Output&);

        /// Write constructor parameters with default values.
        void writeConstructorParams(const ContainedPtr& source, const DataMemberList& members, IceInternal::Output&);

        /// Writes the provided @p remarks in its own subheading in the current comment (if @p remarks is non-empty).
        void writeRemarksDocComment(const StringList& remarks, bool needsNewline, IceInternal::Output& out);

        void writeDocstring(const std::optional<DocComment>&, const std::string&, IceInternal::Output&);
        void writeDocstring(const std::optional<DocComment>&, const DataMemberList&, IceInternal::Output&);
        void writeDocstring(const std::optional<DocComment>&, const EnumPtr&, IceInternal::Output&);

        void writeDocstring(const OperationPtr&, MethodKind, IceInternal::Output&);

        std::string getImportAlias(const ContainedPtr& source, const SyntaxTreeBasePtr& definition);
        std::string getImportAlias(const ContainedPtr& source, const std::string& moduleName, const std::string& name);

        // The list of generated Python code fragments in the current translation unit.
        // Each fragment corresponds to a Slice definition and contains the generated code for that definition.
        std::vector<PythonCodeFragment> _codeFragments;

        ImportsMap _runtimeImports;
        ImportsMap _typingImports;

        /// A map of all import names for each source module.
        /// - Key: the Python generated module name.
        /// - Value: a map where the key is the imported definition name or alias, and the value is the Python
        /// module name where the definition is imported from.
        std::map<std::string, std::map<std::string, std::string>> _allImports;
    };
}

#endif
