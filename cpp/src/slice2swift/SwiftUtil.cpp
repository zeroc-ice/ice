// Copyright (c) ZeroC, Inc.
//

#include "SwiftUtil.h"
#include "../Ice/OutputUtil.h"
#include "../Slice/MetadataValidation.h"
#include "../Slice/Util.h"

#include <algorithm>
#include <cassert>

using namespace std;
using namespace Slice;
using namespace IceInternal;

namespace
{
    static string lookupKwd(const string& name)
    {
        //
        // Keyword list. *Must* be kept in alphabetical order.
        //
        static const string keywordList[] = {
            "Any",
            "Self",
            "Type",
            "as",
            "associatedtype",
            "associativity",
            "async",
            "await",
            "borrowing",
            "break",
            "case",
            "catch",
            "class",
            "consuming",
            "continue",
            "convenience",
            "default",
            "defer",
            "deinit",
            "didSet",
            "do",
            "dynamic",
            "else",
            "enum",
            "extension",
            "fallthrough",
            "false",
            "fileprivate",
            "final",
            "for",
            "func",
            "get",
            "guard",
            "if",
            "import",
            "in",
            "indirect",
            "infix",
            "init",
            "inout",
            "internal",
            "is",
            "lazy",
            "left",
            "let",
            "mutating",
            "nil",
            "none",
            "nonisolated",
            "nonmutating",
            "open",
            "operator",
            "optional",
            "override",
            "package",
            "postfix",
            "precedence",
            "precedencegroup",
            "prefix",
            "private",
            "protocol",
            "public",
            "repeat",
            "required",
            "rethrows",
            "return",
            "right",
            "self",
            "set",
            "some",
            "static",
            "struct",
            "subscript",
            "super",
            "switch",
            "throw",
            "throws",
            "true",
            "try",
            "typealias",
            "unowned",
            "var",
            "weak",
            "where",
            "while",
            "willSet"};
        bool found = binary_search(&keywordList[0], &keywordList[sizeof(keywordList) / sizeof(*keywordList)], name);
        if (found)
        {
            return "`" + name + "`";
        }

        return name;
    }

    string replace(const string& s, const string& patt, const string& val)
    {
        string r{s};
        string::size_type pos = r.find(patt);
        while (pos != string::npos)
        {
            r.replace(pos, patt.size(), val);
            pos += val.size();
            pos = r.find(patt, pos);
        }
        return r;
    }

    string opFormatTypeToString(const OperationPtr& op)
    {
        optional<FormatType> opFormat = op->format();
        if (opFormat)
        {
            switch (*opFormat)
            {
                case CompactFormat:
                    return ".CompactFormat";
                case SlicedFormat:
                    return ".SlicedFormat";
                default:
                    assert(false);
                    return "???";
            }
        }
        else
        {
            return "nil";
        }
    }

    // TODO: fix this to emit double-ticks instead of single-ticks once we've fixed all the links.
    // TODO: this is temporary and will be replaced when we add 'swift:identifier' support.
    string swiftLinkFormatter(const string& rawLink, const ContainedPtr&, const SyntaxTreeBasePtr&)
    {
        string result = "`";

        auto hashPos = rawLink.find('#');
        if (hashPos != string::npos)
        {
            if (hashPos != 0)
            {
                result += fixIdent(rawLink.substr(0, hashPos));
                result += "/";
            }
            result += fixIdent(rawLink.substr(hashPos + 1));
        }
        else
        {
            result += fixIdent(rawLink);
        }

        return result + "`";
    }
}

// Check the given identifier against Swift's list of reserved words. If it matches
// a reserved word, then an escaped version is returned with a leading underscore.
//
// The provided identifier must be a _Swift_ identifier (Ice.Foo) and not a Slice identifier (Ice::Foo).
string
Slice::fixIdent(const string& ident)
{
    assert(ident.find(':' == string::npos));

    // Swift namespaces are flat, so we don't need to check for multiple periods, there can be at most 1.
    string::size_type separator_pos = ident.find('.');
    if (separator_pos != string::npos)
    {
        // If this identifier is scoped, we break it up and escape each piece separately.
        auto scope = ident.substr(0, separator_pos);
        auto name = ident.substr(separator_pos + 1);

        ostringstream result;
        result << lookupKwd(scope) << "." << lookupKwd(name);
        return result.str();
    }
    else
    {
        // If this identifier isn't scoped, we directly escape it.
        return lookupKwd(ident);
    }
}

string
Slice::getSwiftModule(const ModulePtr& module, string& swiftPrefix)
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
        swiftModule = module->name();
        swiftPrefix = "";
    }
    return fixIdent(swiftModule);
}

string
Slice::getSwiftModule(const ModulePtr& module)
{
    string prefix;
    return getSwiftModule(module, prefix);
}

ModulePtr
Slice::getTopLevelModule(const ContainedPtr& cont)
{
    // Traverse to the top-level module.
    ModulePtr m;
    ContainedPtr p = cont;
    while (true)
    {
        if (dynamic_pointer_cast<Module>(p))
        {
            m = dynamic_pointer_cast<Module>(p);
        }

        if (p->isTopLevel())
        {
            break;
        }
        p = dynamic_pointer_cast<Contained>(p->container());
        assert(p);
    }
    return m;
}

void
SwiftGenerator::writeDocLines(IceInternal::Output& out, const StringList& lines, bool commentFirst, const string& space)
{
    StringList l = lines;
    if (!commentFirst)
    {
        out << l.front();
        l.pop_front();
    }

    for (const auto& line : l)
    {
        out << nl << "///";
        if (!line.empty())
        {
            out << space << line;
        }
    }
}

void
SwiftGenerator::writeDocSentence(IceInternal::Output& out, const StringList& lines)
{
    //
    // Write the first sentence.
    //
    for (auto i = lines.begin(); i != lines.end(); ++i)
    {
        const string ws = " \t";

        if (i->empty())
        {
            break;
        }
        if (i != lines.begin() && i->find_first_not_of(ws) == 0)
        {
            out << " ";
        }
        string::size_type pos = i->find('.');
        if (pos == string::npos)
        {
            out << *i;
        }
        else if (pos == i->size() - 1)
        {
            out << *i;
            break;
        }
        else
        {
            //
            // Assume a period followed by whitespace indicates the end of the sentence.
            //
            while (pos != string::npos)
            {
                if (ws.find((*i)[pos + 1]) != string::npos)
                {
                    break;
                }
                pos = i->find('.', pos + 1);
            }
            if (pos != string::npos)
            {
                out << i->substr(0, pos + 1);
                break;
            }
            else
            {
                out << *i;
            }
        }
    }
}

void
SwiftGenerator::writeDocSummary(IceInternal::Output& out, const ContainedPtr& p)
{
    optional<DocComment> doc = DocComment::parseFrom(p, swiftLinkFormatter, true);
    if (!doc)
    {
        return;
    }

    bool hasStarted = false;

    StringList docOverview = doc->overview();
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
        out << nl << "/// ## Deprecated";
        StringList docDeprecated = doc->deprecated();
        if (!docDeprecated.empty())
        {
            writeDocLines(out, docDeprecated);
        }
    }

    // TODO we should add a section for '@see' tags.
}

void
SwiftGenerator::writeOpDocSummary(IceInternal::Output& out, const OperationPtr& p, bool dispatch)
{
    optional<DocComment> doc = DocComment::parseFrom(p, swiftLinkFormatter, true);
    if (!doc)
    {
        return;
    }

    bool hasStarted = false;

    // Write the overview.
    StringList docOverview = doc->overview();
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
        hasStarted = true;

        out << nl << "///  ## Deprecated";
        StringList docDeprecated = doc->deprecated();
        if (!docDeprecated.empty())
        {
            writeDocLines(out, docDeprecated);
        }
    }

    auto docParameters = doc->parameters();

    // Document all the in parameters.
    const ParamInfoList allInParams = getAllInParams(p);
    bool useListStyle = allInParams.size() >= 1; // '>=' instead of '>' to account for the current/context parameter.
    if (hasStarted)
    {
        out << nl << "///";
        // Don't bother setting `hasStarted`. We always emit a comment for parameters. So no need to check anymore.
    }
    if (useListStyle)
    {
        out << nl << "/// - Parameters:";
    }
    for (const auto& inParam : allInParams)
    {
        out << nl << "/// " << (useListStyle ? "  - " : "- Parameter ") << (dispatch ? "" : "iceP_") << inParam.name;
        auto docParameter = docParameters.find(inParam.name);
        if (docParameter != docParameters.end() && !docParameter->second.empty())
        {
            out << ": ";
            writeDocLines(out, docParameter->second, false);
        }
    }
    out << nl << "/// " << (useListStyle ? "  - " : "- Parameter ");
    if (dispatch)
    {
        out << "current: The Current object for the dispatch.";
    }
    else
    {
        out << "context: Optional request context.";
    }

    // Document the return type & any out parameters.
    ParamInfoList allOutParams = getAllOutParams(p);
    useListStyle = allOutParams.size() > 1;
    if (useListStyle)
    {
        out << nl << "///";
        out << nl << "/// - Returns:";
    }

    if (!allOutParams.empty())
    {
        // `getAllOutParams` puts the return-type parameter at the end, we want to move it to the front.
        allOutParams.push_front(allOutParams.back());
        allOutParams.pop_back();

        // Document each of the out parameters.
        for (const auto& outParam : allOutParams)
        {
            // First, check if the user supplied a message in the doc comment for this parameter / return type.
            StringList docMessage;
            if (outParam.param == nullptr) // This means it was a return type, not an out parameter.
            {
                docMessage = doc->returns();
            }
            else
            {
                const auto result = docParameters.find(outParam.name);
                if (result != docParameters.end())
                {
                    docMessage = result->second;
                }
            }

            if (useListStyle)
            {
                out << nl << "///   - " << outParam.name;
                if (!docMessage.empty())
                {
                    out << ": ";
                    writeDocLines(out, docMessage, false);
                }
            }
            else if (!docMessage.empty())
            {
                out << nl << "///";
                out << nl << "/// - Returns: ";
                writeDocLines(out, docMessage, false);
            }
        }
    }

    // Document what exceptions it can throw.
    auto docExceptions = doc->exceptions();
    if (!docExceptions.empty())
    {
        useListStyle = docExceptions.size() < 2;
        out << nl << "///";
        out << nl << "/// - Throws:";
        for (const auto& docException : docExceptions)
        {
            if (useListStyle)
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
SwiftGenerator::writeProxyDocSummary(IceInternal::Output& out, const InterfaceDefPtr& p, const string& swiftModule)
{
    optional<DocComment> doc = DocComment::parseFrom(p, swiftLinkFormatter, true);
    if (!doc)
    {
        return;
    }

    const string name = getRelativeTypeString(p, swiftModule);
    const string prx = name + "Prx";

    StringList docOverview = doc->overview();
    if (docOverview.empty())
    {
        out << nl << "/// " << prx << " overview.";
    }
    else
    {
        writeDocLines(out, docOverview);
    }

    const OperationList ops = p->operations();
    if (!ops.empty())
    {
        out << nl << "///";
        out << nl << "/// " << prx << " Methods:";
        for (const auto& op : ops)
        {
            optional<DocComment> opdoc = DocComment::parseFrom(op, swiftLinkFormatter, true);
            optional<StringList> opDocOverview;
            if (opdoc)
            {
                StringList overview = opdoc->overview();
                if (!overview.empty())
                {
                    opDocOverview = overview;
                }
            }

            out << nl << "///  - " << fixIdent(op->name());
            if (auto overview = opDocOverview)
            {
                out << ": ";
                writeDocSentence(out, *overview);
            }

            out << nl << "///  - " << op->name() << "Async";
            if (auto overview = opDocOverview)
            {
                out << ": ";
                writeDocSentence(out, *overview);
            }
        }
    }
}

void
SwiftGenerator::writeServantDocSummary(IceInternal::Output& out, const InterfaceDefPtr& p, const string& swiftModule)
{
    optional<DocComment> doc = DocComment::parseFrom(p, swiftLinkFormatter, true);
    if (!doc)
    {
        return;
    }

    const string name = getRelativeTypeString(p, swiftModule);

    StringList docOverview = doc->overview();
    if (docOverview.empty())
    {
        out << nl << "/// " << name << " overview.";
    }
    else
    {
        writeDocLines(out, docOverview);
    }

    const OperationList ops = p->operations();
    if (!ops.empty())
    {
        out << nl << "///";
        out << nl << "/// " << name << " Methods:";
        for (const auto& op : ops)
        {
            out << nl << "///  - " << fixIdent(op->name());
            optional<DocComment> opdoc = DocComment::parseFrom(op, swiftLinkFormatter, true);
            if (opdoc)
            {
                StringList opdocOverview = opdoc->overview();
                if (!opdocOverview.empty())
                {
                    out << ": ";
                    writeDocSentence(out, opdocOverview);
                }
            }
        }
    }
}

void
SwiftGenerator::validateMetadata(const UnitPtr& u)
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

    // "swift:inherits"
    MetadataInfo inheritsInfo = {
        .validOn = {typeid(InterfaceDecl)},
        .acceptedArgumentKind = MetadataArgumentKind::RequiredTextArgument,
        .mustBeUnique = false,
    };
    knownMetadata.emplace("swift:inherits", inheritsInfo);

    // "swift:module"
    MetadataInfo moduleInfo = {
        .validOn = {typeid(Module)},
        // Even though it's really 'module:prefix' the validator sees this as a single argument since there's no commas.
        .acceptedArgumentKind = MetadataArgumentKind::SingleArgument,
    };
    knownMetadata.emplace("swift:module", moduleInfo);

    // Pass this information off to the parser's metadata validation logic.
    Slice::validateMetadata(u, "swift", knownMetadata);
}

void
SwiftGenerator::validateSwiftModuleMappings(const UnitPtr& unit)
{
    using ModuleMap = std::map<std::string, std::string>;
    // Each Slice unit has to map all top-level modules to a single Swift module
    ModuleMap mappedModules;
    // With a given Swift module a Slice module has to map to a single prefix
    std::map<std::string, ModuleMap> mappedModulePrefixes;

    // Any modules that are directly contained on the unit are (by definition) top-level modules.
    // And since there is only one unit per compilation, this must be all the top-level modules.
    for (const auto& module : unit->modules())
    {
        string swiftPrefix;
        string swiftModule = getSwiftModule(module, swiftPrefix);

        const string filename = module->definitionContext()->filename();
        auto current = mappedModules.find(filename);

        if (current == mappedModules.end())
        {
            mappedModules[filename] = swiftModule;
        }
        else if (current->second != swiftModule)
        {
            ostringstream os;
            os << "invalid module mapping:\n Slice module '" << module->scoped() << "' should map to Swift module '"
               << current->second << "'" << endl;
            unit->error(module->file(), module->line(), os.str());
        }

        auto prefixes = mappedModulePrefixes.find(swiftModule);
        if (prefixes == mappedModulePrefixes.end())
        {
            ModuleMap mappings;
            mappings[module->name()] = swiftPrefix;
            mappedModulePrefixes[swiftModule] = mappings;
        }
        else
        {
            current = prefixes->second.find(module->name());
            if (current == prefixes->second.end())
            {
                prefixes->second[module->name()] = swiftPrefix;
            }
            else if (current->second != swiftPrefix)
            {
                ostringstream os;
                os << "invalid module prefix:\n Slice module '" << module->scoped() << "' is already using";
                if (current->second.empty())
                {
                    os << " no prefix " << endl;
                }
                else
                {
                    os << " a different Swift module prefix '" << current->second << "'" << endl;
                }
                unit->error(module->file(), module->line(), os.str());
            }
        }
    }
}

string
SwiftGenerator::getRelativeTypeString(const ContainedPtr& contained, const string& currentModule)
{
    string typeString = contained->scoped();

    // Proxy types always end with "Prx".
    if (dynamic_pointer_cast<InterfaceDecl>(contained))
    {
        typeString += "Prx";
    }

    // Remove the leading '::' the Slice compiler always gives us.
    assert(typeString.find("::") == 0);
    typeString.erase(0, 2);
    // Replace the definition top-level module by the corresponding Swift module
    // and append the Swift prefix for the Slice module, then any remaining nested
    // modules become a Swift prefix
    size_t pos = typeString.find("::");
    if (pos != string::npos)
    {
        typeString = typeString.substr(pos + 2);
    }
    string swiftPrefix;
    string swiftModule = getSwiftModule(getTopLevelModule(contained), swiftPrefix);
    auto absoluteIdent = swiftModule + "." + swiftPrefix + replace(typeString, "::", "");
    return getUnqualified(absoluteIdent, currentModule);
}

string
SwiftGenerator::getValue(const string& swiftModule, const TypePtr& type)
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
        return "." + fixIdent((*en->enumerators().begin())->name());
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
SwiftGenerator::writeConstantValue(
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
                out << getRelativeTypeString(ep, swiftModule) << "." << enumerator->name();
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
SwiftGenerator::typeToString(const TypePtr& type, const ContainedPtr& toplevel, bool optional)
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
        "Ice.Disp",      // Object
        "Ice.ObjectPrx", // ObjectPrx
        "Ice.Value"      // Value
    };

    if (!type)
    {
        return "";
    }

    string t = "";
    //
    // The current module where the type is being used
    //
    string currentModule = getSwiftModule(getTopLevelModule(toplevel));
    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);

    if (builtin)
    {
        if (builtin->kind() == Builtin::KindObject)
        {
            t = getUnqualified(builtinTable[Builtin::KindValue], currentModule);
        }
        else
        {
            t = getUnqualified(builtinTable[builtin->kind()], currentModule);
        }
    }

    ClassDeclPtr cl = dynamic_pointer_cast<ClassDecl>(type);
    InterfaceDeclPtr prx = dynamic_pointer_cast<InterfaceDecl>(type);
    ContainedPtr cont = dynamic_pointer_cast<Contained>(type);

    if (cl)
    {
        t += fixIdent(getRelativeTypeString(cl, currentModule));
    }
    else if (prx)
    {
        t = getRelativeTypeString(prx, currentModule);
    }
    else if (cont)
    {
        t = fixIdent(getRelativeTypeString(cont, currentModule));
    }

    if (optional || isNullableType(type))
    {
        t += "?";
    }
    return t;
}

string
SwiftGenerator::getUnqualified(const string& type, const string& localModule)
{
    const string prefix = localModule + ".";
    return type.find(prefix) == 0 ? type.substr(prefix.size()) : type;
}

string
SwiftGenerator::modeToString(Operation::Mode opMode)
{
    string mode;
    switch (opMode)
    {
        case Operation::Normal:
        {
            mode = ".Normal";
            break;
        }
        case Operation::Idempotent:
        {
            mode = ".Idempotent";
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
SwiftGenerator::getOptionalFormat(const TypePtr& type)
{
    return "." + type->getOptionalFormat();
}

bool
SwiftGenerator::isNullableType(const TypePtr& type)
{
    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if (builtin)
    {
        switch (builtin->kind())
        {
            case Builtin::KindObject:
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
SwiftGenerator::writeDefaultInitializer(IceInternal::Output& out, bool required, bool rootClass)
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
SwiftGenerator::writeMemberwiseInitializer(
    IceInternal::Output& out,
    const DataMemberList& members,
    const ContainedPtr& p)
{
    writeMemberwiseInitializer(out, members, DataMemberList(), members, p, true);
}

void
SwiftGenerator::writeMemberwiseInitializer(
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
        out << "public init" << spar;
        for (const auto& m : allMembers)
        {
            out << (fixIdent(m->name()) + ": " + typeToString(m->type(), p, m->optional()));
        }
        out << epar;
        out << sb;
        for (const auto& m : members)
        {
            out << nl << "self." << fixIdent(m->name()) << " = " << fixIdent(m->name());
        }

        if (!rootClass)
        {
            out << nl << "super.init";
            out << spar;
            for (const auto& baseMember : baseMembers)
            {
                const string name = fixIdent(baseMember->name());
                out << (name + ": " + name);
            }
            out << epar;
        }
        out << eb;
    }
}

void
SwiftGenerator::writeMembers(
    IceInternal::Output& out,
    const DataMemberList& members,
    const ContainedPtr& p,
    int typeCtx)
{
    string swiftModule = getSwiftModule(getTopLevelModule(p));
    bool protocol = (typeCtx & TypeContextProtocol) != 0;
    string access = protocol ? "" : "public ";
    for (const auto& member : members)
    {
        TypePtr type = member->type();

        const string memberName = fixIdent(member->name());
        string memberType = typeToString(type, p, member->optional());

        //
        // If the member type is equal to the member name, create a local type alias
        // to avoid ambiguity.
        //
        string alias;
        if (!protocol && memberName == memberType &&
            (dynamic_pointer_cast<Struct>(type) || dynamic_pointer_cast<Sequence>(type) ||
             dynamic_pointer_cast<Dictionary>(type)))
        {
            ModulePtr m = getTopLevelModule(dynamic_pointer_cast<Contained>(type));
            alias = m->name() + "_" + memberType;
            out << nl << "typealias " << alias << " = " << memberType;
        }

        writeDocSummary(out, member);
        out << nl << access << "var " << memberName << ": " << memberType;
        if (protocol)
        {
            out << " { get set }";
        }
        else
        {
            out << " = ";
            if (alias.empty())
            {
                writeConstantValue(
                    out,
                    type,
                    member->defaultValueType(),
                    member->defaultValue().value_or(""), // TODO: revisit this code
                    swiftModule,
                    member->optional());
            }
            else
            {
                out << alias << "()";
            }
        }
    }
}

bool
SwiftGenerator::usesMarshalHelper(const TypePtr& type)
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
SwiftGenerator::writeMarshalUnmarshalCode(
    Output& out,
    const TypePtr& type,
    const ContainedPtr& p,
    const string& param,
    bool marshal,
    int tag)
{
    assert(!(type->isClassType() && tag >= 0)); // Optional classes are disallowed by the parser.

    string swiftModule = getSwiftModule(getTopLevelModule(p));
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
            case Builtin::KindObject:
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

            string alias;
            //
            // If the member type is equal to the member name, create a local type alias
            // to avoid ambiguity.
            //
            if (memberType == memberName)
            {
                ModulePtr m = getTopLevelModule(cl);
                alias = m->name() + "_" + memberType;
                out << nl << "typealias " << alias << " = " << memberType;
            }
            args += fixIdent(alias.empty() ? memberType : alias) + ".self";
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
            string helper = getRelativeTypeString(seq, swiftModule) + "Helper";
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
        string helper = getRelativeTypeString(dict, swiftModule) + "Helper";
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
SwiftGenerator::paramLabel(const string& param, const ParameterList& params)
{
    string s = param;
    for (const auto& q : params)
    {
        if (q->name() == param)
        {
            s = "_" + s;
            break;
        }
    }
    return s;
}

bool
SwiftGenerator::operationReturnIsTuple(const OperationPtr& op)
{
    ParameterList outParams = op->outParameters();
    return (op->returnType() && outParams.size() > 0) || outParams.size() > 1;
}

string
SwiftGenerator::operationReturnType(const OperationPtr& op)
{
    ostringstream os;
    bool returnIsTuple = operationReturnIsTuple(op);
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
            os << (*q)->name() << ": ";
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
SwiftGenerator::operationReturnDeclaration(const OperationPtr& op)
{
    ostringstream os;
    ParameterList outParams = op->outParameters();
    TypePtr returnType = op->returnType();
    bool returnIsTuple = operationReturnIsTuple(op);

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

        os << ("iceP_" + (*q)->name());
    }

    if (returnIsTuple)
    {
        os << ")";
    }

    return os.str();
}

string
SwiftGenerator::operationInParamsDeclaration(const OperationPtr& op)
{
    ostringstream os;

    ParameterList inParams = op->inParameters();
    const bool isTuple = inParams.size() > 1;

    if (!inParams.empty())
    {
        if (isTuple)
        {
            os << "(";
        }
        for (auto q = inParams.begin(); q != inParams.end(); ++q)
        {
            if (q != inParams.begin())
            {
                os << ", ";
            }

            os << ("iceP_" + (*q)->name());
        }
        if (isTuple)
        {
            os << ")";
        }

        os << ": ";

        if (isTuple)
        {
            os << "(";
        }
        for (auto q = inParams.begin(); q != inParams.end(); ++q)
        {
            if (q != inParams.begin())
            {
                os << ", ";
            }

            os << typeToString((*q)->type(), *q, (*q)->optional());
        }
        if (isTuple)
        {
            os << ")";
        }
    }

    return os.str();
}

ParamInfoList
SwiftGenerator::getAllInParams(const OperationPtr& op)
{
    const ParameterList l = op->inParameters();
    ParamInfoList r;
    for (const auto& p : l)
    {
        ParamInfo info;
        info.name = p->name();
        info.type = p->type();
        info.typeStr = typeToString(info.type, op, p->optional());
        info.optional = p->optional();
        info.tag = p->tag();
        info.param = p;
        r.push_back(info);
    }
    return r;
}

void
SwiftGenerator::getInParams(const OperationPtr& op, ParamInfoList& required, ParamInfoList& optional)
{
    const ParamInfoList params = getAllInParams(op);
    for (const auto& param : params)
    {
        if (param.optional)
        {
            optional.push_back(param);
        }
        else
        {
            required.push_back(param);
        }
    }

    //
    // Sort optional parameters by tag.
    //
    class SortFn
    {
    public:
        static bool compare(const ParamInfo& lhs, const ParamInfo& rhs) { return lhs.tag < rhs.tag; }
    };
    optional.sort(SortFn::compare);
}

ParamInfoList
SwiftGenerator::getAllOutParams(const OperationPtr& op)
{
    ParameterList params = op->outParameters();
    ParamInfoList l;

    for (const auto& param : params)
    {
        ParamInfo info;
        info.name = param->name();
        info.type = param->type();
        info.typeStr = typeToString(info.type, op, param->optional());
        info.optional = param->optional();
        info.tag = param->tag();
        info.param = param;
        l.push_back(info);
    }

    if (op->returnType())
    {
        ParamInfo info;
        info.name = paramLabel("returnValue", params);
        info.type = op->returnType();
        info.typeStr = typeToString(info.type, op, op->returnIsOptional());
        info.optional = op->returnIsOptional();
        info.tag = op->returnTag();
        l.push_back(info);
    }

    return l;
}

void
SwiftGenerator::getOutParams(const OperationPtr& op, ParamInfoList& required, ParamInfoList& optional)
{
    const ParamInfoList params = getAllOutParams(op);
    for (const auto& param : params)
    {
        if (param.optional)
        {
            optional.push_back(param);
        }
        else
        {
            required.push_back(param);
        }
    }

    //
    // Sort optional parameters by tag.
    //
    class SortFn
    {
    public:
        static bool compare(const ParamInfo& lhs, const ParamInfo& rhs) { return lhs.tag < rhs.tag; }
    };
    optional.sort(SortFn::compare);
}

void
SwiftGenerator::writeMarshalInParams(::IceInternal::Output& out, const OperationPtr& op)
{
    ParamInfoList requiredInParams, optionalInParams;
    getInParams(op, requiredInParams, optionalInParams);

    out << "{ ostr in";
    out.inc();
    //
    // Marshal parameters
    // 1. required
    // 2. optional
    //

    for (const auto& requiredInParam : requiredInParams)
    {
        writeMarshalUnmarshalCode(out, requiredInParam.type, op, "iceP_" + requiredInParam.name, true);
    }

    for (const auto& optionalInParam : optionalInParams)
    {
        writeMarshalUnmarshalCode(
            out,
            optionalInParam.type,
            op,
            "iceP_" + optionalInParam.name,
            true,
            optionalInParam.tag);
    }

    if (op->sendsClasses())
    {
        out << nl << "ostr.writePendingValues()";
    }
    out.dec();
    out << nl << "}";
}

void
SwiftGenerator::writeMarshalOutParams(::IceInternal::Output& out, const OperationPtr& op)
{
    ParamInfoList requiredOutParams, optionalOutParams;
    getOutParams(op, requiredOutParams, optionalOutParams);

    //
    // Marshal parameters
    // 1. required
    // 2. optional (including optional return)
    //

    for (const auto& requiredOutParam : requiredOutParams)
    {
        writeMarshalUnmarshalCode(out, requiredOutParam.type, op, "iceP_" + requiredOutParam.name, true);
    }

    for (const auto& optionalOutParam : optionalOutParams)
    {
        writeMarshalUnmarshalCode(
            out,
            optionalOutParam.type,
            op,
            "iceP_" + optionalOutParam.name,
            true,
            optionalOutParam.tag);
    }

    if (op->returnsClasses())
    {
        out << nl << "ostr.writePendingValues()";
    }
}

void
SwiftGenerator::writeMarshalAsyncOutParams(::IceInternal::Output& out, const OperationPtr& op)
{
    ParamInfoList requiredOutParams, optionalOutParams;
    getOutParams(op, requiredOutParams, optionalOutParams);

    out << nl << "let " << operationReturnDeclaration(op) << " = value";
    //
    // Marshal parameters
    // 1. required
    // 2. optional (including optional return)
    //

    for (const auto& requiredOutParam : requiredOutParams)
    {
        writeMarshalUnmarshalCode(out, requiredOutParam.type, op, "iceP_" + requiredOutParam.name, true);
    }

    for (const auto& optionalOutParam : optionalOutParams)
    {
        writeMarshalUnmarshalCode(
            out,
            optionalOutParam.type,
            op,
            "iceP_" + optionalOutParam.name,
            true,
            optionalOutParam.tag);
    }

    if (op->returnsClasses())
    {
        out << nl << "ostr.writePendingValues()";
    }
}

void
SwiftGenerator::writeUnmarshalOutParams(::IceInternal::Output& out, const OperationPtr& op)
{
    TypePtr returnType = op->returnType();

    ParamInfoList requiredOutParams, optionalOutParams;
    getOutParams(op, requiredOutParams, optionalOutParams);
    const ParamInfoList allOutParams = getAllOutParams(op);
    //
    // Unmarshal parameters
    // 1. required
    // 2. return
    // 3. optional (including optional return)
    //
    out << "{ istr in";
    out.inc();
    for (const auto& requiredOutParam : requiredOutParams)
    {
        string param;
        if (requiredOutParam.type->isClassType())
        {
            out << nl << "var iceP_" << requiredOutParam.name << ": " << requiredOutParam.typeStr;
            param = "iceP_" + requiredOutParam.name;
        }
        else
        {
            param = "let iceP_" + requiredOutParam.name + ": " + requiredOutParam.typeStr;
        }
        writeMarshalUnmarshalCode(out, requiredOutParam.type, op, param, false);
    }

    for (const auto& optionalOutParam : optionalOutParams)
    {
        string param;
        if (optionalOutParam.type->isClassType())
        {
            out << nl << "var iceP_" << optionalOutParam.name << ": " << optionalOutParam.typeStr;
            param = "iceP_" + optionalOutParam.name;
        }
        else
        {
            param = "let iceP_" + optionalOutParam.name + ": " + optionalOutParam.typeStr;
        }
        writeMarshalUnmarshalCode(out, optionalOutParam.type, op, param, false, optionalOutParam.tag);
    }

    if (op->returnsClasses())
    {
        out << nl << "try istr.readPendingValues()";
    }

    out << nl << "return ";
    if (allOutParams.size() > 1)
    {
        out << spar;
    }

    if (returnType)
    {
        out << ("iceP_" + paramLabel("returnValue", op->outParameters()));
    }

    for (const auto& allOutParam : allOutParams)
    {
        if (allOutParam.param)
        {
            out << ("iceP_" + allOutParam.name);
        }
    }

    if (allOutParams.size() > 1)
    {
        out << epar;
    }

    out.dec();
    out << nl << "}";
}

void
SwiftGenerator::writeUnmarshalInParams(::IceInternal::Output& out, const OperationPtr& op)
{
    ParamInfoList requiredInParams, optionalInParams;
    getInParams(op, requiredInParams, optionalInParams);
    const ParamInfoList allInParams = getAllInParams(op);
    //
    // Unmarshal parameters
    // 1. required
    // 3. optional
    //
    for (const auto& requiredInParam : requiredInParams)
    {
        if (requiredInParam.param)
        {
            string param;
            if (requiredInParam.type->isClassType())
            {
                out << nl << "var iceP_" << requiredInParam.name << ": " << requiredInParam.typeStr;
                param = "iceP_" + requiredInParam.name;
            }
            else
            {
                param = "let iceP_" + requiredInParam.name + ": " + requiredInParam.typeStr;
            }
            writeMarshalUnmarshalCode(out, requiredInParam.type, op, param, false);
        }
    }

    for (const auto& optionalInParam : optionalInParams)
    {
        string param;
        if (optionalInParam.type->isClassType())
        {
            out << nl << "var iceP_" << optionalInParam.name << ": " << optionalInParam.typeStr;
            param = "iceP_" + optionalInParam.name;
        }
        else
        {
            param = "let iceP_" + optionalInParam.name + ": " + optionalInParam.typeStr;
        }
        writeMarshalUnmarshalCode(out, optionalInParam.type, op, param, false, optionalInParam.tag);
    }

    if (op->sendsClasses())
    {
        out << nl << "try istr.readPendingValues()";
    }
}

void
SwiftGenerator::writeUnmarshalUserException(::IceInternal::Output& out, const OperationPtr& op)
{
    const string swiftModule = getSwiftModule(getTopLevelModule(op));

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
        out << " catch let error as " << fixIdent(getRelativeTypeString(thrown, swiftModule)) << sb;
        out << nl << "throw error";
        out << eb;
    }
    out << " catch is " << getUnqualified("Ice.UserException", swiftModule) << " {}";
    out.dec();
    out << nl << "}";
}

void
SwiftGenerator::writeSwiftAttributes(::IceInternal::Output& out, const MetadataList& metadata)
{
    for (const auto& meta : metadata)
    {
        if (meta->directive() == "swift:attribute")
        {
            out << nl << meta->arguments();
        }
    }
}

void
SwiftGenerator::writeProxyOperation(::IceInternal::Output& out, const OperationPtr& op)
{
    const string opName = fixIdent(op->name());

    const ParamInfoList allInParams = getAllInParams(op);
    const ParamInfoList allOutParams = getAllOutParams(op);
    const ExceptionList allExceptions = op->throws();

    const string swiftModule = getSwiftModule(getTopLevelModule(op));

    out << sp;
    writeOpDocSummary(out, op, false);
    out << nl << "func " << opName;
    out << spar;
    for (auto q = allInParams.begin(); q != allInParams.end(); ++q)
    {
        if (allInParams.size() == 1)
        {
            out << ("_ iceP_" + q->name + ": " + q->typeStr + (q->optional ? " = nil" : ""));
        }
        else
        {
            out << (q->name + " iceP_" + q->name + ": " + q->typeStr + (q->optional ? " = nil" : ""));
        }
    }
    out << "context: " + getUnqualified("Ice.Context", swiftModule) + "? = nil";
    out << epar;
    out << " async throws -> ";
    if (allOutParams.empty())
    {
        out << "Swift.Void";
    }
    else
    {
        out << operationReturnType(op);
    }

    out << sb;

    //
    // Invoke
    //
    out << sp;
    out << nl << "return try await _impl._invoke(";

    out.useCurrentPosAsIndent();
    out << "operation: \"" << op->name() << "\",";
    out << nl << "mode: " << modeToString(op->mode()) << ",";

    if (op->format())
    {
        out << nl << "format: " << opFormatTypeToString(op);
        out << ",";
    }

    if (allInParams.size() > 0)
    {
        out << nl << "write: ";
        writeMarshalInParams(out, op);
        out << ",";
    }

    if (allOutParams.size() > 0)
    {
        out << nl << "read: ";
        writeUnmarshalOutParams(out, op);
        out << ",";
    }

    if (allExceptions.size() > 0)
    {
        out << nl << "userException:";
        writeUnmarshalUserException(out, op);
        out << ",";
    }

    out << nl << "context: context)";
    out.restoreIndent();

    out << eb;
}

void
SwiftGenerator::writeDispatchOperation(::IceInternal::Output& out, const OperationPtr& op)
{
    const string opName = op->name();

    const ParamInfoList inParams = getAllInParams(op);
    const ParamInfoList outParams = getAllOutParams(op);

    const string swiftModule = getSwiftModule(getTopLevelModule(op));

    out << sp;
    out << nl << "public func _iceD_" << opName
        << "(_ request: Ice.IncomingRequest) async throws -> Ice.OutgoingResponse";

    out << sb;
    out << nl;

    // TODO: check operation mode

    if (inParams.empty())
    {
        out << nl << "_ = try request.inputStream.skipEmptyEncapsulation()";
    }
    else
    {
        out << nl << "let istr = request.inputStream";
        out << nl << "_ = try istr.startEncapsulation()";
        writeUnmarshalInParams(out, op);
    }

    out << nl;
    if (!outParams.empty())
    {
        out << "let result = ";
    }

    out << "try await self." << fixIdent(opName);
    out << spar;
    for (const auto& q : inParams)
    {
        out << (q.name + ": iceP_" + q.name);
    }
    out << "current: request.current";
    out << epar;

    if (outParams.empty())
    {
        out << nl << "return request.current.makeEmptyOutgoingResponse()";
    }
    else
    {
        out << nl << "return request.current.makeOutgoingResponse(result, formatType: " << opFormatTypeToString(op)
            << ")";
        out << sb;
        out << " ostr, value in ";
        writeMarshalAsyncOutParams(out, op);
        out << eb;
    }

    out << eb;
}
