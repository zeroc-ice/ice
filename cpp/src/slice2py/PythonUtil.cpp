// Copyright (c) ZeroC, Inc.

#include "PythonUtil.h"
#include "../Ice/FileUtil.h"
#include "../Slice/DocCommentParser.h"
#include "../Slice/FileTracker.h"
#include "../Slice/MetadataValidation.h"
#include "../Slice/Preprocessor.h"
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
    const char* const tripleQuotes = R"(""")";
}

string
Slice::Python::getPythonModuleForDefinition(const SyntaxTreeBasePtr& p)
{
    if (auto builtin = dynamic_pointer_cast<Builtin>(p))
    {
        static const char* builtinTable[] = {"", "", "", "", "", "", "", "", "Ice.ObjectPrx", "Ice.Value"};

        return builtinTable[builtin->kind()];
    }
    else
    {
        auto contained = dynamic_pointer_cast<Contained>(p);
        assert(contained);
        return contained->mappedScoped(".");
    }
}

string
Slice::Python::getPythonModuleForForwardDeclaration(const SyntaxTreeBasePtr& p)
{
    string declarationModule = getPythonModuleForDefinition(p);
    if (!declarationModule.empty())
    {
        // Append "_forward" to the module name for generated modules.
        declarationModule += "_forward";
    }
    return declarationModule;
}

string
Slice::Python::getImportAlias(
    const ContainedPtr& source,
    const map<string, string>& allImports,
    const SyntaxTreeBasePtr& p)
{
    if (auto builtin = dynamic_pointer_cast<Builtin>(p))
    {
        if (builtin->kind() == Builtin::KindObjectProxy)
        {
            return getImportAlias(source, allImports, "Ice.ObjectPrx", "ObjectPrx");
        }
        else if (builtin->kind() == Builtin::KindValue)
        {
            return getImportAlias(source, allImports, "Ice.Value", "Value");
        }
        return "";
    }
    else
    {
        auto contained = dynamic_pointer_cast<Contained>(p);
        assert(contained);
        return getImportAlias(source, allImports, contained->mappedScoped("."), contained->mappedName());
    }
}

string
Slice::Python::getImportAlias(
    const ContainedPtr& source,
    const std::map<std::string, std::string>& allImports,
    const string& moduleName,
    const string& name)
{
    // Get the list of definitions exported for the source Python module.
    auto all = getAll(source);

    // Whether we need to use an alias for the import.
    bool useAlias = false;

    if (moduleName == source->mappedScoped("."))
    {
        // If the source module is the same as the module name being imported. We are using a
        // definition from the current module and we don't need an alias.
        useAlias = false;
    }
    else if (find(all.begin(), all.end(), name) != all.end())
    {
        // The name being bound comes from a different module and conflicts with one of the names
        // exported by the source module. We need to use an alias.
        useAlias = true;
    }
    else
    {
        // If the name being bound is already imported from a different module, we need to use an alias.
        auto p = allImports.find(name);
        if (p != allImports.end())
        {
            useAlias = p->second != moduleName;
        }
    }

    if (useAlias)
    {
        string alias = "_m_" + moduleName + "_" + name;
        std::replace(alias.begin(), alias.end(), '.', '_');
        return alias;
    }
    else
    {
        return name;
    }
}

string
Slice::Python::getMetaType(const SyntaxTreeBasePtr& p)
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
            "_Ice_ObjectPrx_t",
            "_Ice_Value_t"};

        return builtinTable[builtin->kind()];
    }
    else
    {
        auto contained = dynamic_pointer_cast<Contained>(p);
        assert(contained);
        string s = "_" + contained->mappedScoped("_");
        // Replace here is required in case any module is remapped with python:identifier into a nested package.
        replace(s.begin(), s.end(), '.', '_');
        if (dynamic_pointer_cast<InterfaceDef>(contained) || dynamic_pointer_cast<InterfaceDecl>(contained))
        {
            s += "Prx";
        }
        s += "_t";
        return s;
    }
}

string
Slice::Python::CodeVisitor::typeToTypeHintString(
    const TypePtr& type,
    bool optional,
    const ContainedPtr& source,
    bool forMarshaling)
{
    assert(type);

    if (optional)
    {
        if (isProxyType(type) || type->isClassType())
        {
            // We map optional proxies and classes like regular ones.
            return typeToTypeHintString(type, false, source, forMarshaling);
        }
        else
        {
            return typeToTypeHintString(type, false, source, forMarshaling) + " | None";
        }
    }

    static constexpr string_view builtinTable[] =
        {"int", "bool", "int", "int", "int", "float", "float", "str", "Ice.ObjectPrx | None", "Ice.Value | None"};

    if (auto builtin = dynamic_pointer_cast<Builtin>(type))
    {
        if (builtin->kind() == Builtin::KindObjectProxy)
        {
            return getImportAlias(source, "Ice.ObjectPrx", "ObjectPrx") + " | None";
        }
        else if (builtin->kind() == Builtin::KindValue)
        {
            return getImportAlias(source, "Ice.Value", "Value") + " | None";
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

        if (auto proxy = dynamic_pointer_cast<InterfaceDecl>(type))
        {
            return getImportAlias(source, proxy->mappedScoped("."), proxy->mappedName() + "Prx") + " | None";
        }
        else if (auto cls = dynamic_pointer_cast<ClassDecl>(type))
        {
            return getImportAlias(source, cls->mappedScoped("."), cls->mappedName()) + " | None";
        }
        else if (auto seq = dynamic_pointer_cast<Sequence>(type))
        {
            ostringstream os;
            // Map Slice built-in numeric types to NumPy types.
            static const char* numpyBuiltinTable[] = {
                "numpy.int8",
                "numpy.bool",
                "numpy.int16",
                "numpy.int32",
                "numpy.int64",
                "numpy.float32",
                "numpy.float64",
                "",
                "",
                ""};

            auto elementType = dynamic_pointer_cast<Builtin>(seq->type());
            bool isByteSequence = elementType && elementType->kind() == Builtin::KindByte;
            if (forMarshaling)
            {
                // For marshaling, we use a generic Sequence type hint, additionally we accept a bytes object for byte
                // sequences, and for sequences with NumPy metadata we use the NumPy NDArray type hint because it
                // doesn't conform to the generic sequence type.
                os << "Sequence[" + typeToTypeHintString(seq->type(), false, source, forMarshaling) + "]";
                if (isByteSequence)
                {
                    os << " | bytes";
                }
                if (seq->hasMetadata("python:numpy.ndarray"))
                {
                    assert(elementType);
                    os << " | numpy.typing.NDArray[" << numpyBuiltinTable[elementType->kind()] << "]";
                }
            }
            else if (seq->hasMetadata("python:list"))
            {
                os << "list[" << typeToTypeHintString(seq->type(), false, source, forMarshaling) << "]";
            }
            else if (seq->hasMetadata("python:tuple"))
            {
                os << "tuple[" << typeToTypeHintString(seq->type(), false, source, forMarshaling) << "]";
            }
            else if (seq->hasMetadata("python:numpy.ndarray"))
            {
                assert(elementType);
                os << "numpy.typing.NDArray[" << numpyBuiltinTable[elementType->kind()] << "]";
            }
            else if (seq->hasMetadata("python:array.array"))
            {
                os << "array.array[" << typeToTypeHintString(seq->type(), false, source, forMarshaling) << "]";
            }
            else if (isByteSequence)
            {
                os << "bytes";
            }
            else
            {
                return "list[" + typeToTypeHintString(seq->type(), false, source, forMarshaling) + "]";
            }
            // TODO add support for python:memoryview.
            return os.str();
        }
        else if (auto dict = dynamic_pointer_cast<Dictionary>(type))
        {
            ostringstream os;
            os << "dict[" << typeToTypeHintString(dict->keyType(), false, source, forMarshaling) << ", "
               << typeToTypeHintString(dict->valueType(), false, source, forMarshaling) << "]";
            return os.str();
        }
        else
        {
            return getImportAlias(source, contained);
        }
    }
}

string
Slice::Python::CodeVisitor::returnTypeHint(const OperationPtr& operation, MethodKind methodKind)
{
    auto source = dynamic_pointer_cast<Contained>(operation->container());
    string returnTypeHint;
    ParameterList outParameters = operation->outParameters();
    bool forMarshaling = methodKind == Dispatch;
    if (operation->returnsMultipleValues())
    {
        ostringstream os;
        os << "tuple[";
        if (operation->returnType())
        {
            os << typeToTypeHintString(operation->returnType(), operation->returnIsOptional(), source, forMarshaling);
            os << ", ";
        }

        for (const auto& param : outParameters)
        {
            os << typeToTypeHintString(param->type(), param->optional(), source, forMarshaling);
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
        returnTypeHint =
            typeToTypeHintString(operation->returnType(), operation->returnIsOptional(), source, forMarshaling);
    }
    else if (!outParameters.empty())
    {
        const auto& param = outParameters.front();
        returnTypeHint = typeToTypeHintString(param->type(), param->optional(), source, forMarshaling);
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

string
Slice::Python::CodeVisitor::operationReturnTypeHint(const OperationPtr& operation, MethodKind methodKind)
{
    return " -> " + returnTypeHint(operation, methodKind);
}

string
Slice::Python::formatFields(const DataMemberList& members)
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

bool
Slice::Python::canBeUsedAsDefaultValue(const TypePtr& type)
{
    return !dynamic_pointer_cast<Struct>(type) && !dynamic_pointer_cast<Sequence>(type) &&
           !dynamic_pointer_cast<Dictionary>(type);
}

Python::PythonCodeFragment
Slice::Python::createCodeFragmentForPythonModule(const ContainedPtr& contained, const string& code)
{
    Python::PythonCodeFragment fragment;
    bool isForwardDeclaration =
        dynamic_pointer_cast<InterfaceDecl>(contained) || dynamic_pointer_cast<ClassDecl>(contained);
    fragment.moduleName = isForwardDeclaration ? getPythonModuleForForwardDeclaration(contained)
                                               : getPythonModuleForDefinition(contained);
    fragment.code = code;
    fragment.sliceFileName = contained->file();
    string fileName = fragment.moduleName;
    replace(fileName.begin(), fileName.end(), '.', '/');
    fragment.fileName = fileName + ".py";
    fragment.isPackageIndex = false;
    return fragment;
}

void
Slice::Python::writeHeader(IceInternal::Output& out)
{
    out << "# Copyright (c) ZeroC, Inc.";
    out << sp;
    out << nl << "# slice2py version " << ICE_STRING_VERSION;
}

void
Slice::Python::writePackageIndex(const std::map<std::string, std::set<std::string>>& imports, IceInternal::Output& out)
{
    out << sp;
    writeHeader(out);
    if (!imports.empty())
    {
        out << sp;
        std::list<string> allDefinitions;
        for (const auto& [moduleName, definitions] : imports)
        {
            for (const auto& name : definitions)
            {
                out << nl << "from ." << moduleName << " import " << name;
                allDefinitions.push_back(name);
            }
        }
        out << nl;

        out << sp;
        out << nl << "__all__ = [";
        out.inc();
        for (auto it = allDefinitions.begin(); it != allDefinitions.end();)
        {
            out << nl << ("\"" + *it + "\"");
            if (++it != allDefinitions.end())
            {
                out << ",";
            }
        }
        out.dec();
        out << nl << "]";
        out << nl;
    }
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
Slice::Python::ImportVisitor::visitStructStart(const StructPtr& p)
{
    addRuntimeImport("dataclasses", "dataclass", p);
    return true;
}

bool
Slice::Python::ImportVisitor::visitClassDefStart(const ClassDefPtr& p)
{
    addRuntimeImport("dataclasses", "dataclass", p);

    // Import the meta type that is created in the Xxx_forward module for forward declarations.
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
        addRuntimeImport("Ice.Value", "Value", p);
    }

    return true;
}

bool
Slice::Python::ImportVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    addRuntimeImport("dataclasses", "dataclass", p);

    // Add imports required for base exception types.
    if (ExceptionPtr base = p->base())
    {
        addRuntimeImport(base, p);
        addRuntimeImportForMetaType(base, p);
    }
    else
    {
        // If the exception has no base, we import the Ice.UserException type.
        addRuntimeImport("Ice.UserException", "UserException", p);
    }
    return true;
}

void
Slice::Python::ImportVisitor::visitDataMember(const DataMemberPtr& p)
{
    auto parent = dynamic_pointer_cast<Contained>(p->container());
    auto type = p->type();

    // Import field if we have at least one data member that cannot be used as a default value.
    // This is required to use the `dataclasses.field` function to initialize the field.
    if (!canBeUsedAsDefaultValue(type))
    {
        addRuntimeImport("dataclasses", "field", parent);
    }

    // Add imports required for data member types.

    // For fields with a type that is a Struct, we need to import it as a RuntimeImport, to
    // initialize the field in the constructor. For other contained types, we only need the
    // import for type hints.
    if (dynamic_pointer_cast<Struct>(type) || dynamic_pointer_cast<Enum>(type))
    {
        addRuntimeImport(type, parent);
    }
    else
    {
        addTypingImport(type, parent, true);
    }
    addRuntimeImportForMetaType(type, parent);

    // If the data member has a default value, and the type of the default value is an Enum or a Const
    // we need to import the corresponding Enum or Const.
    if (p->defaultValue() &&
        (dynamic_pointer_cast<Const>(p->defaultValueType()) || dynamic_pointer_cast<Enum>(p->defaultValueType())))
    {
        addRuntimeImport(p->defaultValueType(), parent);
    }
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
        addRuntimeImport("Ice.ObjectPrx", "ObjectPrx", p);
        addRuntimeImport("Ice.Object", "Object", p);
    }
    else
    {
        for (const auto& base : bases)
        {
            addRuntimeImport(base, p, Proxy);
            addRuntimeImport(base, p, Servant);
        }
    }

    addRuntimeImport("abc", "ABC", p);
    addRuntimeImport("typing", "overload", p);

    addRuntimeImport("Ice.ObjectPrx", "checkedCast", p);
    addRuntimeImport("Ice.ObjectPrx", "checkedCastAsync", p);
    addRuntimeImport("Ice.ObjectPrx", "uncheckedCast", p);

    addTypingImport("Ice.ObjectPrx", "ObjectPrx", p);
    addTypingImport("Ice.Current", "Current", p);

    // Required by the core operations, ice_isA, ice_ids, ice_id, and ice_ping.
    addTypingImport("collections.abc", "Awaitable", p);
    addTypingImport("collections.abc", "Sequence", p);

    // Add imports required for operation parameters and return types.
    const OperationList& operations = p->allOperations();
    if (!operations.empty())
    {
        addRuntimeImport("abc", "abstractmethod", p);
        addRuntimeImport("Ice.OperationMode", "OperationMode", p);
    }

    for (const auto& op : operations)
    {
        // We need to call `addTypingImport` twice per parameter. This is required because for list the marshaling
        // and unmarshaling code might require different type hints.
        auto ret = op->returnType();
        if (ret)
        {
            addTypingImport(ret, p, false);
            addTypingImport(ret, p, true);

            addRuntimeImportForMetaType(ret, p);
        }

        for (const auto& param : op->parameters())
        {
            addTypingImport(param->type(), p, false);
            addTypingImport(param->type(), p, true);

            addRuntimeImportForMetaType(param->type(), p);
        }

        for (const auto& ex : op->throws())
        {
            addRuntimeImportForMetaType(ex, p);
        }

        if (op->format())
        {
            addRuntimeImport("Ice.FormatType", "FormatType", p);
        }
    }

    return false;
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
    addRuntimeImport("enum", "Enum", p);
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

    auto& allImports = _allImports[sourceModule];

    string name;
    string alias;

    if (auto builtin = dynamic_pointer_cast<Builtin>(definition))
    {
        if (builtin->kind() != Builtin::KindObjectProxy && builtin->kind() != Builtin::KindValue)
        {
            // Builtin types other than ObjectPrx and Value don't need imports.
            return;
        }

        name = builtin->kind() == Builtin::KindObjectProxy ? "ObjectPrx" : "Value";
        alias = getImportAlias(source, allImports, definition);
    }
    else
    {
        auto contained = dynamic_pointer_cast<Contained>(definition);
        assert(contained);

        name = contained->mappedName();
        if ((dynamic_pointer_cast<InterfaceDef>(definition) || dynamic_pointer_cast<InterfaceDecl>(definition)) &&
            typeContext == Proxy)
        {
            name += "Prx";
        }

        alias = getImportAlias(source, allImports, contained->mappedScoped("."), name);
    }

    auto& sourceModuleImports = _runtimeImports[sourceModule];
    auto& definitionImports = sourceModuleImports[definitionModule];

    if (name == alias)
    {
        definitionImports.insert({name, ""});
        allImports[name] = definitionModule;
    }
    else
    {
        definitionImports.insert({name, alias});
        allImports[alias] = definitionModule;
    }
}

void
Slice::Python::ImportVisitor::addRuntimeImport(
    const string& definitionModule,
    const string& definition,
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

    auto& imports = _allImports[sourceModule];
    string alias = getImportAlias(source, imports, definitionModule, definition);

    if (definition == alias)
    {
        definitionImports.insert({definition, ""});
        imports[definition] = definitionModule;
    }
    else
    {
        definitionImports.insert({definition, alias});
        imports[alias] = definitionModule;
    }
}

void
Slice::Python::ImportVisitor::addTypingImport(
    const string& moduleName,
    const string& definition,
    const ContainedPtr& source)
{
    // The module importing the definition.
    string sourceModule = getPythonModuleForDefinition(source);
    auto& sourceModuleImports = _typingImports[sourceModule];
    auto& definitionImports = sourceModuleImports[moduleName];

    auto& imports = _allImports[sourceModule];
    string alias = getImportAlias(source, imports, moduleName, definition);

    if (definition == alias)
    {
        definitionImports.insert({definition, ""});
        imports[definition] = moduleName;
    }
    else
    {
        definitionImports.insert({definition, alias});
        imports[alias] = moduleName;
    }

    // If we are importing a type with the TypingImport scope, we also need a runtime import for TYPE_CHECKING from
    // typing.
    addRuntimeImport("typing", "TYPE_CHECKING", source);
}

void
Slice::Python::ImportVisitor::addTypingImport(
    const SyntaxTreeBasePtr& definition,
    const ContainedPtr& source,
    bool forMarshaling)
{
    if (auto builtin = dynamic_pointer_cast<Builtin>(definition))
    {
        if (builtin->kind() == Builtin::KindValue)
        {
            addTypingImport("Ice.Value", "Value", source);
        }
        else if (builtin->kind() == Builtin::KindObjectProxy)
        {
            addTypingImport("Ice.ObjectPrx", "ObjectPrx", source);
        }
    }
    else if (auto sequence = dynamic_pointer_cast<Sequence>(definition))
    {
        if (sequence->hasMetadata("python:numpy.ndarray"))
        {
            // We need both numpy and numpy.typing imports to generate:
            // numpy.typing.NDArray[numpy.int8]
            addTypingImport("numpy", source);
            addTypingImport("numpy.typing", source);
        }

        if (forMarshaling)
        {
            // For marshaling we just require a generic Sequence
            addTypingImport("collections.abc", "Sequence", source);
        }
        else if (sequence->hasMetadata("python:memoryview") || sequence->hasMetadata("python:array.array"))
        {
            addTypingImport("collections.abc", "MutableSequence", source);
        }
        addTypingImport(sequence->type(), source, forMarshaling);
    }
    else if (auto dictionary = dynamic_pointer_cast<Dictionary>(definition))
    {
        addTypingImport(dictionary->keyType(), source, forMarshaling);
        addTypingImport(dictionary->valueType(), source, forMarshaling);
    }
    else if (auto interface = dynamic_pointer_cast<InterfaceDecl>(definition))
    {
        addTypingImport(interface->mappedScoped("."), interface->mappedName() + "Prx", source);
    }
    else if (auto contained = dynamic_pointer_cast<Contained>(definition))
    {
        addTypingImport(contained->mappedScoped("."), contained->mappedName(), source);
    }
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
    addRuntimeImport("typing", "TYPE_CHECKING", source);
}

void
Slice::Python::ImportVisitor::addRuntimeImportForMetaType(
    const SyntaxTreeBasePtr& definition,
    const ContainedPtr& source)
{
    auto builtin = dynamic_pointer_cast<Builtin>(definition);
    if (builtin && builtin->kind() != Builtin::KindObjectProxy && builtin->kind() != Builtin::KindValue)
    {
        // Builtin types other than ObjectPrx and Value don't need imports.
        return;
    }

    // The meta type for a Slice class or interface is always imported from the Xxx_forward module.
    bool isForwardDeclared =
        dynamic_pointer_cast<ClassDecl>(definition) || dynamic_pointer_cast<ClassDef>(definition) ||
        dynamic_pointer_cast<InterfaceDecl>(definition) || dynamic_pointer_cast<InterfaceDef>(definition) || builtin;

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
    string packageName = p->mappedScoped(".");

    // Ensure all parent packages exits, that is necessary to account for modules
    // that are mapped to a nested package using python:identifier metadata.
    vector<string> packageParts;
    IceInternal::splitString(string_view{packageName}, ".", packageParts);
    string current = "";
    for (const auto& part : packageParts)
    {
        current += part + ".";
        if (_imports.find(current) == _imports.end())
        {
            // If the package does not exist, we create an empty map for it.
            _imports[current] = {};
            string currentPath = current;
            replace(currentPath.begin(), currentPath.end(), '.', '/');
            currentPath += "__init__.py";
            _generated[p->unit()->topLevelFile()].insert(currentPath);
        }
    }
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
    string packageName = definition->mappedScope(".");
    string moduleName = definition->mappedName();
    auto& packageImports = _imports[packageName];
    auto& definitions = packageImports[moduleName];
    definitions.insert(definition->mappedName() + prefix);

    // Add the definition to the list of generated Python modules.
    string modulePath = packageName;
    replace(modulePath.begin(), modulePath.end(), '.', '/');

    _generated[definition->unit()->topLevelFile()].insert(modulePath + moduleName + ".py");
}

void
Slice::Python::PackageVisitor::addRuntimeImportForMetaType(const ContainedPtr& definition)
{
    string packageName = definition->mappedScope(".");
    string moduleName = definition->mappedName();

    // The meta type for Slice classes or interfaces is always imported from the Xxx_forward module containing the
    // forward declaration.
    if (dynamic_pointer_cast<ClassDecl>(definition) || dynamic_pointer_cast<InterfaceDecl>(definition))
    {
        moduleName += "_forward";
    }
    auto& packageImports = _imports[packageName];
    auto& definitions = packageImports[moduleName];
    definitions.insert(getMetaType(definition));

    // Add the definition to the list of generated Python modules.
    replace(packageName.begin(), packageName.end(), '.', '/');
    _generated[definition->unit()->topLevelFile()].insert(packageName + moduleName + ".py");
}

// CodeVisitor implementation.

void
Slice::Python::CodeVisitor::writeOperations(const InterfaceDefPtr& p, Output& out)
{
    const string currentAlias = getImportAlias(p, "Ice.Current", "Current");
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
            out << nl << "def " << capName << "MarshaledResult(result, current: " << currentAlias << "):";
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

        for (const auto& param : operation->inParameters())
        {
            out << (param->mappedName() + ": " + typeToTypeHintString(param->type(), param->optional(), p, false));
        }

        const string currentParamName = getEscapedParamName(operation->parameters(), "current");
        out << (currentParamName + ": " + currentAlias);
        out << epar << operationReturnTypeHint(operation, Dispatch) << ":";
        out.inc();

        writeDocstring(operation, Dispatch, out);

        out << nl << "pass";
        out.dec();
    }
}

bool
Slice::Python::CodeVisitor::visitStructStart(const StructPtr& p)
{
    const string name = p->mappedName();
    const DataMemberList members = p->dataMembers();

    _out = std::make_unique<BufferedOutput>();
    auto& out = *_out;

    out << sp;
    out << nl << "@dataclass";
    if (Dictionary::isLegalKeyType(p))
    {
        out << "(order=True, unsafe_hash=True)";
    }

    out << nl << "class " << name << ":";
    out.inc();

    writeDocstring(p->docComment(), members, out);
    return true;
}

void
Slice::Python::CodeVisitor::visitStructEnd(const StructPtr& p)
{
    const string scoped = p->scoped();
    const string name = p->mappedName();
    const string metaTypeName = getMetaType(p);

    assert(_out);
    auto& out = *_out;

    out.dec();

    // Emit the type information.
    out << sp;
    out << nl << metaTypeName << " = IcePy.defineStruct(";
    out.inc();
    out << nl << "\"" << scoped << "\",";
    out << nl << name << ",";
    out << nl;
    writeMetadata(p->getMetadata(), out);
    out << ",";

    writeMetaTypeDataMembers(p, p->dataMembers(), out);
    out << ")";
    out.dec();

    out << sp;
    out << nl << "__all__ = [\"" << name << "\", \"" << metaTypeName << "\"]";

    _codeFragments.push_back(createCodeFragmentForPythonModule(p, out.str()));
    _out.reset();
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

    _codeFragments.push_back(createCodeFragmentForPythonModule(p->declaration(), outF.str()));

    // Emit the class definition.
    _out = std::make_unique<BufferedOutput>();
    auto& out = *_out;

    // Equality for Slice classes is reference equality, so we disable the dataclass eq.
    out << nl << "@dataclass(eq=False)";
    out << nl << "class " << valueName << '('
        << (base ? getImportAlias(p, base) : getImportAlias(p, "Ice.Value", "Value")) << "):";
    out.inc();

    writeDocstring(p->docComment(), members, out);
    return true;
}

void
Slice::Python::CodeVisitor::visitClassDefEnd(const ClassDefPtr& p)
{
    const string scoped = p->scoped();
    const string metaType = getMetaType(p);
    const string valueName = p->mappedName();
    const ClassDefPtr base = p->base();

    assert(_out);
    auto& out = *_out;

    // ice_staticId
    out << sp;
    out << nl << "@staticmethod";
    out << nl << "def ice_staticId() -> str:";
    out.inc();
    out << nl << "return \"" << scoped << "\"";
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

    writeMetaTypeDataMembers(p, p->dataMembers(), out);

    out << ")";
    out.dec();

    // Use setattr to set the _ice_type attribute on the class. Linting tools will complain about this if we
    // don't use setattr, as the _ice_type attribute is not defined in the class body.
    out << sp;
    out << nl << "setattr(" << valueName << ", '_ice_type', " << metaType << ")";

    out << sp;
    out << nl << "__all__ = [\"" << valueName << "\", \"" << metaType << "\"]";

    _codeFragments.push_back(createCodeFragmentForPythonModule(p, out.str()));
    _out.reset();
}

bool
Slice::Python::CodeVisitor::visitExceptionStart(const ExceptionPtr& p)
{
    const string scoped = p->scoped();
    const string name = p->mappedName();
    const ExceptionPtr base = p->base();

    _out = std::make_unique<BufferedOutput>();
    auto& out = *_out;

    const DataMemberList members = p->dataMembers();

    out << sp;
    out << nl << "@dataclass";
    out << nl << "class " << name << '('
        << (base ? getImportAlias(p, base) : getImportAlias(p, "Ice.UserException", "UserException")) << "):";
    out.inc();
    writeDocstring(p->docComment(), members, out);

    return true;
}

void
Slice::Python::CodeVisitor::visitExceptionEnd(const ExceptionPtr& p)
{
    const string scoped = p->scoped();
    const string name = p->mappedName();
    const ExceptionPtr base = p->base();

    assert(_out);
    auto& out = *_out;

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

    writeMetaTypeDataMembers(p, p->dataMembers(), out);

    out << ")";
    out.dec();

    // Use setattr to set the _ice_type attribute on the exception. Linting tools will complain about this if we
    // don't use setattr, as the _ice_type attribute is not defined in the exception body.
    out << sp;
    out << nl << "setattr(" << name << ", '_ice_type', " << metaType << ")";

    out << sp;
    out << nl << "__all__ = [\"" << name << "\", \"" << metaType << "\"]";

    _codeFragments.push_back(createCodeFragmentForPythonModule(p, out.str()));
    _out.reset();
}

void
Slice::Python::CodeVisitor::visitDataMember(const DataMemberPtr& p)
{
    assert(_out);
    auto& out = *_out;
    auto parent = dynamic_pointer_cast<Contained>(p->container());

    out << nl << p->mappedName() << ": " << typeToTypeHintString(p->type(), p->optional(), parent, false);

    if (p->defaultValue())
    {
        out << " = ";
        writeConstantValue(parent, p->type(), p->defaultValueType(), *p->defaultValue(), out);
    }
    else if (p->optional())
    {
        out << " = None";
    }
    else
    {
        out << " = " << getTypeInitializer(parent, p, false);
    }
}

bool
Slice::Python::CodeVisitor::visitInterfaceDefStart(const InterfaceDefPtr& p)
{
    string scoped = p->scoped();
    string className = p->mappedName();
    string prxName = className + "Prx";
    string metaType = getMetaType(p);
    InterfaceList bases = p->bases();
    OperationList operations = p->operations();

    // Emit a forward declarations for the proxy meta type.
    BufferedOutput outF;
    outF << nl << metaType << " = IcePy.declareProxy(\"" << scoped << "\")";
    outF << sp;
    outF << nl << "__all__ = [\"" << metaType << "\"]";
    _codeFragments.push_back(createCodeFragmentForPythonModule(p->declaration(), outF.str()));

    // Emit the proxy class.
    BufferedOutput out;
    out << sp;
    out << nl << "class " << prxName << spar;

    if (bases.empty())
    {
        out << getImportAlias(p, "Ice.ObjectPrx", "ObjectPrx");
    }
    else
    {
        for (const auto& base : bases)
        {
            out << getImportAlias(p, base->mappedScoped("."), base->mappedName() + "Prx");
        }
    }
    out << epar << ":";
    out.inc();

    const string communicatorAlias = getImportAlias(p, "Ice.Communicator", "Communicator");
    const string objectPrxAlias = getImportAlias(p, "Ice.ObjectPrx", "ObjectPrx");
    const string currentAlias = getImportAlias(p, "Ice.Current", "Current");
    const string formatTypeAlias = getImportAlias(p, "Ice.FormatType", "FormatType");

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
            param += ": " + typeToTypeHintString(q->type(), q->optional(), p, true);
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

    const string prxTypeHint = typeToTypeHintString(p->declaration(), false, p, false);

    out << sp;
    out << nl << "@staticmethod";
    out << nl << "def checkedCast(";
    out.inc();
    out << nl << "proxy: " << objectPrxAlias << " | None,";
    out << nl << "facet: str | None = None,";
    out << nl << "context: dict[str, str] | None = None";
    out.dec();
    out << nl << ") -> " << prxTypeHint << ":";
    out.inc();
    out << nl << "return checkedCast(" << prxName << ", proxy, facet, context)";
    out.dec();

    out << sp;
    out << nl << "@staticmethod";
    out << nl << "def checkedCastAsync(";
    out.inc();
    out << nl << "proxy: " << objectPrxAlias << " | None,";
    out << nl << "facet: str | None = None,";
    out << nl << "context: dict[str, str] | None = None";
    out.dec();
    out << nl << ") -> Awaitable[" << prxName << " | None ]:";
    out.inc();
    out << nl << "return checkedCastAsync(" << prxName << ", proxy, facet, context)";
    out.dec();

    out << sp << nl << "@overload";
    out << nl << "@staticmethod";
    out << nl << "def uncheckedCast(proxy: " << objectPrxAlias << ", facet: str | None = None) -> " << prxName << ":";
    out.inc();
    out << nl << "...";
    out.dec();

    out << sp << nl << "@overload";
    out << nl << "@staticmethod";
    out << nl << "def uncheckedCast(proxy: None, facet: str | None = None) -> None:";
    out.inc();
    out << nl << "...";
    out.dec();

    out << sp << nl << "@staticmethod";
    out << nl << "def uncheckedCast(proxy: " << objectPrxAlias << " | None, facet: str | None = None) -> "
        << prxTypeHint << ":";
    out.inc();
    out << nl << "return uncheckedCast(" << prxName << ", proxy, facet)";
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
        out << getImportAlias(p, "Ice.Object", "Object");
    }
    else
    {
        for (const auto& base : bases)
        {
            out << getImportAlias(p, base);
        }
    }
    out << "ABC" << epar << ':';
    out.inc();

    out << sp;
    // Declare _ice_ids class variable to hold the ice_ids.
    out << nl << "_ice_ids: Sequence[str] = (";
    auto ids = p->ids();
    for (const auto& id : ids)
    {
        out << "\"" << id << "\", ";
    }
    out << ")";

    // Pre-declare the _op_ methods
    for (const auto& operation : operations)
    {
        out << nl << "_op_" << operation->name() << ": IcePy.Operation";
    }

    // ice_ids is implemented by the base Ice.Object class using the `_ice_ids` class variable.

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
                    format = formatTypeAlias + ".CompactFormat";
                    break;
                case SlicedFormat:
                    format = formatTypeAlias + ".SlicedFormat";
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

        const string operationMode = operation->mode() == Operation::Mode::Normal ? "Normal" : "Idempotent";

        out << sp;
        out << nl << className << "._op_" << sliceName << " = IcePy.Operation(";
        out.inc();
        out << nl << "\"" << sliceName << "\",";
        out << nl << "\"" << operation->mappedName() << "\",";
        out << nl << getImportAlias(p, "Ice.OperationMode", "OperationMode") << "." << operationMode << ",";
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

        // A trailing comma to ensure that the output is interpreted as a Python tuple.
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

    _codeFragments.push_back(createCodeFragmentForPythonModule(p, out.str()));
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

    _codeFragments.push_back(createCodeFragmentForPythonModule(p, out.str()));
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

    _codeFragments.push_back(createCodeFragmentForPythonModule(p, out.str()));
}

void
Slice::Python::CodeVisitor::visitEnum(const EnumPtr& p)
{
    string scoped = p->scoped();
    string name = p->mappedName();
    string metaType = getMetaType(p);
    EnumeratorList enumerators = p->enumerators();

    BufferedOutput out;
    out << nl << "class " << name << "(Enum):";
    out.inc();

    writeDocstring(p->docComment(), p, out);

    out << nl;
    for (const auto& enumerator : enumerators)
    {
        out << nl << enumerator->mappedName() << " = " << enumerator->value();
    }

    out.dec();

    // Meta type definition.
    out << sp;
    out << nl << metaType << " = IcePy.defineEnum(";
    out.inc();
    out << nl << "\"" << scoped << "\",";
    out << nl << name << ",";
    out << nl;
    writeMetadata(p->getMetadata(), out);
    out << ",";
    out << nl << "{";
    out.inc();
    for (const auto& enumerator : enumerators)
    {
        out << nl << to_string(enumerator->value()) << ": " << name << "." << enumerator->mappedName() << ",";
    }
    out.dec();
    out << nl << "}";
    out.dec();
    out << nl << ")";

    out << sp;
    out << nl << "__all__ = [\"" << name << "\", \"" << metaType << "\"]";

    _codeFragments.push_back(createCodeFragmentForPythonModule(p, out.str()));
}

void
Slice::Python::CodeVisitor::visitConst(const ConstPtr& p)
{
    string name = p->mappedName();
    BufferedOutput out;
    out << sp;
    out << nl << name << " = ";
    writeConstantValue(p, p->type(), p->valueType(), p->value(), out);

    out << sp;
    out << nl << "__all__ = [\"" << name << "\"]";

    _codeFragments.push_back(createCodeFragmentForPythonModule(p, out.str()));
}

string
Slice::Python::CodeVisitor::getTypeInitializer(const ContainedPtr& source, const DataMemberPtr& field, bool constructor)
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
        "None",  // Builtin::KindObjectProxy.
        "None"}; // Builtin::KindValue.

    if (auto builtin = dynamic_pointer_cast<Builtin>(field->type()))
    {
        return string{builtinTable[builtin->kind()]};
    }
    else if (auto enumeration = dynamic_pointer_cast<Enum>(field->type()))
    {
        return getImportAlias(source, enumeration) + "." + enumeration->enumerators().front()->mappedName();
    }
    else if (!constructor && !canBeUsedAsDefaultValue(field->type()))
    {
        string factory;
        if (auto st = dynamic_pointer_cast<Struct>(field->type()))
        {
            factory = getImportAlias(source, st);
        }
        else if (auto seq = dynamic_pointer_cast<Sequence>(field->type()))
        {
            // TODO: handle metadata for sequences.
            auto elementType = dynamic_pointer_cast<Builtin>(seq->type());
            bool isByteSequence = elementType && elementType->kind() == Builtin::KindByte;
            if (isByteSequence)
            {
                factory = "bytes";
            }
            else
            {
                factory = "list";
            }
        }
        else if (auto dict = dynamic_pointer_cast<Dictionary>(field->type()))
        {
            factory = "dict"; // Dictionaries are initialized with an empty dictionary.
        }

        return "field(default_factory=" + factory + ")";
    }

    return "None";
}

void
Slice::Python::CodeVisitor::writeMetaTypeDataMembers(
    const ContainedPtr& parent,
    const DataMemberList& members,
    Output& out)
{
    bool includeOptional = !dynamic_pointer_cast<Struct>(parent);
    out << nl << "(";

    if (members.size() > 1)
    {
        out.inc();
        out << nl;
    }

    bool isFirst = true;
    for (const auto& member : members)
    {
        if (!isFirst)
        {
            out << ',';
            out << nl;
        }
        isFirst = false;
        out << "(\"" << member->mappedName() << "\", ";
        writeMetadata(member->getMetadata(), out);
        out << ", " << getMetaType(member->type());
        if (includeOptional)
        {
            out << ", " << (member->optional() ? "True" : "False");
            out << ", " << (member->optional() ? member->tag() : 0);
        }
        out << ')';
    }

    // A trailing comma is required for Python tuples with a single element.
    if (members.size() == 1)
    {
        out << ',';
    }

    if (members.size() > 1)
    {
        out.dec();
        out << nl;
    }

    out << ")";
}

void
Slice::Python::CodeVisitor::writeMetadata(const MetadataList& metadata, Output& out)
{
    MetadataList pythonMetadata = metadata;
    auto newEnd = remove_if(
        pythonMetadata.begin(),
        pythonMetadata.end(),
        [](const MetadataPtr& meta) { return meta->directive().find("python:") != 0; });
    pythonMetadata.erase(newEnd, pythonMetadata.end());

    out << '(';

    for (const auto& meta : pythonMetadata)
    {
        out << "\"" << *meta << "\"";
        if (meta != pythonMetadata.back() || pythonMetadata.size() == 1)
        {
            out << ", ";
        }
    }

    out << ')';
}

void
Slice::Python::CodeVisitor::writeConstantValue(
    const ContainedPtr& source,
    const TypePtr& type,
    const SyntaxTreeBasePtr& valueType,
    const string& value,
    Output& out)
{
    if (auto constant = dynamic_pointer_cast<Const>(valueType))
    {
        out << getImportAlias(source, constant);
    }
    else if (auto builtin = dynamic_pointer_cast<Slice::Builtin>(type))
    {
        switch (builtin->kind())
        {
            case Builtin::KindBool:
            {
                out << (value == "true" ? "True" : "False");
                break;
            }
            case Builtin::KindByte:
            case Builtin::KindShort:
            case Builtin::KindInt:
            case Builtin::KindFloat:
            case Builtin::KindDouble:
            case Builtin::KindLong:
            {
                out << value;
                break;
            }
            case Builtin::KindString:
            {
                const string controlChars = "\a\b\f\n\r\t\v";
                const unsigned char cutOff = 0;

                out << "\"" << toStringLiteral(value, controlChars, "", UCN, cutOff) << "\"";
                break;
            }
            case Builtin::KindValue:
            case Builtin::KindObjectProxy:
                assert(false);
        }
    }
    else if (auto enumeration = dynamic_pointer_cast<Slice::Enum>(type))
    {
        EnumeratorPtr enumerator = dynamic_pointer_cast<Enumerator>(valueType);
        assert(enumerator);
        out << getImportAlias(source, enumeration) << "." << enumerator->mappedName();
    }
    else
    {
        assert(false); // Unknown const type.
    }
}

// Get a list of all definitions exported for the Python module corresponding to the given Slice definition.
vector<string>
Slice::Python::getAll(const ContainedPtr& definition)
{
    vector<string> all;

    all.push_back(definition->mappedName());

    if (dynamic_pointer_cast<InterfaceDecl>(definition) || dynamic_pointer_cast<InterfaceDef>(definition))
    {
        all.push_back(definition->mappedName() + "Prx");
    }

    if (dynamic_pointer_cast<Type>(definition) || dynamic_pointer_cast<Exception>(definition) ||
        dynamic_pointer_cast<InterfaceDef>(definition))
    {
        // for types and exceptions, we also export the meta type.
        all.push_back(getMetaType(definition));
    }
    return all;
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
        if (const auto& memberDoc = member->docComment())
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
            out << nl << member->mappedName() << " : "
                << typeToTypeHintString(
                       member->type(),
                       member->optional(),
                       dynamic_pointer_cast<Contained>(member->container()),
                       false);
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
        if (const auto& enumeratorDoc = enumerator->docComment())
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
    const optional<DocComment>& comment = op->docComment();
    if (!comment)
    {
        return;
    }

    auto p = op->interface();

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
            out << nl << param->mappedName() << " : "
                << typeToTypeHintString(param->type(), param->optional(), p, methodKind != Dispatch);
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
                out << nl << "        - "
                    << typeToTypeHintString(returnType, op->returnIsOptional(), p, methodKind == Dispatch);
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
                out << nl << "        - "
                    << typeToTypeHintString(param->type(), param->optional(), p, methodKind == Dispatch);
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
            out << nl << typeToTypeHintString(param->type(), param->optional(), p, methodKind == Dispatch);
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

string
Slice::Python::CodeVisitor::getImportAlias(const ContainedPtr& source, const SyntaxTreeBasePtr& definition)
{
    string sourceModule = getPythonModuleForDefinition(source);
    auto& imports = _allImports[sourceModule];
    return Slice::Python::getImportAlias(source, imports, definition);
}

string
Slice::Python::CodeVisitor::getImportAlias(
    const ContainedPtr& source,
    const string& moduleName,
    const string& definitionName)
{
    string sourceModule = getPythonModuleForDefinition(source);
    auto& imports = _allImports[sourceModule];
    return Slice::Python::getImportAlias(source, imports, moduleName, definitionName);
}

namespace
{
    Output&
    getModuleOutputFile(const string& moduleName, map<string, unique_ptr<Output>>& outputFiles, const string& outputDir)
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

        Python::createPackagePath(moduleName, outputPath);
        outputPath += fileName;

        FileTracker::instance()->addFile(outputPath);

        auto output = make_unique<Output>(outputPath.c_str());
        Output& out = *output;
        Python::writeHeader(out);
        outputFiles[moduleName] = std::move(output);
        return out;
    }

    void writeImports(
        const Python::ModuleImportsMap& runtimeImports,
        const Python::ModuleImportsMap& typingImports,
        Output& out)
    {
        out << sp;
        out << nl << "from __future__ import annotations";
        out << nl << "import IcePy";

        set<string> allImports;

        // Write the runtime imports.
        for (const auto& [moduleName, definitions] : runtimeImports)
        {
            out << sp;
            for (const auto& [name, alias] : definitions)
            {
                out << nl << "from " << moduleName << " import " << name;
                if (!alias.empty())
                {
                    out << " as " << alias;
                    allImports.insert(alias);
                }
                else
                {
                    allImports.insert(name);
                }
            }
        }

        // Write typing imports
        if (!typingImports.empty())
        {
            Python::BufferedOutput outT;
            outT << sp;
            outT << nl << "if TYPE_CHECKING:";
            outT.inc();
            bool hasTypingImports = false;
            for (const auto& [moduleName, definitions] : typingImports)
            {
                if (!definitions.empty())
                {
                    for (const auto& [name, alias] : definitions)
                    {
                        bool allreadyImported = !allImports.insert(alias.empty() ? name : alias).second;

                        if (allreadyImported)
                        {
                            continue; // Skip already imported names.
                        }

                        outT << nl << "from " << moduleName << " import " << name;
                        if (!alias.empty())
                        {
                            outT << " as " << alias;
                        }
                        hasTypingImports = true;
                    }
                }
                else
                {
                    // If there are no definitions, we still need to import the module to ensure that the type hints
                    // are available.
                    outT << nl << "import " << moduleName;
                    allImports.insert(moduleName);
                    hasTypingImports = true;
                }
            }
            outT.dec();

            if (hasTypingImports)
            {
                out << outT.str();
            }
        }
    }
}

vector<Slice::Python::PythonCodeFragment>
Slice::Python::dynamicCompile(const vector<string>& files, const vector<string>& preprocessorArgs, bool debug)
{
    // The package visitor is reused to compile all Slice files so that it collects the definitions for all Slice files
    // contributing to a given package.
    PackageVisitor packageVisitor;

    // The import visitor is reused to collect the imports from all generated Python modules, which are later used to
    // compute the order in which Python modules should be evaluated.
    ImportVisitor importVisitor;

    // The list of code fragments generated by the code visitor.
    vector<PythonCodeFragment> fragments;

    for (const auto& fileName : files)
    {
        PreprocessorPtr preprocessor = Preprocessor::create("IcePy", fileName, preprocessorArgs);
        FILE* cppHandle = preprocessor->preprocess("-D__SLICE2PY__");

        UnitPtr unit = Unit::createUnit("python", debug);
        int parseStatus = unit->parse(fileName, cppHandle, false);

        preprocessor->close();

        if (parseStatus == EXIT_FAILURE)
        {
            unit->destroy();
            throw runtime_error("Failed to parse Slice file: " + fileName);
        }

        parseAllDocComments(unit, Slice::Python::pyLinkFormatter);
        validatePythonMetadata(unit);

        unit->visit(&packageVisitor);
        unit->visit(&importVisitor);

        CodeVisitor codeVisitor{
            importVisitor.getRuntimeImports(),
            importVisitor.getTypingImports(),
            importVisitor.getAllImportNames()};
        unit->visit(&codeVisitor);

        const vector<PythonCodeFragment>& newFragments = codeVisitor.codeFragments();
        fragments.insert(fragments.end(), newFragments.begin(), newFragments.end());
        unit->destroy();
    }

    vector<string> generatedModules;
    generatedModules.reserve(fragments.size());
    for (const auto& fragment : fragments)
    {
        // Collect the names of all generated modules.
        generatedModules.push_back(fragment.moduleName);
    }

    // The list of non generated modules that are imported by the generated modules.
    vector<string> builtinModules{
        "Ice.FormatType",
        "Ice.Object",
        "Ice.ObjectPrx",
        "Ice.ObjectPrx_forward",
        "Ice.OperationMode",
        "Ice.StringUtil",
        "Ice.UserException",
        "Ice.Value",
        "Ice.Value_forward",
        "abc",
        "dataclasses",
        "enum",
        "typing"};

    // List of generated modules in reverse topological order.
    // Each module in this list depends only on modules that appear earlier in the list, modules in the built-in
    // modules, or modules that are not part of this compilation.
    vector<PythonCodeFragment> processedFragments;

    ImportsMap runtimeImports = importVisitor.getRuntimeImports();
    ImportsMap typingImports = importVisitor.getTypingImports();
    while (!fragments.empty())
    {
        size_t fragmentsSize = fragments.size();
        for (auto it = fragments.begin(); it != fragments.end();)
        {
            PythonCodeFragment fragment = *it;
            const auto& moduleImports = runtimeImports[fragment.moduleName];

            bool unseenDependencies = false;
            for (const auto& [moduleName, _] : moduleImports)
            {
                // If the current module depends on a module that is being generated but not yet seen we postpone its
                // compilation.
                if (find(generatedModules.begin(), generatedModules.end(), moduleName) != generatedModules.end() &&
                    find_if(
                        processedFragments.begin(),
                        processedFragments.end(),
                        [&moduleName](const auto& element)
                        { return element.moduleName == moduleName; }) == processedFragments.end() &&
                    find(builtinModules.begin(), builtinModules.end(), moduleName) == builtinModules.end())
                {
                    unseenDependencies = true;
                    break;
                }
            }

            if (!unseenDependencies)
            {
                // If we have already processed all the dependencies we can remove it from the runtime imports.
                // add add it to the list of seen modules.
                Python::BufferedOutput out;
                writeImports(runtimeImports[fragment.moduleName], typingImports[fragment.moduleName], out);
                out << sp;
                out << fragment.code;
                processedFragments.push_back(
                    {.sliceFileName = fragment.sliceFileName,
                     .moduleName = fragment.moduleName,
                     .fileName = fragment.fileName,
                     .isPackageIndex = false,
                     .code = out.str()});
                it = fragments.erase(it);
            }
            else
            {
                it++;
            }
        }

        // If we didn't remove any module from the runtime imports, it means that we have a circular dependency between
        // Slice files which is not allowed.
        if (fragments.size() == fragmentsSize)
        {
            assert(false);
            throw runtime_error("Circular dependency detected in Slice files.");
        }
    }

    // Add fragments for the package-index files.
    for (const auto& [name, imports] : packageVisitor.imports())
    {
        string packageName = name;
        // The pop_back call removes the last dot from the package name.
        packageName.pop_back();
        BufferedOutput out;
        writePackageIndex(imports, out);
        string fileName = name;
        replace(fileName.begin(), fileName.end(), '.', '/');
        fileName += "/__init__.py";
        processedFragments.push_back(
            {.sliceFileName = "",
             .moduleName = packageName,
             .fileName = fileName,
             .isPackageIndex = true,
             .code = out.str()});
    }

    return processedFragments;
}

void
Slice::Python::generate(const UnitPtr& unit, const std::string& outputDir)
{
    validatePythonMetadata(unit);

    ImportVisitor importVisitor;
    unit->visit(&importVisitor);

    CodeVisitor codeVisitor{
        importVisitor.getRuntimeImports(),
        importVisitor.getTypingImports(),
        importVisitor.getAllImportNames()};
    unit->visit(&codeVisitor);

    const string fileBaseName = baseName(removeExtension(unit->topLevelFile()));

    // A map from python module names to output files.
    map<string, unique_ptr<Output>> outputFiles;

    ImportsMap runtimeImports = importVisitor.getRuntimeImports();
    ImportsMap typingImports = importVisitor.getTypingImports();

    // Emit the code fragments for the unit.
    for (auto& fragment : codeVisitor.codeFragments())
    {
        Output& out = getModuleOutputFile(fragment.moduleName, outputFiles, outputDir);
        writeImports(runtimeImports[fragment.moduleName], typingImports[fragment.moduleName], out);
        out << sp;
        out << fragment.code;
        out << nl;
    }
}

string
Slice::Python::pyLinkFormatter(const string& rawLink, const ContainedPtr&, const SyntaxTreeBasePtr& target)
{
    ostringstream result;
    if (target)
    {
        if (auto builtinTarget = dynamic_pointer_cast<Builtin>(target))
        {
            if (builtinTarget->kind() == Builtin::KindValue)
            {
                result << ":class:`Ice.Value`";
            }
            else if (builtinTarget->kind() == Builtin::KindObjectProxy)
            {
                result << ":class:`Ice.ObjectPrx`";
            }
            else
            {
                result << "``" << rawLink << "``";
            }
        }
        else if (auto operationTarget = dynamic_pointer_cast<Operation>(target))
        {
            string targetScoped = operationTarget->interface()->mappedScoped(".");

            // link to the method on the proxy interface
            result << ":meth:`" << targetScoped << "Prx." << operationTarget->mappedName() << "`";
        }
        else if (dynamic_pointer_cast<Sequence>(target) || dynamic_pointer_cast<Dictionary>(target))
        {
            // For sequences and dictionaries there is nothing to link to.
            // TODO use the type hint
            result << "``" << rawLink << "``";
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
             typeid(Slice::Parameter),
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
    // We support 2 arguments to this metadata: "list", and "tuple".
    MetadataInfo seqInfo = {
        .validOn = {typeid(Sequence)},
        .acceptedArgumentKind = MetadataArgumentKind::SingleArgument,
        .validArgumentValues = {{"list", "tuple"}},
        .acceptedContext = MetadataApplicationContext::DefinitionsAndTypeReferences,
    };
    knownMetadata.emplace("python:seq", std::move(seqInfo));
    MetadataInfo unqualifiedSeqInfo = {
        .validOn = {typeid(Sequence)},
        .acceptedArgumentKind = MetadataArgumentKind::NoArguments,
        .acceptedContext = MetadataApplicationContext::DefinitionsAndTypeReferences,
    };
    knownMetadata.emplace("python:list", unqualifiedSeqInfo);
    knownMetadata.emplace("python:tuple", std::move(unqualifiedSeqInfo));

    // Pass this information off to the parser's metadata validation logic.
    validateMetadata(unit, "python", std::move(knownMetadata));
}
