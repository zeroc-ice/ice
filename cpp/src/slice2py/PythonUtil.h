// Copyright (c) ZeroC, Inc.

#ifndef SLICE_PYTHON_UTIL_H
#define SLICE_PYTHON_UTIL_H

#include "../Ice/OutputUtil.h"
#include "../Slice/Parser.h"
#include "../Slice/Util.h"

#include <map>
#include <set>
#include <string>

namespace Slice::Python
{
    // The kind of method being documented or generated.
    enum class MethodKind
    {
        SyncInvocation,
        AsyncInvocation,
        Dispatch
    };

    /// Represents the scope of an import statement—either required at runtime or only used for type hints.
    enum class ImportScope
    {
        /// The import is required at runtime by the generated Python code.
        RuntimeImport,

        /// The import is only used for type hints and is not needed at runtime.
        TypingImport
    };

    // The context a type will be used in.
    enum class InterfaceTypeContext
    {
        // If the type is an interface, it is used as a servant (base type).
        Servant,
        // If the type is an interface, it is used as a proxy (base type or parameter).
        Proxy,
    };

    /// Represents the set of symbols imported from a given Python module.
    ///
    /// This includes whether the module itself is imported (possibly with an alias)
    /// and the set of specific definitions (with optional aliases) imported from that module.
    struct ModuleImports
    {
        /// The name of the module being imported.
        std::string moduleName;

        /// The alias used for the module import if not empty and `imported` is true.
        /// For example, for `import Foo as _m_Bar_Foo`, the alias is "_m_Bar_Foo".
        std::string moduleAlias;

        /// Indicates whether the module itself must be imported.
        /// If true, the module will be imported as `import <moduleName>` or
        /// `import <moduleName> as <moduleAlias>`, depending on whether `moduleAlias` is empty.
        bool imported = false;

        /// The set of definitions (with optional aliases) imported from the module.
        /// Each pair consists of the definition name and its alias (the alias may be empty).
        std::set<std::pair<std::string, std::string>> definitions;
    };

    /// A map with the import statements for a generated Python module.
    /// - Key: the imported module name, e.g., "Ice.Communicator".
    /// - Value: the module imports object, representing the definitions imported from the module.
    using ModuleImportsMap = std::map<std::string, ModuleImports>;

    // Maps import statements per generated Python module.
    // - Key: the generated module name, e.g., "Ice.Locator_ice" (we generate one Python module per each unique Slice
    // module in a Slice file).
    // - Value: a map from imported module name to the set of pairs representing the imported name and its alias.
    using ImportsMap = std::map<std::string, ModuleImportsMap>;

    /// Represents a Python code fragment generated for a Slice definition.
    struct CodeFragment
    {
        /// The Slice file name from which this code fragment was generated.
        std::string sliceFileName;

        /// The package name for the generated code.
        std::string packageName;

        /// The Python module for the generated code.
        std::string moduleName;

        /// The Python file name where this code fragment will be written.
        std::string fileName;

        /// Whether this code fragment is a package index file.
        bool isPackageIndex = false;

        /// The generated code.
        std::string code;
    };

    struct CompilationResult
    {
        /// The status of the compilation.
        int status{EXIT_SUCCESS};

        /// The generated Python code fragments.
        std::vector<CodeFragment> fragments;
    };

    enum class CompilationKind
    {
        // Don't generate any Python code.
        None,
        // Generate Python code for Slice definitions.
        Module,
        // Generate Python package index files (__init__.py).
        Index,
        // Generate both modules and package index files.
        All
    };

    /// Returns the fully qualified name of the Python module that corresponds to the given Slice definition.
    ///
    /// Each Slice module is mapped to a Python package with the same name, but with "::" replaced by ".".
    /// Within that package, each Slice definition is mapped to a Python module with the same name as the definition.
    ///
    /// For example:
    /// - A Slice definition named `Baz` in the module `::Bar::Foo` is mapped to the Python module `"Bar.Foo.Baz"`
    ///   in Bar/Foo/Baz.py file.
    ///
    /// @param p The Slice definition to map to a Python module.
    /// @return The fully qualified Python module name corresponding to the Slice definition.
    std::string getPythonModuleForDefinition(const SyntaxTreeBasePtr& p);

    /// Returns the fully qualified name of the Python module where the given Slice definition is forward-declared.
    ///
    /// Forward declarations are generated only for classes and interfaces. The corresponding Python module name is the
    /// same as the definition module returned by `getPythonModuleForDefinition`, with an "_iceF" appended to the end.
    ///
    /// For example, the forward declaration of the class `Bar::MyClass` is placed in the module `"Bar.MyClass_iceF"`
    /// in Bar/MyClass_iceF.py file.
    ///
    /// @param p The Slice definition to get the forward declaration module name for. Must be a class or interface.
    /// @return The fully qualified Python module name for the forward declaration.
    std::string getPythonModuleForForwardDeclaration(const SyntaxTreeBasePtr& p);

    /// Returns the alias used for importing the given Slice definition in Python. If there is not conflict, the alias
    /// is the same as the definition name.
    ///
    /// @param source The Slice definition that is importing the given definition.
    /// @param allImports The map of all imports for the source module. The key is the imported definition name, and
    /// the value is the Python module name where the definition is imported from.
    /// @param p The Slice definition to get the Python import alias for.
    /// @return The alias to use when importing the given definition in generated Python code.
    std::string getImportAlias(
        const ContainedPtr& source,
        const std::map<std::string, std::string>& allImports,
        const SyntaxTreeBasePtr& p);

    /// Returns the alias to use when imported the given @p name from the given @p moduleName. If there is no conflict
    /// with other definitions in the current module or from imported modules, the alias is the same as the name.
    ///
    /// @param source The Slice definition that is importing the given definition.
    /// @param allImports The map of all imports for the source module. The key is the imported definition name, and
    /// the value is the Python module name where the definition is imported from.
    /// @param moduleName The name of the module where the definition is located.
    /// @param name The name of the definition to import.
    /// @return The alias to use when importing the given definition in generated Python code.
    std::string getImportAlias(
        const ContainedPtr& source,
        const std::map<std::string, std::string>& allImports,
        const std::string& moduleName,
        const std::string& name);

    /// Gets the name used for the meta-type of the given Slice definition. IcePy creates a meta-type for each Slice
    /// type. The generated code uses these meta-types to call IcePy.
    /// @param p The Slice definition to get the meta-type name for.
    /// @return The name of the meta-type for the given Slice definition.
    std::string getMetaType(const SyntaxTreeBasePtr& p);

    /// Helper method to emit the generated code that format the fields of a type in __repr__ implementation.
    std::string formatFields(const DataMemberList& members);

    CodeFragment createCodeFragmentForPythonModule(const ContainedPtr& contained, const std::string& code);

    // Get a list of all definitions exported for the Python module corresponding to the given Slice definition.
    std::vector<std::string> getAll(const ContainedPtr& definition);

    /// Get sequence metadata associated with the given sequence and any local metadata.
    Slice::MetadataPtr getSequenceMetadata(const SequencePtr& seq, const MetadataList& localMetadata);

    /// Splits a fully qualified name (FQN) into its Module and Name components.
    /// @param fqn The fully qualified name to split.
    /// @return A pair containing the module and name components.
    std::pair<std::string, std::string> splitFQN(const std::string& fqn);

    /// Splits the arguments the python:memoryview metadata into the factory function and optional type hint.
    /// @param arguments The arguments of python:memoryview
    /// @return A pair containing the factory function and optional type hint
    std::pair<std::string, std::optional<std::string>> splitMemoryviewArguments(const std::string& arguments);

    // A helper class to initialize the _outBuffer member of BufferedOutput before it's
    // passed to the Output constructor.
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

    /// Writes the standard header comment to a generated Python source file.
    ///
    /// @param out The output stream to write the header to.
    void writeHeader(IceInternal::Output& out);

    /// Write the package index that exports the given imports.
    ///
    /// @param imports The map of imports to write to the package index. The key is the module name, and the value is
    /// a set of pairs representing the imported name and its alias.
    /// @param out The output stream to write the package index to.
    void writePackageIndex(const std::map<std::string, std::set<std::string>>& imports, IceInternal::Output& out);

    /// Returns a DocString formatted link to the provided Slice identifier.
    std::string
    pyLinkFormatter(const std::string& rawLink, const ContainedPtr& source, const SyntaxTreeBasePtr& target);

    /// Validates the Slice Python metadata for the given unit.
    void validatePythonMetadata(const UnitPtr&);

    // Collects Python definitions for each generated Python package.
    // Each package corresponds to a Slice module and includes an `__init__.py` file that re-exports selected
    // definitions from the package's internal modules.
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
        [[nodiscard]] const std::map<std::string, std::set<std::string>>& generated() { return _generated; }

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

        std::map<std::string, std::set<std::string>> _generated;
    };

    // Collect the import statements required by each generated Python module.
    class ImportVisitor final : public ParserVisitor
    {
    public:
        bool visitStructStart(const StructPtr&) final;
        bool visitClassDefStart(const ClassDefPtr&) final;
        bool visitExceptionStart(const ExceptionPtr&) final;
        void visitDataMember(const DataMemberPtr&) final;

        bool visitInterfaceDefStart(const InterfaceDefPtr&) final;

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
        /// Add the runtime imports for the given Sequence definition.
        /// @param sequence The Sequence definition being imported.
        /// @param source The Slice definition that requires the import.
        /// @param localMetadata Any additional metadata associated with the import. Such has parameter metadata.
        void addRuntimeImportForSequence(
            const SequencePtr& sequence,
            const ContainedPtr& source,
            const MetadataList& localMetadata = MetadataList());

        /// Adds a runtime import for the given Slice definition if it comes from a different module.
        /// @param definition The Slice definition to import.
        /// @param source The Slice definition that requires the import.
        void addRuntimeImport(
            const SyntaxTreeBasePtr& definition,
            const ContainedPtr& source,
            InterfaceTypeContext typeContext = InterfaceTypeContext::Proxy);

        /// Adds a runtime import for the given definition from the specified Python module.
        ///
        /// @param moduleName The fully qualified name of the Python module to import from.
        /// @param definition The definition to import.
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

        /// Import the meta type for the given Slice definition if it comes from a different module.
        /// @param definition is the Slice definition to import.
        /// @param source is the Slice definition that requires the import.
        void addRuntimeImportForMetaType(const SyntaxTreeBasePtr& definition, const ContainedPtr& source);

        /// Adds an import for the given definition from the specified Python module.
        ///
        /// @param moduleImports The map of imports for the current generated Python module. This can represent either
        /// the runtime or typing imports for the module.
        /// @param moduleName The fully qualified name of the Python module to import from.
        /// @param definition The definition to import, represented as a pair of name and alias.
        /// @param source The Slice definition that requires this import.
        void addImport(
            ModuleImportsMap& moduleImports,
            const std::string& moduleName,
            const std::string& definition,
            const ContainedPtr& source);

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

        bool visitStructStart(const StructPtr&) final;
        void visitStructEnd(const StructPtr&) final;
        bool visitClassDefStart(const ClassDefPtr&) final;
        void visitClassDefEnd(const ClassDefPtr&) final;
        bool visitExceptionStart(const ExceptionPtr&) final;
        void visitExceptionEnd(const ExceptionPtr&) final;
        void visitDataMember(const DataMemberPtr&) final;

        bool visitInterfaceDefStart(const InterfaceDefPtr&) final;

        void visitSequence(const SequencePtr&) final;
        void visitDictionary(const DictionaryPtr&) final;
        void visitEnum(const EnumPtr&) final;
        void visitConst(const ConstPtr&) final;

        [[nodiscard]] const std::vector<CodeFragment>& codeFragments() const { return _codeFragments; }

    private:
        /// Returns a string representation of the type hint for the given Slice type.
        /// @param type The Slice type to convert to a type hint string.
        /// @param optional If true, the type hint will indicate that the type is optional (i.e., it can be `None`).
        /// @param source The Slice definition requesting the type hint.
        /// @param forMarshaling If true, the type is used for marshaling (invocation input parameter, or dispatch
        /// output parameter).
        /// @param localMetadata The local metadata to consider when generating the type hint.
        /// @return The string representation of the type hint for the given Slice type.
        std::string typeToTypeHintString(
            const TypePtr& type,
            bool optional,
            const ContainedPtr& source,
            bool forMarshaling,
            const MetadataList& localMetadata = MetadataList());

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

        // Write Python metadata as a tuple.
        void writeMetadata(const MetadataList&, IceInternal::Output&);

        // Write the data members meta-info for a meta type declaration.
        void writeMetaTypeDataMembers(
            const ContainedPtr& contained,
            const DataMemberList& members,
            IceInternal::Output& out);

        // Write a constant value.
        void writeConstantValue(
            const ContainedPtr& source,
            const TypePtr&,
            const SyntaxTreeBasePtr&,
            const std::string&,
            IceInternal::Output&);

        /// Writes the provided @p remarks in its own subheading in the current comment (if @p remarks is non-empty).
        void writeRemarksDocComment(const StringList& remarks, bool needsNewline, IceInternal::Output& out);

        void writeDocstring(const std::optional<DocComment>&, const std::string&, IceInternal::Output&);
        void writeDocstring(const std::optional<DocComment>&, const DataMemberList&, IceInternal::Output&);
        void writeDocstring(const std::optional<DocComment>&, const EnumPtr&, IceInternal::Output&);

        void writeDocstring(const OperationPtr&, MethodKind, IceInternal::Output&);

        std::string getImportAlias(const ContainedPtr& source, const SyntaxTreeBasePtr& definition);
        std::string
        getImportAlias(const ContainedPtr& source, const std::string& moduleName, const std::string& name = "");

        // The list of generated Python code fragments in the current translation unit.
        // Each fragment corresponds to a Python module generated from a Slice definition with the same name.
        std::vector<CodeFragment> _codeFragments;

        ImportsMap _runtimeImports;
        ImportsMap _typingImports;

        /// A map of all import names for each source module.
        /// - Key: the Python generated module name.
        /// - Value: a map where the key is the imported definition name or alias, and the value is the Python
        /// module name where the definition is imported from.
        std::map<std::string, std::map<std::string, std::string>> _allImports;

        std::unique_ptr<BufferedOutput> _out;
    };

    /// Generates Python modules and packages from the specified Slice files. Returns the generated code
    /// fragments and the compilation status as a CompilationResult.
    ///
    /// @param programName The name of the caller program (typically "slice2py" or "Ice.loadSlice"), used for
    /// parser errors.
    /// @param dependencyGenerator The dependency generator used to collect Slice file dependencies.
    /// @param packageVisitor The package visitor responsible for collecting package and module information.
    /// @param files The list of Slice files to process.
    /// @param preprocessorArgs The arguments to pass to the preprocessor.
    /// @param sortFragments Whether to sort the generated code fragments in the order required by the Python
    /// interpreter.
    /// @param compilationKind The kind of Python code to generate (modules, index files, or both).
    /// @param debug Whether to enable debug output.
    /// @return A CompilationResult containing the generated code fragments and the compilation status.
    CompilationResult compile(
        const std::string& programName,
        const std::unique_ptr<DependencyGenerator>& dependencyGenerator,
        PackageVisitor& packageVisitor,
        const std::vector<std::string>& files,
        const std::vector<std::string>& preprocessorArgs,
        bool sortFragments,
        CompilationKind compilationKind,
        bool debug);

    /// Generates Python modules and packages from the specified Slice files,
    /// writing the generated code to the output directory specified in the arguments.
    /// This method accepts the same arguments as the `slice2py` compiler.
    ///
    /// This overload is used by both the command-line `slice2py` compiler and the
    /// Python `IcePy.compileSlice` function, which is invoked by the `slice2py` script
    /// distributed with the Ice for Python PIP packages.
    ///
    /// Internally, this method parses the arguments and delegates to the main compile overload,
    /// returning the compilation status.
    ///
    /// @param args The command-line arguments passed to the Slice to Python compiler.
    /// @return The status code representing the result of the compilation.
    int compile(const std::vector<std::string>& args);
}

#endif
