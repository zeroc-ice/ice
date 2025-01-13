//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "CPlusPlusUtil.h"
#include "../Slice/Util.h"
#include <algorithm>
#include <cassert>
#include <cstring>

#ifndef _WIN32
#    include <fcntl.h>
#endif

using namespace std;
using namespace Slice;
using namespace IceInternal;

namespace
{
    bool isTriviallyCopyable(const StructPtr& st)
    {
        assert(st);
        for (const auto& m : st->dataMembers())
        {
            if (BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(m->type()))
            {
                if (builtin->isVariableLength())
                {
                    return false;
                }
                // Numeric and bool are fixed-length and trivially copyable.
            }
            else if (StructPtr nestedSt = dynamic_pointer_cast<Struct>(m->type()))
            {
                if (!isTriviallyCopyable(nestedSt))
                {
                    return false;
                }
            }
            else if (!dynamic_pointer_cast<Enum>(m->type()))
            {
                return false;
            }
        }
        return true;
    }

    string stringTypeToString(const TypePtr&, const MetadataList& metadata, TypeContext typeCtx)
    {
        string strType = findMetadata(metadata, typeCtx);

        if (strType == "")
        {
            strType = (typeCtx & TypeContext::UseWstring) != TypeContext::None ? "::std::wstring" : "::std::string";
        }
        else
        {
            assert(strType == "string" || strType == "wstring");
            strType = "::std::" + strType;
        }

        if ((typeCtx & TypeContext::MarshalParam) != TypeContext::None)
        {
            strType += "_view";
        }
        return strType;
    }

    string
    sequenceTypeToString(const SequencePtr& seq, const string& scope, const MetadataList& metadata, TypeContext typeCtx)
    {
        string seqType = findMetadata(metadata, typeCtx);
        if (!seqType.empty())
        {
            if (seqType == "%array")
            {
                BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(seq->type());
                string s;
                if (builtin && builtin->kind() == Builtin::KindByte)
                {
                    s = "std::byte";
                }
                else
                {
                    s = typeToString(
                        seq->type(),
                        false,
                        scope,
                        seq->typeMetadata(),
                        typeCtx | (inWstringModule(seq) ? TypeContext::UseWstring : TypeContext::None));
                }
                return "::std::pair<const " + s + "*, const " + s + "*>";
            }
            else
            {
                return seqType;
            }
        }
        else
        {
            return getUnqualified(fixKwd(seq->scoped()), scope);
        }
    }

    string dictionaryTypeToString(
        const DictionaryPtr& dict,
        const string& scope,
        const MetadataList& metadata,
        TypeContext typeCtx)
    {
        const string dictType = findMetadata(metadata, typeCtx);
        if (dictType.empty())
        {
            return getUnqualified(fixKwd(dict->scoped()), scope);
        }
        else
        {
            return dictType;
        }
    }

    /// Split data members in required and optional members; the optional members are sorted in tag order.
    std::pair<DataMemberList, DataMemberList> split(const DataMemberList& dataMembers)
    {
        DataMemberList requiredMembers;
        DataMemberList optionalMembers;

        for (const auto& q : dataMembers)
        {
            if (q->optional())
            {
                optionalMembers.push_back(q);
            }
            else
            {
                requiredMembers.push_back(q);
            }
        }

        // Sort optional data members
        optionalMembers.sort([](const DataMemberPtr& lhs, const DataMemberPtr& rhs)
                             { return lhs->tag() < rhs->tag(); });

        return {requiredMembers, optionalMembers};
    }

    /// Do we pass this type by value when it's an input parameter?
    bool inputParamByValue(const TypePtr& type, const MetadataList& metadata)
    {
        BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
        if ((builtin && (!builtin->isVariableLength() || builtin->kind() == Builtin::KindString)))
        {
            return true;
        }
        if (dynamic_pointer_cast<Enum>(type))
        {
            return true;
        }
        if (dynamic_pointer_cast<Sequence>(type) || dynamic_pointer_cast<Dictionary>(type))
        {
            // Return true for view-type (sequence and dictionary) and array (sequence only)
            for (const auto& meta : metadata)
            {
                string_view directive = meta->directive();
                if (directive == "cpp:view-type")
                {
                    return true;
                }
                if (directive == "cpp:array")
                {
                    return true;
                }
            }
        }
        return false;
    }

    void writeParamAllocateCode(
        Output& out,
        const TypePtr& type,
        bool optional,
        const string& scope,
        const string& fixedName,
        const MetadataList& metadata,
        TypeContext typeCtx)
    {
        string s = typeToString(type, optional, scope, metadata, typeCtx);
        out << nl << s << ' ' << fixedName << ';';
    }

    void writeMarshalUnmarshalParams(Output& out, const ParameterList& params, const OperationPtr& op, bool marshal)
    {
        const string returnValueS = "ret";
        const string stream = marshal ? "ostr" : "istr";

        // True when unmarshaling a tuple response.
        bool tuple = !marshal && op && (params.size() + (op->returnType() ? 1 : 0)) > 1;

        //
        // Marshal non optional parameters.
        //
        ParameterList requiredParams;
        ParameterList optionals;
        for (ParameterList::const_iterator p = params.begin(); p != params.end(); ++p)
        {
            if ((*p)->optional())
            {
                optionals.push_back(*p);
            }
            else
            {
                requiredParams.push_back(*p);
            }
        }

        int retOffset = op && op->returnType() ? 1 : 0;

        if (!requiredParams.empty() || (op && op->returnType() && !op->returnIsOptional()))
        {
            out << nl;
            if (marshal)
            {
                out << stream << "->writeAll";
            }
            else
            {
                out << stream << "->readAll";
            }
            out << spar;
            for (ParameterList::const_iterator p = requiredParams.begin(); p != requiredParams.end(); ++p)
            {
                if (tuple)
                {
                    auto index = std::distance(params.begin(), std::find(params.begin(), params.end(), *p)) + retOffset;
                    out << "::std::get<" + std::to_string(index) + ">(v)";
                }
                else
                {
                    out << fixKwd(paramPrefix + (*p)->name());
                }
            }
            if (op && op->returnType() && !op->returnIsOptional())
            {
                if (tuple)
                {
                    out << "::std::get<0>(v)";
                }
                else
                {
                    out << returnValueS;
                }
            }
            out << epar << ";";
        }

        if (!optionals.empty() || (op && op->returnType() && op->returnIsOptional()))
        {
            //
            // Sort optional parameters by tag.
            //
            class SortFn
            {
            public:
                static bool compare(const ParameterPtr& lhs, const ParameterPtr& rhs)
                {
                    return lhs->tag() < rhs->tag();
                }
            };
            optionals.sort(SortFn::compare);

            out << nl;
            if (marshal)
            {
                out << stream << "->writeAll";
            }
            else
            {
                out << stream << "->readAll";
            }
            out << spar;

            {
                //
                // Tags
                //
                ostringstream os;
                os << '{';
                bool checkReturnType = op && op->returnIsOptional();
                bool insertComma = false;
                for (ParameterList::const_iterator p = optionals.begin(); p != optionals.end(); ++p)
                {
                    if (checkReturnType && op->returnTag() < (*p)->tag())
                    {
                        os << (insertComma ? ", " : "") << op->returnTag();
                        checkReturnType = false;
                        insertComma = true;
                    }
                    os << (insertComma ? ", " : "") << (*p)->tag();
                    insertComma = true;
                }
                if (checkReturnType)
                {
                    os << (insertComma ? ", " : "") << op->returnTag();
                }
                os << '}';
                out << os.str();
            }

            {
                //
                // Parameters
                //
                bool checkReturnType = op && op->returnIsOptional();
                for (ParameterList::const_iterator p = optionals.begin(); p != optionals.end(); ++p)
                {
                    if (checkReturnType && op->returnTag() < (*p)->tag())
                    {
                        if (tuple)
                        {
                            out << "::std::get<0>(v)";
                        }
                        else
                        {
                            out << returnValueS;
                        }
                        checkReturnType = false;
                    }

                    if (tuple)
                    {
                        auto index =
                            std::distance(params.begin(), std::find(params.begin(), params.end(), *p)) + retOffset;
                        out << "::std::get<" + std::to_string(index) + ">(v)";
                    }
                    else
                    {
                        out << fixKwd(paramPrefix + (*p)->name());
                    }
                }
                if (checkReturnType)
                {
                    if (tuple)
                    {
                        out << "::std::get<0>(v)";
                    }
                    else
                    {
                        out << returnValueS;
                    }
                }
            }
            out << epar << ";";
        }
    }
}

string Slice::paramPrefix = "iceP_"; // NOLINT(cert-err58-cpp)

char
Slice::ToIfdef::operator()(char c)
{
    if (!isalnum(static_cast<unsigned char>(c)))
    {
        return '_';
    }
    else
    {
        return c;
    }
}

void
Slice::printHeader(Output& out)
{
    static const char* header = "//\n"
                                "// Copyright (c) ZeroC, Inc. All rights reserved.\n"
                                "//\n";

    out << header;
    out << "//\n";
    out << "// Ice version " << ICE_STRING_VERSION << "\n";
    out << "//\n";
}

void
Slice::printVersionCheck(Output& out)
{
    out << "\n";
    out << "\n#ifndef ICE_IGNORE_VERSION";
    int iceVersion = ICE_INT_VERSION; // Use this to prevent warning with C++ Builder
    if (iceVersion % 100 >= 50)
    {
        // Beta version: exact match required
        out << "\n#   if ICE_INT_VERSION  != " << ICE_INT_VERSION;
        out << "\n#       error Ice version mismatch: an exact match is required for beta generated code";
        out << "\n#   endif";
    }
    else
    {
        out << "\n#   if ICE_INT_VERSION / 100 != " << ICE_INT_VERSION / 100;
        out << "\n#       error Ice version mismatch!";
        out << "\n#   endif";

        // Generated code is release; reject beta header
        out << "\n#   if ICE_INT_VERSION % 100 >= 50";
        out << "\n#       error Beta header file detected";
        out << "\n#   endif";

        out << "\n#   if ICE_INT_VERSION % 100 < " << ICE_INT_VERSION % 100;
        out << "\n#       error Ice patch level mismatch!";
        out << "\n#   endif";
    }
    out << "\n#endif";
}

void
Slice::printDllExportStuff(Output& out, const string& dllExport)
{
    if (dllExport.size() && dllExport != "ICE_API")
    {
        out << sp;
        out << "\n#ifndef " << dllExport;
        out << "\n#   if defined(" << dllExport << "_EXPORTS)";
        out << "\n#       define " << dllExport << " ICE_DECLSPEC_EXPORT";
        out << "\n#   else";
        out << "\n#       define " << dllExport << " ICE_DECLSPEC_IMPORT";
        out << "\n#   endif";
        out << "\n#endif";
    }
}

bool
Slice::isMovable(const TypePtr& type)
{
    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if (builtin)
    {
        switch (builtin->kind())
        {
            case Builtin::KindString:
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

    if (StructPtr st = dynamic_pointer_cast<Struct>(type))
    {
        return !isTriviallyCopyable(st);
    }

    return !dynamic_pointer_cast<Enum>(type);
}

string
Slice::getUnqualified(const std::string& type, const std::string& scope)
{
    if (type.find("::") != string::npos)
    {
        string prefix;
        if (type.find("const ") == 0)
        {
            prefix += "const ";
        }

        if (type.find(scope, prefix.size()) == prefix.size())
        {
            string t = type.substr(prefix.size() + scope.size());
            if (t.find("::") == string::npos)
            {
                return prefix + t;
            }
        }
    }
    return type;
}

string
Slice::typeToString(
    const TypePtr& type,
    bool optional,
    const string& scope,
    const MetadataList& metadata,
    TypeContext typeCtx)
{
    assert(type);

    if (optional)
    {
        if (isProxyType(type))
        {
            // We map optional proxies like regular proxies, as optional<XxxPrx>.
            return typeToString(type, false, scope, metadata, typeCtx);
        }
        else
        {
            return "::std::optional<" + typeToString(type, false, scope, metadata, typeCtx) + '>';
        }
    }

    static constexpr string_view builtinTable[] = {
        "::std::uint8_t",
        "bool",
        "::std::int16_t",
        "::std::int32_t",
        "::std::int64_t",
        "float",
        "double",
        "****", // string or wstring, see below
        "::Ice::ValuePtr",
        "::std::optional<::Ice::ObjectPrx>",
        "::Ice::ValuePtr"};

    BuiltinPtr builtin = dynamic_pointer_cast<Builtin>(type);
    if (builtin)
    {
        if (builtin->kind() == Builtin::KindString)
        {
            return stringTypeToString(type, metadata, typeCtx);
        }
        else
        {
            if (builtin->kind() == Builtin::KindObject)
            {
                return getUnqualified(string{builtinTable[Builtin::KindValue]}, scope);
            }
            else
            {
                return getUnqualified(string{builtinTable[builtin->kind()]}, scope);
            }
        }
    }

    ClassDeclPtr cl = dynamic_pointer_cast<ClassDecl>(type);
    if (cl)
    {
        return getUnqualified(fixKwd(cl->scoped() + "Ptr"), scope);
    }

    StructPtr st = dynamic_pointer_cast<Struct>(type);
    if (st)
    {
        return getUnqualified(fixKwd(st->scoped()), scope);
    }

    InterfaceDeclPtr proxy = dynamic_pointer_cast<InterfaceDecl>(type);
    if (proxy)
    {
        return "::std::optional<" + getUnqualified(fixKwd(proxy->scoped() + "Prx"), scope) + ">";
    }

    EnumPtr en = dynamic_pointer_cast<Enum>(type);
    if (en)
    {
        return getUnqualified(fixKwd(en->scoped()), scope);
    }

    SequencePtr seq = dynamic_pointer_cast<Sequence>(type);
    if (seq)
    {
        return sequenceTypeToString(seq, scope, metadata, typeCtx);
    }

    DictionaryPtr dict = dynamic_pointer_cast<Dictionary>(type);
    if (dict)
    {
        return dictionaryTypeToString(dict, scope, metadata, typeCtx);
    }

    return "???";
}

string
Slice::inputTypeToString(
    const TypePtr& type,
    bool optional,
    const string& scope,
    const MetadataList& metadata,
    TypeContext typeCtx)
{
    assert(type);
    assert(typeCtx == TypeContext::None || typeCtx == TypeContext::UseWstring);
    typeCtx = (typeCtx | TypeContext::MarshalParam);

    if (inputParamByValue(type, metadata))
    {
        // Pass by value, even if it's optional.
        return typeToString(type, optional, scope, metadata, typeCtx);
    }
    else
    {
        // For all other types, pass by const reference.
        return "const " + typeToString(type, optional, scope, metadata, typeCtx) + '&';
    }
}

string
Slice::outputTypeToString(
    const TypePtr& type,
    bool optional,
    const string& scope,
    const MetadataList& metadata,
    TypeContext typeCtx)
{
    assert(type);
    assert(typeCtx == TypeContext::None || typeCtx == TypeContext::UseWstring);

    return typeToString(type, optional, scope, metadata, typeCtx) + '&';
}

string
Slice::operationModeToString(Operation::Mode mode)
{
    switch (mode)
    {
        case Operation::Normal:
        {
            return "::Ice::OperationMode::Normal";
        }

        case Operation::Idempotent:
        {
            return "::Ice::OperationMode::Idempotent";
        }
        default:
        {
            assert(false);
        }
    }

    return "???";
}

string
Slice::opFormatTypeToString(const OperationPtr& op)
{
    optional<FormatType> opFormat = op->format();
    if (opFormat)
    {
        switch (*opFormat)
        {
            case CompactFormat:
                return "::Ice::FormatType::CompactFormat";
            case SlicedFormat:
                return "::Ice::FormatType::SlicedFormat";

            default:
                assert(false);
                return "???";
        }
    }
    else
    {
        return "::std::nullopt";
    }
}

/// If the passed name is a keyword, return the name with a "_cpp_" prefix;
/// otherwise, return the name unchanged.
static string
lookupKwd(const string& name)
{
    // Keyword list. *Must* be kept in alphabetical order.
    //
    // Note that this keyword list unnecessarily contains C++ keywords
    // that are illegal Slice identifiers -- namely identifiers that
    // are Slice keywords (class, int, etc.). They have not been removed
    // so that the keyword list is kept complete.
    static const string keywordList[] = {
        "alignas",
        "alignof",
        "and",
        "and_eq",
        "asm",
        "auto",
        "bitand",
        "bitor",
        "bool",
        "break",
        "case",
        "catch",
        "char",
        "char16_t",
        "char32_t",
        "char8_t",
        "class",
        "co_await",
        "co_return",
        "co_yield",
        "compl",
        "concept",
        "const",
        "const_cast",
        "consteval",
        "constexpr",
        "constinit",
        "continue",
        "decltype",
        "default",
        "delete",
        "do",
        "double",
        "dynamic_cast",
        "else",
        "enum",
        "explicit",
        "export",
        "extern",
        "false",
        "final",
        "float",
        "for",
        "friend",
        "goto",
        "if",
        "import",
        "inline",
        "int",
        "long",
        "module",
        "mutable",
        "namespace",
        "new",
        "noexcept",
        "not",
        "not_eq",
        "nullptr",
        "operator",
        "or",
        "or_eq",
        "override",
        "private",
        "protected",
        "public",
        "register",
        "reinterpret_cast",
        "requires",
        "return",
        "short",
        "signed",
        "sizeof",
        "static",
        "static_assert",
        "static_cast",
        "struct",
        "switch",
        "template",
        "this",
        "thread_local",
        "throw",
        "true",
        "try",
        "typedef",
        "typeid",
        "typename",
        "union",
        "unsigned",
        "using",
        "virtual",
        "void",
        "volatile",
        "wchar_t",
        "while",
        "xor",
        "xor_eq"};
    bool found = binary_search(&keywordList[0], &keywordList[sizeof(keywordList) / sizeof(*keywordList)], name);
    return found ? "_cpp_" + name : name;
}

/// If the passed name is a scoped name, return the identical scoped name,
/// but with all components that are C++ keywords replaced by
/// their "_cpp_"-prefixed version; otherwise, if the passed name is
/// not scoped, but a C++ keyword, return the "_cpp_"-prefixed name;
/// otherwise, return the name unchanged.
string
Slice::fixKwd(const string& name)
{
    if (name[0] != ':')
    {
        return lookupKwd(name);
    }
    vector<string> ids = splitScopedName(name);
    transform(ids.begin(), ids.end(), ids.begin(), [](const string& id) -> string { return lookupKwd(id); });
    stringstream result;
    for (vector<string>::const_iterator i = ids.begin(); i != ids.end(); ++i)
    {
        result << "::" + *i;
    }
    return result.str();
}

void
Slice::writeMarshalCode(Output& out, const ParameterList& params, const OperationPtr& op)
{
    writeMarshalUnmarshalParams(out, params, op, true);
}

void
Slice::writeUnmarshalCode(Output& out, const ParameterList& params, const OperationPtr& op)
{
    writeMarshalUnmarshalParams(out, params, op, false);
}

void
Slice::writeAllocateCode(
    Output& out,
    const ParameterList& params,
    const OperationPtr& op,
    const string& clScope,
    TypeContext typeCtx)
{
    for (const auto& param : params)
    {
        writeParamAllocateCode(
            out,
            param->type(),
            param->optional(),
            clScope,
            fixKwd(paramPrefix + param->name()),
            param->getMetadata(),
            typeCtx);
    }

    if (op && op->returnType())
    {
        writeParamAllocateCode(
            out,
            op->returnType(),
            op->returnIsOptional(),
            clScope,
            "ret",
            op->getMetadata(),
            typeCtx);
    }
}

void
Slice::writeMarshalUnmarshalAllInHolder(
    IceInternal::Output& out,
    const string& holder,
    const DataMemberList& dataMembers,
    bool optional,
    bool marshal)
{
    if (dataMembers.empty())
    {
        return;
    }

    string stream = marshal ? "ostr" : "istr";
    string streamOp = marshal ? "writeAll" : "readAll";

    out << nl << stream << "->" << streamOp;
    out << spar;

    if (optional)
    {
        ostringstream os;
        os << "{";
        bool firstElement = true;
        for (const auto& q : dataMembers)
        {
            if (firstElement)
            {
                firstElement = false;
            }
            else
            {
                os << ", ";
            }
            os << q->tag();
        }
        os << "}";
        out << os.str();
    }

    for (const auto& q : dataMembers)
    {
        out << holder + fixKwd(q->name());
    }

    out << epar << ";";
}

void
Slice::writeStreamReader(Output& out, const StructPtr& p, const DataMemberList& dataMembers)
{
    string fullName = fixKwd(p->scoped());

    out << nl << "template<>";
    out << nl << "struct StreamReader<" << fullName << ">";
    out << sb;
    out << nl << "static void read(InputStream* istr, " << fullName << "& v)";
    out << sb;

    writeMarshalUnmarshalAllInHolder(out, "v.", dataMembers, false, false);

    out << eb;
    out << eb << ";" << nl;
}

void
Slice::readDataMembers(Output& out, const DataMemberList& dataMembers)
{
    assert(dataMembers.size() > 0);

    auto [requiredMembers, optionalMembers] = split(dataMembers);
    string holder = "this->";

    writeMarshalUnmarshalAllInHolder(out, holder, requiredMembers, false, false);
    writeMarshalUnmarshalAllInHolder(out, holder, optionalMembers, true, false);
}

void
Slice::writeDataMembers(Output& out, const DataMemberList& dataMembers)
{
    assert(dataMembers.size() > 0);

    auto [requiredMembers, optionalMembers] = split(dataMembers);
    string holder = "this->";

    writeMarshalUnmarshalAllInHolder(out, holder, requiredMembers, false, true);
    writeMarshalUnmarshalAllInHolder(out, holder, optionalMembers, true, true);
}

void
Slice::writeIceTuple(::IceInternal::Output& out, const DataMemberList& dataMembers, TypeContext typeCtx)
{
    // Use an empty scope to get full qualified names from calls to typeToString.
    const string scope = "";
    out << nl << "[[nodiscard]] std::tuple<";
    for (DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
    {
        if (q != dataMembers.begin())
        {
            out << ", ";
        }
        out << "const ";
        out << typeToString((*q)->type(), (*q)->optional(), scope, (*q)->getMetadata(), typeCtx) << "&";
    }
    out << "> ice_tuple() const";

    out << sb;
    out << nl << "return std::tie(";
    for (DataMemberList::const_iterator pi = dataMembers.begin(); pi != dataMembers.end(); ++pi)
    {
        if (pi != dataMembers.begin())
        {
            out << ", ";
        }
        out << fixKwd((*pi)->name());
    }
    out << ");" << eb;
}

string
Slice::findMetadata(const MetadataList& metadata, TypeContext typeCtx)
{
    for (const auto& meta : metadata)
    {
        string_view directive = meta->directive();

        // If a marshal param, we first check view-type then type. Otherwise, we check type.
        // Then, if a marshal param or an unmarshal param where the underlying InputStream buffer remains valid for
        // a while, we check for "array".
        if ((typeCtx & TypeContext::MarshalParam) != TypeContext::None)
        {
            if (directive == "cpp:view-type")
            {
                return meta->arguments();
            }
        }

        if (directive == "cpp:type")
        {
            return meta->arguments();
        }

        if ((typeCtx & (TypeContext::MarshalParam | TypeContext::UnmarshalParamZeroCopy)) != TypeContext::None)
        {
            if (directive == "cpp:array")
            {
                return "%array";
            }
        }
    }
    return "";
}

bool
Slice::inWstringModule(const SequencePtr& seq)
{
    ContainerPtr cont = seq->container();
    while (cont)
    {
        ModulePtr mod = dynamic_pointer_cast<Module>(cont);
        if (!mod)
        {
            break;
        }
        if (auto argument = mod->getMetadataArgs("cpp:type"))
        {
            if (argument == "wstring")
            {
                return true;
            }
            else if (argument == "string")
            {
                return false;
            }
        }
        cont = mod->container();
    }
    return false;
}
