// Copyright (c) ZeroC, Inc.

#include "PythonUtil.h"
#include "../Ice/FileUtil.h"
#include "../Slice/FileTracker.h"
#include "../Slice/MetadataValidation.h"
#include "../Slice/Util.h"
#include "Ice/StringUtil.h"

#include <algorithm>
#include <cassert>
#include <climits>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using namespace Slice;
using namespace IceInternal;

namespace
{
    /// Determines the mapped package for a given Slice definition.
    /// @param p The Slice definition to get the mapped package for.
    /// @param packageSeparator Use this character as the separator between package segments.
    /// @return The mapped package name, with the specified separator.
    string getMappedPackage(SyntaxTreeBasePtr p, char packageSeparator = '.')
    {
        if (dynamic_pointer_cast<Builtin>(p))
        {
            return string{"Ice"} + packageSeparator;
        }
        else
        {
            auto contained = dynamic_pointer_cast<Contained>(p);
            assert(contained);
            string package = contained->mappedScope(string{packageSeparator});
            if (packageSeparator != '.')
            {
                // Replace "." with the specified separator.
                replace(package.begin(), package.end(), '.', packageSeparator);
            }
            return package;
        }
    }

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
    string getPythonModuleForDefinition(const SyntaxTreeBasePtr& p)
    {
        if (auto builtin = dynamic_pointer_cast<Builtin>(p))
        {
            static const char* builtinTable[] =
                {"", "", "", "", "", "", "", "", "Ice.Value", "Ice.ObjectPrx", "Ice.Value"};

            return builtinTable[builtin->kind()];
        }
        else
        {
            auto contained = dynamic_pointer_cast<Contained>(p);
            assert(contained);
            return getMappedPackage(contained) + contained->mappedName();
        }
    }

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
    string getPythonModuleForForwardDeclaration(const SyntaxTreeBasePtr& p)
    {
        string declarationModule = getPythonModuleForDefinition(p);
        if (!declarationModule.empty())
        {
            declarationModule += "F";
        }
        return declarationModule;
    }

    /// Returns the alias used for importing the given Slice definition in Python.
    ///
    /// The alias follows the pattern `"M1_M2_Xxx"`, where:
    /// - `M1_M2` is the mapped scope of the definition, using underscores (`_`) as separators instead of `::`.
    /// - `Xxx` is the mapped name of the definition (typically the class or interface name).
    ///
    /// If the definition represents an interface, the suffix `"Prx"` is appended to the alias to refer to the proxy
    /// type.
    ///
    /// @param p The Slice definition to get the Python import alias for.
    /// @return The alias to use when importing the given definition in generated Python code.
    string getImportAlias(const SyntaxTreeBasePtr& p)
    {
        if (auto builtin = dynamic_pointer_cast<Builtin>(p))
        {
            static const char* builtinTable[] =
                {"", "", "", "", "", "", "", "", "Ice_Value", "Ice_ObjectPrx", "Ice_Value"};

            return builtinTable[builtin->kind()];
        }
        else
        {
            auto contained = dynamic_pointer_cast<Contained>(p);
            assert(contained);
            return getMappedPackage(contained, '_') + contained->mappedName();
        }
    }

    /// Gets the name used for the meta-type of the given Slice definition. IcePy creates a meta-type for each Slice
    /// type. The generated code uses these meta-types to call IcePy.
    /// @param p The Slice definition to get the meta-type name for.
    /// @return The name of the meta-type for the given Slice definition.
    string getMetaType(const SyntaxTreeBasePtr& p)
    {
        if (auto builtin = dynamic_pointer_cast<Builtin>(p))
        {
            static const char* builtinTable[] = {
                "IcePy._t_byte",
                "IcePy._t_bool",
                "IcePy._t_short",
                "IcePy._t_int",
                "IcePy._t_long",
                "IcePy._t_float",
                "IcePy._t_double",
                "IcePy._t_string",
                "__Ice_Value_t",
                "__Ice_ObjectPrx_t",
                "__Ice_Value_t"};

            return builtinTable[builtin->kind()];
        }
        else
        {
            auto contained = dynamic_pointer_cast<Contained>(p);
            assert(contained);
            string s = "__" + getMappedPackage(contained, '_') + contained->mappedName();
            if (dynamic_pointer_cast<InterfaceDef>(contained) || dynamic_pointer_cast<InterfaceDecl>(contained))
            {
                s += "Prx";
            }
            s += "_t";
            return s;
        }
    }

    /// Represents a Python code fragment generated for a Slice definition.
    struct PythonCodeFragment
    {
        /// The Slice definition.
        ContainedPtr contained;

        /// The generated code.
        string code;
    };

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
        ServantType,
        // If the type is an interface, it is used as a proxy (base type or parameter).
        ProxyType
    };

    const char* const tripleQuotes = R"(""")";

    /// Returns a string representation of the type hint for the given Slice type.
    /// @param type The Slice type to convert to a type hint string.
    /// @param optional If true, the type hint will indicate that the type is optional (i.e., it can be `None`).
    /// @param source The Slice definition requesting the type hint.
    /// @return The string representation of the type hint for the given Slice type.
    string typeToTypeHintString(const TypePtr& type, bool optional, const SyntaxTreeBasePtr& source)
    {
        assert(type);

        if (optional)
        {
            if (isProxyType(type))
            {
                // We map optional proxies like regular proxies, as XxxPrx or None.
                return typeToTypeHintString(type, false, source);
            }
            else
            {
                return typeToTypeHintString(type, false, source) + " | None";
            }
        }

        static constexpr string_view builtinTable[] = {
            "int",
            "bool",
            "int",
            "int",
            "int",
            "float",
            "float",
            "str",
            "Ice.Object | None", // Not used anymore
            "Ice.ObjectPrx | None",
            "Ice.Value | None"};

        if (auto builtin = dynamic_pointer_cast<Builtin>(type))
        {
            if (builtin->kind() == Builtin::KindObject)
            {
                return string{builtinTable[Builtin::KindValue]};
            }
            else
            {
                return string{builtinTable[builtin->kind()]};
            }
        }
        else
        {
            string definitionModule = getPythonModuleForDefinition(type);
            string sourceModule = getPythonModuleForDefinition(source);

            auto contained = dynamic_pointer_cast<Contained>(type);
            assert(contained);

            string prefix = sourceModule == definitionModule ? "" : getMappedPackage(contained);

            if (auto proxy = dynamic_pointer_cast<InterfaceDecl>(type))
            {
                return prefix + proxy->mappedName() + "Prx | None";
            }
            else if (auto seq = dynamic_pointer_cast<Sequence>(type))
            {
                return "Sequence[" + typeToTypeHintString(seq->type(), false, source) + "]";
            }
            else if (auto dict = dynamic_pointer_cast<Dictionary>(type))
            {
                ostringstream os;
                os << "dict[" << typeToTypeHintString(dict->keyType(), false, source) << ", "
                   << typeToTypeHintString(dict->valueType(), false, source) << "]";
                return os.str();
            }
            else
            {
                return prefix + contained->mappedName();
            }
        }
    }

    /// Returns a string representation of the return type hint for the given operation.
    /// @param operation The Slice operation to get the return type hint for.
    /// @param methodKind The kind of method being documented or generated (sync, async, or dispatch).
    /// @return The string representation of the return type hint for the given operation.
    string returnTypeHint(const OperationPtr& operation, MethodKind methodKind)
    {
        auto source = dynamic_pointer_cast<Contained>(operation->container());
        string returnTypeHint;
        ParameterList outParameters = operation->outParameters();
        if (operation->returnsMultipleValues())
        {
            ostringstream os;
            os << "tuple[";
            if (operation->returnType())
            {
                os << typeToTypeHintString(operation->returnType(), operation->returnIsOptional(), source);
                os << ", ";
            }

            for (const auto& param : outParameters)
            {
                os << typeToTypeHintString(param->type(), param->optional(), source);
                if (param != outParameters.back())
                {
                    os << ", ";
                }
            }
            os << "]";
            returnTypeHint = os.str();
        }
        else if (operation->returnType())
        {
            returnTypeHint = typeToTypeHintString(operation->returnType(), operation->returnIsOptional(), source);
        }
        else if (!outParameters.empty())
        {
            const auto& param = outParameters.front();
            returnTypeHint = typeToTypeHintString(param->type(), param->optional(), source);
        }
        else
        {
            returnTypeHint = "None";
        }

        switch (methodKind)
        {
            case AsyncInvocation:
                return "Awaitable[" + returnTypeHint + "]";
            case Dispatch:
                return returnTypeHint + " | Awaitable[" + returnTypeHint + "]";
            case SyncInvocation:
            default:
                return returnTypeHint;
        }
    }

    /// Returns a string representation of the operation's return type hint. This is the same as `returnTypeHint`, but
    /// with the " -> " prefix for use in function signatures.
    /// @param operation The Slice operation to get the return type hint for.
    /// @param methodKind The kind of method being documented or generated (sync, async, or dispatch).
    /// @return The string representation of the operation's return type hint, prefixed with " -> ".
    string operationReturnTypeHint(const OperationPtr& operation, MethodKind methodKind)
    {
        return " -> " + returnTypeHint(operation, methodKind);
    }

    /// Helper method to emit the generated code that format the fields of a type in __repr__ implementation.
    string formatFields(const DataMemberList& members)
    {
        if (members.empty())
        {
            return "";
        }

        ostringstream os;
        bool first = true;
        os << "{format_fields(";
        for (const auto& dataMember : members)
        {
            if (!first)
            {
                os << ", ";
            }
            first = false;
            os << dataMember->mappedName() << "=self." << dataMember->mappedName();
        }
        os << ")}";
        return os.str();
    }
}

namespace Slice::Python
{
    class BufferedOutputBase
    {
    protected:
        std::ostringstream _outBuffer;
    };

    /// A output class that writes to a stream, used by the CodeVisitor to write Python code fragments.
    class BufferedOutput final : public BufferedOutputBase, public Output
    {
    public:
        BufferedOutput() : Output(_outBuffer) {}

        /// Returns the string representation of the buffered output.
        /// @return the string containing the buffered output.
        string str() const { return _outBuffer.str(); }
    };

    // CodeVisitor generates the Python mapping for a translation unit.
    class CodeVisitor final : public ParserVisitor
    {
    public:
        bool visitClassDefStart(const ClassDefPtr&) final;
        bool visitInterfaceDefStart(const InterfaceDefPtr&) final;
        bool visitExceptionStart(const ExceptionPtr&) final;
        bool visitStructStart(const StructPtr&) final;
        void visitSequence(const SequencePtr&) final;
        void visitDictionary(const DictionaryPtr&) final;
        void visitEnum(const EnumPtr&) final;
        void visitConst(const ConstPtr&) final;

        const vector<PythonCodeFragment>& codeFragments() const { return _codeFragments; }

    private:
        // Emit Python code for operations
        void writeOperations(const InterfaceDefPtr&, Output&);

        // Get the default value for initializing a given type.
        string getTypeInitializer(const DataMemberPtr&);

        // Write Python metadata as a tuple.
        void writeMetadata(const MetadataList&, Output&);

        // Convert an operation mode into a string.
        string getOperationMode(Slice::Operation::Mode);

        // Write a member assignment statement for a constructor.
        void writeAssign(const DataMemberPtr& member, Output& out);

        // Write a constant value.
        void writeConstantValue(const TypePtr&, const SyntaxTreeBasePtr&, const string&, Output&);

        /// Write constructor parameters with default values.
        void writeConstructorParams(const DataMemberList& members, Output&);

        /// Writes the provided @p remarks in its own subheading in the current comment (if @p remarks is non-empty).
        void writeRemarksDocComment(const StringList& remarks, bool needsNewline, Output& out);

        void writeDocstring(const optional<DocComment>&, const string&, Output&);
        void writeDocstring(const optional<DocComment>&, const DataMemberList&, Output&);
        void writeDocstring(const optional<DocComment>&, const EnumPtr&, Output&);

        void writeDocstring(const OperationPtr&, MethodKind, Output&);

        // The list of generated Python code fragments in the current translation unit.
        // Each fragment corresponds to a Slice definition and contains the generated code for that definition.
        vector<PythonCodeFragment> _codeFragments;
    };

    // Maps import statements per generated Python module.
    // - Key: the generated module name, e.g., "Ice.Locator_ice" (we generate one Python module per each unique Slice
    // module in a Slice file).
    // - Value: a map from imported module name to the set of pairs representing the imported name and its alias.
    using ImportsMap = map<string, map<string, set<pair<string, string>>>>;

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

        const ImportsMap& getRuntimeImports() const { return _runtimeImports; }

        const ImportsMap& getTypingImports() const { return _typingImports; }

    private:
        void visitDataMembers(const ContainedPtr&, const list<DataMemberPtr>&);

        /// Adds a runtime import for the given Slice definition if it comes from a different module.
        /// @p definition is the Slice definition to import.
        /// @p source is the Slice definition that requires the import.
        void addRuntimeImport(
            const SyntaxTreeBasePtr& definition,
            const ContainedPtr& source,
            TypeContext typeContext = ProxyType);

        /// Adds a runtime import for the given definition from the specified Python module.
        ///
        /// @param moduleName The fully qualified name of the Python module to import from.
        /// @param definition A pair consisting of the name and alias to use for the imported symbol.
        ///                   If the alias is empty, the name is used as the alias.
        /// @param source The Slice definition that requires this import.
        void addRuntimeImport(const string& moduleName, pair<string, string> definition, const ContainedPtr& source);

        /// Adds a typing import for the given definition from the specified Python module.
        ///
        /// Typing imports are generated inside an `if TYPE_CHECKING:` block, so they are only used for type hints.
        ///
        /// @param moduleName The fully qualified name of the Python module to import from.
        /// @param definition The definition to import, represented as a pair of name and alias.
        /// @param source The Slice definition that requires this import.
        void addTypingImport(const string& moduleName, pair<string, string> definition, const ContainedPtr& source);

        /// Adds a typing import for the package containing the given Slice definition.
        ///
        /// Typing imports are generated inside an `if TYPE_CHECKING:` block, so they are only used for type hints.
        ///
        /// @param definition The definition to import the containing package.
        /// @param source The Slice definition that requires this import.
        void addTypingImport(const SyntaxTreeBasePtr& definition, const ContainedPtr& source);

        /// Adds a typing import for the given package.
        ///
        /// Typing imports are generated inside an `if TYPE_CHECKING:` block, so they are only used for type hints.
        ///
        /// @param packageName The name of the package to import.
        /// @param source The Slice definition that requires this import.
        void addTypingImport(const string& packageName, const ContainedPtr& source);

        /// Import the meta type for the given Slice definition if it comes from a different module.
        /// @p definition is the Slice definition to import.
        /// @p source is the Slice definition that requires the import.
        void addRuntimeImportForMetaType(const SyntaxTreeBasePtr& definition, const ContainedPtr& source);

        ImportsMap _runtimeImports;
        ImportsMap _typingImports;
    };
}

void
Slice::Python::writeHeader(IceInternal::Output& out)
{
    out << "# Copyright (c) ZeroC, Inc.";
    out << sp;
    out << nl << "# slice2py version " << ICE_STRING_VERSION;
}

void
Slice::Python::createPackagePath(const string& moduleName, const string& outputPath)
{
    vector<string> packageParts;
    IceInternal::splitString(string_view{moduleName}, ".", packageParts);
    assert(!packageParts.empty());
    packageParts.pop_back(); // Remove the last part, which is the module name.
    string packagePath = outputPath;
    for (const auto& part : packageParts)
    {
        packagePath += "/" + part;
        int err = IceInternal::mkdir(packagePath, 0777);
        if (err == 0)
        {
            FileTracker::instance()->addDirectory(packagePath);
        }
        else if (errno == EEXIST && IceInternal::directoryExists(packagePath))
        {
            // If the Slice compiler is run concurrently, it's possible that another instance of it has already
            // created the directory.
        }
        else
        {
            ostringstream os;
            os << "cannot create directory '" << packagePath << "': " << IceInternal::errorToString(errno);
            throw FileException(os.str());
        }
    }
}

bool
Slice::Python::ImportVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    addRuntimeImport("Ice.Util", {"format_fields", ""}, p);
    // Import the meta type that is created in the Xxx_iceF module for forward declarations.
    addRuntimeImportForMetaType(p->declaration(), p);

    // Add imports required for the base class type.
    if (ClassDefPtr base = p->base())
    {
        addRuntimeImport(base, p);
        addRuntimeImportForMetaType(base, p);
    }
    else
    {
        // If the class has no base, we import the Ice.Object type.
        addRuntimeImport("Ice.Value", {"Value", "Ice_Value"}, p);
    }

    // Add imports required for the data members.
    visitDataMembers(p, p->allDataMembers());
    return false;
}

bool
Slice::Python::ImportVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    // Import the proxy meta type that is created in the XxxF module for forward declarations.
    addRuntimeImportForMetaType(p->declaration(), p);

    // Add imports required for base interfaces types.
    const InterfaceList& bases = p->bases();
    if (bases.empty())
    {
        addRuntimeImport("Ice.ObjectPrx", {"ObjectPrx", "Ice_ObjectPrx"}, p);
        addRuntimeImport("Ice.Object", {"Object", "Ice_Object"}, p);
    }
    else
    {
        for (const auto& base : bases)
        {
            addRuntimeImport(base, p, ProxyType);
            addRuntimeImport(base, p, ServantType);
        }
    }

    addRuntimeImport("abc", {"ABC", ""}, p);

    addRuntimeImport("Ice.ObjectPrx", {"checkedCast", "Ice_checkedCast"}, p);
    addRuntimeImport("Ice.ObjectPrx", {"checkedCastAsync", "Ice_checkedCastAsync"}, p);
    addRuntimeImport("Ice.ObjectPrx", {"uncheckedCast", "Ice_uncheckedCast"}, p);

    addRuntimeImport("typing", {"TYPE_CHECKING", ""}, p);

    // Add imports required for operation parameters and return types.
    const OperationList& operations = p->allOperations();
    if (!operations.empty())
    {
        addRuntimeImport("abc", {"abstractmethod", ""}, p);
        // If the interface has no operations, we still need to import the Ice.ObjectPrx type.
        addTypingImport("collections.abc", {"Awaitable", ""}, p);
        addTypingImport("collections.abc", {"Sequence", ""}, p);

        addRuntimeImport("Ice.OperationMode", {"OperationMode", "Ice_OperationMode"}, p);
    }

    for (const auto& op : operations)
    {
        auto ret = op->returnType();
        if (ret)
        {
            addTypingImport(ret, p);
            addRuntimeImportForMetaType(ret, p);
        }

        for (const auto& param : op->parameters())
        {
            addTypingImport(param->type(), p);
            addRuntimeImportForMetaType(param->type(), p);
        }

        for (const auto& ex : op->throws())
        {
            addTypingImport(ex, p);
            addRuntimeImportForMetaType(ex, p);
        }

        if (op->format())
        {
            addRuntimeImport("Ice.FormatType", {"FormatType", "Ice_FormatType"}, p);
        }
    }

    // Types that are used in the Prx interface.
    addTypingImport("Ice", p);

    return false;
}

bool
Slice::Python::ImportVisitor::visitStructStart(const StructPtr& p)
{
    // Visit the data members.
    addRuntimeImport("typing", {"TYPE_CHECKING", ""}, p);
    addRuntimeImport("Ice.Util", {"format_fields", ""}, p);
    addRuntimeImport("dataclasses", {"dataclass", ""}, p);
    addRuntimeImport("dataclasses", {"field", ""}, p);
    // Add imports required for the data members.
    visitDataMembers(p, p->dataMembers());
    return false;
}

bool
Slice::Python::ImportVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    addRuntimeImport("Ice.Util", {"format_fields", ""}, p);
    // Add imports required for base exception types.
    if (ExceptionPtr base = p->base())
    {
        addRuntimeImport(base, p);
        addRuntimeImportForMetaType(base, p);
    }
    else
    {
        // If the exception has no base, we import the Ice.UserException type.
        addRuntimeImport("Ice.UserException", {"UserException", "Ice_UserException"}, p);
    }
    // Add imports required for the data members.
    visitDataMembers(p, p->allDataMembers());
    return false;
}

void
Slice::Python::ImportVisitor::visitDataMembers(const ContainedPtr& parent, const list<DataMemberPtr>& members)
{
    for (const auto& member : members)
    {
        // Add imports required for data member types.
        auto type = member->type();

        // For fields with a type that is a Struct, we need to import it as a RuntimeImport, to
        // initialize the field in the constructor. For other contained types, we only need the
        // import for type hints.
        if (dynamic_pointer_cast<Struct>(type) || dynamic_pointer_cast<Enum>(type))
        {
            addRuntimeImport(type, parent, ProxyType);
        }
        else
        {
            addTypingImport(type, parent);
        }
        addRuntimeImportForMetaType(type, parent);

        // If the data member has a default value, and the type of the default value is an Enum or a Const
        // we need to import the corresponding Enum or Const.
        if (member->defaultValue() && (dynamic_pointer_cast<Const>(member->defaultValueType()) ||
                                       dynamic_pointer_cast<Enum>(member->defaultValueType())))
        {
            addRuntimeImport(member->defaultValueType(), parent, ProxyType);
        }
    }
}

void
Slice::Python::ImportVisitor::visitSequence(const SequencePtr& p)
{
    // Add import required for the sequence element type.
    addRuntimeImportForMetaType(p->type(), p);
}

void
Slice::Python::ImportVisitor::visitDictionary(const DictionaryPtr& p)
{
    // Add imports required for the dictionary key and value meta types
    addRuntimeImportForMetaType(p->keyType(), p);
    addRuntimeImportForMetaType(p->valueType(), p);
}

void
Slice::Python::ImportVisitor::visitEnum(const EnumPtr& p)
{
    // TODO if a value is initialized with a constant, we need to import the type of the constant.
    addRuntimeImport("enum", {"Enum", ""}, p);
}

void
Slice::Python::ImportVisitor::visitConst(const ConstPtr& p)
{
    // If the constant value is a Slice enum, we need to import the enum type.
    if (dynamic_pointer_cast<Enum>(p->type()))
    {
        addRuntimeImport(p->type(), p);
    }
}

void
Slice::Python::ImportVisitor::addRuntimeImport(
    const SyntaxTreeBasePtr& definition,
    const ContainedPtr& source,
    TypeContext typeContext)
{
    // The module containing the definition we want to import.
    auto definitionModule = getPythonModuleForDefinition(definition);

    // The module importing the definition.
    string sourceModule = getPythonModuleForDefinition(source);

    if (definitionModule == sourceModule)
    {
        // If the definition and source are in the same module, we don't need to import it.
        return;
    }

    vector<pair<string, string>> names;
    if (auto builtin = dynamic_pointer_cast<Builtin>(definition))
    {
        if (builtin->kind() != Builtin::KindObjectProxy && builtin->kind() != Builtin::KindValue &&
            builtin->kind() != Builtin::KindObject)
        {
            // Builtin types other than ObjectPrx and Value don't need imports.
            return;
        }
        names.emplace_back(
            builtin->kind() == Builtin::KindObjectProxy ? "ObjectPrx" : "Value",
            getImportAlias(definition));
    }
    else
    {
        auto contained = dynamic_pointer_cast<Contained>(definition);
        assert(contained);

        if ((dynamic_pointer_cast<InterfaceDef>(definition) || dynamic_pointer_cast<InterfaceDecl>(definition)) &&
            typeContext == ProxyType)
        {
            names.emplace_back(contained->mappedName() + "Prx", getImportAlias(definition) + "Prx");
        }
        else
        {
            names.emplace_back(contained->mappedName(), getImportAlias(definition));
        }
    }

    auto& sourceModuleImports = _runtimeImports[sourceModule];
    auto& definitionImports = sourceModuleImports[definitionModule];
    definitionImports.insert(names.begin(), names.end());
}

void
Slice::Python::ImportVisitor::addRuntimeImport(
    const string& definitionModule,
    pair<string, string> definition,
    const ContainedPtr& source)
{
    // The module importing the definition.
    string sourceModule = getPythonModuleForDefinition(source);

    if (definitionModule == sourceModule)
    {
        // If the definition and source are in the same module, we don't need to import it.
        return;
    }

    auto& sourceModuleImports = _runtimeImports[sourceModule];
    auto& definitionImports = sourceModuleImports[definitionModule];
    definitionImports.insert(definition);
}

void
Slice::Python::ImportVisitor::addTypingImport(
    const string& moduleName,
    pair<string, string> definition,
    const ContainedPtr& source)
{
    // The module importing the definition.
    string sourceModule = getPythonModuleForDefinition(source);
    auto& sourceModuleImports = _typingImports[sourceModule];
    auto& definitionImports = sourceModuleImports[moduleName];
    definitionImports.insert(definition);

    // If we are importing a type with the TypingImport scope, we also need a runtime import for TYPE_CHECKING from
    // typing.
    addRuntimeImport("typing", {"TYPE_CHECKING", ""}, source);
}

void
Slice::Python::ImportVisitor::addTypingImport(const SyntaxTreeBasePtr& definition, const ContainedPtr& source)
{
    string packageName = getMappedPackage(definition);
    packageName.pop_back(); // Remove the final dot.

    addTypingImport(packageName, source);
}

void
Slice::Python::ImportVisitor::addTypingImport(const string& packageName, const ContainedPtr& source)
{
    // The module importing the definition.
    string sourceModule = getPythonModuleForDefinition(source);

    auto& sourceModuleImports = _typingImports[sourceModule];
    if (sourceModuleImports.find(packageName) == sourceModuleImports.end())
    {
        // If the package does not exist, we create an empty map for it.
        sourceModuleImports[packageName] = {};
    }

    // If we are importing a type with the TypingImport scope, we also need a runtime import for TYPE_CHECKING from
    // typing.
    addRuntimeImport("typing", {"TYPE_CHECKING", ""}, source);
}

void
Slice::Python::ImportVisitor::addRuntimeImportForMetaType(
    const SyntaxTreeBasePtr& definition,
    const ContainedPtr& source)
{
    auto builtin = dynamic_pointer_cast<Builtin>(definition);
    if (builtin && builtin->kind() != Builtin::KindObjectProxy && builtin->kind() != Builtin::KindValue &&
        builtin->kind() != Builtin::KindObject)
    {
        // Builtin types other than ObjectPrx and Value don't need imports.
        return;
    }

    // The meta type for a Slice class or interface is always imported from the Xxx_iceF module.
    bool isForwardDeclared =
        dynamic_pointer_cast<ClassDecl>(definition) || dynamic_pointer_cast<InterfaceDecl>(definition) || builtin;

    // The module containing the definition we want to import.
    string definitionModule =
        isForwardDeclared ? getPythonModuleForForwardDeclaration(definition) : getPythonModuleForDefinition(definition);

    // The module importing the definition.
    string sourceModule = getPythonModuleForDefinition(source);

    if (definitionModule == sourceModule)
    {
        // If the definition and source are in the same module, we don't need to import it.
        return;
    }

    auto& sourceModuleImports = _runtimeImports[sourceModule];
    auto& definitionImports = sourceModuleImports[definitionModule];
    definitionImports.insert({getMetaType(definition), ""});
}

bool
Slice::Python::PackageVisitor::visitModuleStart(const ModulePtr& p)
{
    // Add the __init__.py file to the list of generated modules.
    _packageIndexFiles.insert(getMappedPackage(p) + "/__init__.py");
    return true;
}

bool
Slice::Python::PackageVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    // Add the class to the package imports.
    addRuntimeImport(p);
    addRuntimeImportForMetaType(p->declaration());
    return false;
}

bool
Slice::Python::PackageVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    addRuntimeImport(p);
    addRuntimeImport(p, "Prx");
    addRuntimeImportForMetaType(p->declaration());

    return false;
}

bool
Slice::Python::PackageVisitor::visitStructStart(const StructPtr& p)
{
    addRuntimeImport(p);
    addRuntimeImportForMetaType(p);
    return false;
}

bool
Slice::Python::PackageVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    addRuntimeImport(p);
    addRuntimeImportForMetaType(p);
    return false;
}

void
Slice::Python::PackageVisitor::visitSequence(const SequencePtr& p)
{
    addRuntimeImportForMetaType(p);
}

void
Slice::Python::PackageVisitor::visitDictionary(const DictionaryPtr& p)
{
    addRuntimeImportForMetaType(p);
}

void
Slice::Python::PackageVisitor::visitEnum(const EnumPtr& p)
{
    addRuntimeImport(p);
    addRuntimeImportForMetaType(p);
}

void
Slice::Python::PackageVisitor::visitConst(const ConstPtr& p)
{
    addRuntimeImport(p);
}

void
Slice::Python::PackageVisitor::addRuntimeImport(const ContainedPtr& definition, const string& prefix)
{
    string packageName = getMappedPackage(definition);
    string moduleName = definition->mappedName();
    auto& packageImports = _imports[packageName];
    auto& definitions = packageImports[moduleName];
    definitions.insert(definition->mappedName() + prefix);

    // Add the definition to the list of generated Python modules.
    string modulePath = packageName;
    replace(modulePath.begin(), modulePath.end(), '.', '/');
    _generatedModules.insert(modulePath + moduleName + ".py");

    // Ensure all parent packages exits, that is necessary to account for modules
    // that are mapped to a nested package using python:identifier metadata.
    vector<string> packageParts;
    IceInternal::splitString(string_view{packageName}, ".", packageParts);
    packageName = "";
    for (const auto& part : packageParts)
    {
        packageName += part + ".";
        if (_imports.find(packageName) == _imports.end())
        {
            // If the package does not exist, we create an empty map for it.
            _imports[packageName] = {};
            _packageIndexFiles.insert(packageName + "/__init__.py");
        }
    }
}

void
Slice::Python::PackageVisitor::addRuntimeImportForMetaType(const ContainedPtr& definition)
{
    string packageName = getMappedPackage(definition);
    string moduleName = definition->mappedName();

    // The meta type for Slice classes or interfaces is always imported from the XxxF module containing the forward
    // declaration.
    if (dynamic_pointer_cast<ClassDecl>(definition) || dynamic_pointer_cast<InterfaceDecl>(definition))
    {
        moduleName += "F";
    }
    auto& packageImports = _imports[packageName];
    auto& definitions = packageImports[moduleName];
    definitions.insert(getMetaType(definition));

    // Add the definition to the list of generated Python modules.
    replace(packageName.begin(), packageName.end(), '.', '/');
    _generatedModules.insert(packageName + moduleName + ".py");
}

// CodeVisitor implementation.

void
Slice::Python::CodeVisitor::writeOperations(const InterfaceDefPtr& p, Output& out)
{
    // Emits an abstract method for each operation.
    for (const auto& operation : p->operations())
    {
        const string sliceName = operation->name();
        const string mappedName = operation->mappedName();

        if (operation->hasMarshaledResult())
        {
            string capName = sliceName;
            capName[0] = static_cast<char>(toupper(static_cast<unsigned char>(capName[0])));
            out << sp;
            out << nl << "@staticmethod";
            out << nl << "def " << capName << "MarshaledResult(result, current: Ice.Current):";
            out.inc();
            out << nl << tripleQuotes;
            out << nl << "Immediately marshals the result of an invocation of " << sliceName;
            out << nl << "and returns an object that the servant implementation must return";
            out << nl << "as its result.";
            out << nl;
            out << nl << "Args:";
            out << nl << "  result: The result (or result tuple) of the invocation.";
            out << nl << "  current: The Current object passed to the invocation.";
            out << nl;
            out << nl << "Returns";
            out << nl << "  An object containing the marshaled result.";
            out << nl << tripleQuotes;
            out << nl << "return IcePy.MarshaledResult(result, " << p->mappedName() << "._op_" << sliceName
                << ", current.adapter.getCommunicator()._getImpl(), current.encoding)";
            out.dec();
        }

        out << sp;
        out << nl << "@abstractmethod";
        out << nl << "def " << mappedName << spar << "self";

        for (const auto& param : operation->parameters())
        {
            if (!param->isOutParam())
            {
                out << (param->mappedName() + ": " + typeToTypeHintString(param->type(), param->optional(), p));
            }
        }

        const string currentParamName = getEscapedParamName(operation->parameters(), "current");
        out << (currentParamName + ": Ice.Current");
        out << epar << operationReturnTypeHint(operation, Dispatch) << ":";
        out.inc();

        writeDocstring(operation, Dispatch, out);

        out << nl << "pass";
        out.dec();
    }
}

bool
Slice::Python::CodeVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    const string scoped = p->scoped();
    const string metaType = getMetaType(p);
    const string valueName = p->mappedName();
    const ClassDefPtr base = p->base();
    const DataMemberList members = p->dataMembers();

    // Emit a forward declaration for the class meta-type.
    BufferedOutput outF;
    outF << nl << metaType << " = IcePy.declareValue(\"" << p->scoped() << "\")";

    outF << sp;
    outF << nl << "__all__ = [\"" << metaType << "\"]";

    _codeFragments.push_back({p->declaration(), outF.str()});

    // Emit the class definition.
    BufferedOutput out;
    out << nl << "class " << valueName << '(' << (base ? getImportAlias(base) : "Ice_Value") << "):";
    out.inc();

    writeDocstring(DocComment::parseFrom(p), members, out);

    // __init__
    out << nl << "def __init__(";
    writeConstructorParams(p->allDataMembers(), out);
    out << "):";
    out.inc();

    out << nl << "super().__init__";
    out.spar("(");
    if (base)
    {
        for (const auto& member : base->allDataMembers())
        {
            out << member->mappedName();
        }
    }
    out.epar(")");

    for (const auto& member : members)
    {
        writeAssign(member, out);
    }
    out.dec();

    // ice_id
    out << sp;
    out << nl << "def ice_id(self):";
    out.inc();
    out << nl << "return \"" << scoped << "\"";
    out.dec();

    // ice_staticId
    out << sp;
    out << nl << "@staticmethod";
    out << nl << "def ice_staticId():";
    out.inc();
    out << nl << "return \"" << scoped << "\"";
    out.dec();

    // Generate the __repr__ method for this Value class.
    // The default __str__ method inherited from Ice.Value calls __repr__().
    out << sp;
    out << nl << "def __repr__(self):";
    out.inc();
    const auto& allDataMembers = p->allDataMembers();
    if (allDataMembers.empty())
    {
        out << nl << "return \"" << getImportAlias(p) << "()\"";
    }
    else
    {
        out << nl << "return f\"" << getImportAlias(p) << "(" << formatFields(allDataMembers) << ")\"";
    }
    out.dec();

    out.dec();

    out << sp;
    out << nl << metaType << " = IcePy.defineValue(";
    out.inc();
    out << nl << "\"" << scoped << "\",";
    out << nl << valueName << ",";
    out << nl << p->compactId() << ",";
    out << nl;
    writeMetadata(p->getMetadata(), out);
    out << ",";
    out << nl << "False,";
    out << nl << (base ? getMetaType(base) : "None") << ",";
    out << nl << "(";

    // Members
    //
    // Data members are represented as a tuple:
    //
    //   ("MemberName", MemberMetadata, MemberType, Optional, Tag)
    //
    // where MemberType is either a primitive type constant (T_INT, etc.) or the id of a user-defined type.
    if (members.size() > 1)
    {
        out.inc();
        out << nl;
    }

    for (auto r = members.begin(); r != members.end(); ++r)
    {
        if (r != members.begin())
        {
            out << ',' << nl;
        }
        out << "(\"" << (*r)->mappedName() << "\", ";
        writeMetadata((*r)->getMetadata(), out);
        out << ", " << getMetaType((*r)->type());
        out << ", " << ((*r)->optional() ? "True" : "False") << ", " << ((*r)->optional() ? (*r)->tag() : 0) << ')';
    }

    if (members.size() == 1)
    {
        out << ',';
    }
    else if (members.size() > 1)
    {
        out.dec();
        out << nl;
    }
    out << "))";
    out.dec();

    out << sp;
    out << nl << valueName << "._ice_type = " << metaType;

    out << sp;
    out << nl << "__all__ = [\"" << valueName << "\", \"" << metaType << "\"]";

    _codeFragments.push_back({p, out.str()});
    return false;
}

bool
Slice::Python::CodeVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    string scoped = p->scoped();
    string className = p->mappedName();
    string prxName = className + "Prx";
    string metaType = getMetaType(p);
    InterfaceList bases = p->bases();

    // Emit a forward declarations for the proxy meta type.
    BufferedOutput outF;
    outF << nl << metaType << " = IcePy.declareProxy(\"" << scoped << "\")";
    outF << sp;
    outF << nl << "__all__ = [\"" << metaType << "\"]";
    _codeFragments.push_back({p->declaration(), outF.str()});

    // Emit the proxy class.
    BufferedOutput out;
    out << sp;
    out << nl << "class " << prxName << '(';

    vector<string> baseClasses;
    for (const auto& base : bases)
    {
        InterfaceDefPtr d = base;
        baseClasses.push_back(getImportAlias(base) + "Prx");
    }

    if (baseClasses.empty())
    {
        out << "Ice_ObjectPrx";
    }
    else
    {
        auto q = baseClasses.begin();
        while (q != baseClasses.end())
        {
            out << *q;

            if (++q != baseClasses.end())
            {
                out << ", ";
            }
        }
    }
    out << "):";
    out.inc();

    out << sp;
    out << nl << "def __init__(self, communicator: Ice.Communicator, proxyString: str):";
    out.inc();
    out << nl << tripleQuotes;
    out << nl << "Creates a new " << prxName << " proxy";
    out << nl;
    out << nl << "Parameters";
    out << nl << "----------";
    out << nl << "communicator : Ice.Communicator";
    out << nl << "    The communicator of the new proxy.";
    out << nl << "proxyString : str";
    out << nl << "    The string representation of the proxy.";
    out << nl;
    out << nl << "Raises";
    out << nl << "------";
    out << nl << "ParseException";
    out << nl << "    Thrown when proxyString is not a valid proxy string.";
    out << nl << tripleQuotes;
    out << nl << "super().__init__(communicator, proxyString)";
    out.dec();

    OperationList operations = p->operations();
    for (const auto& operation : operations)
    {
        const string opName = operation->name();
        string mappedOpName = operation->mappedName();
        if (mappedOpName == "checkedCast" || mappedOpName == "uncheckedCast")
        {
            mappedOpName.insert(0, "_");
        }
        TypePtr ret = operation->returnType();
        ParameterList paramList = operation->parameters();
        string inParams;
        string inParamsDecl;

        // Find the last required parameter, all optional parameters after the last required parameter will use
        // None as the default.
        ParameterPtr lastRequiredParameter;
        for (const auto& q : operation->inParameters())
        {
            if (!q->optional())
            {
                lastRequiredParameter = q;
            }
        }

        bool afterLastRequiredParameter = lastRequiredParameter == nullptr;
        for (const auto& q : operation->inParameters())
        {
            if (!inParams.empty())
            {
                inParams.append(", ");
                inParamsDecl.append(", ");
            }
            string param = q->mappedName();
            inParams.append(param);
            param += ": " + typeToTypeHintString(q->type(), q->optional(), p);
            if (afterLastRequiredParameter)
            {
                param += " = None";
            }
            inParamsDecl.append(param);

            if (q == lastRequiredParameter)
            {
                afterLastRequiredParameter = true;
            }
        }

        out << sp;
        out << nl << "def " << mappedOpName << "(self";
        if (!inParamsDecl.empty())
        {
            out << ", " << inParamsDecl;
        }
        const string contextParamName = getEscapedParamName(operation->parameters(), "context");
        out << ", " << contextParamName << ": dict[str, str] | None = None)"
            << operationReturnTypeHint(operation, SyncInvocation) << ":";
        out.inc();
        writeDocstring(operation, SyncInvocation, out);
        out << nl << "return " << className << "._op_" << opName << ".invoke(self, ((" << inParams;
        if (!inParams.empty() && inParams.find(',') == string::npos)
        {
            out << ", ";
        }
        out << "), " << contextParamName << "))";
        out.dec();

        // Async operations.
        out << sp;
        out << nl << "def " << mappedOpName << "Async(self";
        if (!inParams.empty())
        {
            out << ", " << inParamsDecl;
        }
        out << ", " << contextParamName << ": dict[str, str] | None = None)"
            << operationReturnTypeHint(operation, AsyncInvocation) << ":";
        out.inc();
        writeDocstring(operation, AsyncInvocation, out);
        out << nl << "return " << className << "._op_" << opName << ".invokeAsync(self, ((" << inParams;
        if (!inParams.empty() && inParams.find(',') == string::npos)
        {
            out << ", ";
        }
        out << "), " << contextParamName << "))";
        out.dec();
    }

    const string prxTypeHint = typeToTypeHintString(p->declaration(), false, p);

    out << sp;
    out << nl << "@staticmethod";
    out << nl << "def checkedCast(";
    out.inc();
    out << nl << "proxy: Ice_ObjectPrx | None,";
    out << nl << "facet: str | None = None,";
    out << nl << "context: dict[str, str] | None = None";
    out.dec();
    out << nl << ") -> " << prxTypeHint << ":";
    out.inc();
    out << nl << "return Ice_checkedCast(" << prxName << ", proxy, facet, context)";
    out.dec();

    out << sp;
    out << nl << "@staticmethod";
    out << nl << "def checkedCastAsync(";
    out.inc();
    out << nl << "proxy: Ice_ObjectPrx | None,";
    out << nl << "facet: str | None = None,";
    out << nl << "context: dict[str, str] | None = None";
    out.dec();
    out << nl << ") -> Awaitable[" << prxTypeHint << "]:";
    out.inc();
    out << nl << "return Ice_checkedCastAsync(" << prxName << ", proxy, facet, context)";
    out.dec();

    out << sp << nl << "@staticmethod";
    out << nl << "def uncheckedCast(proxy: Ice_ObjectPrx | None, facet: str | None = None) -> " << prxTypeHint << ":";
    out.inc();
    out << nl << "return Ice_uncheckedCast(" << prxName << ", proxy, facet)";
    out.dec();

    // ice_staticId
    out << sp;
    out << nl << "@staticmethod";
    out << nl << "def ice_staticId() -> str:";
    out.inc();
    out << nl << "return \"" << scoped << "\"";
    out.dec();

    out.dec(); // end prx class

    out << sp;
    out << nl << "IcePy.defineProxy(\"" << scoped << "\", " << prxName << ")";

    // Emit the servant class (to the same code fragment as the proxy class).
    out << sp;
    out << nl << "class " << className;
    out << spar;
    if (bases.empty())
    {
        out << "Ice_Object";
    }
    else
    {
        for (const auto& base : bases)
        {
            out << getImportAlias(base);
        }
    }
    out << "ABC" << epar << ':';
    out.inc();

    // ice_ids
    StringList ids = p->ids();
    out << sp;
    out << nl << "def ice_ids(self, current: Ice.Current) -> Sequence[str] | Awaitable[Sequence[str]]:";
    out.inc();
    out << nl << "return (";
    for (auto q = ids.begin(); q != ids.end(); ++q)
    {
        if (q != ids.begin())
        {
            out << ", ";
        }
        out << "\"" << *q << "\"";
    }
    out << ')';
    out.dec();

    // ice_id
    out << sp;
    out << nl << "def ice_id(self, current: Ice.Current) -> str | Awaitable[str]:";
    out.inc();
    out << nl << "return \"" << scoped << "\"";
    out.dec();

    // ice_staticId
    out << sp;
    out << nl << "@staticmethod";
    out << nl << "def ice_staticId() -> str:";
    out.inc();
    out << nl << "return \"" << scoped << "\"";
    out.dec();

    writeOperations(p, out);

    out.dec();

    //
    // Define each operation. The arguments to the IcePy.Operation constructor are:
    //
    // "sliceOpName", "mappedOpName", Mode, AMD, Format, Metadata, (InParams), (OutParams), ReturnParam, (Exceptions)
    //
    // where InParams and OutParams are tuples of type descriptions, and Exceptions
    // is a tuple of exception type ids.

    for (const auto& operation : operations)
    {
        string format;
        optional<FormatType> opFormat = operation->format();
        if (opFormat)
        {
            switch (*opFormat)
            {
                case CompactFormat:
                    format = "Ice_FormatType.CompactFormat";
                    break;
                case SlicedFormat:
                    format = "Ice_FormatType.SlicedFormat";
                    break;
                default:
                    assert(false);
            }
        }
        else
        {
            format = "None";
        }

        const string sliceName = operation->name();

        out << sp;
        out << nl << className << "._op_" << sliceName << " = IcePy.Operation(";
        out.inc();
        out << nl << "\"" << sliceName << "\",";
        out << nl << "\"" << operation->mappedName() << "\",";
        out << nl << getOperationMode(operation->mode()) << ",";
        out << nl << format << ",";
        out << nl;
        writeMetadata(operation->getMetadata(), out);
        out << ",";
        out << nl << "(";
        for (const auto& param : operation->inParameters())
        {
            if (param != operation->inParameters().front())
            {
                out << ", ";
            }
            out << '(';
            writeMetadata(param->getMetadata(), out);
            out << ", " << getMetaType(param->type());
            out << ", ";
            if (param->optional())
            {
                out << "True, " << param->tag();
            }
            else
            {
                out << "False, 0";
            }
            out << ')';
        }

        // A trailing command to ensure that the outut is interpreted as a Python tuple.
        if (operation->inParameters().size() == 1)
        {
            out << ',';
        }
        out << "),";
        out << nl << "(";
        for (const auto& param : operation->outParameters())
        {
            if (param != operation->outParameters().front())
            {
                out << ", ";
            }
            out << '(';
            writeMetadata(param->getMetadata(), out);
            out << ", " << getMetaType(param->type());
            out << ", ";
            if (param->optional())
            {
                out << "True, " << param->tag();
            }
            else
            {
                out << "False, 0";
            }
            out << ')';
        }

        // A trailing command to ensure that the outut is interpreted as a Python tuple.
        if (operation->outParameters().size() == 1)
        {
            out << ',';
        }
        out << "),";

        out << nl;
        TypePtr returnType = operation->returnType();
        if (returnType)
        {
            // The return type has the same format as an in/out parameter:
            //
            // Metadata, Type, Optional?, OptionalTag
            out << "((), " << getMetaType(returnType) << ", ";
            if (operation->returnIsOptional())
            {
                out << "True, " << operation->returnTag();
            }
            else
            {
                out << "False, 0";
            }
            out << ')';
        }
        else
        {
            out << "None";
        }
        out << ",";
        out << nl << "(";
        for (const auto& ex : operation->throws())
        {
            if (ex != operation->throws().front())
            {
                out << ", ";
            }
            out << getMetaType(ex);
        }

        // A trailing command to ensure that the outut is interpreted as a Python tuple.
        if (operation->throws().size() == 1)
        {
            out << ',';
        }
        out << "))";
        out.dec();

        if (operation->isDeprecated())
        {
            // Get the deprecation reason if present, or default to an empty string.
            string reason = operation->getDeprecationReason().value_or("");
            out << nl << className << "._op_" << sliceName << ".deprecate(\"" << reason << "\")";
        }
    }

    out << sp;
    out << nl << "__all__ = [\"" << className << "\", \"" << prxName << "\", \"" << metaType << "\"]";

    _codeFragments.push_back({p, out.str()});
    return false;
}

bool
Slice::Python::CodeVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    const string scoped = p->scoped();
    const string name = p->mappedName();

    const ExceptionPtr base = p->base();
    BufferedOutput out;

    const DataMemberList members = p->dataMembers();

    out << sp;
    out << nl << "class " << name << '(' << (base ? getImportAlias(base) : "Ice_UserException") << "):";
    out.inc();

    writeDocstring(DocComment::parseFrom(p), members, out);

    // __init__
    out << nl << "def __init__(";
    writeConstructorParams(p->allDataMembers(), out);
    out << "):";
    out.inc();

    out << nl << "super().__init__";
    out.spar("(");
    if (base)
    {
        for (const auto& member : base->allDataMembers())
        {
            out << member->mappedName();
        }
    }
    out.epar(")");

    for (const auto& member : members)
    {
        writeAssign(member, out);
    }

    out.dec();

    // Generate the __repr__ method for this Exception class.
    // The default __str__ method inherited from Ice.UserException calls __repr__().
    out << sp;
    out << nl << "def __repr__(self) -> str:";
    out.inc();
    const auto& allDataMembers = p->allDataMembers();
    if (allDataMembers.empty())
    {
        out << nl << "return \"" << getImportAlias(p) << "()\"";
    }
    else
    {
        out << nl << "return f\"" << getImportAlias(p) << "(" << formatFields(p->allDataMembers()) << ")\"";
    }
    out.dec();

    // _ice_id
    out << sp;
    out << nl << "_ice_id = \"" << scoped << "\"";

    out.dec();

    // Emit the type information.
    string metaType = getMetaType(p);
    out << sp;
    out << nl << metaType << " = IcePy.defineException(";
    out.inc();
    out << nl << "\"" << scoped << "\",";
    out << nl << name << ",";
    out << nl;
    writeMetadata(p->getMetadata(), out);
    out << ",";
    out << nl << (base ? getMetaType(base) : "None") << ",";

    out << nl << "(";
    if (members.size() > 1)
    {
        out.inc();
        out << nl;
    }
    //
    // Data members are represented as a tuple:
    //
    //   ("MemberName", MemberMetadata, MemberType, Optional, Tag)
    //
    // where MemberType is either a primitive type constant (T_INT, etc.) or the id of a user-defined type.
    //
    for (auto dmli = members.begin(); dmli != members.end(); ++dmli)
    {
        if (dmli != members.begin())
        {
            out << ',' << nl;
        }
        out << "(\"" << (*dmli)->mappedName() << "\", ";
        writeMetadata((*dmli)->getMetadata(), out);
        out << ", " << getMetaType((*dmli)->type());
        out << ", " << ((*dmli)->optional() ? "True" : "False") << ", " << ((*dmli)->optional() ? (*dmli)->tag() : 0)
            << ')';
    }
    if (members.size() == 1)
    {
        out << ',';
    }
    else if (members.size() > 1)
    {
        out.dec();
        out << nl;
    }
    out << "))";
    out.dec();

    out << sp;
    out << nl << name << "._ice_type = " << metaType;

    out << sp;
    out << nl << "__all__ = [\"" << name << "\", \"" << metaType << "\"]";

    _codeFragments.push_back({p, out.str()});
    return false;
}

bool
Slice::Python::CodeVisitor::visitStructStart(const StructPtr& p)
{
    const string scoped = p->scoped();
    const string name = p->mappedName();
    const string metaTypeName = getMetaType(p);
    const DataMemberList members = p->dataMembers();
    BufferedOutput out;

    out << sp;
    out << nl << "@dataclass";
    if (Dictionary::isLegalKeyType(p))
    {
        out << "(order=True, unsafe_hash=True)";
    }

    out << nl << "class " << name << ":";
    out.inc();

    writeDocstring(DocComment::parseFrom(p), members, out);

    for (const auto& field : members)
    {
        out << nl << field->mappedName() << ": " << typeToTypeHintString(field->type(), field->optional(), p);

        if (field->defaultValue())
        {
            out << " = ";
            writeConstantValue(field->type(), field->defaultValueType(), *field->defaultValue(), out);
        }
        else if (field->optional())
        {
            out << " = None";
        }
        else if (auto st = dynamic_pointer_cast<Struct>(field->type()))
        {
            // See writeAssign.
            out << " = " << "field(default_factory=" << getImportAlias(st) << ')';
        }
        else
        {
            out << " = " + getTypeInitializer(field);
        }
    }
    out.dec();

    //
    // Emit the type information.
    //
    out << sp;
    out << nl << metaTypeName << " = IcePy.defineStruct(";
    out.inc();
    out << nl << "\"" << scoped << "\",";
    out << nl << name << ",";
    out << nl;
    writeMetadata(p->getMetadata(), out);
    out << ",";
    //
    // Data members are represented as a tuple:
    //
    //   ("MemberName", MemberMetadata, MemberType)
    //
    // where MemberType is either a primitive type constant (T_INT, etc.) or the id of a user-defined type.
    //
    out << nl << "(";
    if (members.size() > 1)
    {
        out.inc();
        out << nl;
    }

    for (auto r = members.begin(); r != members.end(); ++r)
    {
        if (r != members.begin())
        {
            out << ',' << nl;
        }
        out << "(\"" << (*r)->mappedName() << "\", ";
        writeMetadata((*r)->getMetadata(), out);
        out << ", " << getMetaType((*r)->type());
        out << ')';
    }

    if (members.size() == 1)
    {
        out << ',';
    }
    else if (members.size() > 1)
    {
        out.dec();
        out << nl;
    }
    out << "))";
    out.dec();

    out << sp;
    out << nl << "__all__ = [\"" << name << "\", \"" << metaTypeName << "\"]";

    _codeFragments.push_back({p, out.str()});
    return false;
}

void
Slice::Python::CodeVisitor::visitSequence(const SequencePtr& p)
{
    string metaType = getMetaType(p);

    BufferedOutput out;
    out << nl << metaType << " = IcePy.defineSequence(\"" << p->scoped() << "\", ";
    writeMetadata(p->getMetadata(), out);
    out << ", " << getMetaType(p->type());
    out << ")";

    out << sp;
    out << nl << "__all__ = [\"" << metaType << "\"]";

    _codeFragments.push_back({p, out.str()});
}

void
Slice::Python::CodeVisitor::visitDictionary(const DictionaryPtr& p)
{
    BufferedOutput out;
    string metaType = getMetaType(p);
    out << nl << metaType << " = IcePy.defineDictionary(\"" << p->scoped() << "\", ";
    writeMetadata(p->getMetadata(), out);
    out << ", " << getMetaType(p->keyType()) << ", " << getMetaType(p->valueType()) << ")";

    out << sp;
    out << nl << "__all__ = [\"" << metaType << "\"]";

    _codeFragments.push_back({p, out.str()});
}

void
Slice::Python::CodeVisitor::visitEnum(const EnumPtr& p)
{
    string scoped = p->scoped();
    string name = p->mappedName();
    EnumeratorList enumerators = p->enumerators();

    BufferedOutput out;
    out << nl << "class " << name << "(Enum):";
    out.inc();

    writeDocstring(DocComment::parseFrom(p), p, out);

    out << nl;
    for (const auto& enumerator : enumerators)
    {
        out << nl << enumerator->mappedName() << " = " << enumerator->value();
    }

    out.dec();

    // Meta type definition.
    out << sp;
    out << nl << getMetaType(p) << " = IcePy.defineEnum(";
    out.inc();
    out << nl << "\"" << scoped << "\",";
    out << nl << name << ",";
    out << nl;
    writeMetadata(p->getMetadata(), out);
    out << ",";
    out << nl;
    out.spar("{");
    for (const auto& enumerator : enumerators)
    {
        out << (std::to_string(enumerator->value()) + ": " + name + "." + enumerator->mappedName());
    }
    out.epar("}");
    out << ")";
    out.dec();

    out << sp;
    out << nl << "__all__ = [\"" << name << "\", \"" << getMetaType(p) << "\"]";

    _codeFragments.push_back({p, out.str()});
}

void
Slice::Python::CodeVisitor::visitConst(const ConstPtr& p)
{
    string name = p->mappedName();
    BufferedOutput out;
    out << sp;
    out << nl << name << " = ";
    writeConstantValue(p->type(), p->valueType(), p->value(), out);

    out << sp;
    out << nl << "__all__ = [\"" << name << "\"]";

    _codeFragments.push_back({p, out.str()});
}

string
Slice::Python::CodeVisitor::getTypeInitializer(const DataMemberPtr& field)
{
    static constexpr string_view builtinTable[] = {
        "0",     // Builtin::KindByte
        "False", // Builtin::KindBool
        "0",     // Builtin::KindShort
        "0",     // Builtin::KindInt
        "0",     // Builtin::KindLong
        "0.0",   // Builtin::KindFloat
        "0.0",   // Builtin::KindDouble
        R"("")", // Builtin::KindString
        "None",  // Builtin::KindObject. Not used anymore
        "None",  // Builtin::KindObjectProxy.
        "None"}; // Builtin::KindValue.

    if (auto builtin = dynamic_pointer_cast<Builtin>(field->type()))
    {
        return string{builtinTable[builtin->kind()]};
    }
    else if (auto enumeration = dynamic_pointer_cast<Enum>(field->type()))
    {
        return getImportAlias(enumeration) + "." + enumeration->enumerators().front()->mappedName();
    }
    else
    {
        return "None";
    }
}

void
Slice::Python::CodeVisitor::writeMetadata(const MetadataList& metadata, Output& out)
{
    MetadataList pythonMetadata = metadata;
    auto newEnd = std::remove_if(
        pythonMetadata.begin(),
        pythonMetadata.end(),
        [](const MetadataPtr& meta) { return meta->directive().find("python:") != 0; });
    pythonMetadata.erase(newEnd, pythonMetadata.end());

    out << '(';
    for (const auto& meta : pythonMetadata)
    {
        out << "\"" << *meta << "\"";
        if (meta != pythonMetadata.back())
        {
            out << ", ";
        }
    }

    if (pythonMetadata.size() == 1)
    {
        out << ',';
    }

    out << ')';
}

void
Slice::Python::CodeVisitor::writeAssign(const DataMemberPtr& member, Output& out)
{
    const string memberName = member->mappedName();

    // Structures are treated differently (see bug 3676).
    StructPtr st = dynamic_pointer_cast<Struct>(member->type());
    if (st && !member->optional())
    {
        out << nl << "self." << memberName << " = " << memberName << " if " << memberName << " is not None else "
            << getImportAlias(st) << "()";
    }
    else
    {
        out << nl << "self." << memberName << " = " << memberName;
    }
}

void
Slice::Python::CodeVisitor::writeConstantValue(
    const TypePtr& type,
    const SyntaxTreeBasePtr& valueType,
    const string& value,
    Output& out)
{
    ConstPtr constant = dynamic_pointer_cast<Const>(valueType);
    if (constant)
    {
        out << getImportAlias(constant);
    }
    else if (auto builtin = dynamic_pointer_cast<Slice::Builtin>(type))
    {
        switch (builtin->kind())
        {
            case Slice::Builtin::KindBool:
            {
                out << (value == "true" ? "True" : "False");
                break;
            }
            case Slice::Builtin::KindByte:
            case Slice::Builtin::KindShort:
            case Slice::Builtin::KindInt:
            case Slice::Builtin::KindFloat:
            case Slice::Builtin::KindDouble:
            case Slice::Builtin::KindLong:
            {
                out << value;
                break;
            }
            case Slice::Builtin::KindString:
            {
                const string controlChars = "\a\b\f\n\r\t\v";
                const unsigned char cutOff = 0;

                out << "\"" << toStringLiteral(value, controlChars, "", UCN, cutOff) << "\"";
                break;
            }
            case Slice::Builtin::KindValue:
            case Slice::Builtin::KindObject:
            case Slice::Builtin::KindObjectProxy:
                assert(false);
        }
    }
    else if (auto enumeration = dynamic_pointer_cast<Slice::Enum>(type))
    {
        EnumeratorPtr enumerator = dynamic_pointer_cast<Enumerator>(valueType);
        assert(enumerator);
        out << getImportAlias(enumeration) << "." << enumerator->mappedName();
    }
    else
    {
        assert(false); // Unknown const type.
    }
}

void
Slice::Python::CodeVisitor::writeConstructorParams(const DataMemberList& members, Output& out)
{
    out << "self";
    for (const auto& member : members)
    {
        out << ", " << member->mappedName() << "=";
        if (member->defaultValue())
        {
            writeConstantValue(member->type(), member->defaultValueType(), *member->defaultValue(), out);
        }
        else if (member->optional())
        {
            out << "None";
        }
        else
        {
            out << getTypeInitializer(member);
        }
    }
}

string
Slice::Python::CodeVisitor::getOperationMode(Slice::Operation::Mode mode)
{
    return mode == Slice::Operation::Mode::Normal ? "Ice_OperationMode.Normal" : "Ice_OperationMode.Idempotent";
}

void
Slice::Python::CodeVisitor::writeRemarksDocComment(const StringList& remarks, bool needsNewline, Output& out)
{
    if (!remarks.empty())
    {
        if (needsNewline)
        {
            out << nl;
        }
        out << nl << "Notes";
        out << nl << "-----";
        for (const auto& line : remarks)
        {
            out << nl << "    " << line;
        }
    }
}

void
Slice::Python::CodeVisitor::writeDocstring(const optional<DocComment>& comment, const string& prefix, Output& out)
{
    if (!comment)
    {
        return;
    }

    const StringList& overview = comment->overview();
    const StringList& remarks = comment->remarks();
    if (overview.empty() && remarks.empty())
    {
        return;
    }

    out << nl << prefix << tripleQuotes;
    for (const auto& line : overview)
    {
        out << nl << line;
    }

    writeRemarksDocComment(remarks, !overview.empty(), out);

    out << nl << tripleQuotes;
}

void
Slice::Python::CodeVisitor::writeDocstring(
    const optional<DocComment>& comment,
    const DataMemberList& members,
    Output& out)
{
    if (!comment)
    {
        return;
    }

    // Collect docstrings (if any) for the members.
    map<string, list<string>> docs;
    for (const auto& member : members)
    {
        if (auto memberDoc = DocComment::parseFrom(member))
        {
            const StringList& memberOverview = memberDoc->overview();
            if (!memberOverview.empty())
            {
                docs[member->name()] = memberOverview;
            }
        }
    }

    const StringList& overview = comment->overview();
    const StringList& remarks = comment->remarks();
    if (overview.empty() && remarks.empty() && docs.empty())
    {
        return;
    }

    out << nl << tripleQuotes;

    for (const auto& line : overview)
    {
        out << nl << line;
    }

    // Only emit members if there's a docstring for at least one member.
    if (!docs.empty())
    {
        if (!overview.empty())
        {
            out << nl;
        }
        out << nl << "Attributes";
        out << nl << "----------";
        for (const auto& member : members)
        {
            ;
            out << nl << member->mappedName() << " : "
                << typeToTypeHintString(
                       member->type(),
                       member->optional(),
                       dynamic_pointer_cast<Contained>(member->container()));
            auto p = docs.find(member->name());
            if (p != docs.end())
            {
                for (const auto& line : p->second)
                {
                    out << nl << "    " << line;
                }
            }
        }
    }

    writeRemarksDocComment(remarks, !overview.empty() || !docs.empty(), out);

    out << nl << tripleQuotes;
}

void
Slice::Python::CodeVisitor::writeDocstring(const optional<DocComment>& comment, const EnumPtr& enumeration, Output& out)
{
    if (!comment)
    {
        return;
    }

    // Collect docstrings (if any) for the enumerators.
    const EnumeratorList& enumerators = enumeration->enumerators();
    map<string, list<string>> docs;
    for (const auto& enumerator : enumerators)
    {
        if (auto enumeratorDoc = DocComment::parseFrom(enumerator))
        {
            const StringList& enumeratorOverview = enumeratorDoc->overview();
            if (!enumeratorOverview.empty())
            {
                docs[enumerator->name()] = enumeratorOverview;
            }
        }
    }

    const StringList& overview = comment->overview();
    const StringList& remarks = comment->remarks();
    if (overview.empty() && remarks.empty() && docs.empty())
    {
        return;
    }

    out << nl << tripleQuotes;

    for (const auto& line : overview)
    {
        out << nl << line;
    }

    // Only emit enumerators if there's a docstring for at least one enumerator.
    if (!docs.empty())
    {
        if (!overview.empty())
        {
            out << nl;
        }
        out << nl << "Enumerators:";
        for (const auto& enumerator : enumerators)
        {
            out << nl << nl << "- " << enumerator->mappedName();
            auto p = docs.find(enumerator->name());
            if (p != docs.end())
            {
                out << ":"; // Only emit a trailing ':' if there's documentation to emit for it.
                for (const auto& line : p->second)
                {
                    out << nl << "    " << line;
                }
            }
        }
    }

    writeRemarksDocComment(remarks, !overview.empty() || !docs.empty(), out);

    out << nl << tripleQuotes;
}

void
Slice::Python::CodeVisitor::writeDocstring(const OperationPtr& op, MethodKind methodKind, Output& out)
{
    optional<DocComment> comment = DocComment::parseFrom(op);
    if (!comment)
    {
        return;
    }

    auto p = dynamic_pointer_cast<Contained>(op->container());

    TypePtr returnType = op->returnType();
    ParameterList params = op->parameters();
    ParameterList inParams = op->inParameters();
    ParameterList outParams = op->outParameters();

    const StringList& overview = comment->overview();
    const StringList& remarks = comment->remarks();
    const StringList& returnsDoc = comment->returns();
    const auto& parametersDoc = comment->parameters();
    const auto& exceptionsDoc = comment->exceptions();

    if (overview.empty() && remarks.empty())
    {
        if ((methodKind == SyncInvocation || methodKind == Dispatch) && parametersDoc.empty() &&
            exceptionsDoc.empty() && returnsDoc.empty())
        {
            return;
        }
        else if (methodKind == AsyncInvocation && inParams.empty())
        {
            return;
        }
        else if (methodKind == Dispatch && inParams.empty() && exceptionsDoc.empty())
        {
            return;
        }
    }

    // Emit the general description.
    out << nl << tripleQuotes;
    for (const string& line : overview)
    {
        out << nl << line;
    }

    // Emit arguments.
    bool needArgs = false;
    switch (methodKind)
    {
        case SyncInvocation:
        case AsyncInvocation:
        case Dispatch:
            needArgs = true;
            break;
    }

    if (needArgs)
    {
        if (!overview.empty())
        {
            out << nl;
        }

        out << nl << "Parameters";
        out << nl << "----------";
        for (const auto& param : inParams)
        {
            out << nl << param->mappedName() << " : " << typeToTypeHintString(param->type(), param->optional(), p);
            const auto r = parametersDoc.find(param->name());
            if (r != parametersDoc.end())
            {
                for (const auto& line : r->second)
                {
                    out << nl << "    " << line;
                }
            }
        }

        if (methodKind == SyncInvocation || methodKind == AsyncInvocation)
        {
            const string contextParamName = getEscapedParamName(op->parameters(), "context");
            out << nl << contextParamName << " : dict[str, str]";
            out << nl << "    The request context for the invocation.";
        }

        if (methodKind == Dispatch)
        {
            const string currentParamName = getEscapedParamName(op->parameters(), "current");
            out << nl << currentParamName << " : Ice.Current";
            out << nl << "    The Current object for the dispatch.";
        }
    }

    // Emit return value(s).
    bool hasReturnValue = false;
    if (!op->returnsAnyValues() && (methodKind == AsyncInvocation || methodKind == Dispatch))
    {
        hasReturnValue = true;
        if (!overview.empty() || needArgs)
        {
            out << nl;
        }
        out << nl << "Returns";
        out << nl << "-------";
        out << nl << returnTypeHint(op, methodKind);
        if (methodKind == AsyncInvocation)
        {
            out << nl << "    An awaitable that is completed when the invocation completes.";
        }
        else if (methodKind == Dispatch)
        {
            out << nl << "    None or an awaitable that completes when the dispatch completes.";
        }
    }
    else if (op->returnsAnyValues())
    {
        hasReturnValue = true;
        if (!overview.empty() || needArgs)
        {
            out << nl;
        }
        out << nl << "Returns";
        out << nl << "-------";
        out << nl << returnTypeHint(op, methodKind);

        if (op->returnsMultipleValues())
        {
            out << nl;
            out << nl << "    A tuple containing:";
            if (returnType)
            {
                out << nl << "        - " << typeToTypeHintString(returnType, op->returnIsOptional(), p);
                bool firstLine = true;
                for (const string& line : returnsDoc)
                {
                    if (firstLine)
                    {
                        firstLine = false;
                        out << " " << line;
                    }
                    else
                    {
                        out << nl << "          " << line;
                    }
                }
            }

            for (const auto& param : outParams)
            {
                out << nl << "        - " << typeToTypeHintString(param->type(), param->optional(), p);
                const auto r = parametersDoc.find(param->name());
                if (r != parametersDoc.end())
                {
                    bool firstLine = true;
                    for (const string& line : r->second)
                    {
                        if (firstLine)
                        {
                            firstLine = false;
                            out << " " << line;
                        }
                        else
                        {
                            out << nl << "          " << line;
                        }
                    }
                }
            }
        }
        else if (returnType)
        {
            for (const string& line : returnsDoc)
            {
                out << nl << "    " << line;
            }
        }
        else if (!outParams.empty())
        {
            assert(outParams.size() == 1);
            const auto& param = outParams.front();
            out << nl << typeToTypeHintString(param->type(), param->optional(), p);
            const auto r = parametersDoc.find(param->name());
            if (r != parametersDoc.end())
            {
                for (const string& line : r->second)
                {
                    out << nl << "    " << line;
                }
            }
        }
    }

    // Emit exceptions.
    if ((methodKind == SyncInvocation || methodKind == Dispatch) && !exceptionsDoc.empty())
    {
        if (!overview.empty() || needArgs || hasReturnValue)
        {
            out << nl;
        }
        out << nl << "Raises";
        out << nl << "------";
        for (const auto& [exception, exceptionDescription] : exceptionsDoc)
        {
            out << nl << exception;
            for (const auto& line : exceptionDescription)
            {
                out << nl << "    " << line;
            }
        }
    }

    writeRemarksDocComment(remarks, true, out);

    out << nl << tripleQuotes;
}

namespace
{
    Output& getModuleOutputFile(
        const string& moduleName,
        std::map<string, unique_ptr<Output>>& outputFiles,
        const string& outputDir)
    {
        auto it = outputFiles.find(moduleName);
        if (it != outputFiles.end())
        {
            return *it->second;
        }

        // Create a new output file for this module.
        string fileName = moduleName;
        replace(fileName.begin(), fileName.end(), '.', '/');
        fileName += ".py";

        string outputPath;
        if (!outputDir.empty())
        {
            outputPath = outputDir + "/";
        }
        else
        {
            outputPath = "./";
        }
        Slice::Python::createPackagePath(moduleName, outputPath);
        outputPath += fileName;

        FileTracker::instance()->addFile(outputPath);

        auto output = make_unique<Output>(outputPath.c_str());
        Output& out = *output;
        Slice::Python::writeHeader(out);

        out << sp;
        out << nl << "from __future__ import annotations";
        out << nl << "import IcePy";

        outputFiles[moduleName] = std::move(output);
        return out;
    }
}

void
Slice::Python::generate(const Slice::UnitPtr& unit, const std::string& outputDir)
{
    validatePythonMetadata(unit);

    ImportVisitor importVisitor;
    unit->visit(&importVisitor);

    CodeVisitor codeVisitor;
    unit->visit(&codeVisitor);

    const string fileBaseName = baseName(removeExtension(unit->topLevelFile()));

    // A map from python module names to output files.
    std::map<string, unique_ptr<Output>> outputFiles;

    // Write the runtime imports.
    ImportsMap runtimeImports = importVisitor.getRuntimeImports();
    for (const auto& [sourceModuleName, imports] : runtimeImports)
    {
        Output& out = getModuleOutputFile(sourceModuleName, outputFiles, outputDir);
        for (const auto& [moduleName, definitions] : imports)
        {
            out << sp;
            for (const auto& [name, alias] : definitions)
            {
                out << nl << "from " << moduleName << " import " << name << " as " << (alias.empty() ? name : alias);
            }
        }
    }

    // Write typing imports
    ImportsMap typingImports = importVisitor.getTypingImports();
    for (const auto& [file, imports] : typingImports)
    {
        Output& out = getModuleOutputFile(file, outputFiles, outputDir);
        out << sp;
        out << nl << "if TYPE_CHECKING:";
        out.inc();
        for (const auto& [moduleName, definitions] : imports)
        {
            if (!definitions.empty())
            {
                out << sp;
                for (const auto& [name, alias] : definitions)
                {
                    // TODO skip type hints that are already imported as part of the runtime imports.
                    out << nl << "from " << moduleName << " import " << name << " as "
                        << (alias.empty() ? name : alias);
                }
            }
            else
            {
                // If there are no definitions, we still need to import the module to ensure that the type hints
                // are available.
                out << nl << "import " << moduleName;
            }
        }
        out.dec();
    }

    // Emit the code fragments for the unit.
    for (const auto& fragment : codeVisitor.codeFragments())
    {
        string moduleName;
        if (dynamic_pointer_cast<ClassDecl>(fragment.contained) ||
            dynamic_pointer_cast<InterfaceDecl>(fragment.contained))
        {
            moduleName = getPythonModuleForForwardDeclaration(fragment.contained);
        }
        else
        {
            moduleName = getPythonModuleForDefinition(fragment.contained);
        }

        Output& out = getModuleOutputFile(moduleName, outputFiles, outputDir);
        out << sp;
        out << fragment.code;
        out << nl;
    }
}

string
Slice::Python::pyLinkFormatter(const string& rawLink, const ContainedPtr& p, const SyntaxTreeBasePtr& target)
{
    ostringstream result;
    if (target)
    {
        if (auto builtinTarget = dynamic_pointer_cast<Builtin>(target))
        {
            if (builtinTarget->kind() == Builtin::KindObject)
            {
                result << ":class:`Ice.Object`";
            }
            else if (builtinTarget->kind() == Builtin::KindValue)
            {
                result << ":class:`Ice.Value`";
            }
            else if (builtinTarget->kind() == Builtin::KindObjectProxy)
            {
                result << ":class:`Ice.ObjectPrx`";
            }
            else
            {
                result << "``" << typeToTypeHintString(builtinTarget, false, p) << "``";
            }
        }
        else if (auto operationTarget = dynamic_pointer_cast<Operation>(target))
        {
            string targetScoped = operationTarget->interface()->mappedScoped(".");

            // link to the method on the proxy interface
            result << ":meth:`" << targetScoped << "Prx." << operationTarget->mappedName() << "`";
        }
        else
        {
            string targetScoped = dynamic_pointer_cast<Contained>(target)->mappedScoped(".");
            result << ":class:`" << targetScoped;
            if (auto interfaceTarget = dynamic_pointer_cast<InterfaceDecl>(target))
            {
                // link to the proxy interface
                result << "Prx";
            }
            result << "`";
        }
    }
    else
    {
        result << "``";

        auto hashPos = rawLink.find('#');
        if (hashPos != string::npos)
        {
            if (hashPos != 0)
            {
                result << rawLink.substr(0, hashPos) << ".";
            }
            result << rawLink.substr(hashPos + 1);
        }
        else
        {
            result << rawLink;
        }

        result << "``";
    }
    return result.str();
}

void
Slice::Python::validatePythonMetadata(const UnitPtr& unit)
{
    auto pythonArrayTypeValidationFunc = [](const MetadataPtr& m, const SyntaxTreeBasePtr& p) -> optional<string>
    {
        if (auto sequence = dynamic_pointer_cast<Sequence>(p))
        {
            BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(sequence->type());
            if (!builtin || !(builtin->isNumericType() || builtin->kind() == Builtin::KindBool))
            {
                return "the '" + m->directive() +
                       "' metadata can only be applied to sequences of bools, bytes, shorts, ints, longs, floats, "
                       "or doubles";
            }
        }
        return nullopt;
    };

    map<string, MetadataInfo> knownMetadata;

    // "python:<array-type>"
    MetadataInfo arrayTypeInfo = {
        .validOn = {typeid(Sequence)},
        .acceptedArgumentKind = MetadataArgumentKind::NoArguments,
        .acceptedContext = MetadataApplicationContext::DefinitionsAndTypeReferences,
        .extraValidation = pythonArrayTypeValidationFunc,
    };
    knownMetadata.emplace("python:array.array", arrayTypeInfo);
    knownMetadata.emplace("python:numpy.ndarray", std::move(arrayTypeInfo));

    // "python:identifier"
    MetadataInfo identifierInfo = {
        .validOn =
            {typeid(Module),
             typeid(InterfaceDecl),
             typeid(Operation),
             typeid(ClassDecl),
             typeid(Slice::Exception),
             typeid(Struct),
             typeid(Sequence),
             typeid(Dictionary),
             typeid(Enum),
             typeid(Enumerator),
             typeid(Const),
             typeid(Parameter),
             typeid(DataMember)},
        .acceptedArgumentKind = MetadataArgumentKind::SingleArgument,
    };
    knownMetadata.emplace("python:identifier", std::move(identifierInfo));

    // "python:memoryview"
    MetadataInfo memoryViewInfo = {
        .validOn = {typeid(Sequence)},
        .acceptedArgumentKind = MetadataArgumentKind::RequiredTextArgument,
        .acceptedContext = MetadataApplicationContext::DefinitionsAndTypeReferences,
        .extraValidation = pythonArrayTypeValidationFunc,
    };
    knownMetadata.emplace("python:memoryview", std::move(memoryViewInfo));

    // "python:seq"
    // We support 3 arguments to this metadata: "default", "list", and "tuple".
    // We also allow users to omit the "seq" in the middle, ie. "python:seq:list" and "python:list" are equivalent.
    MetadataInfo seqInfo = {
        .validOn = {typeid(Sequence)},
        .acceptedArgumentKind = MetadataArgumentKind::SingleArgument,
        .validArgumentValues = {{"default", "list", "tuple"}},
        .acceptedContext = MetadataApplicationContext::DefinitionsAndTypeReferences,
    };
    knownMetadata.emplace("python:seq", std::move(seqInfo));
    MetadataInfo unqualifiedSeqInfo = {
        .validOn = {typeid(Sequence)},
        .acceptedArgumentKind = MetadataArgumentKind::NoArguments,
        .acceptedContext = MetadataApplicationContext::DefinitionsAndTypeReferences,
    };
    knownMetadata.emplace("python:default", unqualifiedSeqInfo);
    knownMetadata.emplace("python:list", unqualifiedSeqInfo);
    knownMetadata.emplace("python:tuple", std::move(unqualifiedSeqInfo));

    // Pass this information off to the parser's metadata validation logic.
    Slice::validateMetadata(unit, "python", std::move(knownMetadata));
}
