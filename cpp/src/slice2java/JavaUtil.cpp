// Copyright (c) ZeroC, Inc.

#include "JavaUtil.h"
#include "../Ice/FileUtil.h"
#include "../Slice/FileTracker.h"
#include "../Slice/MetadataValidation.h"
#include "../Slice/Util.h"
#include "Ice/StringUtil.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <sys/types.h>

#ifdef _WIN32
#    include <direct.h>
#endif

#ifndef _WIN32
#    include <unistd.h>
#endif

using namespace std;
using namespace Slice;
using namespace IceInternal;

namespace
{
    string typeToBufferString(const TypePtr& type)
    {
        static const char* builtinBufferTable[] = {
            "java.nio.ByteBuffer",
            "???",
            "java.nio.ShortBuffer",
            "java.nio.IntBuffer",
            "java.nio.LongBuffer",
            "java.nio.FloatBuffer",
            "java.nio.DoubleBuffer",
            "???",
            "???",
            "???"};

        BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
        if (!builtin)
        {
            return "???";
        }
        else
        {
            return builtinBufferTable[builtin->kind()];
        }
    }
}

string
Slice::Java::getResultType(const OperationPtr& op, const string& package, bool object)
{
    if (op->returnsMultipleValues())
    {
        const ContainedPtr c = dynamic_pointer_cast<Contained>(op->container());
        assert(c);
        const string abs = getUnqualified(c, package);
        string name = op->mappedName();
        name[0] = static_cast<char>(toupper(static_cast<unsigned char>(name[0])));
        return abs + "." + name + "Result";
    }
    else
    {
        TypePtr type = op->returnType();
        bool optional = op->returnIsOptional();
        if (!type)
        {
            const ParameterList outParams = op->outParameters();
            if (!outParams.empty())
            {
                assert(outParams.size() == 1);
                type = outParams.front()->type();
                optional = outParams.front()->optional();
            }
        }
        if (type)
        {
            if (optional)
            {
                return typeToString(type, TypeModeReturn, package, op->getMetadata(), true, true);
            }
            else if (object)
            {
                return typeToObjectString(type, TypeModeReturn, package, op->getMetadata(), true);
            }
            else
            {
                return typeToString(type, TypeModeReturn, package, op->getMetadata(), true, false);
            }
        }
        else
        {
            return object ? "Void" : "void";
        }
    }
}

vector<string>
Slice::Java::getParamsProxy(const OperationPtr& op, const string& package, bool optionalMapping, bool internal)
{
    vector<string> params;
    for (const auto& param : op->inParameters())
    {
        const string typeString = typeToString(
            param->type(),
            TypeModeIn,
            package,
            param->getMetadata(),
            true,
            optionalMapping && param->optional());
        params.push_back(typeString + ' ' + (internal ? "iceP_" : "") + param->mappedName());
    }
    return params;
}

vector<string>
Slice::Java::getInArgs(const OperationPtr& op, bool internal)
{
    vector<string> args;
    for (const auto& q : op->inParameters())
    {
        args.push_back((internal ? "iceP_" : "") + q->mappedName());
    }
    return args;
}

string
Slice::Java::getSerialVersionUID(const ContainedPtr& p)
{
    int64_t serialVersionUID;

    // Check if the user provided their own UID value with metadata.
    if (auto meta = p->getMetadataArgs("java:serialVersionUID"))
    {
        serialVersionUID = std::stoll(*meta, nullptr, 0);
    }
    else
    {
        serialVersionUID = computeDefaultSerialVersionUID(p);
    }

    ostringstream os;
    os << "private static final long serialVersionUID = " << serialVersionUID << "L;";
    return os.str();
}

int64_t
Slice::Java::computeDefaultSerialVersionUID(const ContainedPtr& p)
{
    string name = p->mappedScoped(".", true);
    DataMemberList members;
    optional<string> baseName;
    if (ClassDefPtr cl = dynamic_pointer_cast<ClassDef>(p))
    {
        members = cl->dataMembers();
        baseName = (cl->base()) ? cl->base()->mappedScoped(".", true) : "";
    }
    if (ExceptionPtr ex = dynamic_pointer_cast<Exception>(p))
    {
        members = ex->dataMembers();
        baseName = nullopt;
    }
    if (StructPtr st = dynamic_pointer_cast<Struct>(p))
    {
        members = st->dataMembers();
        baseName = nullopt;
    }

    // Actually compute the `SerialVersionUID` value.
    ostringstream os;
    os << name << ":";
    if (baseName)
    {
        os << *baseName;
    }
    os << ";";
    for (const auto& member : members)
    {
        const MetadataList& metadata = member->getMetadata();
        const string typeString = typeToString(member->type(), TypeModeMember, "", metadata);
        os << member->mappedName() << ":" << typeString << ",";
    }

    // We use a custom hash instead of relying on `std::hash` to ensure cross-platform consistency.
    const string data = os.str();
    int64_t hashCode = 5381;
    for (const auto& c : data)
    {
        hashCode = ((hashCode << 5) + hashCode) ^ c;
    }
    return hashCode;
}

bool
Slice::Java::isValidMethodParameterList(const DataMemberList& members, int additionalUnits)
{
    // The maximum length of a method parameter list is 255 units, including the implicit 'this' parameter.
    // Each parameter is 1 unit, except for long and double parameters, which are 2 units.
    // Start the length at 1 to account for the implicit 'this' parameter (plus any additional units).
    int length = 1 + additionalUnits;
    for (const auto& member : members)
    {
        BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(member->type());
        if (builtin && (builtin->kind() == Builtin::KindLong || builtin->kind() == Builtin::KindDouble))
        {
            length += 2;
        }
        else
        {
            length++;
        }
    }
    return length <= 255;
}

bool
Slice::Java::mapsToJavaBuiltinType(const TypePtr& p)
{
    if (auto builtin = dynamic_pointer_cast<Builtin>(p))
    {
        return builtin->kind() <= Builtin::KindString;
    }
    return false;
}

string
Slice::Java::getPackagePrefix(const ContainedPtr& contained)
{
    ModulePtr topLevelModule = contained->getTopLevelModule();

    // The 'java:package' metadata can be defined as file metadata or applied to a top-level module.
    // We check for the metadata at the top-level module first and then fall back to the global scope.
    if (auto metadataArgs = topLevelModule->getMetadataArgs("java:package"))
    {
        return *metadataArgs;
    }
    string_view file = contained->file();
    DefinitionContextPtr dc = contained->unit()->findDefinitionContext(file);
    assert(dc);
    return dc->getMetadataArgs("java:package").value_or("");
}

string
Slice::Java::getPackage(const ContainedPtr& contained)
{
    string scope = contained->mappedScope(".");
    scope.pop_back(); // Remove the trailing '.' separator.

    string prefix = getPackagePrefix(contained);
    if (!prefix.empty())
    {
        if (!scope.empty())
        {
            return prefix + "." + scope;
        }
        else
        {
            return prefix;
        }
    }

    return scope;
}

string
Slice::Java::getUnqualified(const std::string& type, const std::string& package)
{
    if (type.find('.') != string::npos && type.find(package) == 0 && type.find('.', package.size() + 1) == string::npos)
    {
        return type.substr(package.size() + 1);
    }
    return type;
}

string
Slice::Java::getUnqualified(const ContainedPtr& cont, const string& package, const string& prefix)
{
    string name = prefix + cont->mappedName();
    string contPkg = getPackage(cont);
    return (contPkg == package || contPkg.empty()) ? name : contPkg + "." + name;
}

string
Slice::Java::getStaticId(const TypePtr& type, const string& package)
{
    BuiltinPtr b = dynamic_pointer_cast<Builtin>(type);
    ClassDeclPtr cl = dynamic_pointer_cast<ClassDecl>(type);

    assert((b && b->usesClasses()) || cl);

    if (b && b->kind() == Builtin::KindValue)
    {
        return "com.zeroc.Ice.Value.ice_staticId()";
    }
    else
    {
        return getUnqualified(cl, package) + ".ice_staticId()";
    }
}

string
Slice::Java::getOptionalFormat(const TypePtr& type)
{
    return "com.zeroc.Ice.OptionalFormat." + type->getOptionalFormat();
}

string
Slice::Java::typeToString(
    const TypePtr& type,
    TypeMode mode,
    const string& package,
    const MetadataList& metadata,
    bool formal,
    bool optional)
{
    static const char* builtinTable[] = {
        "byte",
        "boolean",
        "short",
        "int",
        "long",
        "float",
        "double",
        "java.lang.String",
        "com.zeroc.Ice.ObjectPrx",
        "com.zeroc.Ice.Value"};

    static const char* builtinOptionalTable[] = {
        "java.util.Optional<java.lang.Byte>",
        "java.util.Optional<java.lang.Boolean>",
        "java.util.Optional<java.lang.Short>",
        "java.util.OptionalInt",
        "java.util.OptionalLong",
        "java.util.Optional<java.lang.Float>",
        "java.util.OptionalDouble",
        "???",
        "???",
        "???"};

    if (!type)
    {
        assert(mode == TypeModeReturn);
        return "void";
    }

    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if (builtin)
    {
        if (optional)
        {
            switch (builtin->kind())
            {
                case Builtin::KindByte:
                case Builtin::KindBool:
                case Builtin::KindShort:
                case Builtin::KindInt:
                case Builtin::KindLong:
                case Builtin::KindFloat:
                case Builtin::KindDouble:
                {
                    return getUnqualified(builtinOptionalTable[builtin->kind()], package);
                }
                case Builtin::KindString:
                case Builtin::KindObjectProxy:
                case Builtin::KindValue:
                {
                    break;
                }
            }
        }
        else
        {
            return getUnqualified(builtinTable[builtin->kind()], package);
        }
    }

    if (optional)
    {
        return "java.util.Optional<" + typeToObjectString(type, mode, package, metadata, formal) + ">";
    }

    ClassDeclPtr cl = dynamic_pointer_cast<ClassDecl>(type);
    if (cl)
    {
        return getUnqualified(cl, package);
    }

    InterfaceDeclPtr proxy = dynamic_pointer_cast<InterfaceDecl>(type);
    if (proxy)
    {
        return getUnqualified(proxy, package) + "Prx";
    }

    DictionaryPtr dict = dynamic_pointer_cast<Dictionary>(type);
    if (dict)
    {
        string instanceType, formalType;
        getDictionaryTypes(dict, package, metadata, instanceType, formalType);
        return formal ? formalType : instanceType;
    }

    SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
    if (seq)
    {
        string instanceType, formalType;
        getSequenceTypes(seq, package, metadata, instanceType, formalType);
        return formal ? formalType : instanceType;
    }

    ContainedPtr contained = dynamic_pointer_cast<Contained>(type);
    if (contained)
    {
        if (mode == TypeModeOut)
        {
            return getUnqualified(contained, package) + "Holder";
        }
        else
        {
            return getUnqualified(contained, package);
        }
    }

    return "???";
}

string
Slice::Java::typeToObjectString(
    const TypePtr& type,
    TypeMode mode,
    const string& package,
    const MetadataList& metadata,
    bool formal)
{
    static const char* builtinTable[] = {
        "java.lang.Byte",
        "java.lang.Boolean",
        "java.lang.Short",
        "java.lang.Integer",
        "java.lang.Long",
        "java.lang.Float",
        "java.lang.Double",
        "java.lang.String",
        "com.zeroc.Ice.ObjectPrx",
        "com.zeroc.Ice.Value"};

    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if (builtin && mode != TypeModeOut)
    {
        return builtinTable[builtin->kind()];
    }

    return typeToString(type, mode, package, metadata, formal, false);
}

bool
Slice::Java::hasMetadata(const string& directive, const MetadataList& metadata)
{
    for (const auto& m : metadata)
    {
        if (m->directive() == directive)
        {
            return true;
        }
    }

    return false;
}

bool
Slice::Java::getTypeMetadata(const MetadataList& metadata, string& instanceType, string& formalType)
{
    //
    // Extract the instance type and an optional formal type.
    // The correct syntax is "java:type:instance-type[:formal-type]".
    //
    for (const auto& m : metadata)
    {
        if (m->directive() == "java:type")
        {
            string arguments = m->arguments();
            string::size_type pos = arguments.find(':');
            if (pos != string::npos)
            {
                instanceType = arguments.substr(0, pos);
                formalType = arguments.substr(pos + 1);
            }
            else
            {
                instanceType = std::move(arguments);
                formalType.clear();
            }
            return true;
        }
    }

    return false;
}

bool
Slice::Java::hasTypeMetadata(const SequencePtr& seq, const MetadataList& localMetadata)
{
    if (seq->hasMetadata("java:type") || seq->hasMetadata("java:buffer") || seq->hasMetadata("java:serializable"))
    {
        return true;
    }
    if (hasMetadata("java:type", localMetadata) || hasMetadata("java:buffer", localMetadata))
    {
        return true;
    }
    return false;
}

bool
Slice::Java::getDictionaryTypes(
    const DictionaryPtr& dict,
    const string& package,
    const MetadataList& metadata,
    string& instanceType,
    string& formalType)
{
    //
    // Get the types of the key and value.
    //
    string keyTypeStr = typeToObjectString(dict->keyType(), TypeModeIn, package, MetadataList(), true);
    string valueTypeStr = typeToObjectString(dict->valueType(), TypeModeIn, package, MetadataList(), true);

    //
    // Collect metadata for a custom type.
    //
    if (getTypeMetadata(metadata, instanceType, formalType) ||
        getTypeMetadata(dict->getMetadata(), instanceType, formalType))
    {
        assert(!instanceType.empty());
        if (formalType.empty())
        {
            formalType = "java.util.Map<" + keyTypeStr + ", " + valueTypeStr + ">";
        }
        return true;
    }

    //
    // Return a default type for the platform.
    //
    instanceType = "java.util.HashMap<" + keyTypeStr + ", " + valueTypeStr + ">";
    formalType = "java.util.Map<" + keyTypeStr + ", " + valueTypeStr + ">";
    return false;
}

bool
Slice::Java::getSequenceTypes(
    const SequencePtr& seq,
    const string& package,
    const MetadataList& metadata,
    string& instanceType,
    string& formalType)
{
    if (auto meta = seq->getMetadataArgs("java:serializable"))
    {
        instanceType = formalType = *meta;
        return true;
    }

    string prefix = "java:buffer";
    if ((seq->hasMetadata(prefix) || hasMetadata(prefix, metadata)) && !hasMetadata("java:type", metadata))
    {
        instanceType = formalType = typeToBufferString(seq->type());
        return true;
    }

    if (getTypeMetadata(metadata, instanceType, formalType) ||
        getTypeMetadata(seq->getMetadata(), instanceType, formalType))
    {
        assert(!instanceType.empty());
        if (formalType.empty())
        {
            formalType =
                "java.util.List<" + typeToObjectString(seq->type(), TypeModeIn, package, MetadataList(), true) + ">";
        }
        return true;
    }

    // The default mapping is a native array.
    instanceType = formalType = typeToString(seq->type(), TypeModeIn, package, metadata, true, false) + "[]";
    return false;
}

std::pair<bool, string>
Slice::Java::javaLinkFormatter(const string& rawLink, const ContainedPtr& source, const SyntaxTreeBasePtr& target)
{
    string sourceScope = getPackage(source);
    string mappedLink;

    if (auto builtinTarget = dynamic_pointer_cast<Builtin>(target))
    {
        mappedLink = typeToObjectString(builtinTarget, TypeModeIn);
    }
    else if (auto operationTarget = dynamic_pointer_cast<Operation>(target))
    {
        // Link to the method on the proxy interface.
        mappedLink = getUnqualified(operationTarget->interface(), sourceScope) + "Prx#" + operationTarget->mappedName();
    }
    else if (auto fieldTarget = dynamic_pointer_cast<DataMember>(target))
    {
        // Link to the field on its parent type.
        auto parent = dynamic_pointer_cast<Contained>(fieldTarget->container());
        mappedLink = getUnqualified(parent, sourceScope) + "#" + fieldTarget->mappedName();
    }
    else if (auto interfaceTarget = dynamic_pointer_cast<InterfaceDecl>(target))
    {
        // Link to the proxy interface.
        mappedLink = getUnqualified(interfaceTarget, sourceScope) + "Prx";
    }
    else if (auto contained = dynamic_pointer_cast<Contained>(target))
    {
        mappedLink = getUnqualified(contained, sourceScope);

        if (dynamic_pointer_cast<Sequence>(contained) || dynamic_pointer_cast<Dictionary>(contained))
        {
            // slice2java doesn't generate types for sequences or dictionaries, so there's nothing to link to.
            // We return 'false' to signal this.
            return {false, mappedLink};
        }
    }
    else
    {
        mappedLink = rawLink;
    }

    return {true, mappedLink};
}

void
Slice::Java::validateJavaMetadata(const UnitPtr& u)
{
    map<string, MetadataInfo> knownMetadata;

    // "java:buffer"
    MetadataInfo bufferInfo = {
        .validOn = {typeid(Sequence)},
        .acceptedArgumentKind = MetadataArgumentKind::NoArguments,
        .acceptedContext = MetadataApplicationContext::DefinitionsAndTypeReferences,
        .extraValidation = [](const MetadataPtr&, const SyntaxTreeBasePtr& p) -> optional<string>
        {
            if (auto seq = dynamic_pointer_cast<Sequence>(p))
            {
                auto builtin = dynamic_pointer_cast<Builtin>(seq->type());
                if (!builtin || (builtin->kind() != Builtin::KindByte && builtin->kind() != Builtin::KindShort &&
                                 builtin->kind() != Builtin::KindInt && builtin->kind() != Builtin::KindLong &&
                                 builtin->kind() != Builtin::KindFloat && builtin->kind() != Builtin::KindDouble))
                {
                    return "the 'java:buffer' metadata can only be applied to sequences of bytes, shorts, ints, longs, "
                           "floats, or doubles";
                }

                // This check for 'hasMetadata("java:buffer")' looks redundant, but is necessary to ensure
                // that 'java:buffer' was placed on the _definition_ and not on where the sequence was used.
                if (seq->hasMetadata("java:type") && seq->hasMetadata("java:buffer"))
                {
                    return "the 'java:buffer' metadata cannot be used alongside 'java:type' - both change the mapped "
                           "type of this sequence";
                }
                if (seq->hasMetadata("java:serializable"))
                {
                    return "the 'java:buffer' metadata cannot be used alongside 'java:serializable' - both change the "
                           "mapped type of this sequence";
                }
            }
            return nullopt;
        },
    };
    knownMetadata.emplace("java:buffer", std::move(bufferInfo));

    // "java:getset"
    MetadataInfo getsetInfo = {
        .validOn = {typeid(ClassDecl), typeid(Slice::Exception), typeid(Struct), typeid(DataMember)},
        .acceptedArgumentKind = MetadataArgumentKind::NoArguments,
    };
    knownMetadata.emplace("java:getset", std::move(getsetInfo));

    // "java:identifier"
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
    knownMetadata.emplace("java:identifier", std::move(identifierInfo));

    // "java:package"
    MetadataInfo packageInfo = {
        .validOn = {typeid(Unit), typeid(Module)},
        .acceptedArgumentKind = MetadataArgumentKind::SingleArgument,
        .extraValidation = [](const MetadataPtr& metadata, const SyntaxTreeBasePtr& p) -> optional<string>
        {
            const string msg = "'java:package' is deprecated; use 'java:identifier' to remap modules instead";
            p->unit()->warning(metadata->file(), metadata->line(), Deprecated, msg);

            if (auto element = dynamic_pointer_cast<Contained>(p); element && element->hasMetadata("java:identifier"))
            {
                return "A Slice element can only have one of 'java:package' and 'java:identifier' applied to it";
            }

            // If 'java:package' is applied to a module, it must be a top-level module.
            // // Top-level modules are contained by the 'Unit'. Non-top-level modules are contained in 'Module's.
            if (auto mod = dynamic_pointer_cast<Module>(p); mod && !mod->isTopLevel())
            {
                return "the 'java:package' metadata can only be applied at the file level or to top-level modules";
            }
            return nullopt;
        },
    };
    knownMetadata.emplace("java:package", std::move(packageInfo));

    // "java:serializable"
    MetadataInfo serializableInfo = {
        .validOn = {typeid(Sequence)},
        .acceptedArgumentKind = MetadataArgumentKind::RequiredTextArgument,
        .extraValidation = [](const MetadataPtr&, const SyntaxTreeBasePtr& p) -> optional<string>
        {
            if (auto seq = dynamic_pointer_cast<Sequence>(p))
            {
                auto builtin = dynamic_pointer_cast<Builtin>(seq->type());
                if (!builtin || builtin->kind() != Builtin::KindByte)
                {
                    return "the 'java:serializable' metadata can only be applied to byte sequences (`sequence<byte>`)";
                }
            }
            // This metadata conflicts with 'java:type' and 'java:buffer', but we let the validation functions for those
            // metadata emit the conflict warnings instead of this one, since this can only go on the definition.
            return nullopt;
        },
    };
    knownMetadata.emplace("java:serializable", std::move(serializableInfo));

    // "java:serialVersionUID"
    MetadataInfo serialVersionUIDInfo = {
        .validOn = {typeid(ClassDecl), typeid(Slice::Exception), typeid(Struct)},
        .acceptedArgumentKind = MetadataArgumentKind::SingleArgument,
        .extraValidation = [](const MetadataPtr& meta, const SyntaxTreeBasePtr&) -> optional<string>
        {
            const string& value = meta->arguments();
            try
            {
                [[maybe_unused]] auto _ = std::stoll(value, nullptr, 0);
            }
            catch (const std::exception&)
            {
                return "serialVersionUID '" + value + "' is not a valid integer literal; using default value instead";
            }
            return nullopt;
        },
    };
    knownMetadata.emplace("java:serialVersionUID", std::move(serialVersionUIDInfo));

    // "java:type"
    MetadataInfo typeInfo = {
        .validOn = {typeid(Sequence), typeid(Dictionary)},
        .acceptedArgumentKind = MetadataArgumentKind::RequiredTextArgument,
        .acceptedContext = MetadataApplicationContext::DefinitionsAndTypeReferences,
        .extraValidation = [](const MetadataPtr&, const SyntaxTreeBasePtr& p) -> optional<string>
        {
            if (auto seq = dynamic_pointer_cast<Sequence>(p))
            {
                if (seq->hasMetadata("java:serializable"))
                {
                    return "the 'java:type' metadata cannot be used alongside 'java:serializable' - both change the "
                           "mapped type of this sequence";
                }
            }
            // This metadata conflicts with 'java:buffer', but we let the validation functions for that metadata
            // emit the conflict warnings instead of this one, since this can only go on the definition.
            return nullopt;
        },
    };
    knownMetadata.emplace("java:type", std::move(typeInfo));

    // "java:UserException"
    MetadataInfo userExceptionInfo = {
        .validOn = {typeid(Operation)},
        .acceptedArgumentKind = MetadataArgumentKind::NoArguments,
    };
    knownMetadata.emplace("java:UserException", std::move(userExceptionInfo));

    // Pass this information off to the parser's metadata validation logic.
    Slice::validateMetadata(u, "java", std::move(knownMetadata));
}

Slice::JavaOutput::JavaOutput() : Output(false, false) {}

void
Slice::JavaOutput::openClass(const string& cls, const string& prefix, const string& sliceFile)
{
    string::size_type pos = cls.rfind('.');
    // The generated classes are always in a package corresponding to the Slice module.
    assert(pos != string::npos);
    string package = cls.substr(0, pos);
    string file = cls.substr(pos + 1) + ".java";

    string packagePath = package;
    std::replace(packagePath.begin(), packagePath.end(), '.', '/');
    createPackagePath(packagePath, prefix);

    string path = (prefix.empty() ? "" : prefix + '/') + packagePath + '/' + file;

    // Open class file.
    open(path.c_str());
    if (isOpen())
    {
        FileTracker::instance()->addFile(path);
        printHeader();
        printGeneratedHeader(*this, sliceFile);
        newline();

        if (!package.empty())
        {
            separator();
            print("package ");
            print(package.c_str());
            print(";");
        }
    }
    else
    {
        ostringstream os;
        os << "cannot open file '" << path << "': " << IceInternal::lastErrorToString();
        throw FileException(os.str());
    }
}

void
Slice::JavaOutput::printHeader()
{
    print("// Copyright (c) ZeroC, Inc.\n");
    print("\n// slice2java version " ICE_STRING_VERSION);
}

Slice::JavaGenerator::JavaGenerator(string dir) : _dir(std::move(dir)) {}

Slice::JavaGenerator::~JavaGenerator()
{
    // If open throws an exception other generators could be left open
    // during the stack unwind.
    if (_out != nullptr)
    {
        close();
    }
    assert(_out == nullptr);
}

void
Slice::JavaGenerator::open(const string& qualifiedEntity, const string& sliceFile)
{
    assert(_out == nullptr);

    auto* out = new JavaOutput;
    try
    {
        out->openClass(qualifiedEntity, _dir, sliceFile);
    }
    catch (const FileException&)
    {
        delete out;
        throw;
    }
    _out = out;
}

void
Slice::JavaGenerator::close()
{
    assert(_out != nullptr);
    *_out << nl;
    delete _out;
    _out = nullptr;
}

Output&
Slice::JavaGenerator::output() const
{
    assert(_out != nullptr);
    return *_out;
}
