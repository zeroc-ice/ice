// Copyright (c) ZeroC, Inc.
//

#include "SwiftUtil.h"
#include "../Ice/OutputUtil.h"
#include "../Slice/MetadataValidation.h"
#include "../Slice/Util.h"

#include <algorithm>
#include <cassert>
#include <regex>

using namespace std;
using namespace Slice;
using namespace IceInternal;

namespace
{
    void writeDocLines(Output& out, const StringList& lines, bool commentFirst = true, string_view space = " ")
    {
        for (const auto& line : lines)
        {
            if (!commentFirst)
            {
                out << line;
                commentFirst = true;
                continue;
            }

            out << nl << "///";
            if (!line.empty())
            {
                out << space << line;
            }
        }
    }

    void writeDeprecatedDocComment(Output& out, const DocComment& comment)
    {
        assert(comment.isDeprecated());
        out << nl << "/// ## Deprecated";
        const StringList& docDeprecated = comment.deprecated();
        writeDocLines(out, docDeprecated);
    }
}

string
Slice::Swift::getSwiftModule(const ModulePtr& module, string& swiftPrefix)
{
    string swiftModule;
    if (auto argument = module->getMetadataArgs("swift:module"))
    {
        size_t pos = argument->find(':');
        if (pos != string::npos)
        {
            swiftModule = argument->substr(0, pos);
            swiftPrefix = argument->substr(pos + 1);
        }
        else
        {
            swiftModule = *argument;
            swiftPrefix = "";
        }
    }
    else
    {
        swiftModule = module->mappedName();
        swiftPrefix = "";
    }
    return swiftModule;
}

string
Slice::Swift::getSwiftModule(const ModulePtr& module)
{
    string prefix;
    return getSwiftModule(module, prefix);
}

void
Slice::Swift::writeDocSummary(IceInternal::Output& out, const ContainedPtr& p)
{
    const optional<DocComment>& doc = p->docComment();
    if (!doc)
    {
        return;
    }

    bool hasStarted{false};

    const StringList& docOverview = doc->overview();
    if (!docOverview.empty())
    {
        writeDocLines(out, docOverview);
        hasStarted = true;
    }

    if (doc->isDeprecated())
    {
        if (hasStarted)
        {
            out << nl << "///";
        }
        writeDeprecatedDocComment(out, *doc);
        hasStarted = true;
    }

    const StringList& remarks = doc->remarks();
    if (!remarks.empty())
    {
        if (hasStarted)
        {
            out << nl << "///";
        }
        out << nl << "/// - Remark:";
        writeDocLines(out, remarks);
    }

    const StringList& seeAlso = doc->seeAlso();
    if (!seeAlso.empty())
    {
        if (hasStarted)
        {
            out << nl << "///";
        }
        out << nl << "/// - See Also:";
        writeDocLines(out, seeAlso);
    }
}

void
Slice::Swift::writeOpDocSummary(IceInternal::Output& out, const OperationPtr& p, bool dispatch)
{
    const optional<DocComment>& doc = p->docComment();
    if (!doc)
    {
        return;
    }

    bool hasStarted{false};

    // Write the overview.
    const StringList& docOverview = doc->overview();
    if (!docOverview.empty())
    {
        writeDocLines(out, docOverview);
        hasStarted = true;
    }

    // If the comment contained an `@deprecated` include it as a section in the overview.
    if (doc->isDeprecated())
    {
        if (hasStarted)
        {
            out << nl << "///";
        }
        writeDeprecatedDocComment(out, *doc);
        hasStarted = true;
    }

    const StringList& remarks = doc->remarks();
    if (!remarks.empty())
    {
        if (hasStarted)
        {
            out << nl << "///";
        }
        out << nl << "/// ## Remarks";
        writeDocLines(out, remarks);
        hasStarted = true;
    }

    if (hasStarted && (p->parameters().size() > 0 || p->returnType() || p->throws().size() > 0))
    {
        out << nl << "///";
    }

    const auto& docParameters = doc->parameters();

    // Document all the in parameters.
    const ParameterList inParams = p->inParameters();
    bool useListStyle = inParams.size() >= 1; // '>=' instead of '>' to account for the current/context parameter.
    const string& parameterLineStart = (useListStyle ? "  - " : "- Parameter ");

    if (useListStyle)
    {
        out << nl << "/// - Parameters:";
    }
    for (const auto& param : inParams)
    {
        out << nl << "/// " << parameterLineStart << (dispatch ? "" : "iceP_") << removeEscaping(param->mappedName());
        auto docParameter = docParameters.find(param->name());
        if (docParameter != docParameters.end() && !docParameter->second.empty())
        {
            out << ": ";
            writeDocLines(out, docParameter->second, false);
        }
    }
    out << nl << "/// " << parameterLineStart;
    if (dispatch)
    {
        out << "current: The Current object for the dispatch.";
    }
    else
    {
        out << "context: Optional request context.";
    }

    // Document the return type & any out parameters.
    const ParameterList outParams = p->outParameters();
    useListStyle = p->returnsMultipleValues();
    if (useListStyle)
    {
        out << nl << "/// - Returns:";
    }
    if (p->returnType())
    {
        string returnValueName = getEscapedParamName(outParams, "returnValue");

        // First, check if the user supplied a message in the doc comment for this return type.
        const StringList& docMessage = doc->returns();
        if (useListStyle)
        {
            out << nl << "///   - " << returnValueName;
            if (!docMessage.empty())
            {
                out << ": ";
                writeDocLines(out, docMessage, false);
            }
        }
        else if (!docMessage.empty())
        {
            out << nl << "/// - Returns: ";
            writeDocLines(out, docMessage, false);
        }
    }
    for (const auto& param : outParams)
    {
        // First, check if the user supplied a message in the doc comment for this parameter / return type.
        StringList docMessage;
        const auto result = docParameters.find(param->name());
        if (result != docParameters.end())
        {
            docMessage = result->second;
        }
        if (useListStyle)
        {
            out << nl << "///   - " << removeEscaping(param->mappedName());
            if (!docMessage.empty())
            {
                out << ": ";
                writeDocLines(out, docMessage, false);
            }
        }
        else if (!docMessage.empty())
        {
            out << nl << "/// - Returns: ";
            writeDocLines(out, docMessage, false);
        }
    }

    // Document what exceptions it can throw.
    const auto& docExceptions = doc->exceptions();
    if (!docExceptions.empty())
    {
        out << nl << "/// - Throws:";
        for (const auto& docException : docExceptions)
        {
            if (docExceptions.size() > 1)
            {
                out << nl << "///   -";
            }
            out << " " << docException.first;
            if (!docException.second.empty())
            {
                out << " ";
                writeDocLines(out, docException.second, false, "     ");
            }
        }
    }
}

void
Slice::Swift::writeProxyDocSummary(IceInternal::Output& out, const InterfaceDefPtr& p, const string& swiftModule)
{
    const optional<DocComment>& doc = p->docComment();
    if (!doc)
    {
        return;
    }

    const string prx = removeEscaping(getRelativeTypeString(p, swiftModule)) + "Prx";

    const StringList& docOverview = doc->overview();
    if (docOverview.empty())
    {
        out << nl << "/// " << prx << " overview.";
    }
    else
    {
        writeDocLines(out, docOverview);
    }
}

void
Slice::Swift::writeServantDocSummary(IceInternal::Output& out, const InterfaceDefPtr& p, const string& swiftModule)
{
    const optional<DocComment>& doc = p->docComment();
    if (!doc)
    {
        return;
    }

    const string name = removeEscaping(getRelativeTypeString(p, swiftModule));

    const StringList& docOverview = doc->overview();
    if (docOverview.empty())
    {
        out << nl << "/// " << name << " overview.";
    }
    else
    {
        writeDocLines(out, docOverview);
    }
}

string
Slice::Swift::swiftLinkFormatter(const string& rawLink, const ContainedPtr& source, const SyntaxTreeBasePtr& target)
{
    if (auto contained = dynamic_pointer_cast<Contained>(target))
    {
        string nameSuffix;

        // If the link is to a data member or an enumerator, we store its name in `nameSuffix`, then navigate up to
        // its parent. Due to the flat nature of Swift, we always need to work up to something defined in a module.
        if (dynamic_pointer_cast<DataMember>(contained) || dynamic_pointer_cast<Enumerator>(contained))
        {
            nameSuffix = "/" + contained->mappedName();

            // Move up a level to the container type.
            contained = dynamic_pointer_cast<Contained>(contained->container());
            assert(contained);
        }

        // If the link is to an operation, DocC requires you to list the operation's labels in the link signature.
        // We also have to work up to the interface type afterwards, due to the flat nature of Swift.
        else if (auto operation = dynamic_pointer_cast<Operation>(contained))
        {
            nameSuffix = "/" + operation->mappedName() + "(";
            const ParameterList inParams = operation->inParameters();
            for (const auto& param : inParams)
            {
                const string paramLabel = (inParams.size() == 1 ? "_" : param->mappedName());
                nameSuffix += paramLabel + ":";
            }
            nameSuffix += "context:)";

            // Move up a level to the interface type.
            // Afterwards, we are guaranteed to hit the 'interfaceDef' branch underneath this block.
            contained = dynamic_pointer_cast<Contained>(contained->container());
            assert(contained);
        }

        // If the link involves an interface definition, we need to switch to its declaration.
        // The code-gen considers `Def` the servant type, and `Decl` the proxy type. We want to link to the proxy.
        if (auto interfaceDef = dynamic_pointer_cast<InterfaceDef>(contained))
        {
            contained = interfaceDef->declaration();
        }

        const string sourceModule = Slice::Swift::getSwiftModule(source->getTopLevelModule());
        const string targetModule = Slice::Swift::getSwiftModule(contained->getTopLevelModule());

        // Get the mapped-and-qualified name of the thing we're linking to, but replace any '.' with '/'.
        // DocC uses forward slashes to separate symbols instead of periods.
        string mappedLink = Slice::Swift::getRelativeTypeString(contained, sourceModule) + nameSuffix;
        std::replace(mappedLink.begin(), mappedLink.end(), '.', '/');

        // DocC does not support cross-module linking.
        // So we can only generate a DocC link (using double back-ticks) if the source and target are in the same
        // module. Otherwise we emit the mapped name in monospace formatting (using single back-ticks).
        const string ticks = (sourceModule == targetModule ? "``" : "`");
        return ticks + mappedLink + ticks;
    }
    else if (auto builtin = dynamic_pointer_cast<Builtin>(target))
    {
        // We have no way to link to the builtin types, so we just emit the mapped type in a monospaced format.
        return "`" + Slice::Swift::typeToString(builtin, source, false) + "`";
    }
    else // We couldn't resolve the link target and make a best-effort attempt to map the raw link.
    {
        // For Swift all we can do is replace any doxygen separators with the DocC separator ('/').
        static const std::regex separatorRegex{"::|#"};
        return "``" + std::regex_replace(rawLink, separatorRegex, "/") + "``";
    }
}

void
Slice::Swift::validateSwiftMetadata(const UnitPtr& u)
{
    map<string, MetadataInfo> knownMetadata;

    // "swift:attribute"
    MetadataInfo attributeInfo = {
        .validOn = {typeid(ClassDecl), typeid(Struct), typeid(Slice::Exception), typeid(Enum)},
        .acceptedArgumentKind = MetadataArgumentKind::RequiredTextArgument,
        .mustBeUnique = false,
    };
    knownMetadata.emplace("swift:attribute", attributeInfo);

    // "swift:class-resolver-prefix"
    MetadataInfo classResolverPrefixInfo = {
        .validOn = {typeid(Unit)},
        .acceptedArgumentKind = MetadataArgumentKind::SingleArgument,
    };
    knownMetadata.emplace("swift:class-resolver-prefix", classResolverPrefixInfo);

    // "swift:identifier"
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
    knownMetadata.emplace("swift:identifier", std::move(identifierInfo));

    // "swift:module"
    MetadataInfo moduleInfo = {
        .validOn = {typeid(Module)},
        // Even though it's really 'module:prefix' the validator sees this as a single argument since there's no commas.
        .acceptedArgumentKind = MetadataArgumentKind::SingleArgument,
        .extraValidation = [](const MetadataPtr& metadata, const SyntaxTreeBasePtr& p) -> optional<string>
        {
            const string msg = "'swift:module' is deprecated; use 'swift:identifier' to remap modules instead";
            p->unit()->warning(metadata->file(), metadata->line(), Deprecated, msg);

            if (auto cont = dynamic_pointer_cast<Contained>(p); cont && cont->hasMetadata("swift:identifier"))
            {
                return "A Slice element can only have one of 'swift:module' and 'swift:identifier' applied to it";
            }
            return nullopt;
        }};
    knownMetadata.emplace("swift:module", moduleInfo);

    // Pass this information off to the parser's metadata validation logic.
    Slice::validateMetadata(u, "swift", std::move(knownMetadata));
}

void
Slice::Swift::validateSwiftModuleMappings(const UnitPtr& unit)
{
    // Each Slice unit has to map all top-level modules to a single Swift module.
    string mappedModuleName = "";

    // Any modules that are directly contained on the unit are (by definition) top-level modules.
    // And since there is only one unit per compilation, this must be all the top-level modules.
    for (const auto& mod : unit->modules())
    {
        // We only check modules that are in the file we're compiling. We don't check modules from included files.
        if (mod->includeLevel() != 0)
        {
            continue;
        }

        const string swiftModule = getSwiftModule(mod);
        if (mappedModuleName.empty())
        {
            mappedModuleName = swiftModule;
        }
        else if (swiftModule != mappedModuleName)
        {
            ostringstream os;
            os << "invalid module mapping: the contents of a Slice file must map to a single Swift module" << endl;
            os << "note: you can use 'swift:identifier:" << mappedModuleName << "' metadata to map Slice module '"
               << mod->scoped() << "' to Swift module '" << mappedModuleName << "'" << endl;
            unit->error(mod->file(), mod->line(), os.str());
        }
    }
}

string
Slice::Swift::getRelativeTypeString(const ContainedPtr& contained, const string& currentModule)
{
    // Get the fully scoped identifier for this element, and split it up by '::' separators.
    vector<string> ids = splitScopedName(contained->mappedScoped("::", true));

    // Remove the top-level module's identifier from the vector,
    // it's going to be replaced with the mapped Swift module later.
    if (ids.size() > 1)
    {
        ids.erase(ids.begin());
    }
    // And then flatten the remaining scopes into a single prefix (since Swift doesn't support nested packages).
    string typeString;
    if (ids.size() == 1)
    {
        typeString = ids.front();
    }
    else
    {
        // If the element is in a nested module, we need to remove any escaping before flattening the scopes together.
        for (auto id : ids)
        {
            typeString += removeEscaping(std::move(id));
        }
    }

    // Determine which Swift module this element will be mapped into.
    string swiftPrefix;
    string swiftModule = getSwiftModule(contained->getTopLevelModule(), swiftPrefix);

    // If a swift prefix was provided, we need to remove any escaping before appending it to the type string.
    string prefixedTypeString;
    if (swiftPrefix.empty())
    {
        prefixedTypeString = typeString;
    }
    else
    {
        prefixedTypeString = removeEscaping(std::move(swiftPrefix)) + removeEscaping(std::move(typeString));
    }

    // Proxy types always end with "Prx".
    if (dynamic_pointer_cast<InterfaceDecl>(contained))
    {
        prefixedTypeString = removeEscaping(std::move(prefixedTypeString)) + "Prx";
    }

    // Finally, put everything together to get the mapped, fully scoped, identifier.
    string absoluteIdent = swiftModule + "." + prefixedTypeString;
    return getUnqualified(absoluteIdent, currentModule);
}

string
Slice::Swift::getValue(const string& swiftModule, const TypePtr& type)
{
    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if (builtin)
    {
        switch (builtin->kind())
        {
            case Builtin::KindBool:
            {
                return "false";
            }
            case Builtin::KindByte:
            case Builtin::KindShort:
            case Builtin::KindInt:
            case Builtin::KindLong:
            {
                return "0";
            }
            case Builtin::KindFloat:
            case Builtin::KindDouble:
            {
                return "0.0";
            }
            case Builtin::KindString:
            {
                return "\"\"";
            }
            default:
            {
                return "nil";
            }
        }
    }

    EnumPtr en = dynamic_pointer_cast<Enum>(type);
    if (en)
    {
        return "." + (*en->enumerators().begin())->mappedName();
    }

    StructPtr st = dynamic_pointer_cast<Struct>(type);
    if (st)
    {
        return getRelativeTypeString(st, swiftModule) + "()";
    }

    SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
    if (seq)
    {
        return getRelativeTypeString(seq, swiftModule) + "()";
    }

    DictionaryPtr dict = dynamic_pointer_cast<Dictionary>(type);
    if (dict)
    {
        return getRelativeTypeString(dict, swiftModule) + "()";
    }

    return "nil";
}

void
Slice::Swift::writeConstantValue(
    IceInternal::Output& out,
    const TypePtr& type,
    const SyntaxTreeBasePtr& valueType,
    const string& value,
    const string& swiftModule,
    bool optional)
{
    ConstPtr constant = dynamic_pointer_cast<Const>(valueType);
    if (constant)
    {
        out << getRelativeTypeString(constant, swiftModule);
    }
    else
    {
        if (valueType)
        {
            BuiltinPtr bp = dynamic_pointer_cast<Builtin>(type);
            EnumPtr ep = dynamic_pointer_cast<Enum>(type);
            if (bp && bp->kind() == Builtin::KindString)
            {
                out << "\"";
                out << toStringLiteral(value, "\n\r\t", "", EC6UCN, 0);
                out << "\"";
            }
            else if (ep)
            {
                assert(valueType);
                EnumeratorPtr enumerator = dynamic_pointer_cast<Enumerator>(valueType);
                assert(enumerator);
                out << getRelativeTypeString(ep, swiftModule) << "." << enumerator->mappedName();
            }
            else
            {
                out << value;
            }
        }
        else if (optional)
        {
            out << "nil";
        }
        else
        {
            out << getValue(swiftModule, type);
        }
    }
}

string
Slice::Swift::typeToString(const TypePtr& type, const ContainedPtr& usedBy, bool optional)
{
    static const char* builtinTable[] = {
        "Swift.UInt8",
        "Swift.Bool",
        "Swift.Int16",
        "Swift.Int32",
        "Swift.Int64",
        "Swift.Float",
        "Swift.Double",
        "Swift.String",
        "Ice.ObjectPrx", // ObjectPrx
        "Ice.Value"      // Value
    };

    if (!type)
    {
        return "";
    }

    string t;

    // The current module where the type is being used
    string currentModule = getSwiftModule(usedBy->getTopLevelModule());

    if (auto builtin = dynamic_pointer_cast<Builtin>(type))
    {
        t = getUnqualified(builtinTable[builtin->kind()], currentModule);
    }
    else
    {
        ContainedPtr cont = dynamic_pointer_cast<Contained>(type);
        assert(cont); // Types must be either a builtin type, or a `Contained`.
        t = getRelativeTypeString(cont, currentModule);
    }

    if (optional || isNullableType(type))
    {
        t += "?";
    }
    return t;
}

string
Slice::Swift::removeEscaping(string ident)
{
    if (!ident.empty() && ident.front() == '`')
    {
        ident.erase(0, 1);
    }
    if (!ident.empty() && ident.back() == '`')
    {
        ident.pop_back();
    }
    return ident;
}

string
Slice::Swift::getUnqualified(const string& type, const string& localModule)
{
    const string prefix = localModule + ".";
    return type.find(prefix) == 0 ? type.substr(prefix.size()) : type;
}

string
Slice::Swift::modeToString(Operation::Mode opMode)
{
    string mode;
    switch (opMode)
    {
        case Operation::Normal:
        {
            mode = ".normal";
            break;
        }
        case Operation::Idempotent:
        {
            mode = ".idempotent";
            break;
        }
        default:
        {
            assert(false);
            break;
        }
    }
    return mode;
}

string
Slice::Swift::getOptionalFormat(const TypePtr& type)
{
    return "." + type->getOptionalFormat();
}

bool
Slice::Swift::isNullableType(const TypePtr& type)
{
    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if (builtin)
    {
        switch (builtin->kind())
        {
            case Builtin::KindObjectProxy:
            case Builtin::KindValue:
            {
                return true;
            }
            default:
            {
                return false;
            }
        }
    }

    return dynamic_pointer_cast<ClassDecl>(type) || dynamic_pointer_cast<InterfaceDecl>(type);
}

void
Slice::Swift::writeDefaultInitializer(IceInternal::Output& out, bool required, bool rootClass)
{
    out << sp;
    out << nl << "public ";
    if (required)
    {
        out << "required ";
    }
    if (rootClass)
    {
        out << "init() {}";
    }
    else
    {
        assert(required);
        out << "init()";
        out << sb;
        out << nl << "super.init()";
        out << eb;
    }
}

void
Slice::Swift::writeMemberwiseInitializer(IceInternal::Output& out, const DataMemberList& members, const ContainedPtr& p)
{
    writeMemberwiseInitializer(out, members, DataMemberList(), members, p, true);
}

void
Slice::Swift::writeMemberwiseInitializer(
    IceInternal::Output& out,
    const DataMemberList& members,
    const DataMemberList& baseMembers,
    const DataMemberList& allMembers,
    const ContainedPtr& p,
    bool rootClass)
{
    if (!members.empty())
    {
        out << sp;
        out << nl;
        out << "public init(";
        bool firstMember = true;
        for (const auto& m : allMembers)
        {
            if (firstMember)
            {
                firstMember = false;
            }
            else
            {
                out << ", ";
            }

            out << m->mappedName() << ": " << typeToString(m->type(), p, m->optional());
            if (m->defaultValueType())
            {
                out << " = ";
                writeConstantValue(
                    out,
                    m->type(),
                    m->defaultValueType(),
                    m->defaultValue().value_or(""),
                    getSwiftModule(p->getTopLevelModule()),
                    m->optional());
            }
        }
        out << ")";
        out << sb;
        for (const auto& m : members)
        {
            const string name = m->mappedName();
            out << nl << "self." << name << " = " << name;
        }

        if (!rootClass)
        {
            out << nl << "super.init";
            out << spar;
            for (const auto& baseMember : baseMembers)
            {
                const string name = baseMember->mappedName();
                out << (name + ": " + name);
            }
            out << epar;
        }
        out << eb;
    }
}

void
Slice::Swift::writeMembers(IceInternal::Output& out, const DataMemberList& members, const ContainedPtr& p)
{
    string swiftModule = getSwiftModule(p->getTopLevelModule());
    for (const auto& member : members)
    {
        TypePtr type = member->type();

        const string memberName = member->mappedName();
        string memberType = typeToString(type, p, member->optional());

        // If the member type is equal to the member name, create a local type alias to avoid ambiguity.
        string alias;
        if (memberName == memberType && (dynamic_pointer_cast<Struct>(type) || dynamic_pointer_cast<Sequence>(type) ||
                                         dynamic_pointer_cast<Dictionary>(type)))
        {
            ModulePtr topLevelModule = (dynamic_pointer_cast<Contained>(type))->getTopLevelModule();
            alias = removeEscaping(topLevelModule->mappedName()) + "_" + removeEscaping(memberType);
            out << nl << "typealias " << alias << " = " << memberType;
        }

        writeDocSummary(out, member);
        out << nl << "public var " << memberName << ": " << memberType << " = ";
        if (alias.empty())
        {
            writeConstantValue(
                out,
                type,
                member->defaultValueType(),
                member->defaultValue().value_or(""),
                swiftModule,
                member->optional());
        }
        else
        {
            out << alias << "()";
        }
    }
}

bool
Slice::Swift::usesMarshalHelper(const TypePtr& type)
{
    SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
    if (seq)
    {
        BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(seq->type());
        if (builtin)
        {
            return builtin->kind() > Builtin::KindString;
        }
        return true;
    }
    return dynamic_pointer_cast<Dictionary>(type) != nullptr;
}

void
Slice::Swift::writeMarshalUnmarshalCode(
    Output& out,
    const TypePtr& type,
    const ContainedPtr& p,
    const string& param,
    bool marshal,
    int32_t tag)
{
    assert(!(type->isClassType() && tag >= 0)); // Optional classes are disallowed by the parser.

    string swiftModule = getSwiftModule(p->getTopLevelModule());
    string stream = dynamic_pointer_cast<Struct>(p) ? "self" : marshal ? "ostr" : "istr";

    string args;
    if (tag >= 0)
    {
        args += "tag: " + std::to_string(tag);
        if (marshal)
        {
            args += ", ";
        }
    }

    if (marshal)
    {
        if (tag >= 0 || usesMarshalHelper(type))
        {
            args += "value: ";
        }
        args += param;
    }

    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if (builtin)
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
            case Builtin::KindString:
            {
                if (marshal)
                {
                    out << nl << stream << ".write(" << args << ")";
                }
                else
                {
                    out << nl << param << " = try " << stream << ".read(" << args << ")";
                }
                break;
            }
            case Builtin::KindObjectProxy:
            {
                if (marshal)
                {
                    out << nl << stream << ".write(" << args << ")";
                }
                else
                {
                    if (tag >= 0)
                    {
                        args += ", type: ";
                    }
                    args += getUnqualified("Ice.ObjectPrx", swiftModule) + ".self";

                    out << nl << param << " = try " << stream << ".read(" << args << ")";
                }
                break;
            }
            case Builtin::KindValue:
            {
                if (marshal)
                {
                    out << nl << stream << ".write(" << args << ")";
                }
                else
                {
                    out << nl << "try " << stream << ".read(" << args << ") { " << param << " = $0 }";
                }
                break;
            }
            default:
            {
                break;
            }
        }
    }

    ClassDeclPtr cl = dynamic_pointer_cast<ClassDecl>(type);
    if (cl)
    {
        if (marshal)
        {
            out << nl << stream << ".write(" << args << ")";
        }
        else
        {
            string memberType = getRelativeTypeString(cl, swiftModule);
            string memberName;
            const string memberPrefix = "self.";
            if (param.find(memberPrefix) == 0)
            {
                memberName = param.substr(memberPrefix.size());
            }

            // If the member type is equal to the member name, create a local type alias to avoid ambiguity.
            string alias;
            if (memberType == memberName)
            {
                ModulePtr topLevelModule = cl->getTopLevelModule();
                alias = removeEscaping(topLevelModule->mappedName()) + "_" + removeEscaping(memberType);
                out << nl << "typealias " << alias << " = " << memberType;
            }
            args += (alias.empty() ? memberType : alias) + ".self";
            out << nl << "try " << stream << ".read(" << args << ") { " << param << " = $0 "
                << "}";
        }
        return;
    }

    EnumPtr en = dynamic_pointer_cast<Enum>(type);
    if (en)
    {
        if (marshal)
        {
            out << nl << stream << ".write(" << args << ")";
        }
        else
        {
            out << nl << param << " = try " << stream << ".read(" << args << ")";
        }
        return;
    }

    InterfaceDeclPtr prx = dynamic_pointer_cast<InterfaceDecl>(type);
    if (prx)
    {
        if (marshal)
        {
            out << nl << stream << ".write(" << args << ")";
        }
        else
        {
            if (tag >= 0)
            {
                args += ", type: ";
            }

            args += getRelativeTypeString(prx, swiftModule) + ".self";
            out << nl << param << " = try " << stream << ".read(" << args << ")";
        }
        return;
    }

    if (dynamic_pointer_cast<Struct>(type))
    {
        if (marshal)
        {
            out << nl << stream << ".write(" << args << ")";
        }
        else
        {
            out << nl << param << " = try " << stream << ".read(" << args << ")";
        }
        return;
    }

    SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
    if (seq)
    {
        BuiltinPtr seqBuiltin = dynamic_pointer_cast<Builtin>(seq->type());
        if (seqBuiltin && seqBuiltin->kind() <= Builtin::KindString)
        {
            if (marshal)
            {
                out << nl << stream << ".write(" << args << ")";
            }
            else
            {
                out << nl << param << " = try " << stream << ".read(" << args << ")";
            }
        }
        else
        {
            string helper = removeEscaping(getRelativeTypeString(seq, swiftModule)) + "Helper";
            if (marshal)
            {
                out << nl << helper << ".write(to: " << stream << ", " << args << ")";
            }
            else
            {
                out << nl << param << " = try " << helper << ".read(from: " << stream;
                if (!args.empty())
                {
                    out << ", " << args;
                }
                out << ")";
            }
        }
        return;
    }

    DictionaryPtr dict = dynamic_pointer_cast<Dictionary>(type);
    if (dict)
    {
        string helper = removeEscaping(getRelativeTypeString(dict, swiftModule)) + "Helper";
        if (marshal)
        {
            out << nl << helper << ".write(to: " << stream << ", " << args << ")";
        }
        else
        {
            out << nl << param << " = try " << helper << ".read(from: " << stream;
            if (!args.empty())
            {
                out << ", " << args;
            }
            out << ")";
        }
        return;
    }
}

string
Slice::Swift::paramLabel(const string& param, const ParameterList& params)
{
    for (const auto& q : params)
    {
        if (q->mappedName() == param)
        {
            return "_" + removeEscaping(param);
        }
    }
    return param;
}

string
Slice::Swift::operationReturnType(const OperationPtr& op)
{
    ostringstream os;
    bool returnIsTuple = op->returnsMultipleValues();
    if (returnIsTuple)
    {
        os << "(";
    }

    ParameterList outParams = op->outParameters();
    TypePtr returnType = op->returnType();
    if (returnType)
    {
        if (returnIsTuple)
        {
            os << paramLabel("returnValue", outParams) << ": ";
        }
        os << typeToString(returnType, op, op->returnIsOptional());
    }

    for (auto q = outParams.begin(); q != outParams.end(); ++q)
    {
        if (returnType || q != outParams.begin())
        {
            os << ", ";
        }

        if (returnIsTuple)
        {
            os << (*q)->mappedName() << ": ";
        }

        os << typeToString((*q)->type(), *q, (*q)->optional());
    }

    if (returnIsTuple)
    {
        os << ")";
    }

    return os.str();
}

std::string
Slice::Swift::operationReturnDeclaration(const OperationPtr& op)
{
    ostringstream os;
    ParameterList outParams = op->outParameters();
    TypePtr returnType = op->returnType();
    bool returnIsTuple = op->returnsMultipleValues();

    if (returnIsTuple)
    {
        os << "(";
    }

    if (returnType)
    {
        os << ("iceP_" + paramLabel("returnValue", outParams));
    }

    for (auto q = outParams.begin(); q != outParams.end(); ++q)
    {
        if (returnType || q != outParams.begin())
        {
            os << ", ";
        }

        os << ("iceP_" + removeEscaping((*q)->mappedName()));
    }

    if (returnIsTuple)
    {
        os << ")";
    }

    return os.str();
}

void
Slice::Swift::writeMarshalInParams(::IceInternal::Output& out, const OperationPtr& op)
{
    // Marshal parameters in this order '(required..., optional...)'.

    out << "{ ostr in";
    out.inc();
    for (const auto& param : op->sortedInParameters())
    {
        const string paramName = "iceP_" + removeEscaping(param->mappedName());
        writeMarshalUnmarshalCode(out, param->type(), op, paramName, true, param->tag());
    }
    if (op->sendsClasses())
    {
        out << nl << "ostr.writePendingValues()";
    }
    out.dec();
    out << nl << "}";
}

void
Slice::Swift::writeMarshalAsyncOutParams(::IceInternal::Output& out, const OperationPtr& op)
{
    // Marshal parameters in this order '(required..., optional...)'.

    out << nl << "let " << operationReturnDeclaration(op) << " = value";
    for (const auto& param : op->sortedReturnAndOutParameters(getEscapedParamName(op->outParameters(), "returnValue")))
    {
        writeMarshalUnmarshalCode(
            out,
            param->type(),
            op,
            "iceP_" + removeEscaping(param->mappedName()),
            true,
            param->tag());
    }
    if (op->returnsClasses())
    {
        out << nl << "ostr.writePendingValues()";
    }
}

void
Slice::Swift::writeUnmarshalOutParams(::IceInternal::Output& out, const OperationPtr& op)
{
    // We need a separate nested function for the nonisolated(unsafe) variable when unmarshaling classes.

    out << sp;
    out << nl << "func read(istr: Ice.InputStream) throws -> sending " << operationReturnType(op) << sb;

    const ParameterList outParams = op->outParameters();
    const bool returnsMultipleValues = op->returnsMultipleValues();

    //
    // Unmarshal parameters
    // 1. required
    // 2. non-optional return
    // 3. optional (including optional return)
    //

    for (const auto& param : op->sortedReturnAndOutParameters(getEscapedParamName(op->outParameters(), "returnValue")))
    {
        const TypePtr paramType = param->type();
        const string typeString = typeToString(paramType, op, param->optional());
        const string paramName = "iceP_" + removeEscaping(param->mappedName());
        string paramString;
        if (paramType->isClassType())
        {
            out << nl << "nonisolated(unsafe) var " << paramName << ": " << typeString;
            paramString = paramName;
        }
        else
        {
            paramString = "let " + paramName + ": " + typeString;
        }
        writeMarshalUnmarshalCode(out, paramType, op, paramString, false, param->tag());
    }
    if (op->returnsClasses())
    {
        out << nl << "try istr.readPendingValues()";
    }

    out << nl << "return ";

    if (returnsMultipleValues)
    {
        out << spar;
    }

    if (op->returnType())
    {
        string returnValueName = getEscapedParamName(outParams, "returnValue");
        out << ("iceP_" + returnValueName);
    }
    for (const auto& param : outParams)
    {
        out << ("iceP_" + removeEscaping(param->mappedName()));
    }

    if (returnsMultipleValues)
    {
        out << epar;
    }

    out << eb;
}

void
Slice::Swift::writeUnmarshalInParams(::IceInternal::Output& out, const OperationPtr& op)
{
    // Unmarshal parameters in this order '(required..., optional...)'.

    for (const auto& param : op->sortedInParameters())
    {
        const TypePtr paramType = param->type();
        const string paramName = "iceP_" + removeEscaping(param->mappedName());
        const string typeString = typeToString(paramType, op, param->optional());
        string paramString;
        if (paramType->isClassType())
        {
            out << nl << "nonisolated(unsafe) var " << paramName << ": " << typeString;
            paramString = paramName;
        }
        else
        {
            paramString = "let " + paramName + ": " + typeString;
        }
        writeMarshalUnmarshalCode(out, paramType, op, paramString, false, param->tag());
    }

    if (op->sendsClasses())
    {
        out << nl << "try istr.readPendingValues()";
    }
}

void
Slice::Swift::writeUnmarshalUserException(::IceInternal::Output& out, const OperationPtr& op)
{
    const string swiftModule = getSwiftModule(op->getTopLevelModule());

    // Arrange exceptions into most-derived to least-derived order. If we don't
    // do this, a base exception handler can appear before a derived exception
    // handler, causing compiler warnings and resulting in the base exception
    // being marshaled instead of the derived exception.
    ExceptionList throws = op->throws();
    throws.sort(Slice::DerivedToBaseCompare());

    out << "{ ex in";
    out.inc();
    out << nl << "do ";
    out << sb;
    out << nl << "throw ex";
    out << eb;

    for (const auto& thrown : throws)
    {
        out << " catch let error as " << getRelativeTypeString(thrown, swiftModule) << sb;
        out << nl << "throw error";
        out << eb;
    }
    out << " catch is " << getUnqualified("Ice.UserException", swiftModule) << " {}";
    out.dec();
    out << nl << "}";
}

void
Slice::Swift::writeSwiftAttributes(::IceInternal::Output& out, const MetadataList& metadata)
{
    for (const auto& meta : metadata)
    {
        if (meta->directive() == "swift:attribute")
        {
            out << nl << meta->arguments();
        }
    }
}
